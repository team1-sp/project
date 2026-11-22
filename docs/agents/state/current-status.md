---
last_updated: 2026-11-22
updated_by: codex/gpt-5/118-assistant
---

# Current Status

Snapshot of the project. Overwrite this file each session (keep it short). History belongs in `handoff-log.md`. **The truth is the `Accepted` entries in `history/decision-log.md`.**

## One-line summary
Early implementation. Architecture and the core engine stack are defined. The C++ skeleton builds on Windows, macOS and headless Linux. The fixed game loop, pause API, Editor main window, status bar, read-only assistant core and structured Console are merged. US-33.1's Editor entry points are implemented and verified in draft PR #145. The custom ECS is designed but not built. **The demo game is undecided** (Q-016).

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

- **Accepted:** core entries DEC-001…009 and DEC-013…067 as indexed in the decision log, except entries explicitly marked Proposed or Superseded. US-1.2 through US-1.4, the Editor main window and status bar, and US-11.2 are merged in PRs #133–#141. In review: DEC-068 (US-24.1 structured Console contract). DEC-058 and DEC-061 govern agent work on a member's computer versus a cloud copy.
- **Proposed / deferred by the human ("too early"):** MCP later (DEC-010); LLM-instructive component metadata (Q-013). Agents do not raise these.
- **Open (asked of the human):** which demo game to build (Q-016); repo leftovers — visibility/branch protection, license, access, LFS (Q-014); TASK-### vs PB-### ids (Q-015); sprint dates (Q-002); confirmation of DEC-001…008 (Q-004); the PB-008 spike's R-3/R-4 exception (DEC-037, Q-017); the PB-011 structure choices (DEC-046, Q-019); the PB-012 loop rules (DEC-047, Q-020).

