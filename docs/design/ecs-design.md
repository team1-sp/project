# ECS Design

Status: **design intent** — the ECS is **custom-built in C++** (DEC-018, DEC-013). Reason given by the human: components must be "more LLM-instructive" through the semantic layer. What that means concretely is open (Q-013). This file defines the *contract* any implementation must satisfy. Requirements: ip.fr.2.1–2.4. Layer: Engine.

## 1. Contract (what the ECS must offer)

| Capability | Requirement | Notes |
| --- | --- | --- |
| Create / destroy entity at runtime, unique ID | ip.fr.2.1 | IDs are never reused within a run, or are versioned (generation counter) to detect stale handles. |
| Add / remove / get / modify component | ip.fr.2.2 | Component types are plain data. |
| Register systems; run each once per frame in defined, repeatable order | ip.fr.2.3 | Order is explicit (list), not implicit (hash order). |
| Query by component combination | ip.fr.2.4 | e.g. `Query(Position, Velocity)`. |
| Introspection (**needed by the semantic graph**) | ip.fr.10.* | List component types, entities and their components; list systems with their declared read/write sets. |
| Deferred structural changes | — | Creating/destroying entities or adding/removing components while iterating must be deferred (command buffer) or otherwise safe. Common bug: E-003 in `common-errors.md`. |

## 2. Components (initial set)

Pure data. No methods that change state, no references to other layers.

| Component | Fields (initial) | Written by | Read by |
| --- | --- | --- | --- |
| `PositionComponent` | `x, y` (float) | PhysicsSystem (write-back) / spawn code | Render, Gameplay, Telemetry |
| `VelocityComponent` | `vx, vy` | MovementSystem; adapter write-back `[TBD]` | Physics sync, Telemetry |
| `ColliderComponent` | shape (box/circle), size, body type (static/dynamic/kinematic), restitution, friction | spawn code | Physics sync |
| `SpriteComponent` | texture id, layer, state→sprite map `[TBD]` | spawn code / gameplay | Render |
| `InventoryComponent` | list of item IDs | GameplaySystem | Gameplay, Telemetry |
| `HealthComponent` | `hp` | Gameplay | Gameplay, UI |
| `DoorStateComponent` | `isOpen` | GameplaySystem | Render, Telemetry |
| `RequiredItemComponent` | `itemId` | spawn code | Gameplay |
| `PlayerControlledComponent` (tag) | — | spawn code | Input/Movement |

`[TBD]`: whether velocity is authoritative in ECS or in the physics body (see `physics-adapter-design.md` §4).

## 3. Systems (initial set) and frame placement

Order follows the frame contract (ip.fr.1.1, R-9). Each system declares `reads / writes / emits`.

| # | System | Reads | Writes | Emits | Frame step |
| --- | --- | --- | --- | --- | --- |
| 1 | InputSystem | device/agent input | input action state | — | Input |
| 2 | MovementSystem | input actions, PlayerControlled | Velocity | — | Gameplay input processing |
| 3 | PhysicsSyncSystem | Position, Velocity, Collider | (adapter bodies) | — | Physics synchronization |
| 4 | PhysicsSystem | — | Position (write-back) | CollisionEvent raw | Physics step |
| 5 | CollisionSystem | raw collisions | — | CollisionEvent (entity IDs) | Collision event processing |
| 6 | GameplaySystem / InteractionSystem | CollisionEvent, Inventory, DoorState, RequiredItem, input | Inventory, DoorState | DoorOpenedEvent, DoorOpenFailedEvent, KeyPickedUpEvent, GoalReachedEvent | Gameplay rules |
| 7 | TelemetrySystem | all recorded components, events | telemetry buffer | — | Telemetry recording |
| 8 | RenderSystem | Position, Sprite, DoorState (via sprite state) | — | — | Rendering (normal mode only) |

Guidelines:
- A system never calls another system. Communication = components + events.
- Systems are registered with an explicit order index.
- RenderSystem is registered only in normal mode.

## 4. Events

Events are data records with a **frame number** and entity IDs. They live for the frame in which they are emitted (consumers in later steps of the same frame read them) and are appended to telemetry. Catalogue and schemas: [telemetry-and-events.md](telemetry-and-events.md).

## 5. Query semantics

- `Query(A, B)` returns entities having **both** A and B at the moment of the call.
- Iteration order must be deterministic (e.g. ascending entity ID).
- Query results must not be cached across frames.

Example: MovementSystem uses `Query(Position, Velocity)` → selects Player and Enemy, not Wall or Door.

## 6. Test checklist (maps to `testing/test-checklists.md`)

Entity creation · component add/remove · component read/write · system selects correct entities · entity deletion (including cleanup of physics body, see adapter) · deterministic iteration order · structural change during iteration.

## 7. LLM-instructive components (DEC-018) — design goal, details open

Because we own the ECS, every component type should be describable to the semantic layer and the LLM, not only storable. Candidate metadata (**proposal only — the human decides, Q-013**): human-readable name and purpose; per-field description, unit and valid range; owner (the one system that writes it); which systems read it; example values. The ECS must expose this metadata through the same introspection used for the semantic graph (§1). Keep the metadata next to the component definition so it cannot drift from the code.

## 8. Open questions

- Q-013: exact meaning/shape of "LLM-instructive" metadata and how it is declared in C++ (e.g. a registration function per component vs. macros/reflection-style tables).
- Memory layout (archetypes vs sparse sets). Not needed for the MVP's 200-entity target (ip.nfr.1.1); choose the simplest correct one.
- Scene ↔ World relation.
