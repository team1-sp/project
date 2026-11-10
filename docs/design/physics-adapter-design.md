# Physics Adapter Design

Status: **interface written in C++ (PB-025, 2026-11-10); Box2D implementation not started (PB-026).** Library: **Box2D** (DEC-014), version **v3.1.1** (DEC-020). Requirements: ip.fr.3.1–3.8 (reference). Decisions: DEC-003, DEC-014, DEC-016 (2D only), DEC-049 (name, `BodyId` addressing, shared types). Rule: R-3.

## 1. Purpose

Isolate the physics library from the engine so it can be swapped and so the ECS never sees library types.

```
ECS / Engine ──► IPhysicsAdapter ──► Box2D
                    (our types)     (Box2D types stay in here)
```

## 2. Interface (engine-facing) — ip.fr.3.1

Written in PB-025. The interface is `engine::physics::IPhysicsAdapter` in the `engine::physics` target (`engine/physics/`, header-only, links only `engine::core`). The ClickUp text named it `IPhysicsWorld`; the team kept the name this document always used (DEC-049).

| File | Contents |
| --- | --- |
| `engine/core/include/engine/core/vec2.hpp` | `engine::Vec2` (x, y) |
| `engine/core/include/engine/core/entity_id.hpp` | `engine::EntityId`, `noEntity`, as in `ecs-api-design.md` §4; in `core` so the ECS reuses it |
| `engine/physics/include/engine/physics/body_desc.hpp` | `BodyType` (Static, Kinematic, Dynamic), `ShapeType` (Box, Circle), `BodyDesc` (fields mirror `RigidBodyComponent` and the collider components of `ecs-design.md`) |
| `engine/physics/include/engine/physics/physics_types.hpp` | `BodyId`, `noBody`, `BodyState`, `ContactPhase`, `CollisionEvent`, `RaycastHit` |
| `engine/physics/include/engine/physics/physics_adapter.hpp` | `IPhysicsAdapter` |

| Function | Purpose | Notes |
| --- | --- | --- |
| `createBody(EntityId, const BodyDesc&) → BodyId` | Create body + collider from ECS data | Registers the entity↔body mapping; a second body for the same entity is a programming error |
| `destroyBody(BodyId)` | Remove body | Must be called when the entity is destroyed (E-001) |
| `findBody(EntityId) → optional<BodyId>` | Entity → body lookup | For PhysicsSyncIn |
| `setTransform(BodyId, Vec2, angle)` | Teleport | Sync direction ECS→physics |
| `setLinearVelocity(BodyId, Vec2)` | Set velocity | |
| `applyForce(BodyId, Vec2)` | Continuous push for this step | Newtons |
| `applyImpulse(BodyId, Vec2)` | Instant velocity change | Newton-seconds |
| `step(dtSeconds)` | Advance simulation | Called with fixed `dt = 1/60` (R-5) |
| `bodyState(BodyId) → BodyState` | Position, angle, linear and angular velocity after the step | For PhysicsSyncOut write-back |
| `collisionEvents() → span<const CollisionEvent>` | Begin/end contacts of the last step, as **entity** IDs | Sorted by (`entityA`, `entityB`), `entityA` the smaller; valid until the next step (ip.fr.3.6) |
| `raycast(origin, direction, maxDistance) → optional<RaycastHit>` | First entity hit (ip.fr.3.7) | Returns `EntityId`, point, normal |

All values are meters, kilograms, seconds and radians, y up (DEC-036). No exceptions cross the interface; misuse is reported loudly by the implementation (coding standards rule 9). Contract tests with a fake adapter: `tests/unit/physics/physics_adapter_test.cpp`.

**Boundary (R-3).** `engine::physics` never links Box2D. `tools/check_includes.py` (CI `format` job) fails a PR that includes Box2D outside `engine/physics-box2d/` or raylib outside `engine/rendering-raylib/`, anywhere in `engine/` or `editor/`. `sandbox/` is not checked because of the PB-008 spike (DEC-037).