## In progress / next
- **US-33.1 (#118): open the assistant with useful context** — Review, draft PR #145. AI
  menu/toolbar entry points, Ctrl+Shift+A toggle, typed Selection and Last run
  contexts, consent-aware diagnosis routing and AI-and-Privacy settings navigation are covered by
  focused headless tests. #72's public interfaces are unchanged; #71/#119/#120/#121 remain separate.
- **US-24.1 (#104): structured Console logging** — Review. Canonical `LogEntry` and optional `ILogSink` in `engine/core`; window-free owning `ConsoleModel`, non-mutating filters/search/collapse, selection/details, Error Pause and Play lifecycle seams, external-editor and entity navigation abstractions in `editor_core`; ImGui Console panel filled. DEC-068 is carried for PR review. US-17.2 (#85) calls the implemented Play start/stop seam when its controller lands.
- **US-11.2 (#72): read-only assistant and configured AI service** — Done (PR #141). `aegis::assistant` exposes only read-only Semantic Graph input and text output, obtains responses through AgentBridge, cancels streaming safely, and persists `AEGIS_CHAT 1` histories under an injected user-local project path. #71, #119 and #121 remain separate.
- **US-14.2 (#78): status bar with the editor's current state** — Done (PR #138). `EditorStatus` and `buildStatusBar()` in `editor_core` (srs.fr.14.7 to 14.11); clicking the message opens the Console. The AI service and REC clicks wait for US-33.5 and EP-30; the sources (Console, Play mode, scenes, telemetry, debug flags) fill `EditorStatus` in their stories.
- **US-14.1 (#77): Editor main window with menus, toolbar and panels** — Done (PR #137). Dear ImGui with rlImGui (DEC-063); the nine menus of D-01 to D-09 with unbuilt items disabled, the C-01 toolbar, the Default panel layout, the status bar, the title with project, scene and "*", the unsaved-changes check on close, Ctrl/Cmd shortcuts. F-01 waits for US-15.x, the project for the Project Hub (US-13.x).
- **US-1.4 (#44): pause, resume and step** — Done (PR #135). `Application::requestPause()`, `resume()`, `requestStep()`, `isPaused()` (srs.fr.1.16 to 1.18, 17.14). Play mode controls wait for US-17.2; US-24.1 uses the pause API for Error Pause.
- **US-1.3 (#43): the same tick phases in normal and headless mode** — Done (PR #134). `TickPhase` in `engine/core`; `Application::runOneTick()` builds the tick's `InputActionState` from the run's `IInputSource` (keyboard queue, None) and calls `ISimulation::runPhase()` for the seven phases in order (srs.fr.1.12 to 1.15); DEC-059 in the PR for review. Keys reach the keyboard queue with US-5.1.
- **US-1.2 (#42): windowed and headless runs** — Done (PR #133). `RunError` handling in the game loop (srs.fr.1.11), `parseRunOptions()` for `--headless`/`--max-ticks`/`--seed`/`--input-recording` (srs.fr.1.8, 7.33), `WindowConfig::fullscreen`; DEC-057 in the PR for review. Editor parts wait for US-17.2, US-17.3, US-27.4.
0. **TASK-006 (ClickUp PB-011, PB-012): window, game loop and fixed timestep** — Done: PR #35 and #37 merged 2026-11-10; DEC-046 and DEC-047 accepted 2026-11-11.
1. **TASK-018 (ClickUp PB-122): the editor hosts `Application` and opens the main window** — Review, issue #38, branch `feat/pb-122-make-the-editor-host-the-application-and` (DEC-048).
2. **TASK-019 (ClickUp PB-025): physics adapter interface** — Review, issue #32, PR #33. `engine/physics/` with `IPhysicsAdapter`, `Vec2`/`EntityId` in `engine/core`, include check in CI. Next: PB-026 (Box2D implementation, needs PB-012).
3. **PB-010.3:** ECS design review and ADR-001 (sparse-set storage) are still open.
4. **DEC-037** (PB-008 spike exception) still waits on Işık. Human answers Q-014 (leftovers) and Q-015 (task ids).
5. Then the ECS Phase 1 implementation (`design/ecs-api-design.md` §17), which fills `ISimulation`.

## Health

| Check | Result |
| --- | --- |
| Build | Skeleton builds on Linux (GCC 13) Windows (MSVC 19.51, Visual Studio 2026) `[VERIFIED 2026-10-24]` and macOS (Apple Clang) `[VERIFIED 2026-10-27]` |
| Tests | With PB-011 and PB-012: 49/49 on Windows (MSVC 19.44, `windows-debug`) and 41/41 in the `headless` preset on the same machine `[VERIFIED 2026-11-10]`; macOS and Linux not run yet. Before: 2/2 pass on Linux, Windows and macOS `[VERIFIED 2026-10-27]`; with PB-008, 9/9 on Linux (GCC 13), macOS (Zeynep Ertürk) and Windows (local Visual Studio 2026 build and the `windows-latest` CI job); the spike window itself has not been run on Windows `[VERIFIED 2026-11-01]` |
| Python | Python 3.12.13 clean-environment install succeeds; Ruff clean; pytest 1/1; `pip check` clean `[VERIFIED 2026-11-04]` |
| Docs | Scaffold created 2026-10-12; updated 2026-10-13 for the stack decision and the decision-authority rule |

## Things a new agent should know immediately

- **Humans decide; agents consult.** Put every real uncertainty to the human (`DECISION NEEDED`); log their decision as `Accepted`; update every doc (`playbooks/record-decision.md`).
- Course documents are not the truth; the decision log is.
- Stack is fixed: C++20 / custom ECS / Box2D v3.1.1 (behind the adapter) / Raylib 6.2 (behind the rendering abstraction) / CMake + GoogleTest; `ai-services` uses Python 3.12 (DEC-039). **2D only** — no 3D work.
- Work through GitHub: branch `<type>/pb-<nnn>-…`, PR with Conventional Commit title, Işık Dönger or an area owner approves, agents on a member's computer commit and push only after that member approves, agents in a cloud copy commit and push without asking, then open the draft PR with `tools/open_pr.py` (DEC-024/027/053/054/058; `guidelines/git-workflow.md`).
- Raylib must not be initialised in headless mode. The window is opened only through `RaylibWindow::open`, by `Application`, in normal mode.
- The `editor` is the application that hosts the engine and owns the main window; `sandbox` is only a developer and CI test host; games are built inside the editor (DEC-048).
- The demo game is `[UNDECIDED]` (Q-016). Do not design or build a demo game, or treat any example (door, key, maze) as its spec, until the team chooses it.
- The course requires disclosure of AI-generated text in deliverables (`guidelines/genai-usage-rules.md`).
- Project manager: Işık Dönger. Docs in this repo are English.
