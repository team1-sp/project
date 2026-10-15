# Roadmap — Phases and Exit Criteria

Source: Senior Project Guide §7 (phases 1–9) and Initial Plan §4.1 (WBS narrative) — course documents, used as input; the decision log is the truth. **2D only (DEC-016): no phase contains 3D work.** Phases 10+ are the AI-native parts planned for the second semester. Dates live in ClickUp (Gantt) — not here. Live status: `agents/state/plan.md`.

Principle: **working 2D engine prototype first**, then the AI-native layers; automated testing, telemetry and the semantic graph are core, not optional.

## Phase table

| # | Phase | What | Exit criteria (all verifiable) | Requirements |
| --- | --- | --- | --- | --- |
| 0 | Docs scaffold | This documentation + agent protocol | Folder exists; team reviewed decisions | — |
| 1 | Project and technology decisions | Stack **decided** (C++, Box2D, Raylib, 2D only — DEC-013…016). plus custom ECS (DEC-018), C++20 (DEC-019), Box2D v3.1.1 (DEC-020), Raylib v6.0 (DEC-021), one repository and layout (DEC-022, DEC-026), CMake + GoogleTest (DEC-023), review/agent/branch rules (DEC-024, 025, 027, 028), platforms (DEC-029), compilers (DEC-030). Nothing else blocks phase 2 | Remaining decisions `Accepted` in the log (human-decided); repo skeleton plan agreed | — |
| 2 | Engine window and game loop | Window opens, Update/Draw cycle, fixed-timestep accumulator, mode flag, clean exit | Window opens and closes without error; physics step rate independent of FPS; `--headless` flag recognised | ip.fr.1.1–1.4 |
| 3 | Basic ECS World (custom, with introspection and LLM-instructive component metadata — DEC-018) | Create/destroy entities, add/remove components, queries, run systems in order | ECS test checklist green | ip.fr.2.1–2.4 |
| 4 | First ECS systems | Input, Movement, Render (no physics yet) | A square moves with the keyboard, drawn from ECS data | ip.fr.4.1, 5.1, 6.1 |
| 5 | Physics Adapter integration | Physics world, bodies, colliders, entity↔body mapping, write-back | Player falls, lands on ground, does not pass through; physics checklist green | ip.fr.3.1–3.5 |
| 6 | Collision events | Adapter → `CollisionEvent`; gameplay responds | Collision with wall produces event with correct entity IDs | ip.fr.3.6, 6.2 |
| 7 | Physics–rendering sync | Body position → `PositionComponent` → RenderSystem → sprite | Sprite always at the body's position (test compares render data) | ip.fr.3.5, 4.1–4.3 |
| 8 | Headless mode + telemetry | Run without rendering; record state/events/timings; export | Headless run completes; results identical to normal mode for same inputs; telemetry file produced; overhead ≤ 10% | ip.fr.7.*, 8.*, ip.nfr.1.2–1.3 |
| 9 | Demo game with deliberate bugs | Player, walls, goal, door, key; injectable bugs | Each flag reproduces its bug; scenarios exist | ip.fr.12.* |
| 10 | Semantic Architecture Layer | Build graph from telemetry; query API | Acceptance sketch in `semantic-layer.md` §5 passes | ip.fr.10.*, ip.nfr.1.4 |
| 11 | Automated Playtest Agent + bug detection | Agent plays headless; detects 4 bug classes | Each injected bug detected with class/frame/entities; no false positives on clean build | ip.fr.9.*, 5.2, 7.2, 7.5 |
| 12 | AI Assistant Layer | Chat, graph-based answers with citations, abstention | Evaluation set from injected bugs passes (`ai-assistant-layer.md` §7); latency target | ip.fr.11.*, ip.nfr.1.5, 2.1 |
| 13 | Stretch | Narrative/RAG assistant, UI polish, camera, gamepad, polygons (still 2D) | Only if 1–12 done | ip.fr.3.8, 4.4, 5.3 |

## Increment mapping (from WBS narrative)

- **1st semester → "1st Increment Product":** foundational ECS development, open-source framework integration, Unity-style UI prototype (phases 1–7, maybe 8).
- **2nd semester:** AI integration, accelerated headless mode, RL agents for playtesting, RAG narrative assistant hook-up (phases 8–13).

`[TBD]` exact sprint-to-phase mapping — owned by the Scrum Master.

## Rules

- Do not start a phase's work before the previous phase's exit criteria hold, unless the plan says it can overlap.
- A change to this table is a process decision (log it).
- Any phase can add tasks; none can silently add scope (S-6).
