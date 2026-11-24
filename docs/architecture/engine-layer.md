# Engine Layer

The layer where the game runs. Responsible for the **game loop**. Contains four sub-layers: Physics, ECS/Component, Graphics rendering, UI.

## 1. Game loop

Frame order is a contract (ip.fr.1.1, DEC-004, rule R-9):

```
Initialize
  ↓
Input
  ↓
Gameplay input processing
  ↓
Physics synchronization      (ECS → physics bodies)
  ↓
Physics step                 (fixed timestep 1/60 s, possibly several per frame)
  ↓
Collision event processing   (physics results → ECS + CollisionEvents)
  ↓
Gameplay rules
  ↓
Telemetry recording
  ↓
Rendering                    (skipped in headless mode)
  ↓
Present frame
```

Notes:
- The simulation runs in **ticks of exactly 1/60 s**; rendering runs at a variable frame rate. An accumulator adds the real time of each render frame and runs one tick for each whole 1/60 s. A render frame runs at most **5 ticks**; when more are due, the time of the further whole ticks is dropped, the part of a tick is kept, and the warning "Simulation is running slower than real time" is logged at most once per second (srs.fr.1.3, 1.4; DEC-047 `Proposed`). This stops the "spiral of death".
- The engine's `Update` changes game state; `Draw` only renders current state. Raylib does not impose a loop structure: our own game loop owns the order (R-9) and calls Raylib only from the rendering step.
- In headless mode time is *simulated*: each loop iteration advances exactly one fixed step; no sleeping and no dependence on the wall clock.

**Where the loop lives (PB-011, DEC-046).** `engine/core` has `Application` (`init`, `run`, `shutdown`), which owns the loop and the window. It sees the window only as the `IWindow` interface; the window itself is `RaylibWindow` in `engine/rendering-raylib`, opened through a factory that a headless run never calls (R-4). The loop reaches the game through two interfaces in `engine/core`: `ISimulation::runPhase()` for everything that runs per tick, which the ECS implements as `EcsSimulation` (US-2.3), and `IFrameRenderer::renderFrame()` for the render frame. Both run modes run a tick through the same function, `Application::runOneTick()` (US-1.3, DEC-059): it takes the tick's transitions from the run's `IInputSource`, applies them to the run's `InputActionState` by the rules of srs.fr.5.6 (srs.fr.1.14), then calls `runPhase()` for each of the seven `TickPhase`s of srs.fr.1.12 in order (`engine/core/tick_phase.hpp`). Normal mode's default source is the keyboard queue (`Application::keyboardQueue()`, srs.fr.5.5), headless mode's is None (srs.fr.7.3); `ApplicationConfig::inputSource` sets another, and the keyboard is then ignored (srs.fr.5.11). Keys reach the queue once the Input Map exists (US-5.1). A normal-mode render frame follows srs.fr.1.1 (US-1.1, DEC-051): `collectInput` (the only input poll of the frame), run the ticks, `beginFrame` and `renderFrame`, then `presentFrame` (present, then wait for the frame-rate cap). raylib is built with `SUPPORT_CUSTOM_FRAME_CONTROL`, so its `EndDrawing()` neither presents nor polls; `RaylibWindow` does both at the right step. Closing the window ends the run after the current render frame with exit code 0 (srs.fr.1.10). An unrecoverable run error is an `engine::RunError` (`engine/core/run_error.hpp`) thrown inside a tick; `Application` is the only code that catches it (DEC-042). It writes `error: tick N: <message>` to the standard error output, runs no further tick and renders no further frame, and `run()` returns a non-zero exit code with the outcome `Error` (srs.fr.1.11, US-1.2, DEC-057). `Application::outcome()` says why a run ended: `WindowClosed`, `MaxTicks` or `Error`. A game process reads its run mode and run settings from the command line with `parseRunOptions()` (`engine/core/run_options.hpp`): `--headless`, `--max-ticks` (default 10 000 headless), `--seed` and `--input-recording` (srs.fr.1.8, 7.33); `WindowConfig::fullscreen` opens the window fullscreen (srs.fr.1.9). The `editor` executable is the application that hosts `Application` and owns the main window; `sandbox` is a developer and CI test host, and games are built inside the editor, never as hosts (DEC-048). The editor draws its main window with Dear ImGui through `engine::rendering_raylib::ImGuiRenderer` (DEC-063, US-14.1). It keeps the window open while its unsaved-changes check refuses the close (`ApplicationConfig::confirmClose`, srs.fr.14.3), ends the run from File > Exit with `Application::requestClose()`, sets the title with `Application::setWindowTitle()` (srs.fr.14.2) and turns off closing on Escape (`WindowConfig::escapeClosesWindow`). `IFrameRenderer::releaseGraphics()` runs right before the window closes.

