# Semantic Architecture Layer

Converts the engine's runtime data into a structured **graph** that represents relationships between game objects, systems and behaviours. The AI layer reads this graph. Sub-layers: **Data → Graph → Relationships**.

Status: **planned**; not started (roadmap phase 10). Everything below is design intent derived from the requirements (ip.fr.10.*), not implemented behaviour.

## 1. Sub-layers

| Sub-layer | Responsibility | Input | Output |
| --- | --- | --- | --- |
| Data layer | Collect and normalise entity states, component values, events and telemetry | Engine telemetry export (ip.fr.8.4), ECS metadata | Normalised records |
| Graph layer | Build nodes and edges | Normalised records | Graph store (in memory first) |
| Relationships layer | Derive and query higher-level relations (who collided with whom, which system wrote which component, which event triggered which state change) | Graph | Query API for the AI layer (ip.fr.10.4) |

## 2. Graph model (minimum — ip.fr.10.1, 10.2)

Node types: `Entity`, `Component`, `System`, `Event`.

Edge types (minimum):

| Edge | Meaning | Source of truth |
| --- | --- | --- |
| `Entity —has→ Component` | Entity currently has this component (with value snapshot) | ECS metadata / telemetry |
| `System —reads→ Component` | System's declared read set | System registration contract |
| `System —writes→ Component` | System's declared write set | System registration contract |
| `Entity —collidedWith→ Entity` | A collision happened (frame number) | CollisionEvent |
| `Event —triggered→ StateChange` | Event caused a state change (e.g. `DoorOpenedEvent` → `DoorState.IsOpen=true`) | Gameplay events + before/after values |

Every runtime-derived edge carries a **frame number** so the AI can cite evidence (ip.fr.11.3).

```
PlayerEntity ──has──► PositionComponent ◄──writes── MovementSystem
      │                                    ▲
      └─collidedWith(frame 412)─► WallEntity         ▲
                                              reads── RenderSystem
DoorOpenFailedEvent(frame 530) ──triggered──► (DoorState unchanged)
```

## 3. Design constraints

- Built from the engine's **runtime data and telemetry only**, no changes to game content (ip.fr.10.3).
- The custom ECS (DEC-018) must expose **declared** read/write sets per system and descriptive metadata per component (what it means, units, owner), otherwise the graph can only infer them; this is the point of owning the ECS. This is why systems should declare `reads/writes` (see `design/ecs-design.md`).
- Exportable to a documented machine-readable file (ip.fr.10.5, should).
- Performance: graph for a 10,000-frame recording in < 60 s (ip.nfr.1.4).
- One-way dependency: it reads engine output; the engine does not know it exists (R-1).
- Static knowledge (system/component declarations, source locations) and runtime knowledge (frame-stamped events) are kept distinguishable so the AI can say what is structure vs. what happened.

## 4. Open design questions `[TBD]`

- Graph storage: in-memory object graph vs. embedded graph/SQL store.
- Live (streaming) vs. post-run (batch) graph building. Start with batch from the telemetry file.
- Interchange format between engine and this layer (JSON lines? protobuf? — see DEC-011 proposed).
- Visual graph viewer (Initial Plan describes a "live, node-based visual knowledge graph workspace"): scope for the MVP.

## 5. Acceptance sketch

Given a headless run of the demo game with the injected bug "door opens without key", the graph must let a query find: the `DoorOpenedEvent` (frame N), the player's `InventoryComponent` at frame N (empty), the system that wrote `DoorState.IsOpen`, and the missing relation "player holds RequiredItem".
