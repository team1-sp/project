---
last_updated: 2026-11-11
updated_by: claude (PB-025 session, merged with main)
---

# Task Board

Statuses: `Backlog` · `Ready` · `Claimed` · `Review` · `Done` · `Blocked` · `Dropped`.
Rules: claim before working; one owner per task; see [communication-protocol.md](../communication-protocol.md). Entry template: [templates/task.md](../templates/task.md).

## Board (summary — keep in sync with entries below)

| ID | Title | Status | Owner / Claimed by | Depends on | Serves |
| --- | --- | --- | --- | --- | --- |
| TASK-000 | Create docs scaffold and agent protocol | Done | claude-code/sonnet-5.5/012r | — | process |
| TASK-001 | Graphics framework / shader benchmark (OpenGL, Raylib, SFML) | Done | humans | — | DEC-012 (superseded by DEC-015) |
| TASK-002 | Decide graphics framework | Done (DEC-015: Raylib) | human decision | — | ip.fr.4.* |
| TASK-003 | Decide programming language and physics library | Done (DEC-013 C++, DEC-014 Box2D) | human decision | — | ip.fr.3.* |
| TASK-004 | Decide ECS approach | Done (DEC-018: custom) | human decision | — | ip.fr.2.* |
| TASK-005 | Bootstrap `project/` skeleton (CMake, folders, GoogleTest, Raylib + Box2D linked, empty window) — ClickUp PB-002 | Review | Işık Dönger + claude-code (PB-002 session) | — | all |
| TASK-006 | Phase 2: window + game loop with fixed-timestep accumulator — ClickUp PB-011, PB-012 | Done (PR #35, #37; DEC-046, DEC-047 accepted) | Arda Karabulut + claude-code (PB-011 and PB-012 sessions) | TASK-005 | ip.fr.1.1–1.4 |
| TASK-007 | Fill `environment-setup.md` once the toolchain is chosen | Review (first version written with PB-002; needs a clean-machine check on Windows and macOS) | — | TASK-005 | — |
| TASK-008 | Decide world units and coordinate convention (y-up vs y-down, pixels-per-meter) | Done (DEC-036) | Zeynep Ertürk (PB-008.3) | — | ip.fr.3.5, 4.1 |
| TASK-009 | Propagate the 2026-10-13 decisions (DEC-013…017) and role/name fixes through all docs | Done | claude-code/sonnet-5.5/012r | — | DEC-017 |
| TASK-010 | Design the custom ECS incl. LLM-instructive component metadata | Deferred by the human ("too early") | — | Q-013 | ip.fr.2.*, 10.* |
| TASK-012 | Set up the GitHub repository and project board | Done (DEC-022…028, PB-001); leftovers in Q-014 | humans + claude-code (session 0171) | — | DEC-009, process |
| TASK-013 | Reconcile local docs with GitHub main (merge, renumber colliding IDs, update stale state) | Done | claude-code/sonnet-5.5/012r | — | DEC-017 |
| TASK-011 | Propagate round-2 decisions (DEC-018…021) | Done | claude-code/sonnet-5.5/012r | — | DEC-017 |
| TASK-014 | Integrate GoogleTest and CTest, how-to-add-a-test guide — ClickUp PB-003 | Done (PR #14) | Işık Dönger + claude-code (PB-003 session) | TASK-005 | process, test-strategy |
| TASK-015 | Set up CI with GitHub Actions — ClickUp PB-004 | Review | Işık Dönger + claude-code (PB-004 session) | TASK-005, TASK-014 | process |
| TASK-016 | Spike: Box2D v3 + raylib falling-box prototype — ClickUp PB-008 | Review (DEC-037 awaits Işık) | Zeynep Ertürk + claude (PB-008 session) | TASK-005 | DEC-014, DEC-015, TASK-008 |
| TASK-017 | Create the Python `ai-services` project skeleton — ClickUp PB-006 | Review | codex/gpt-5/pb006 | TASK-012 | AI layer tooling |
| TASK-018 | Editor hosts `Application` and opens the main window — ClickUp PB-122 | Review (issue #38) | Işık Dönger + claude (PB-122 thread) | TASK-006 | ip.fr.1.3, srs.fr.1.13, DEC-048 |
| TASK-019 | Define the physics adapter interface — ClickUp PB-025 | Review | Zeynep Ertürk + claude (PB-025 session) | TASK-016 | ip.fr.3.1, R-3 |

Blocked tasks become `Ready` when the dependency is `Done` **and** the related decision is `Accepted` (decided by a human).

## Entries

### TASK-000 — Create docs scaffold and agent protocol
- **Status:** Done (2026-10-12)
- **Claimed by:** claude-code/sonnet-5.5/012r
- **Result:** Folder tree per `docs/README.md`; seeded logs, state files, templates, playbooks. Reviewed by the project manager 2026-10-13 ("looks good", with four corrections → TASK-009).

### TASK-001 — Graphics framework / shader benchmark
- **Status:** Done (reported by Işık Dönger 2026-10-13): benchmark finished, no notable difference; team chose by code complexity and ease of use (DEC-015).
- **Spec:** `senior-project/shader-test/test-guideline.md`; summary in `testing/benchmark-protocol.md`.

### TASK-002 — Decide graphics framework
- **Status:** Done — human decision recorded as DEC-015 (Raylib), 2026-10-13.

### TASK-003 — Decide programming language and physics library
- **Status:** Done — DEC-013 (C++), DEC-014 (Box2D), 2026-10-13. Scope: DEC-016 (2D only).

### TASK-004 — Decide ECS approach
- **Status:** Done — DEC-018 (custom ECS), 2026-10-13. Follow-up: TASK-010.

### TASK-005 — Bootstrap project skeleton
- **Status:** Review (2026-10-24), branch `ci/pb-002-ci-create-the-cmake-build-skeleton`. Was Ready (unblocked 2026-10-16).
- **Result:** Root `CMakeLists.txt`, `CMakePresets.json` (windows-debug/-release, macos-debug/-release, headless; configure, build, test and workflow presets), `cmake/Dependencies.cmake` (Box2D v3.1.1, raylib 6.2, GoogleTest v1.17.0 by git tag), `cmake/CompilerWarnings.cmake`; targets `engine_core`, `engine_physics_box2d` (only Box2D user), `engine_rendering_raylib` (only raylib user), `editor` (placeholder), `demo-maze` (opens and closes a window), `engine_tests` (2 tests, no window). CI `build` job on Windows, macOS and Linux-headless, required through `ci-ok`.
- **Verified:** Linux (see HND-010); Windows by Işık Dönger on 2026-10-24 (Visual Studio 2026, MSVC 19.51: workflow preset ran, demo window opened). macOS by a teammate on 2026-10-27 (Apple Clang: workflow preset ran, 2/2 tests, demo window opened).
- **Inputs:** ClickUp PB-002. CMake + GoogleTest (DEC-023), C++20 (DEC-019), Raylib 6.2 (DEC-033), Box2D v3.1.1 (DEC-020), compilers per platform need (DEC-030), Windows + macOS development (DEC-029), layout in `guidelines/repository-structure.md` (DEC-022, DEC-026).
- **Acceptance:** Clean checkout builds an executable that links Raylib and Box2D and opens/closes a window on Windows (and builds on macOS, DEC-029); the test runner runs one trivial test headless (no window); `repository-structure.md` updated to reality; `environment-setup.md` has exact commands. Note: the layout lists presets `windows-debug`, `windows-release`, `headless` only — add macOS presets (DEC-029).

### TASK-006 — Window + game loop (ClickUp PB-011, PB-012)
- **Status:** Done (2026-11-11): PR #35 and #37 merged 2026-11-10, DEC-046 and DEC-047 accepted by Işık Dönger 2026-11-11. Was Review (2026-11-10). PB-011: issue #34, branch `feat/pb-011-feat-open-app-win-n-run-main-loop`. PB-012: issue #36, branch `feat/pb-012-feat-implement-fixed-timestep-1-60-s`, stacked on PB-011. Was Blocked by TASK-005 (PB-002, merged).
- **Claimed by:** Arda Karabulut + claude-code (PB-011 session) on 2026-11-10
- **Acceptance:** Window opens/closes cleanly (ip.fr.1.3); fixed-timestep accumulator for physics at 1/60 s decoupled from render (ip.fr.1.2); mode flag normal/headless parsed (ip.fr.1.4) — headless path must not initialise Raylib.
- **Acceptance (ClickUp PB-011):** window opens, Update and Draw are called each frame; closing exits with code 0 and no leaks; window code only in the platform module (read as `engine/rendering-raylib`, DEC-046).
- **Plan (PB-011):** `IWindow` in `engine/core`, `RaylibWindow` in `engine/rendering-raylib`; `Application` with `init`/`run`/`shutdown` over `ISimulation` and `IFrameRenderer`; guarded leak report; `sandbox` on `Application`; unit tests with a fake window; a headless sandbox run as a CTest test.
- **Result (PB-011):** done as planned. Serves srs.fr.1.1, 1.7 to 1.10, 1.13, 1.15 and 7.1, 7.4, 7.7. `runSmokeWindow` removed. The loop runs one tick per render frame until PB-012. Not handled here: `RunError` (srs.fr.1.11), pause and step (srs.fr.1.16 to 1.18), fullscreen (srs.fr.1.9), the keyboard queue.
- **Verification:** Windows 11, MSVC 19.44, `ENGINE_WARNINGS_AS_ERRORS=ON`: `windows-debug` builds with no warnings, `ctest` 24/24; `headless` preset builds without raylib, `ctest` 16/16; `sandbox.exe --max-ticks 120` opened the window, exit code 0, nothing on stderr; with a deliberate 64-byte leak the report appeared and `Sandbox.HeadlessRun_ExitsCleanly` failed; with the tick moved after present, two `ApplicationTest` tests failed. See HND-018.
- **Acceptance (ClickUp PB-012):** the simulation steps 0..N times per rendered frame; a maximum of steps per frame protects against the spiral of death; 60 steps are 1.0 s in tests.
- **Plan (PB-012):** `Tick` and the 1/60 s constants in `tick.hpp`; `FrameClock` over `steady_clock` with an injectable time source; `FixedTimestep` as its own class with exact integer math; `Application` uses both and keeps the tick counter; unit tests feed frame times as numbers.
- **Result (PB-012):** done as planned. Serves srs.fr.1.2 to 1.6 and 1.15. `ISimulation::runTick` now takes the tick number. The sandbox shows ticks, simulated time, real time and frame time, with `--fps N`.
- **Verification (PB-012):** same machine and flags: `windows-debug` builds with no warnings, `ctest` 49/49; `headless` preset 41/41; `sandbox.exe --max-ticks 300` took 5.00 s, 5.01 s, 5.00 s and 5.00 s of real time at `--fps 30`, `60`, `144` and `0`, exit code 0, nothing on stderr; a tick rounded to whole nanoseconds failed 7 tests, and incrementing the counter before the tick failed 7. See HND-019.
- **Links:** DEC-046 and DEC-047 (`Accepted` 2026-11-11), Q-019, Q-020, HND-018, HND-019.

### TASK-007 — Environment setup doc
- **Status:** Blocked by TASK-005
- **Acceptance:** A new team member can build and run from a clean machine using only `environment-setup.md`.

### TASK-008 — World units and coordinate convention
- **Status:** Done (2026-11-01) — DEC-036, decided by Zeynep Ertürk in PB-008.3: meters, y up, 32 pixels per meter, conversion only in the renderer
- **Why it matters:** Raylib's screen space is y-down; Box2D is y-up and works best in meters; mixing them is a classic bug (`common-errors.md` E-004, E-005).
- **Acceptance:** Accepted decision; documented in `design/physics-adapter-design.md`.

### TASK-009 — Propagate the 2026-10-13 decisions
- **Status:** Done (2026-10-13), claude-code/sonnet-5.5/012r
- **Result:** DEC-013…017 logged; AGENT-PROTOCOL rewritten (Truth Rule, Raising uncertainty, Disagreeing); `playbooks/record-decision.md` added; stack/scope updates applied across docs; Işık Dönger recorded as Project Manager; full names for Arda Karabulut and Zeynep Ertürk.
- **Not verified:** correctness of the team's agreement beyond the project manager's statement (Q-010).

### TASK-010 — Design the custom ECS (LLM-instructive components)
- **Status:** Deferred by the human ("too early", 2026-10-15); also needs Q-013
- **Serves:** DEC-018, ip.fr.2.*, ip.fr.10.*
- **Acceptance:** Design doc (from `design/templates/design-doc-template.md`) satisfying `design/ecs-design.md` §1 and the metadata decided in Q-013; accepted by the human.

### TASK-011 — Propagate round-2 decisions
- **Status:** Done (2026-10-13), claude-code/sonnet-5.5/012r. DEC-018, DEC-020 Accepted; DEC-019, DEC-021 Proposed (corrected 2026-10-13); docs updated; Q-003/Q-008/Q-010/Q-012 closed; Q-009 recorded as deferred; Q-007 narrowed; Q-013 added.

### TASK-012 — Set up the GitHub repository and project board
- **Status:** Done 2026-10-15 by the team and claude-code (session 0171): one repository (DEC-022), CMake + GoogleTest (DEC-023), CODEOWNERS review (DEC-024), agent push rule (DEC-025), `semantic/` folder (DEC-026), trunk-based workflow (DEC-027), ClickUp-created issues (DEC-028). Repo files: `CONTRIBUTING.md`, `.github/`, `tools/`, `.editorconfig`, `.gitattributes`, `.gitignore`.
- **Left over:** license, access/visibility, Git LFS — see Q-014. Project name answered: Aegis (DEC-034).

### TASK-013 — Reconcile local docs with GitHub main
- **Status:** Done 2026-10-16, claude-code/sonnet-5.5/012r. Three-way merge (base `95cb8d9`, upstream `69670b0`); local decisions DEC-022/023 renumbered DEC-029/030; stale state files updated; see HND-009.

### TASK-014 — Integrate GoogleTest and CTest (ClickUp PB-003)
- **Status:** Done (2026-10-27), merged as PR #14.
- **Acceptance (ClickUp):** test target builds and runs with `ctest`; a sample test passes; how-to-add-a-test documented.
- **Result:** GoogleTest/CTest integration came with TASK-005 (PB-002). Added `tests/README.md`; `tests/unit/core/version_test.cpp` is the sample test (human choice, no dummy test). See HND-012.

### TASK-015 — Set up CI with GitHub Actions (ClickUp PB-004)
- **Status:** Review (2026-10-27), branch `ci/pb-004-set-up-ci-with-github-actions`, issue #15.
- **Acceptance (ClickUp):** engine builds and tests on windows-latest for every PR; formatting checked; Python runs ruff + pytest; checks required before merge.
- **Result:** Already there before this task: `build` job (PB-002, Windows/macOS/headless presets + `ctest`), `format` job (PB-005), `ci-ok` required by the "Protect Main" ruleset (DEC-032). Added: cache of the FetchContent downloads in `build` (PB-004.2) and a `python` job (PB-004.4) that runs `ruff check` and `pytest` for `ai-services/` and `semantic/` once they have a `pyproject.toml` (PB-006 creates them). See HND-013.

### TASK-016 — Spike: Box2D v3 + raylib falling-box prototype (ClickUp PB-008)
- **Status:** Review (2026-11-01). Zeynep Ertürk + claude (PB-008 session)
- **Acceptance:** ground + falling box simulated by Box2D and drawn by raylib; units decided; findings written for the guide (`docs/research/box2d.md`), presented at sprint review.
- **Result:** `sandbox_falling_box` (Box2D only, our types in its header) and `box2d_raylib_spike` (raylib window) in `sandbox/`; 7 tests in `tests/unit/sandbox/`; units decided (DEC-036); findings in `research/box2d.md`; pitfalls E-026…E-028. The R-3/R-4 exception for the spike is `Proposed` (DEC-037, Q-017). See HND-014.

### TASK-017 — Create the Python `ai-services` project skeleton (ClickUp PB-006)
- **Status:** Review (2026-11-04); implemented by codex/gpt-5/pb006.
- **Goal:** Add the `ai-services/` Python project skeleton required by GitHub issue #20: a `pyproject.toml` with a pinned Python version, package layout, environment instructions, Ruff and pytest configuration, and a passing sample test.
- **Plan:**
  1. Record the human's Python version decision and propagate it to the relevant project docs.
  2. Create a `src/` package layout under `ai-services/` without implementing deferred AI capabilities.
  3. Configure Ruff and pytest in `ai-services/pyproject.toml`, with pinned development-tool versions.
  4. Add concise setup/test instructions and one sample package test.
  5. Run Ruff and pytest from `ai-services/`, then inspect the final diff.
- **Expected files:** `ai-services/pyproject.toml`, `ai-services/README.md`, `ai-services/src/`, `ai-services/tests/`, and project state/decision documentation required by the agent protocol.
- **Risks:** Python's minor version is not specified; choosing one would silently make a technology decision. Keep the skeleton isolated from `engine/` and do not add model/runtime dependencies.
- **Verification:** create a clean virtual environment, install the project development dependencies, run `ruff check .` and `pytest` from `ai-services/`.
- **Result:** Added a Python 3.12 `src/`-layout package, pinned setuptools/Ruff/pytest configuration, platform setup instructions and a package-metadata smoke test. CI now installs each Python package's pinned `dev` extra. Ruff, pytest (1/1) and `pip check` pass in a fresh Python 3.12 environment. See HND-015.

### TASK-018 — Editor hosts `Application` and opens the main window (ClickUp PB-122)
- **Status:** Review (2026-11-11). Issue #38, branch `feat/pb-122-make-the-editor-host-the-application-and`.
- **Claimed by:** Işık Dönger + claude (PB-122 thread) on 2026-11-11
- **Acceptance:** `editor` opens the main window through `Application` and `RaylibWindow`; closing exits with code 0 and no leaks; the `headless` preset still builds without raylib.
- **Result:** `editor/src/main.cpp` runs `Application` with an empty simulation (no project open) and a status line; `--max-ticks N` closes it after N ticks. `editor` links `engine::rendering_raylib` and, like `sandbox`, is built only when `ENGINE_WITH_RAYLIB` is on.
- **Links:** DEC-048, HND-020, PB-110.

### TASK-019 — Define the physics adapter interface (ClickUp PB-025)
- **Status:** Review (2026-11-10). Zeynep Ertürk + claude (PB-025 session)
- **Acceptance:** the adapter interface with the required operations; no Box2D header outside the adapter implementation; reviewed and documented.
- **Result:** `engine::physics` target (header-only) with `BodyDesc`, `BodyId`, `BodyState`, `CollisionEvent`, `RaycastHit` and `IPhysicsAdapter`; `Vec2` and `EntityId` in `engine/core`; `tools/check_includes.py` in CI; 4 contract tests with a fake adapter. Name kept as `IPhysicsAdapter` (ClickUp said `IPhysicsWorld`). Choices accepted by Işık as DEC-049. See HND-021.
