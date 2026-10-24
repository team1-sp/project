---
last_updated: 2026-10-24
updated_by: claude-code (PB-002 session)
---

# Current Status

Snapshot of the project. Overwrite this file each session (keep it short). History belongs in `handoff-log.md`. **The truth is the `Accepted` entries in `history/decision-log.md`.**

## One-line summary
Pre-implementation. Architecture is defined, the tech stack and the GitHub repository workflow are **decided**: C++20, custom ECS, Box2D v3.1.1, Raylib 6.2, CMake + GoogleTest, 2D only (3D possible later); developed on Windows and macOS, games built for Windows only. The build skeleton (TASK-005 = ClickUp PB-002) is written and in review: CMake presets for Windows, macOS and headless, Box2D and raylib linked, an empty window, two GoogleTest tests.

## Phase
Roadmap phase **1 — Project and technology decisions**: all exit criteria are met (see `plan.md`); the human confirms closing it. Phase 2 starts with the skeleton (TASK-005).

## What exists `[VERIFIED 2026-10-12/13 by directory listing]`

| Item | State |
| --- | --- |
| `team1-sp/project` (GitHub, public) | `README.md`, `CONTRIBUTING.md`, `.github/` (CI, CODEOWNERS, issue/PR templates, labels), `tools/` (PR and link checks), `.editorconfig`, `.gitattributes`, `.gitignore`, and this `docs/` folder. No engine source code. |
| `senior-project/docs/` (course deliverables) | Initial Plan sections mostly written. **Not authoritative; not edited by agents.** It may still say "C++ or C#" etc. (see "Course documents affected" in the decision entries) |
| `senior-project/shader-test/` | Graphics benchmark spec, shared assets, per-library folders. Benchmark finished (no notable difference; team chose by code complexity and ease of use). Raylib decided (DEC-015) |
| Process | Scrum, 2-week sprints, weekly progress meeting, sprint review with advisor |

## Decisions status (details: `history/decision-log.md`)

- **Accepted:** three-layer architecture; ECS as data hub; Physics Adapter; fixed 1/60 s timestep; headless mode; telemetry in loop; graphics abstraction; docs/agent system; C++ (DEC-013); Box2D v3.1.1 (DEC-014, DEC-020); Raylib 6.2 (DEC-015, DEC-033); 2D only (DEC-016); humans decide (DEC-017); custom ECS (DEC-018); C++20 (DEC-019); one repository (DEC-022); CMake + GoogleTest (DEC-023); Işık Dönger reviews every PR (DEC-024); agents push only after human approval (DEC-025); `semantic/` is its own folder (DEC-026); trunk-based workflow, branch protection deferred (DEC-027); GitHub issues created from ClickUp (DEC-028); develop on Windows and macOS, games Windows-only (DEC-029); compilers per platform need (DEC-030); product name Aegis (DEC-034); PR titles `<type>(PB-<nnn>): …` (DEC-035).
- **Proposed / deferred by the human ("too early"):** MCP later (DEC-010); Python + gRPC for the AI service (DEC-011); LLM-instructive component metadata (Q-013). Agents do not raise these.
- **Open (asked of the human):** repo leftovers — visibility/branch protection, license, access, LFS (Q-014); TASK-### vs PB-### ids (Q-015); world units (TASK-008); sprint dates (Q-002); confirmation of DEC-001…008 (Q-004).

## In progress / next
1. **TASK-005 (ClickUp PB-002): repo skeleton** — Review. CI must go green on Windows and macOS, then every member builds it (PB-002.6).
2. Human answers Q-014 (leftovers) and Q-015 (task ids).
3. Then phase 2: window + game loop (TASK-006). The custom ECS design (TASK-010) waits for the human (Q-013).

## Health

| Check | Result |
| --- | --- |
| Build | Skeleton builds on Linux (GCC 13), headless and with raylib `[VERIFIED 2026-10-24]`; Windows/macOS via CI pending |
| Tests | 2/2 pass (`ctest --preset headless`) `[VERIFIED 2026-10-24, Linux]` |
| Docs | Scaffold created 2026-10-12; updated 2026-10-13 for the stack decision and the decision-authority rule |

## Things a new agent should know immediately

- **Humans decide; agents consult.** Put every real uncertainty to the human (`DECISION NEEDED`); log their decision as `Accepted`; update every doc (`playbooks/record-decision.md`).
- Course documents are not the truth; the decision log is.
- Stack is fixed: C++20 / custom ECS / Box2D v3.1.1 (behind the adapter) / Raylib 6.2 (behind the rendering abstraction) / CMake + GoogleTest. **2D only** — no 3D work. Develop on Windows and macOS (Windows priority); games Windows-only (DEC-029).
- Work through GitHub: branch `<type>/pb-<nnn>-…`, PR with Conventional Commit title, Işık Dönger or an area owner approves, agents push only after a human approves (DEC-024/025/027; `guidelines/git-workflow.md`).
- Raylib must not be initialised in headless mode.
- The course requires disclosure of AI-generated text in deliverables (`guidelines/genai-usage-rules.md`).
- Project manager: Işık Dönger. Docs in this repo are English.