**Time in the loop (PB-012, DEC-047).** `FrameClock` measures the real time of each render frame with `std::chrono::steady_clock`; it is read once per render frame, in `Application`, and nowhere in the simulation (R-6). `FixedTimestep` turns those frame times into ticks. Its accumulator counts nanoseconds times 60, a unit in which one tick is exactly 1 000 000 000, so frame times that add up to one second give exactly 60 ticks at any frame rate; a `double` accumulator runs 59 ticks in a second at 144 frames per second. `Application` keeps the tick counter (`Tick`, a `std::uint64_t` in `engine/core/tick.hpp`): 0 when a run starts and one more after every completed tick, so the tick number passed to `ISimulation::runPhase()` is the counter's value at the start of that tick (srs.fr.1.5), and simulated time is the counter times 1/60 s (srs.fr.1.6). The ECS will hand the same number to systems as `ctx.tick()`. A headless run uses no clock: it runs one tick after the other until `maxTicks` (srs.fr.7.4, 7.7). **Pause, resume and step (US-1.4, DEC-060).** `Application::requestPause()` pauses a normal-mode run at the next tick boundary, also when asked from inside a tick (Error Pause, srs.fr.24.6): the render frame's ticks that have not started are dropped and `FixedTimestep` keeps the part of a tick (srs.fr.1.16). While paused, render frames go on but the timestep gets no time (srs.fr.1.17). `resume()` takes the paused part of the next frame time off (`FrameClock::sinceFrameStart()`), so no paused time is simulated and no slow-simulation warning follows (srs.fr.1.18). `requestStep()` while paused runs one tick in the next render frame without the accumulator (srs.fr.17.14). Headless runs cannot pause.

**Structured logs and the Console (US-24.1, DEC-068).** `engine/core/logging.hpp` defines the
owning `LogEntry` contract and the non-owning `ILogSink` seam. `ApplicationConfig::logSink` routes
run warnings and errors to a structured destination; with no sink, stderr remains the headless and
command-line fallback. The Editor's window-independent `ConsoleModel` owns entries in insertion
order, computes counters and non-mutating filter/search/collapse projections, and updates
`EditorStatus.lastConsoleMessage`. Error Pause injects `Application::requestPause()` and therefore
uses the loop's existing tick-boundary behavior. Clear on Play and the first-error gate are reset by
the `onPlayStarted()` event that US-17.2's controller will call. Full contract and identity rules:
[Console and Structured Logging](../design/console-logging.md).

## 2. Scene

A **scene** is the container for everything in one part of the game: lights, sound, environment elements, UI elements and game objects (entities). `[TBD: whether a Scene wraps a World 1:1]`

## 3. ECS / Component layer

Main job: store and process all game data.

| Concept | Definition |
| --- | --- |
| Entity | Unique ID of a game object. No data or behaviour. |
| Component | Pure data attached to an entity (e.g. `PositionComponent {x, y}`). No logic. |
| System | Logic that processes entities having specific components. |
| World | Container owning entities, components and systems. |
| Query | Filter selecting entities with a given component combination. |

Example entities (the Phase 1 test scene, DEC-042): player (Position, Velocity, RigidBody, BoxCollider, Sprite, PlayerInput, Inventory), wall (Position, BoxCollider, Sprite, tag Obstacle), door (Position, BoxCollider, Sprite, Unlockable), key (Position, BoxCollider with isSensor, Sprite, Collectible), goal (Position, BoxCollider with isSensor, Objective).

