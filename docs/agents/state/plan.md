---
last_updated: 2026-10-16
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
- [x] Platforms: develop on Windows+macOS (Windows priority), games Windows-only for now (DEC-029)
- [x] C++20 (DEC-019), Raylib v6.0 (DEC-021)
- [x] Compilers: whatever each platform needs (DEC-030)
- [x] Build system and test framework: CMake + GoogleTest (DEC-023)
- [x] Repo layout accepted (`guidelines/repository-structure.md`, DEC-022, DEC-026)
- [x] Version control conventions accepted (`guidelines/git-workflow.md`, DEC-024/025/027/028)

All criteria are met; the human confirms closing phase 1.

## Next 3 steps
1. Build the repo skeleton: TASK-005 / ClickUp PB-002 (with macOS support, DEC-029).
2. Human answers the Q-014 leftovers and Q-015 (task ids).
3. Window + game loop (TASK-006), then the basic ECS (phase 3). Deferred by the human: Q-013, DEC-010, DEC-011.

## Phase overview

| Phase | Name | Status |
| --- | --- | --- |
| 0 | Docs scaffold & agent protocol | Done 2026-10-12 |
| 1 | Project and technology decisions | **Criteria met**, awaiting human confirmation |
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
