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
- Physics runs at a **fixed timestep**; rendering at variable FPS. Use an accumulator: add elapsed time, run `Step(1/60)` while `accumulator >= 1/60`. Cap the number of steps per frame to avoid a "spiral of death" `[PROPOSED: cap 5]`.
- The engine's `Update` changes game state; `Draw` only renders current state. Raylib does not impose a loop structure: our own game loop owns the order (R-9) and calls Raylib only from the rendering step.
- In headless mode time is *simulated*: each loop iteration advances exactly one fixed step; no sleeping and no dependence on the wall clock.

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

Example entities: `PlayerEntity` (Position, Velocity, Collider, Health, Sprite, Inventory), `WallEntity` (Position, Collider, Sprite), `DoorEntity` (Position, Collider, Sprite, DoorState, RequiredItem), `GoalEntity`.

Example systems: `InputSystem`, `MovementSystem`, `PhysicsSystem`, `CollisionSystem`, `GameplaySystem` (+ `InteractionSystem`), `RenderSystem`, `TelemetrySystem`.

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

Recorded each frame: state, events, timings (ip.fr.8.*). Details in [../design/telemetry-and-events.md](../design/telemetry-and-events.md). The engine only *records and exports*; interpreting is the Semantic layer's job.

## 9. Worked example: pressing "right"

1. InputSystem reads the key → move-right action.
2. MovementSystem updates `VelocityComponent`.
3. Physics synchronisation pushes velocity to the body.
4. Physics step (1/60 s) computes gravity, collisions.
5. Adapter writes the body's new position to `PositionComponent`.
6. Collision events are created.
7. GameplaySystem processes the events.
8. TelemetrySystem records state and events.
9. RenderSystem reads Position + Sprite and draws.

Each step must be observable on its own; a bug is located by finding the first step where the data is wrong.
