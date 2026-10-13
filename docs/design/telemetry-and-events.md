# Telemetry and Events

Status: **schema proposal** `[PROPOSED]` — must be reviewed by a human before implementation. Requirements: ip.fr.8.1–8.5, ip.fr.9.3, ip.nfr.1.3. Decision: DEC-008.

## 1. Goals

1. Make every state change and event observable per frame (observability principle).
2. Feed bug detection (ip.fr.9.*) and the semantic graph (ip.fr.10.*).
3. Cost ≤ 10% extra average frame time (ip.nfr.1.3); switchable by configuration (ip.fr.8.5).

## 2. What is recorded

| Kind | Content | Req |
| --- | --- | --- |
| Frame state | frame number; for each entity with Position/Velocity: entity ID, position, velocity | ip.fr.8.1 |
| Events | every collision, door, key and goal event + frame number | ip.fr.8.2 |
| Timing | total frame time, physics step time per frame | ip.fr.8.3 |
| Bug detections | class, frame, entity IDs | ip.fr.9.3 |
| Run metadata | run ID, mode (normal/headless), seed, config hash, build/commit, input stream ID | `[PROPOSED]` |

## 3. Event catalogue (initial)

All events carry `frame` and a monotonically increasing `seq` (order within the frame).

| Event | Emitted by | Payload |
| --- | --- | --- |
| `CollisionEvent` | CollisionSystem | entityA, entityB, normal, kind (begin/end) |
| `KeyPickedUpEvent` | GameplaySystem | player, key entity, item id |
| `DoorOpenedEvent` | GameplaySystem | player, door, required item |
| `DoorOpenFailedEvent` | GameplaySystem | player, door, reason (e.g. missing item) |
| `GoalReachedEvent` | GameplaySystem | player, goal |
| `BugDetectedEvent` | bug detector | class, frame, entities, message |

State-change evidence for the graph (`Event —triggered→ StateChange`): events that modify components SHOULD carry `before`/`after` of the changed field (e.g. `DoorState.IsOpen false→true`).

## 4. Bug classes (ip.fr.9.2)

| Code | Class | Detection idea |
| --- | --- | --- |
| `B-NO-COLLIDER` | Entity expected to collide has no collider | Static check at load + runtime: entity tagged "solid" without ColliderComponent |
| `B-DOOR-NO-KEY` | Door opens without required item | `DoorOpenedEvent` where player's inventory lacked `RequiredItem` |
| `B-GOAL-UNREACHABLE` | Goal unreachable | Agent exhausts frame budget/explores reachable area without `GoalReachedEvent` (+ reachability check) |
| `B-OUT-OF-BOUNDS` | Player leaves map | Position outside map bounds |

## 5. File format `[PROPOSED]`

Line-delimited JSON (one record per line) — easy to stream, grep and parse in any language. Header record first, then per-frame records.

```json
{"type":"run","runId":"r-0001","mode":"headless","seed":42,"schema":1}
{"type":"frame","frame":412,"dtTotalMs":0.9,"dtPhysicsMs":0.3,"entities":[{"id":7,"pos":[10.0,5.0],"vel":[0.0,0.0]}]}
{"type":"event","frame":412,"seq":0,"name":"CollisionEvent","a":7,"b":12,"normal":[1,0],"kind":"begin"}
{"type":"bug","frame":530,"class":"B-DOOR-NO-KEY","entities":[7,15]}
```

Rules: `schema` version in the header; add fields, never repurpose them; document every change here.

Optimisations if overhead is too high: record entities only when changed (delta), ring buffer, binary format. Measure first (benchmark in `testing/`).

## 6. Rules for developers

- New component or event → add to this catalogue and to `ecs-design.md`.
- Telemetry is written by TelemetrySystem only (single writer).
- No telemetry call inside inner physics loops; sample after the step.
- Telemetry must not alter simulation results (R-6): it only reads.
- Recording off ⇒ zero allocations for telemetry.

## 7. Consumers

| Consumer | Uses |
| --- | --- |
| Bug detector / test report (ip.fr.7.5) | events, state |
| Semantic layer | frame state, events, run metadata |
| AI Assistant | via the semantic layer; cites `frame` and entity IDs |
| Developers | replay/inspection in tests |