Phase 1 systems: `InputApplication`, `PlayerMovement`, `PhysicsSyncIn`, `PhysicsSyncOut`, `Collect`, `Objective`, `Unlock`. Rendering and telemetry recording are not systems (srs.fr.1.13, DEC-042).

Requirements: ip.fr.2.1–2.4 (create/destroy entities, add/remove/read/modify components, register systems with defined repeatable order, queries). Detail: [../design/ecs-design.md](../design/ecs-design.md).

## 4. Physics layer

The physics **library** (Box2D, DEC-014) does the maths. The engine's physics **layer** wraps it via the **Physics Adapter**.

```
ECS / Engine
    ↓
Physics Adapter (middleware)
    ↓
Selected physics library
```

Adapter functions (ip.fr.3.1): `CreateBody, DestroyBody, SetBodyPosition, SetLinearVelocity, ApplyForce, ApplyImpulse, Raycast, Step, GetCollisionEvents`.

Key behaviours:
- **Entity ↔ body mapping** (`PlayerEntity ↔ PhysicsBody 17`), both directions (ip.fr.3.4).
- **Write-back:** after each step, body position → `PositionComponent` (ip.fr.3.5).
- **Collision translation:** `PhysicsBody 17 collided with PhysicsBody 23` → `PlayerEntity collided with WallEntity` → `CollisionEvent` (ip.fr.3.6).
- Body types: static, dynamic, kinematic; shapes: box, circle (polygon: should) (ip.fr.3.2, 3.8).
- Physics gives positions and collisions only; it never decides game rules (R-7).

Detail: [../design/physics-adapter-design.md](../design/physics-adapter-design.md).

## 5. Graphics rendering layer

Converts game state into visible output. The graphics **framework** (Raylib, DEC-015) is behind a rendering abstraction (R-4). Raylib needs a window and GL context, so it is never initialised in headless mode.

```
ECS data → Transform (position, rotation, scale) → vertex data / mesh
  → vertex shader → primitive assembly → rasterization → fragment shader
  → framebuffer → screen
```

Rules:
- RenderSystem **reads** ECS only; it never changes simulation state (ip.fr.4.3).
- It draws entities with `PositionComponent` + `SpriteComponent` at their position (ip.fr.4.1) and chooses the sprite for the entity's state, e.g. open/closed door (ip.fr.4.2).
- Camera following a selected entity: should (ip.fr.4.4).
- Rendering is skipped entirely in headless mode (ip.fr.7.1).

## 6. UI layer

Lets the user interact with the engine and test tools (e.g. mode selection, chat panel for the AI assistant, graph viewer). `[TBD]` scope and technology; Initial Plan WBS mentions "Unity-style UI prototype" for the 1st increment. Keep UI code out of simulation code.

## 7. Input

Keyboard (gamepad: may) translated to actions: move, interact (ip.fr.5.1, 5.3). The **same action interface** accepts actions from the Automated Playtest Agent (ip.fr.5.2) — design input as a stream of abstract actions, not raw key events inside simulation code.

## 8. Telemetry (engine-side)

Recorded per tick: samples, events, entity lifecycle, timings (ip.fr.8.*, srs.fr.8.*), by a recorder that observes the World and the event bus. Details in [../design/telemetry-and-events.md](../design/telemetry-and-events.md). The engine only *records and exports*; interpreting is the Semantic layer's job.

## 9. Worked example: pressing "right"

1. The keyboard transition is queued in the render frame; `InputApplication` turns it into the MoveRight action in `InputStateComponent` at the start of the tick.
2. `PlayerMovement` writes `VelocityComponent` (a pending write, DEC-040).
3. `PhysicsSyncIn` applies the pending velocity to the body.
4. Physics step (1/60 s) computes gravity and collisions.
5. `PhysicsSyncOut` writes the body's new position and velocity back and publishes `CollisionEvent`s.
6. Handlers of `Collect` and `Objective` run at the end of that phase.
7. `Unlock` runs in GameplayRules if Interact was pressed.
8. The telemetry recorder records the tick's sample and events.
9. After the tick, the renderer reads Position + Sprite and draws the frame.

Each step must be observable on its own; a bug is located by finding the first step where the data is wrong.
