# Telemetry and Events

Status: **Phase 1 record set decided (DEC-042, 2026-11-05)**; exact field spelling is settled by the telemetry PB task, owned by the semantic layer. Requirements: ip.fr.8.1–8.5, ip.fr.9.3, ip.nfr.1.3, srs.fr.8.*. Decisions: DEC-008, DEC-042, DEC-043.

Telemetry is numbered by **tick** (one 1/60 s simulation step, srs.fr.1.12), never by render frame. Event types and their payloads are defined in [ecs-design.md](ecs-design.md) §4 and the [event bus design](ecs-event-bus-design.md); component types and fields come from the registry and are listed in the schema record.

## 1. Goals

1. Make every state change and event observable per tick (observability principle).
2. Feed bug detection (ip.fr.9.*) and the semantic graph (ip.fr.10.*).
3. Cost at most 10 % of average tick time (ip.nfr.1.3, srs.fr.8.7); switchable by configuration (ip.fr.8.5).

## 2. Who writes it

A **telemetry recorder** in `semantic/` (C++, linked into the engine) writes the file. It is not an ECS system: it observes the World (`IWorldObserver`) and the event bus (`IEventObserver`), and the scheduler calls it at the end of the TelemetryRecording phase in the order of srs.fr.8.28 (systems, dispatch, playback, sample, write, timing). It only reads (R-6). With recording off it allocates nothing.

## 3. Phase 1 records

One JSON object per line. The first two records are always the run header and the schema.

| Record `type` | When | Content | Req |
| --- | --- | --- | --- |
| `run` | first line | run ID, mode (normal/headless), scene, seed, input source, build, format version, sample every N ticks | srs.fr.8.10 |
| `schema` | second line | the ECS schema: phases, components with fields, units, layout and descriptions, events, enums, systems with declarations and `runsIn`, derived readBy/writtenBy (`ecs-api-design.md` §12) | ip.fr.10.*, Q-013 |
| `entity` | at creation, destruction, enable and disable | tick, `EntityId`, change, issuing system | srs.fr.8.28 |
| `event` | at publication | seq, tick, phase, name, publisher, entity IDs, payload | srs.fr.2.81, 8.2 |
| `sample` | every Nth tick and the last tick | tick; position and rotation of every entity with Position; velocity of every entity with Velocity; tick time and physics step time in ms; system times when ticked | srs.fr.8.1–8.7 |
| `bug` | when a check fires | class, tick, entity IDs, message | ip.fr.9.3 |

Later: final event status records (`eventStatus`, srs.fr.2.81, 2.83), field-change records, Play-mode start-state records, delta or binary encoding if overhead requires it (measure first).

```json
{"type":"run","runId":"r-0001","mode":"headless","scene":"test-scene","seed":42,"input":"agent","format":1,"sampleEvery":1}
{"type":"schema","schemaVersion":1,"components":[...],"systems":[...]}
{"type":"event","seq":118,"tick":412,"phase":"PhysicsSyncOut","name":"Collision","publisher":"PhysicsSyncOut","entities":[3,7],"payload":{"a":3,"b":7,"kind":"Begin"}}
{"type":"entity","tick":412,"id":7,"change":"disabled","by":"Collect"}
{"type":"sample","tick":412,"tickMs":0.9,"physicsMs":0.3,"entities":[{"id":3,"pos":[10.0,5.0],"rot":0.0,"vel":[0.0,0.0]}]}
{"type":"bug","tick":530,"class":"unlock-without-item","entities":[3,15]}
```

`EntityId`s are unsigned 64-bit; runtime IDs start at 2³² and stay below 2⁵³, so JSON numbers are safe. Rules: the format version is in the header; add fields, never repurpose them; document every change here.

## 4. Bug classes (ip.fr.9.2, srs.fr.9.*)

| Code | Class | Detection |
| --- | --- | --- |
| `missing-collider` | An entity that needs a collider has none | srs.fr.9.18: enabled entity with an enabled RigidBody, Collectible, Unlockable or Objective, or tag Obstacle, without a collider |
| `unlock-without-item` | Unlock without the required item | srs.fr.9.5: an `Unlocked` payload whose inventory lacks the required item; uses only the payload |
| `objective-not-reached` | Objective not reached within the budget | srs.fr.9.6: session-level, Playtest Agent sessions only |
| `out-of-bounds` | Player leaves the map | Player position outside the map bounds |

## 5. Rules for developers

- A new component or event is registered with descriptions (`ecs-api-design.md` §11); it then appears in the schema record automatically. Add it to `ecs-design.md` §2 or §4.
- Only the recorder writes telemetry files (single writer).
- No telemetry call inside physics or system inner loops; the recorder samples at the end of the tick.
- Telemetry must not change simulation results (R-6).

## 6. Consumers

| Consumer | Uses |
| --- | --- |
| Bug detector and test report (ip.fr.7.5) | events, samples |
| Semantic layer | schema, samples, events, entity records, run header |
| AI Assistant | through the semantic layer; cites ticks and entity IDs |
| Developers | replay and inspection in tests |

## Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-11-05 | Rewritten for DEC-042: ticks instead of frames, recorder instead of TelemetrySystem, schema and entity records, SRS event names and bug classes | claude (PR #28 review) |