## 3. Responsibilities

1. **Entity ↔ body mapping**, both directions (ip.fr.3.4): `PlayerEntity ↔ PhysicsBody 17`. Mapping is the only place where entity IDs and library body handles meet.
2. **Write-back** after every `Step` (ip.fr.3.5): body position, rotation and velocity → `PositionComponent` and `VelocityComponent` of the mapped entity, in the PhysicsSyncOut system (§7, DEC-040).
3. **Collision translation** (ip.fr.3.6): `Body 17 collided with Body 23` → `CollisionEvent(PlayerEntity, WallEntity)`, published by the PhysicsSyncOut system with its tick (§7).
4. **Body types and shapes** (ip.fr.3.2): static, dynamic, kinematic; box, circle; polygon (should).
5. **Gravity** applied to dynamic bodies (ip.fr.3.3) — configurable vector in adapter config.
6. **Lifecycle safety:** no orphan bodies, no stale mappings (see `common-errors.md` E-001/E-002).

## 4. Open design points (the human decides; ask — do not choose; TASK-008)

| Point | Options | Leaning |
| --- | --- | --- |
| Authoritative velocity | (a) ECS owns velocity, pushes into body every step; (b) body owns velocity, adapter writes back to ECS | **Decided (DEC-040):** (b). The body owns velocity; `VelocityComponent` mirrors it after each step. Gameplay changes motion only through pending writes of Position/Velocity or force/impulse commands, applied in the next sync-in (srs.fr.3.18–3.22, 3.51). Static bodies ignore Velocity; a kinematic body keeps a written Velocity until it is written again |
| How the ECS calls the adapter | an ECS observer reacting to component adds, or declared systems | **Decided (DEC-041):** only the two declared systems `PhysicsSyncIn` and `PhysicsSyncOut` call the adapter; an observer only reports destroyed and disabled entities. See §7 |
| Units | meters + pixels-per-meter scale vs raw pixels | **Decided (DEC-036):** meters everywhere in simulation; 32 pixels per meter, applied only in the renderer |
| Coordinate convention | y-up (Box2D) vs y-down (Raylib screen) | **Decided (DEC-036):** world is y up, angles counter-clockwise; the renderer flips y and negates angles (E-004). Findings: [research/box2d.md](../research/box2d.md) |
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

## 7. Place in the tick (DEC-040, DEC-041)

| Tick phase | Who | What |
| --- | --- | --- |
| PhysicsSyncIn | `PhysicsSyncIn` system | In the order of srs.fr.3.61: destroy bodies of entities destroyed or disabled earlier (reported by the adapter's `IWorldObserver`); create bodies for new entities with a collider, ascending `EntityId`; apply shape, body-type and material changes, the Unlockable sensor rule (srs.fr.6.10); apply pending Position and Velocity writes, then forces and impulses; clear the pending marks it applied |
| PhysicsStep | game loop | `adapter.step(1/60)`; no systems run |
| PhysicsSyncOut | `PhysicsSyncOut` system | Write position, rotation and velocity of dynamic bodies back to the ECS without marking them pending, skipping components with a pending write; publish one `CollisionEvent` per begin/end contact, sorted by (`EntityId` a, `EntityId` b) |

Both systems declare their reads, writes and publishes (`ecs-api-design.md` §8, §10), so the Semantic Graph shows every physics read and write. Each World gets its own adapter instance, passed to the two systems' factories.

## 8. Visualising physics (optional aid)

`box2d-testbed` (Python: `pip install "box2d-python[testbed]"`, plus `glfw`, `pillow`) can be used to see Box2D behaviour. It is a learning tool, not part of the engine, and may be a different Box2D version.

## Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-11-10 | Interface written in C++: files, final function list with `BodyId` and `findBody`, include check (PB-025) | Zeynep Ertürk + claude (PB-025 session) |
