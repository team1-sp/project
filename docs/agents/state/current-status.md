---
last_updated: 2026-11-10
updated_by: claude (PB-025 session)
---

# Current Status

Snapshot of the project. Overwrite this file each session (keep it short). History belongs in `handoff-log.md`. **The truth is the `Accepted` entries in `history/decision-log.md`.**

## One-line summary
Early implementation. Architecture and the core engine stack are defined. The C++ skeleton builds on Windows, macOS and headless Linux. PB-006 adds an isolated Python 3.12 `ai-services` package skeleton with pinned Ruff/pytest tooling; AI capabilities remain deferred. **The demo game is undecided** (Q-016).

## Phase
Roadmap phase **1 — Project and technology decisions**: all exit criteria are met (see `plan.md`); the human confirms closing it. Phase 2 starts with the skeleton (TASK-005).

## What exists `[VERIFIED 2026-10-12/13 by directory listing]`

| Item | State |
| --- | --- |
| `team1-sp/project` (GitHub, public) | C++/CMake engine skeleton, sandbox, tests, engineering docs and workflows; `ai-services/` now contains the Python 3.12 package/tooling skeleton. |
| `senior-project/docs/` (course deliverables) | Initial Plan sections mostly written. **Not authoritative; not edited by agents.** It may still say "C++ or C#" etc. (see "Course documents affected" in the decision entries) |
| `senior-project/shader-test/` | Graphics benchmark spec, shared assets, per-library folders. Benchmark finished (no notable difference; team chose by code complexity and ease of use). Raylib decided (DEC-015) |
| Process | Scrum, 2-week sprints, weekly progress meeting, sprint review with advisor |

## Decisions status (details: `history/decision-log.md`)

- **Accepted:** core entries DEC-001…009 and DEC-013…036 as indexed in the decision log; Python 3.12 with pinned `ai-services` tooling (DEC-039); gRPC/protobuf behind `AgentBridge` for the engine-to-Python bridge (DEC-038, ADR-002).
- **Proposed / deferred by the human ("too early"):** MCP later (DEC-010); LLM-instructive component metadata (Q-013). Agents do not raise these.
- **Open (asked of the human):** which demo game to build (Q-016); repo leftovers — visibility/branch protection, license, access, LFS (Q-014); TASK-### vs PB-### ids (Q-015); sprint dates (Q-002); confirmation of DEC-001…008 (Q-004); the PB-008 spike's R-3/R-4 exception (DEC-037, Q-017).

## In progress / next
- **TASK-018 (ClickUp PB-025): physics adapter interface** — Review. `engine/physics/` with `IPhysicsAdapter`, `Vec2`/`EntityId` in `engine/core`, include check in CI. Next: PB-026 (Box2D implementation).
0. **TASK-017 (ClickUp PB-006): Python `ai-services` skeleton** — Review. Python 3.12, `src/` package, pinned tools, setup instructions, Ruff and pytest verified locally.
1. **TASK-016 (ClickUp PB-008): Box2D + raylib falling-box spike** — Review; waits on Işık for DEC-037.
2. Human answers Q-014 (leftovers) and Q-015 (task ids).
3. Then phase 2: window + game loop (TASK-006). The custom ECS design waits for Q-013.

## Health

| Check | Result |
| --- | --- |
| Build | Skeleton builds on Linux (GCC 13) Windows (MSVC 19.51, Visual Studio 2026) `[VERIFIED 2026-10-24]` and macOS (Apple Clang) `[VERIFIED 2026-10-27]` |
| Tests | 2/2 pass on Linux, Windows and macOS `[VERIFIED 2026-10-27]`; with PB-008, 9/9 on Linux (GCC 13), macOS (Zeynep Ertürk) and Windows (local Visual Studio 2026 build and the `windows-latest` CI job); the spike window itself has not been run on Windows `[VERIFIED 2026-11-01]` |
| Python | Python 3.12.13 clean-environment install succeeds; Ruff clean; pytest 1/1; `pip check` clean `[VERIFIED 2026-11-04]` |
| Docs | Scaffold created 2026-10-12; updated 2026-10-13 for the stack decision and the decision-authority rule |

## Things a new agent should know immediately

- **Humans decide; agents consult.** Put every real uncertainty to the human (`DECISION NEEDED`); log their decision as `Accepted`; update every doc (`playbooks/record-decision.md`).
- Course documents are not the truth; the decision log is.
- Stack is fixed: C++20 / custom ECS / Box2D v3.1.1 (behind the adapter) / Raylib 6.2 (behind the rendering abstraction) / CMake + GoogleTest; `ai-services` uses Python 3.12 (DEC-039). **2D only** — no 3D work.
- Work through GitHub: branch `<type>/pb-<nnn>-…`, PR with Conventional Commit title, Işık Dönger or an area owner approves, agents push only after a human approves (DEC-024/025/027; `guidelines/git-workflow.md`).
- Raylib must not be initialised in headless mode.
- The demo game is `[UNDECIDED]` (Q-016). Do not design or build a demo game, or treat any example (door, key, maze) as its spec, until the team chooses it.
- The course requires disclosure of AI-generated text in deliverables (`guidelines/genai-usage-rules.md`).
- Project manager: Işık Dönger. Docs in this repo are English.
