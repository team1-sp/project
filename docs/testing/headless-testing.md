# Headless Testing

Headless mode runs the **complete simulation without a window and without invoking any rendering function** (ip.fr.7.1). It is the foundation of automated tests, the Automated Playtest Agent and telemetry capture.

```
Normal  : Input → ECS → Physics → Gameplay → Telemetry → Rendering → Window
Headless: Agent/Script input → ECS → Physics → Gameplay → Telemetry → Bug detection → Test report
```

## 1. Requirements recap

| ID | Requirement |
| --- | --- |
| ip.fr.1.4 | Mode chosen at startup |
| ip.fr.7.1 | No window, no render calls |
| ip.fr.7.2 | Accept a stream of input actions from the agent |
| ip.fr.7.3 | Identical results to normal mode for identical inputs |
| ip.fr.7.4 | Limit by max frames or terminate on goal/failure event |
| ip.fr.7.5 | Produce a test report (outcome, frames, detected bugs) |
| ip.nfr.1.2 | ≥ 600 simulation steps/s on the demo game (reference hardware) |

## 2. Design rules that make it work

1. **Simulated time.** One loop iteration = one fixed step (1/60 s). No sleeping, no real-time pacing, no `DateTime.Now`/`chrono::now` in simulation.
2. **No render references** in simulation code. RenderSystem is not registered in headless mode; **Raylib is never initialised** (no `InitWindow`, no GL context). Because Raylib's `LoadTexture`/`LoadShader`/`LoadFont` need a context, simulation refers to assets by ID only; the rendering layer resolves IDs to loaded resources.
3. **Input via actions.** The loop pulls `(frame → actions)` from an `IActionSource`; implementations: keyboard, script, agent.
4. **Single entry point:** `Engine.Run(config)` with `config.mode ∈ {normal, headless}`, `maxFrames`, `seed`, `telemetry`, `actionSource`.
5. **Deterministic RNG** seeded from config.
6. **Termination conditions** (ip.fr.7.4): `maxFrames`, `GoalReachedEvent`, fatal bug event, explicit stop.

## 3. Test harness sketch `[PROPOSED]` (names are provisional)

```text
result = RunHeadless(
    scenario   = "demo/door-with-key",
    inputs     = [(0,"right"), (120,"interact")],
    maxFrames  = 600,
    seed       = 42,
    bugFlags   = {})
assert result.outcome == "goal_reached"
assert result.events contains DoorOpenedEvent at frame 121
assert result.bugs is empty
```

## 4. Equivalence test (normal vs headless)

1. Run scenario S with input stream I in headless mode → record state hash per frame and events.
2. Run the same S/I in normal mode (rendering on; whether Raylib can run with a hidden window on the team's machines/CI is open — ask).
3. Compare per-frame state hashes and event lists. Any difference is a bug in R-6 (something depends on rendering/time).

## 5. Reading the report

The report must contain: outcome, frames executed, seed, mode, list of detected bugs (class, frame, entity IDs), path to telemetry file. See `design/telemetry-and-events.md`.

## 6. Common headless failures

| Symptom | Likely cause |
| --- | --- |
| Crash at startup in headless | something initialises the graphics framework unconditionally |
| Different results than normal mode | simulation reads time or render state; unordered iteration; physics stepped with variable dt |
| Run never terminates | missing termination rule; goal unreachable (bug or genuine) |
| Too slow (< 600 steps/s) | telemetry allocation, per-frame logging, debug asserts in hot loop, physics debug draw enabled |

More: [../troubleshooting/debugging-playbook.md](../troubleshooting/debugging-playbook.md).
