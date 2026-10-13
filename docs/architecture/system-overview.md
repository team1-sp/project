# System Overview

Status: derived from the Senior Project Guide and Initial Plan §2, which are course documents and not authoritative. Binding parts are the `Accepted` decisions in `history/decision-log.md`.

## 1. Purpose

Build a **2D game engine prototype that is AI-native**: it exposes its structure, runtime state and telemetry to an AI assistant that can explain the engine, diagnose bugs and help with design. It is *not* only a renderer, and the unique value is the AI-native architecture, not the engine itself.

## 2. The three layers

```
┌────────────────────────────────────────────────────────────┐
│ 3. AI ASSISTANT LAYER                                      │
│    architecture reader · LLM model · user assistant (chat) │
│    reads graph + source; explains, diagnoses, suggests     │
└───────────────────────────▲────────────────────────────────┘
                            │ queries graph (ip.fr.10.4)
┌───────────────────────────┴────────────────────────────────┐
│ 2. SEMANTIC ARCHITECTURE LAYER                             │
│    data layer · graph layer · relationships layer          │
│    builds graph from runtime data + telemetry              │
└───────────────────────────▲────────────────────────────────┘
                            │ runtime data, events, telemetry
┌───────────────────────────┴────────────────────────────────┐
│ 1. ENGINE LAYER                                            │
│    physics layer · ECS/component layer ·                   │
│    graphics rendering layer · UI layer                     │
│    game loop, scenes, input, gameplay systems, telemetry   │
└────────────────────────────────────────────────────────────┘
```

Dependency rule (R-1): **upper layers depend on lower layers, never the reverse.** The engine emits data; it does not know who reads it.

| Layer | Sub-layers | Responsibility | Detail |
| --- | --- | --- | --- |
| Engine | Physics, ECS/Component, Rendering, UI | Runs the game: loop, state, physics, drawing, tools UI | [engine-layer.md](engine-layer.md) |
| Semantic Architecture | Data, Graph, Relationships | Convert runtime data into a graph of entities, components, systems, events and their relations | [semantic-layer.md](semantic-layer.md) |
| AI Assistant | Architecture reading, LLM model, User assistant | Natural-language Q&A, diagnosis with cited frames/entities, design suggestions; read-only | [ai-assistant-layer.md](ai-assistant-layer.md) |

## 3. Core data flow (the chain)

```
Input
  ↓
ECS components (game state)
  ↓
Gameplay & movement systems
  ↓
Physics adapter ──► Physics library
  ↓
Updated ECS state
  ↓
Collision & gameplay events
  ↓
Telemetry
  ↓
Rendering ──► Screen          (skipped in headless mode)
```

**ECS is the central data hub.** Physics reads from and writes to ECS components. Rendering reads from ECS components. They never talk to each other directly. This makes three things possible:

1. **Headless mode** — the whole simulation runs with no rendering.
2. **Independent rates** — physics fixed at 1/60 s, rendering at variable FPS.
3. **Observability** — every state change and event can be recorded as telemetry and later mapped to the semantic graph.

## 4. Operating modes

```
Normal : Input → ECS → Physics → Gameplay → Telemetry → Rendering → Window
Headless: Agent input → ECS → Physics → Gameplay → Telemetry → Bug detection → Test report
```

Same simulation results for identical inputs (ip.fr.7.3).

## 5. Invariants (never violate without a decision entry)

Restated in [../agents/rules.md](../agents/rules.md) section A:

1. One-way layer dependencies.
2. ECS is the data hub; components are data; systems are behaviour.
3. Physics only via the Physics Adapter.
4. Graphics framework only via the rendering abstraction.
5. Fixed physics timestep (1/60 s).
6. Simulation independent of rendering (headless-safe).
7. Gameplay decisions are made by gameplay systems.
8. Fixed frame order.

## 6. Reference scenario: the Red Door

The scenario used everywhere to explain how layers interact:

```
PlayerEntity: Position=(10,5), Inventory=empty
DoorEntity:   Position=(20,5), DoorState.IsOpen=false, RequiredItem=GoldenKey
```

1. Player moves toward the door (MovementSystem updates Velocity).
2. Physics computes movement and proximity.
3. Player presses Interact.
4. InteractionSystem checks: near door? has GoldenKey?
5. No key → `DoorOpenFailedEvent`; door stays closed.
6. Key → `DoorOpenedEvent`; `DoorState.IsOpen = true`.
7. RenderSystem draws the open/closed sprite.

Lesson: physics only supplies positions and collisions; the *decision* is gameplay (ip.fr.6.3, 6.4, 6.6). The AI assistant should be able to answer "Why did the door not open?" from the graph and telemetry (ip.fr.11.2).

## 7. Constraints that shape every decision

- Senior project, fixed timeline: choose realistic technology for the team's capacity; prefer practical over theoretically perfect.
- **2D only** (DEC-016): no 3D work; keep seams so 3D can be added later. Stack: C++, Box2D, Raylib (DEC-013…015).
- Automated testing, telemetry and semantic graph are **core**, not extras.
- Headless mode is mandatory (DEC-005).
- Criteria for any future technology choice (the human decides): learning time, setup ease, team knowledge, 2D suitability, physics/rendering integration, headless support, telemetry ease, semantic-graph data extraction, documentation quality.

## 8. Planned product components (from the Initial Plan)

| Component | Layer | Notes |
| --- | --- | --- |
| Core engine (ECS) | Engine | Modular, data-driven |
| Automated Playtest Agent (RL agents in headless mode) | Uses Engine via action interface (ip.fr.5.2, 9.1) | Finds collision exploits, unreachable goals, logic failures |
| Semantic Architecture & Debugging Graph | Semantic | Node-based live graph + telemetry cross-reference |
| Systemic Narrative Design Assistant (LLM + RAG, "Ludonarrative Checker") | AI | Stretch; compares design docs with the graph |
