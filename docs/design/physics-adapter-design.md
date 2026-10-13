# Physics Adapter Design

Status: **interface contract defined; implementation not started.** Library: **Box2D** (DEC-014), version **v3.1.1** (DEC-020). Requirements: ip.fr.3.1–3.8 (reference). Decisions: DEC-003, DEC-014, DEC-016 (2D only). Rule: R-3.

## 1. Purpose

Isolate the physics library from the engine so it can be swapped and so the ECS never sees library types.

```
ECS / Engine ──► IPhysicsAdapter ──► Box2D
                    (our types)     (Box2D types stay in here)
```

## 2. Interface (engine-facing) — ip.fr.3.1

Language-neutral signatures; types are ours (`BodyId`, `Vec2`, `EntityId`, `BodyDesc`, `RaycastHit`, `RawCollision`).

| Function | Purpose | Notes |
| --- | --- | --- |
| `CreateBody(entity, desc) → BodyId` | Create body + collider from ECS data | Registers entity↔body mapping |
| `DestroyBody(entity or BodyId)` | Remove body | Must be called when the entity is destroyed (E-001) |
| `SetBodyPosition(BodyId, Vec2)` | Teleport | Sync direction ECS→physics |
| `SetLinearVelocity(BodyId, Vec2)` | Set velocity | |
| `ApplyForce(BodyId, Vec2)` | Continuous push for this step | |
| `ApplyImpulse(BodyId, Vec2)` | Instant velocity change | |
| `Raycast(origin, dir, maxDist) → hit or none` | First entity hit (ip.fr.3.7) | Returns `EntityId`, point, normal |
| `Step(dt)` | Advance simulation | Called with fixed `dt = 1/60` (R-5) |
| `GetCollisionEvents() → list` | Collisions since last step, as **entity** IDs | Translates body IDs → entity IDs (ip.fr.3.6) |

## 3. Responsibilities

1. **Entity ↔ body mapping**, both directions (ip.fr.3.4): `PlayerEntity ↔ PhysicsBody 17`. Mapping is the only place where entity IDs and library body handles meet.
2. **Write-back** after every `Step` (ip.fr.3.5): body position → `PositionComponent` of the mapped entity. (Velocity write-back: see §4.)
3. **Collision translation** (ip.fr.3.6): `Body 17 collided with Body 23` → `CollisionEvent(PlayerEntity, WallEntity, frame)`.
4. **Body types and shapes** (ip.fr.3.2): static, dynamic, kinematic; box, circle; polygon (should).
5. **Gravity** applied to dynamic bodies (ip.fr.3.3) — configurable vector in adapter config.
6. **Lifecycle safety:** no orphan bodies, no stale mappings (see `common-errors.md` E-001/E-002).

## 4. Open design points (the human decides; ask — do not choose; TASK-008)

| Point | Options | Leaning |
| --- | --- | --- |
| Authoritative velocity | (a) ECS owns velocity, pushes into body every step; (b) body owns velocity, adapter writes back to ECS | `[PROPOSED]` (b) for dynamic bodies, (a) for kinematic; always mirror to ECS for telemetry |
| Units | meters + pixels-per-meter scale vs raw pixels | `[PROPOSED]` Box2D is tuned for roughly 0.1–10 m objects, so simulate in meters and convert at render |
| Coordinate convention | y-up (Box2D) vs y-down (Raylib screen) | One *world* convention; convert only at the render boundary (E-004) |
| Box2D handle type | **Decided: v3.1.1** (C API, id handles instead of pointers) | DEC-020 — the entity↔body mapping stores Box2D ids wrapped in our own type; verify names against the v3.1.1 headers before coding `[ASSUMPTION]` |
| Collision event contents | begin/end/stay? contact points, normals, impulses | Start with begin/end + normal; extend when telemetry needs it |
| Sensors/triggers | needed for door interaction range, key pickup | likely needed (interaction range = sensor or distance query) |
| Fixed-step accumulator location | game loop vs adapter | Game loop owns it; adapter just `Step(1/60)` |

## 5. Rules

- Library headers/types **never** appear outside the adapter implementation (R-3).
- The adapter never makes gameplay decisions (R-7).
- The adapter does not know about rendering.
- Deterministic: same inputs → same collision order. Sort events by (frame, bodyA, bodyB) before returning.
- In headless mode the adapter is identical to normal mode.

## 6. Tests (see `testing/test-checklists.md`)

Dynamic body falls under gravity · static body does not move · collider produces collision · collision event carries correct entity IDs · raycast finds correct entity / none · fixed timestep gives identical trajectories across two runs · destroying entity removes body · write-back equals body position.

## 7. Visualising physics (optional aid)

`box2d-testbed` (Python: `pip install "box2d-python[testbed]"`, plus `glfw`, `pillow`) can be used to see Box2D behaviour. It is a learning tool, not part of the engine, and may be a different Box2D version.
