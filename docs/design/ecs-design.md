# ECS Design

Status: **contract and Phase 1 content decided** (DEC-018, DEC-040 … DEC-043, 2026-11-05). The ECS is custom-built in C++20 (DEC-013, DEC-018). This file is the short contract: what the ECS offers, which components, systems and events Phase 1 has, and how they meet the rest of the engine. The detailed API is [ecs-api-design.md](ecs-api-design.md), the event bus [ecs-event-bus-design.md](ecs-event-bus-design.md), the storage choice [../research/ecs-storage-research.md](../research/ecs-storage-research.md). The PB-010 D-decisions in those files are reviewed in PB-010.3 and logged as ADR-001. Requirements: ip.fr.2.1–2.4, srs.fr.2.*, srs.fr.3.*, srs.fr.6.*, srs.fr.22.34–22.45. Layer: Engine.

Words: a **tick** is one fixed simulation step of 1/60 s that runs the seven tick phases (srs.fr.1.12). A **frame** is a render frame only (srs.fr.1.1). Ticks, not frames, number simulation state, events and telemetry (DEC-042).

## 1. Contract (what the ECS must offer)

| Capability | Requirement | Where designed |
| --- | --- | --- |
| Create / destroy entities at runtime; a never-reused 64-bit `EntityId` plus a generation-checked runtime handle | ip.fr.2.1, srs.fr.2.1–2.2, 2.107 | API §4 |
| Add / remove / get / modify components; plain data only (R-8) | ip.fr.2.2 | API §5–6 |
| Systems run once per tick in their tick phase, in a defined, repeatable order | ip.fr.2.3, srs.fr.2.14 | API §8 |
| Queries by component combination, ascending `EntityId` | ip.fr.2.4, rule 6 | API §7 |
| Declared reads, writes, publishes, subscribes, entity creation/destruction, enable changes and random use, checked in every build | ip.fr.10.*, DEC-018 | API §8, §10 |
| Introspection and a `Schema` (types, fields, units, descriptions, systems, derived readBy/writtenBy) written to telemetry | ip.fr.10.*, Q-013 | API §11–12 |
| Deferred structural changes through a `CommandBuffer`, applied at the end of each tick phase | srs.fr.2.119, E-003 | API §9 |
| Typed events pushed to subscribed systems at the end of the publishing phase | srs.fr.2.45–2.47 | Event bus |
| Several independent Worlds per process | ADR-002 | API §5, §16 |

## 2. Phase 1 components (DEC-042, srs.fr.22.34–22.45)

Units follow [../components.md](../components.md) (DEC-036): meters, seconds, kilograms, radians, y up. The Inspector may show degrees; components store radians. Every component has `static constexpr std::string_view typeName` and a registered description per field (API §11). "Phase 1" is the slice built this semester (DEC-043, API §17).

| Component (`typeName`) | Fields | Kind | Written by | Read by |
| --- | --- | --- | --- | --- |
| `NameComponent` (`Name`, built-in) | `name` (`FixedString<64>`) | unmanaged | spawn code | Inspector, telemetry |
| `EntityInfoComponent` (`EntityInfo`, built-in) | `tag` (`TagId`), `layer` (0–31), `sourceScene` (`AssetId`) | unmanaged | spawn code | queries, telemetry |
| `PositionComponent` (`Position`) | `xMeters`, `yMeters`, `rotationRadians`, `scaleX`, `scaleY` | unmanaged, always enabled, write-tracked | spawn code, gameplay (pending write), PhysicsSyncOut (write-back) | PhysicsSyncIn, gameplay, renderer, telemetry |
| `VelocityComponent` (`Velocity`) | `xMetersPerSecond`, `yMetersPerSecond` | unmanaged, always enabled, write-tracked | PlayerMovement (pending write), PhysicsSyncOut (write-back) | PhysicsSyncIn, telemetry |
| `RigidBodyComponent` (`RigidBody`) | `bodyType` (Static, Kinematic, Dynamic), `massKilograms`, `gravityScale`, `linearDamping`, `fixedRotation` | unmanaged | spawn code | PhysicsSyncIn |
| `BoxColliderComponent` (`BoxCollider`) | `widthMeters`, `heightMeters`, `offsetXMeters`, `offsetYMeters`, `friction`, `restitution`, `isSensor` | unmanaged | spawn code | PhysicsSyncIn |
| `CircleColliderComponent` (`CircleCollider`) | `radiusMeters`, `offsetXMeters`, `offsetYMeters`, `friction`, `restitution`, `isSensor` | unmanaged | spawn code | PhysicsSyncIn |
| `SpriteComponent` (`Sprite`) | `sprite` (`AssetId`), `tint` (RGBA `uint32`), `orderInLayer`, `flipX`, `flipY` | unmanaged | spawn code | renderer |
| `PlayerInputComponent` (`PlayerInput`) | `inputMap` (`AssetId`, empty = project map), `speedMetersPerSecond` | unmanaged | spawn code | PlayerMovement, Collect, Unlock, Objective |
| `InventoryComponent` (`Inventory`) | `items` (`FixedArray<AssetId, 8>`) | unmanaged in Phase 1 | Collect | Unlock, telemetry |
| `CollectibleComponent` (`Collectible`) | `itemId` (`AssetId`) | unmanaged | spawn code | Collect |
| `UnlockableComponent` (`Unlockable`) | `requiredItem` (`AssetId`), `interactionRangeMeters`, `isUnlocked`, `lockedSprite`, `unlockedSprite` | unmanaged | Unlock | PhysicsSyncIn (sensor rule, srs.fr.6.10), renderer |
| `ObjectiveComponent` (`Objective`) | none | empty, but a component (not a tag), so it can get its own enabled flag later | spawn code | Objective |
| `InputStateComponent` (`InputState`, singleton, runtime only) | per action `pressed`, `released`, `held` bits for MoveUp, MoveDown, MoveLeft, MoveRight, Interact; `moveX`, `moveY` (movement vector, srs.fr.5.19) | unmanaged | InputApplication | PlayerMovement, Unlock, agent bridge |

