---
last_updated: 2026-10-13
updated_by: claude-code/sonnet-5.5/012r
---

# Live Plan

Stable phase definitions and exit criteria are in [`process/roadmap.md`](../../process/roadmap.md). This file is the **live** view: which phase is active, what is next, what changed. Dates are `[TBD]` until the team fixes sprint dates (the Gantt chart is in ClickUp, linked from Initial Plan §4.2).

## Active phase

**Phase 1 — Project and technology decisions.**

Exit criteria (all must be true):
- [x] Graphics framework chosen: Raylib (DEC-015)
- [x] Programming language chosen: C++ (DEC-013)
- [x] Physics library chosen: Box2D (DEC-014), behind the Physics Adapter
- [x] Scope fixed: 2D only (DEC-016)
- [x] ECS approach: custom (DEC-018)
- [x] Box2D v3.1.1 (DEC-020)
- [ ] C++ standard (C++20 proposed, DEC-019), platforms (dev Windows+macOS, Windows-only target proposed), Raylib version (v6.0 proposed, DEC-021), compilers — Q-007, still open
- [ ] Build system and test framework — deferred by the human (Q-009)
- [ ] Repo layout finalised (`guidelines/repository-structure.md`)
- [ ] Version control conventions confirmed (`guidelines/git-workflow.md`, Q-006)

## Next 3 steps

1. Human answers the Q-007, Q-013, and Q-009 when ready.
2. Record each answer as `Accepted` and propagate (`playbooks/record-decision.md`).
3. Bootstrap the repo skeleton (TASK-005), then the window + game loop (TASK-006).

## Phase overview

| Phase | Name | Status |
| --- | --- | --- |
| 0 | Docs scaffold & agent protocol | Done 2026-10-12 |
| 1 | Project and technology decisions | **Active** (core stack decided; details open) |
| 2 | Engine window and game loop | Not started |
| 3 | Basic ECS World | Not started |
| 4 | First ECS systems (Input, Movement, Render; no physics) | Not started |
| 5 | Physics Adapter integration | Not started |
| 6 | Collision events | Not started |
| 7 | Physics–rendering synchronisation | Not started |
| 8 | Headless mode + telemetry | Not started |
| 9 | Demo game with deliberate bugs | Not started |
| 10 | Semantic Architecture Layer (graph from telemetry) | Not started |
| 11 | Automated Playtest Agent + bug detection | Not started |
| 12 | AI Assistant Layer | Not started |
| 13 | Stretch: narrative/RAG assistant, UI polish | Not started |

Phases 10–13 order may change after the advisor review (ask the human). 3D is out of scope (DEC-016).

## Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Initial live plan created from Senior Project Guide §7 and Initial Plan §4 | claude-code/sonnet-5.5/012r |
| 2026-10-13 | Phase 1 exit criteria updated after DEC-013…016 | claude-code/sonnet-5.5/012r |
