# Debugging Playbook

Method first, then symptom → likely cause tables. Search [../history/problem-log.md](../history/problem-log.md) before starting; record afterwards.

## 1. Method (applies to every bug)

1. **Reproduce deterministically**: fixed inputs, fixed seed, headless. Write it as a failing test.
2. **Walk the chain** and inspect data at each link; find the *first* wrong value:

```
Input action ─► Velocity/State in ECS ─► Body state in physics (after sync)
   ─► Body state after Step ─► ECS Position after write-back ─► Collision events
   ─► Gameplay events/state ─► Telemetry record ─► Render data ─► Pixels
```

3. **Bisect**: disable systems / halve the scenario / compare with a known-good commit.
4. **One hypothesis, one change.** Two failed attempts ⇒ re-read the design doc, search the problem log, then escalate (AGENT-PROTOCOL).
5. **Fix at the root layer**, add a regression test, log the problem.

## 2. Tools to build early (they pay for themselves)

- Telemetry dump of a frame range (entities, positions, velocities, events).
- A per-frame state hash for equivalence comparisons.
- Assertions: entity↔body mapping consistent; every entity with a Collider flagged solid has a body.
- Optional debug overlay drawing colliders and velocities (normal mode only; must not alter simulation).
- Run with physics-only (no gameplay) and gameplay-only (mock physics) to isolate.

## 3. Symptom → likely cause

### Physics / adapter

| Symptom | Check first |
| --- | --- |
| Player falls through the ground | Missing/disabled collider; ground not static; body created at wrong position/size (unit mismatch); step `dt` too large or variable (tunnelling); fast body → need continuous collision or smaller step |
| Player doesn't move | Velocity set in ECS but not synced to body; body type static/kinematic; velocity overwritten by write-back order; input action never produced |
| Body moves but sprite doesn't | Write-back missing; RenderSystem reading stale component; wrong entity↔body mapping |
| Sprite is mirrored vertically | y-up physics vs y-down screen mixed (E-004) |
| No collision event | Collision filtering/masks; both bodies static; events read after being cleared; adapter not translating body→entity |
| Collision event with wrong entities | Stale mapping after entity ID reuse (E-002) |
| Jitter when resting on ground | Variable timestep; position written back then re-synced with rounding; restitution/friction extremes |
| Different results each run | Unseeded randomness; unordered iteration; variable `dt` |

### ECS

| Symptom | Check first |
| --- | --- |
| System processes wrong entities | Query component set; component added late (structural change timing) |
| Crash/undefined behaviour when destroying entities | Destroy inside iteration (E-003); dangling reference to component storage |
| Entity "ghost" still collides | Body not destroyed with entity (E-001) |

### Gameplay

| Symptom | Check first |
| --- | --- |
| Door opens without key | Gameplay system checks proximity but not inventory; or injected bug flag active (`bug.doorWithoutKey`) |
| Door doesn't open with key | Interaction range sensor; inventory not updated (key pickup event ordering); `RequiredItem` id mismatch |
| Goal never reached | Collider geometry blocks path; door stays closed; goal behind walls (flag `bug.unreachableGoal`) |

### Headless / telemetry

| Symptom | Check first |
| --- | --- |
| Headless ≠ normal | Simulation touching render/time; telemetry mutating state |
| Headless crash at start | Graphics framework initialised unconditionally |
| Telemetry overhead too high | Allocations per frame, string formatting in hot path, writing every frame to disk synchronously |

### Rendering

| Symptom | Check first |
| --- | --- |
| Shader edge artefacts | Texture wrap mode (PRB-001) |
| Wrong colours / alpha | Blend function; premultiplied alpha; sRGB handling |
| Blurry sprites | Filtering mode; scaling by non-integer; mipmaps |
| Low FPS | Per-frame allocation; draw calls per entity; shader cost; VSync state |

### Build / environment

See [environment-setup.md](environment-setup.md) and [common-errors.md](common-errors.md).

## 4. When it is not a code bug

- Requirement/design mismatch → raise as a question or proposed decision, not a patch.
- Test is wrong → fix the test *and explain why* in the PR/task.
- Tool/library bug → create a minimal reproduction, log in the problem log with versions, add a workaround note.

## 5. After fixing

Regression test ✔ · `PRB-###` ✔ · `common-errors.md` updated if general ✔ · doc updated if behaviour changed ✔ · handoff ✔.