- Required components (srs.fr.3.36): colliders and RigidBody need Position; RigidBody needs Velocity and a collider; PlayerInput needs Position, Velocity and Inventory; Collectible, Unlockable and Objective need Position. Phase 1 spawn code adds them explicitly; automatic adding (srs.fr.3.37) comes with the Editor.
- The player is the enabled entity with a `PlayerInputComponent`, lowest `EntityId` first (srs.fr.9.22). There is no separate player tag.
- `Obstacle` is a tag value (`EntityInfo.tag`) from the project's tag list (srs.fr.2.53), not a tag component.
- Later: Polygon collider, hierarchy, state sprites, managed components, component enabled flags.

### Velocity and position authority (DEC-040)

The physics body owns velocity. After each physics step, PhysicsSyncOut writes the body's position, rotation and velocity into `PositionComponent` and `VelocityComponent` of dynamic bodies. Gameplay changes motion only by writing Position or Velocity, which marks a **pending write** (API §6), or by force and impulse commands; the next PhysicsSyncIn applies them, and write-back skips a component with a pending write (srs.fr.3.20–3.22, 3.51). Static bodies follow Position and ignore Velocity; a kinematic body keeps a written Velocity until it is written again (srs.fr.3.18, 3.19, 3.28).

## 3. Phase 1 systems and tick placement (DEC-041, DEC-042)

Each tick runs the phases of srs.fr.1.12 in order. In each phase: systems run in registration order, then events published in the phase are dispatched to their handlers, then structural changes are played back (API §9). Rendering runs per render frame after the ticks of that frame, reads the World, and is **not** a system (srs.fr.1.13). Telemetry is written by a recorder that observes the World and the event bus at the end of the telemetry phase (srs.fr.8.28), not by a system.

| # | Tick phase | System | Reads | Writes | Publishes / handles |
| --- | --- | --- | --- | --- | --- |
| 1 | InputApplication | `InputApplication` (engine) | the tick's input source: keyboard queue, recording, agent or none (srs.fr.5.7) | `InputState` | — |
| 2 | GameplayInput | `PlayerMovement` | `InputState`, `PlayerInput` | `Velocity` (pending write, srs.fr.6.1) | — |
| 3 | PhysicsSyncIn | `PhysicsSyncIn` (engine) | `Position`, `Velocity`, `RigidBody`, colliders, `Unlockable` | adapter bodies; clears pending marks | — |
| 4 | PhysicsStep | none; the game loop calls `adapter.step(1/60)` | — | — | — |
| 5 | PhysicsSyncOut | `PhysicsSyncOut` (engine) | adapter bodies | `Position`, `Velocity` (untracked write-back) | publishes `Collision` |
| 5 | (dispatch) | `Collect` handler | `Collectible`, `PlayerInput` | `Inventory`; toggles enabled | handles `Collision`, publishes `ItemCollected` (srs.fr.6.2) |
| 5 | (dispatch) | `Objective` handler | `Objective`, `PlayerInput` | — | handles `Collision`, publishes `ObjectiveReached` (srs.fr.6.5) |
| 6 | GameplayRules | `Unlock` | `InputState`, `Position`, `PlayerInput`, `Inventory` | `Unlockable` | publishes `Unlocked`, `UnlockFailed` (srs.fr.6.3, 6.4, 6.8) |
| 7 | TelemetryRecording | (later: `AgentObservation`, API §16) | — | — | — |

