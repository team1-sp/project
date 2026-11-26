# Current Status

Increment-level snapshot of the project (DEC-149.1). **Edit it only when your change makes something on this page wrong**: a phase change, a new module or library, a health check you re-ran, a new trap every agent must know. Per-PR progress belongs in your [handoff entry](handoff-log.md), and what is planned or in progress, and who has it, is on the [GitHub project](https://github.com/orgs/team1-sp/projects/3). **The truth is the `Accepted` entries in [`history/decision-log.md`](../../history/decision-log.md).**

## Phase

Increment 1 (prototype): the Increment 1 stories (SRS B) are the issues in the Increment 1 milestone (DEC-050). The C++ skeleton builds on Windows, macOS and headless Linux. For which stories are done, in review or next, read the project board, the milestone and the last handoff entries.

## What exists

| Area | Where | State |
| --- | --- | --- |
| Engine core | `engine/core` | Fixed game loop with the seven tick phases, windowed and headless runs, pause/resume/step, input sources, structured logging. |
| ECS | `engine/ecs` | Entities, components, systems, queries; `EcsSimulation` implements `ISimulation` (US-2.1 to 2.3). |
| Physics | `engine/physics`, `-box2d`, `-sync` | Box2D adapter; PhysicsSyncIn makes bodies (US-3.1). |
| Rendering | `engine/rendering-raylib` | Raylib window behind the rendering abstraction; never initialised headless. |
| AgentBridge | `engine/bridge`, `proto/` | Engine side of the gRPC/protobuf contract to the Python AI service (localhost only for the prototype). |
| Editor | `editor/` | Hosts `Application` and owns the main window (DEC-048). Dear ImGui + rlImGui (DEC-063): menus, toolbar, panels, status bar, Console, AI Assistant panel and AI and Privacy settings. |
| Assistant | `assistant/` | Read-only assistant core, grounded answers with typed citations, user-local chat history. |
| AI service | `ai-services/` | Python 3.12 package (DEC-039). |
| Sandbox | `sandbox/` | Developer and CI test host only; not a product. |
| Tests | `tests/` | GoogleTest, headless. |
| Requirements | `docs/requirements/` | Epics, stories, FR/NFR; owned here, read on demand (DEC-152.2). |
| Course deliverables | separate `senior-project/docs/` repo (`team1-sp/docs`) | **Not authoritative; not edited by agents.** Built from this repo at submission (DEC-152.2). |

Folder rules: [guidelines/repository-structure.md](../../guidelines/repository-structure.md).

## Open questions and decisions

Read [the decision index](../../history/decision-index.md) and [open-questions.md](open-questions.md). Do not copy their lists here.

## Health

| Check | Result |
| --- | --- |
| CI | Every PR runs `ci.yml`: builds on Windows, macOS and headless Linux, tests, Python, docs, format and hygiene checks; `ci-ok` is required on `main`. |
| Build | Skeleton builds on Linux (GCC 13), Windows (MSVC, Visual Studio 2026) `[VERIFIED 2026-10-24]` and macOS (Apple Clang) `[VERIFIED 2026-10-27]`. |
| Python | Python 3.12 clean-environment install, Ruff, pytest and `pip check` pass `[VERIFIED 2026-11-04]`. |

## Things a new agent should know immediately

- **Humans decide; agents consult.** Put every real uncertainty to the human (`DECISION NEEDED`); log their decision as `Accepted`; update every doc (`playbooks/record-decision.md`).
- Course documents are not the truth; the decision log is.
- Stack is fixed: C++20 / custom ECS / Box2D v3.1.1 (behind the adapter) / Raylib 6.2 (behind the rendering abstraction) / CMake + GoogleTest; `ai-services` uses Python 3.12 (DEC-039). **2D only**, no 3D work.
- Work through GitHub ([CONTRIBUTING.md](../../../CONTRIBUTING.md), [git-workflow.md](../../guidelines/git-workflow.md)): one branch per issue (`feat/us-15.2-…`, `fix/142-…`, or `ci/<PR #>-…` with `no-issue`), Conventional Commit PR titles, merge `main` before each push. Agents on a member's computer commit and push only after that member approves; agents in a cloud copy commit, push and open the draft PR without asking (DEC-058, DEC-061).
- Tasks are GitHub issues; the issue's assignee is the claim (DEC-149.1). New record ids carry the GitHub number: `DEC-80.1`, `HND-80.2` (DEC-148.1).
- Raylib must not be initialised headless; only `Application` opens the window, through `RaylibWindow::open`, in windowed mode.
- The `editor` is the application that hosts the engine and owns the main window; `sandbox` is only a developer and CI test host; games are built inside the editor (DEC-048).
- The demo game is `[UNDECIDED]` (Q-016): do not design or build one, or treat an example (door, key, maze) as its spec.
- The course requires disclosure of AI-generated text in deliverables (`guidelines/genai-usage-rules.md`).
- Project manager: Işık Dönger. Docs in this repo are English.