`Collect` and `Objective` are declared in phase GameplayRules, but their handlers run at the dispatch point of PhysicsSyncOut, where `Collision` is published. The schema exports this as `runsIn` (D-22). The physics systems are the only simulation code that calls the Physics Adapter (DEC-041); see [physics-adapter-design.md](physics-adapter-design.md) §7.

Guidelines:
- A system never calls another system. Communication is components plus events (R-2).
- A system declares everything it touches in `describe()`; an undeclared access stops the run (API §10).
- Systems hold no static mutable state; each World creates its own instances (D-23).

## 4. Events (DEC-042)

Events are **pushed**, not read later in the tick. A system or the PhysicsSyncOut system publishes a typed event; at the end of the tick phase in which it was published, the bus calls every subscribed system's handler in system run order (srs.fr.2.46). A handler's own events wait for the next dispatch point. Every event carries a sequence number, its publication tick and phase. Details: [ecs-event-bus-design.md](ecs-event-bus-design.md).

| Event | Published by | Payload (fields are `EntityId` or `AssetId` unless stated) |
| --- | --- | --- |
| `Collision` | PhysicsSyncOut | `a`, `b`, `kind` (Begin, End) |
| `ItemCollected` | Collect | `collector`, `item`, `itemId` |
| `Unlocked` | Unlock | `player`, `unlockable`, `requiredItem`, `inventory` (`FixedArray<AssetId, 8>`, srs.fr.2.84) |
| `UnlockFailed` | Unlock | same fields as `Unlocked` |
| `ObjectiveReached` | Objective | `player`, `objective` |

## 5. Query semantics

- `each<A, B>` visits every alive, enabled entity that has both A and B at the moment of the call, in ascending `EntityId` (API §7).
- Results are never cached across calls.
- Example: `PlayerMovement` uses `each<const PlayerInputComponent, VelocityComponent>` and selects only the player.

## 6. Time, randomness, ids and errors (DEC-042)

- **Fixed step:** systems read `ctx.fixedDeltaSeconds()` (1/60 s, R-5). Nothing reads wall-clock time.
- **Randomness:** one gameplay generator, PCG32 seeded with the run's seed (srs.fr.7.5). Systems reach it with `ctx.random()` and must declare `usesRandom()`. Draws happen in system run order, so the same seed gives the same sequence.
- **`AssetId`:** 64-bit FNV-1a of the asset name (as for `TypeId`); the registry keeps the name so telemetry and the schema show it. **`TagId`:** index into the project's tag list; Phase 1 list: `Untagged`, `Obstacle`.
- **Errors:** precondition and access violations throw `engine::RunError` with the message, tick, system and `EntityId`. Only the game loop catches it: it logs the message and tick, goes to Error Pause in the Editor, and exits with a non-zero code headless (srs.fr.1.11). These checks are on in every build. Recoverable problems (a skipped command at playback) are logged warnings. Exceptions never cross the Physics Adapter boundary.

## 7. Scenes and the World (DEC-042)

In Phase 1 a scene is C++ spawn code: `buildTestScene(World&)` creates the player, walls, door (Unlockable), key (Collectible) and goal (Objective) with explicit Editor-range `EntityId`s (1 … 2³² − 1), so runs are reproducible and IDs are stable in telemetry. One World holds one loaded scene. The `.scene` file format, scene loading and ID collisions (srs.fr.2.19–2.21, 2.66) wait for the Editor.

## 8. LLM-instructive metadata (DEC-018, Q-013 answered for Phase 1)

Every component, field, event, enum and system is registered with a description; fields also carry a unit. Which systems read and write each component is **derived** from the system declarations, never typed twice. All of it is exported as the schema record of every telemetry file (API §12), which is what the semantic layer and the LLM read. Valid ranges, example values and notes can be added to the registry later without changing the API.

## 9. Test checklist

Phase 1 items of [ecs-api-design.md](ecs-api-design.md) §15 and [ecs-event-bus-design.md](ecs-event-bus-design.md) §12, plus: the test scene builds with the expected IDs · player input moves the player through a pending Velocity write · write-back does not overwrite a pending write · collecting the key adds it to the inventory and disables the key · Interact with the key unlocks the door, without it publishes `UnlockFailed` · reaching the goal publishes `ObjectiveReached` once · two runs with the same seed and inputs give identical telemetry · an undeclared access throws `RunError` and a headless run exits non-zero.

## Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-11-05 | Rewritten to the PB-010 design and DEC-040 … DEC-043: ticks, pushed events, SRS component set, Phase 1 systems, time/RNG/ids/errors, scenes as spawn code, Q-013 for Phase 1 | claude (PR #28 review) |
