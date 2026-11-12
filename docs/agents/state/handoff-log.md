# Handoff Log

**Append-only. Newest entry at the bottom.** Never edit or delete earlier entries; add a correcting entry instead. Template: [../templates/handoff.md](../templates/handoff.md). Read the last ~5 entries at session start.

---

### HND-001 — Docs scaffold created
- **Date:** 2026-10-12
- **Agent:** claude-code/sonnet-5.5/012r
- **Task:** TASK-000
- **Goal:** Create developer docs and the agent coordination system under `project/docs/`.
- **Result:** Done, awaiting human review.
- **Files changed:** entire `project/docs/` tree (new).
- **Verified:** Directory listing after write. Cross-checked terminology and flow against the Senior Project Guide, the Project Context Prompt and Initial Plan §2 (requirements IDs). No code exists, so nothing was built or run.
- **Not verified:** Whether the team agrees with every `Accepted` decision in the decision log (they were extracted from the guide and Initial Plan, not formally voted). Status of the shader benchmark implementations.
- **Repo state:** No code. `project/` contains README, .gitignore, docs.
- **Next:** (1) Humans review `decision-log.md` DEC-001…DEC-009 and change anything wrong; (2) answer Q-001…Q-004 in `open-questions.md`; (3) finish TASK-001 benchmark; (4) an agent may then prepare decision briefs (TASK-002…004).
- **Traps:** Technology choices are intentionally `[TBD]`. Do not start engine code before TASK-005.

---

### HND-002 — Feedback round: roles, stack decision, decision-authority protocol
- **Date:** 2026-10-13
- **Agent:** claude-code/sonnet-5.5/012r
- **Task:** TASK-009
- **Goal:** Apply the project manager's four corrections: (1) Işık Dönger is Project Manager; (2) full names; (3) stack decided: C++, Box2D, Raylib, 2D only with 3D possible later; (4) humans decide, uncertainty always surfaced, decision log is the single truth, course docs not authoritative.
- **Result:** Done.
- **Files changed:** `AGENT-PROTOCOL.md` (rewritten), new `playbooks/record-decision.md`, `templates/decision-entry.md`, `rules.md`, `communication-protocol.md`, `model-compatibility.md`, `agents/README.md`, `decision-log.md` (DEC-013…017; DEC-006, DEC-012 superseded), `tech-stack.md` (rewritten), state files, coding standards, repo structure, environment setup, common errors, engine layer, glossary, physics adapter, headless testing, benchmark protocol, roadmap, test strategy, development process, documentation rules, README. The user had already edited `README.md` and `process/development-process.md` for names/roles; their versions were used as the base.
- **Verified:** relative links checked by script; grep for old terms (C#, MonoGame, BEPUphysics, "Arda K.", "Zeynep" alone) leaves mentions only in history/decision entries. No code exists; nothing built.
- **Not verified:** that the whole team agrees with the stack (Q-010); rationale for each choice (not given).
- **Next:** humans answer Q-007, Q-008, Q-009, Q-012 → record + propagate → TASK-005.
- **Traps:** Raylib needs a window/GL context for loading textures — keep it out of headless mode. Box2D v2.4 and v3 have different APIs (Q-008). The Initial Plan still says "C++ or C#" etc.; it is not authoritative and was not edited.
- **Records:** DEC-013…017, Q-007…Q-010, Q-012, TASK-009.

---

### HND-003 — Round-2 answers recorded and propagated
- **Date:** 2026-10-13
- **Agent:** claude-code/sonnet-5.5/012r
- **Task:** TASK-011
- **Goal:** Record Işık Dönger's answers to Q-012, Q-009, Q-007, Q-008, Q-003, Q-010 and update every document (record-decision playbook).
- **Result:** Done; two items intentionally open.
- **Decided / recorded:** DEC-018 custom ECS; DEC-019 C++20, dev Windows+macOS, Windows-only target; DEC-020 Box2D v3.1.1; amendments on DEC-013/014/015. DEC-021 (Raylib v6.0) left `Proposed` — the human wrote "still open". Q-009 recorded as deferred. New Q-013.
- **Files changed:** decision log + entry template (Amendments field), record-decision playbook, tech-stack, coding-standards, repository-structure, physics-adapter-design, ecs-design, semantic-layer, common-errors, environment-setup, test-strategy, benchmark-protocol, roadmap, state files.
- **Not verified:** Box2D v3.1.1 / Raylib v6.0 API details (from memory, `[ASSUMPTION]`); team agreement on DEC-013/014/016.
- **Next:** human answers rest of Q-007 and Q-013; Q-009 when ready → TASK-005, TASK-010.
- **Traps:** Box2D v3 is a C API with ids; most tutorials are v2.4. Do not start the skeleton: build system deferred.
- **Records:** DEC-018…021, Q-013, TASK-010, TASK-011.

---

### HND-004 — Correction: C++20/platforms are proposals, not decisions
- **Date:** 2026-10-13
- **Agent:** claude-code/sonnet-5.5/012r
- **Task:** TASK-011 (follow-up)
- **What happened:** In HND-003 I recorded DEC-019 (C++20, platforms) as Accepted, reading "still open" as applying only to Raylib. Işık Dönger clarified that "still open" covered the whole Q-007 answer. DEC-019 is now Proposed (with an Amendment describing my error); docs that assumed C++20/platforms were reverted to "proposed".
- **Also recorded:** the team agrees on C++, Box2D and 2D-only (amendments on DEC-013, DEC-014, DEC-016).
- **Lesson:** when an answer contains a hedge, confirm its scope before recording `Accepted` (record-decision.md: "If you are not sure the human decided, ask once more").
- **Records:** DEC-013, DEC-014, DEC-016, DEC-019, DEC-021, Q-007.

### HND-005 — Platforms decided (DEC-029)
- **Date:** 2026-10-15
- **Agent:** claude-code/sonnet-5.5/012r
- **Recorded:** DEC-029 Accepted: develop on Windows and macOS (Windows priority); games built for Windows only; other platforms/builds delayed. DEC-019 now covers only C++20 (still Proposed). Q-007 narrowed to C++ standard, Raylib version, compilers.
- **Updated:** decision log, tech-stack, coding-standards, environment-setup, current-status, plan, task-board, open-questions.
- **Not decided:** my reading that the engine must build and run on macOS (marked `[ASSUMPTION]` in DEC-029). I did not turn my earlier suggestions (platform layer, single tick function) into rules — only the human's words were recorded.
- **Next:** human answers Q-007 rest, Q-013; Q-009 when ready.

### HND-006 — macOS requirement confirmed; compilers decided
- **Date:** 2026-10-15
- **Agent:** claude-code/sonnet-5.5/012r
- **Recorded:** Amendment on DEC-029 (two team members use macOS; engine application and development tool must run on macOS — assumption confirmed). DEC-030 Accepted: compilers are whatever each platform needs; none pinned. Q-007 narrowed to C++ standard and Raylib version.
- **Updated:** decision log, tech-stack, coding-standards, environment-setup, roadmap, current-status, plan, task-board, open-questions.
- **Not verified:** which compiler runs on each OS (the human did not name one); what "development tool" means precisely (read as the engine's editor/dev tooling).
- **Next:** human answers C++20 and Raylib v6.0 (Q-007), Q-013, Q-009 when ready.

### HND-007 — C++20 and Raylib v6.0 accepted
- **Date:** 2026-10-15
- **Agent:** claude-code/sonnet-5.5/012r
- **Recorded:** DEC-019 (C++20) and DEC-021 (Raylib v6.0) Accepted on Işık Dönger's "all decisions are decided". Q-007 closed. I read "all decisions" as covering these two open proposals only.
- **Not covered / still open:** Q-009 (deferred by the human), Q-013 (LLM-instructive metadata), TASK-008 (world units), Q-002/005/006; DEC-010 and DEC-011 are still Proposed (I did not treat the statement as accepting them).
- **Not verified:** that a Raylib v6.0 release exists and fits our API assumptions — check before pinning (TASK-005).
- **Next:** TASK-005 still blocked by Q-009.

### HND-008 — Raylib v6.0 verified; priority is GitHub repository setup
- **Date:** 2026-10-15
- **Agent:** claude-code/sonnet-5.5/012r
- **Verified:** Raylib 6.0 exists (GitHub release tag `6.0`, latest; 5.5 before it). DEC-021 amended; `[ASSUMPTION]` replaced by `[VERIFIED]`. API changes to check at TASK-005: High-DPI flag, fullscreen, skeletal animation. 6.0 also has a software renderer and a headless Memory backend — noted for headless mode, not decided.
- **Recorded:** Q-009, Q-013, DEC-010, DEC-011 deferred by the human ("too early") — do not raise. New Q-014 (11 repo-setup items) and TASK-012; TASK-010 deferred.
- **Next:** human answers Q-014 → TASK-012.

### HND-009 — Reconciled local docs with GitHub main
- **Date:** 2026-10-16
- **Agent:** claude-code/sonnet-5.5/012r
- **Task:** TASK-013
- **Found:** `team1-sp/project` main (`69670b0`) had moved on from the docs commit `95cb8d9`: another agent session (0171) and the team added the repo foundation, CI, CODEOWNERS and decisions DEC-022…028 on 2026-10-15. My local, uncommitted edits (platforms, compilers, C++20/Raylib accepted, deferrals, Q-014) collided: five files carried merge-conflict markers, and my DEC-022/DEC-023 reused IDs already taken on main.
- **Done:** three-way merge (base `95cb8d9`, upstream `69670b0`, mine). My platform and compiler decisions are now DEC-029 and DEC-030 (amendments say so). Upstream DEC-022…028 kept as written; DEC-023 got an amendment. DEC-019 and DEC-021 are Accepted. Q-009 and Q-006 follow upstream (answered). Q-014 reduced to its leftovers; new Q-015 (TASK vs PB ids). State files updated: TASK-005 is Ready, TASK-012 Done.
- **Not verified:** that the macOS requirement (DEC-029) is covered by the CI workflow and presets — the layout lists Windows presets only. Whether the repo should stay public (Q-014).
- **Next:** human reviews, commits the merged docs on a `docs/` branch and opens a PR (DEC-025, DEC-027); answers Q-014, Q-015. Then TASK-005.

---

### HND-010 — CMake build skeleton (PB-002)
- **Date:** 2026-10-24
- **Agent:** claude-code (PB-002 session)
- **Task:** TASK-005 (ClickUp PB-002)
- **Goal:** C++20 CMake project with engine, editor, demo and tests targets; raylib and Box2D fetched at pinned versions; Windows and macOS presets plus headless.
- **Result:** Done, in review on `ci/pb-002-ci-create-the-cmake-build-skeleton`; not pushed by the agent (DEC-025).
- **Files changed:** `CMakeLists.txt`, `CMakePresets.json`, `cmake/*`, `engine/{core,physics-box2d,rendering-raylib}/*`, `editor/*`, `games/demo-maze/*`, `tests/*`, `.github/workflows/ci.yml` (new `build` job in `ci-ok`), `README.md`, `guidelines/repository-structure.md`, `troubleshooting/environment-setup.md`, decision log (DEC-033 raylib 6.2, DEC-034 name Aegis, DEC-035 PR title format; amendments on DEC-021, DEC-023), `tools/start_task.py` and `CONTRIBUTING.md`/`git-workflow.md`/PR template for DEC-035, tech-stack, coding-standards, common-errors, roadmap, state files.
- **Verified (Linux, GCC 13.3, CMake 3.28):** `cmake --workflow --preset headless` builds and 2/2 tests pass; same with `-DENGINE_WARNINGS_AS_ERRORS=ON` on GCC and Clang, no warnings; full build with raylib, `demo-maze --frames 30` under Xvfb printed engine 0.1.0, box2d 3.1.1, raylib 6.2.0 and exited 0 (rerun after the move to 6.2); clang-format 19.1.7 `--dry-run -Werror` clean.
- **Verified by the human (Windows):** Işık Dönger ran `cmake --workflow --preset windows-debug` (Visual Studio 2026, MSVC 19.51, CMake 4.4.3) and `demo-maze.exe` showed its window.
- **Verified by a teammate (macOS, 2026-10-27):** `cmake --workflow --preset macos-debug` built and passed 2/2 tests; `demo-maze` opened its window. The Apple linker prints a harmless `ignoring duplicate libraries: libengine_core.a` warning.
- **Not verified:** the CI workflow on GitHub (not yet seen running).
- **Next:** (1) human pushes the branch and marks the draft PR ready; (2) fix whatever the Windows/macOS CI jobs report; (3) each member runs `cmake --workflow --preset windows-debug` or `macos-debug` (PB-002.6) and confirms the compilers in `environment-setup.md` §3; (4) TASK-006.
- **Traps:** the Windows presets use the default (Visual Studio, multi-config) generator, so build and test presets pass `configuration`; executables land in `.../Debug/`. Box2D's samples and tests are skipped automatically because it is not the top-level project. The CMake project is `aegis` (DEC-034); namespaces and target names stay `engine` until someone decides otherwise.

---

### HND-011 — Demo game marked undecided; demo-maze removed
- **Date:** 2026-10-27
- **Agent:** claude-code (demo-undecided session)
- **Task:** GitHub issue #10 (no ClickUp task); branch to be renamed `fix/10-fix-demo-game-documentation`
- **Goal:** The team never agreed on the maze demo. Remove `games/demo-maze/` and its docs, and mark the demo game undecided.
- **Result:** Done, in review as PR #12. `games/demo-maze/` and `design/demo-game-spec.md` deleted. The raylib smoke window now runs from a new `sandbox/` app (same code, new name), so the Windows/macOS builds still prove raylib links. New Q-016 (which demo game). Status, plan and roadmap say the demo is undecided.
- **Files changed:** `CMakeLists.txt`, `sandbox/*` (new), `games/demo-maze/*` (deleted), `docs/design/demo-game-spec.md` (deleted), `docs/README.md`, `guidelines/repository-structure.md`, `troubleshooting/environment-setup.md`, `agents/state/{current-status,plan,open-questions,handoff-log}.md`, `process/roadmap.md`.
- **Also:** branch names tightened to the active ruleset: only `fix/` may use a bare issue number (`tools/check_pr.py`, `git-workflow.md`, `CONTRIBUTING.md`).
- **Not changed on purpose:** door/key/goal examples in architecture, design, testing and troubleshooting docs (e.g. `DoorOpenedEvent`, `B-DOOR-NO-KEY`) are general examples, not the demo spec. Earlier history entries (HND-010, task board) keep the old name.
- **Not verified:** the full Windows/macOS build after the rename (CI `build` job will run it).
- **Next:** team answers Q-016; then a decision entry, a spec under `design/` and `games/<name>/`.

---

### HND-012 — How-to-add-a-test guide (PB-003)
- **Date:** 2026-10-27
- **Agent:** claude-code (PB-003 session)
- **Task:** ClickUp PB-003 (TASK-014), branch `ci/pb-003-integrate-googletest-and-ctest`
- **Goal:** GoogleTest + CTest integrated, a passing sample test, how-to-add-a-test documented.
- **Result:** In review. PB-002 had already delivered PB-003.1 (GoogleTest v1.17.0 via FetchContent, `enable_testing()`, `gtest_discover_tests`, tests link the engine libraries, CI runs `ctest`). This task adds `tests/README.md` (where tests go, naming, registering a file, running all or one test). Işık Dönger chose to use the existing `version_test.cpp` as the sample instead of a dummy `sample_test.cpp` (2026-10-27). Unit tests live in `tests/unit/<folder>/`, not `tests/core/` as the ClickUp text said; test names follow `coding-standards.md`, not ClickUp's `ClassName_Behavior`.
- **Files changed:** `tests/README.md` (new), `docs/testing/test-strategy.md` (framework no longer "not chosen"), `docs/troubleshooting/environment-setup.md` (link), `agents/state/{task-board,current-status,handoff-log}.md`.
- **Verified (Linux, GCC 13, CMake 3.28):** `cmake --workflow --preset headless` builds, 2/2 tests pass; `ctest --preset headless -R Box2d` runs 1 test; `engine_tests --gtest_filter=EngineVersion.*` runs 1 test.
- **Not verified:** the Windows `engine_tests.exe` path in `tests/README.md` (`build\windows-debug\tests\Debug\`) is inferred from the multi-config generator, not run.
- **Next:** human commits, pushes and marks the draft PR ready; then TASK-006 (window + game loop).
- **Traps:** branches named like `ci(pb-003)-…` (parentheses, from the PR title) fail the branch rule; use the branch the Issue start workflow creates.

---

### HND-013 — CI with GitHub Actions (PB-004)
- **Date:** 2026-10-27
- **Agent:** claude-code (PB-004 session)
- **Task:** ClickUp PB-004 (TASK-015), branch `ci/pb-004-set-up-ci-with-github-actions`, issue #15
- **Goal:** Every PR is built and tested automatically; formatting checked; Python runs ruff + pytest; checks required before merge.
- **Result:** In review. PB-004.1 (build + `ctest` on windows-latest, plus macOS and headless) came with PB-002, PB-004.3 (clang-format 19 check) with PB-005, and PB-004.5 (`ci-ok` required on `main`) with the "Protect Main" ruleset (DEC-032). New: the `build` job caches `build/<preset>/_deps` without the `*-build` folders, keyed on OS, preset and the hash of `cmake/Dependencies.cmake` (PB-004.2); a `python` job runs `ruff check` and `pytest` in each of `ai-services/` and `semantic/` that has a `pyproject.toml`, and is part of `ci-ok` (PB-004.4). `main-guard.yml` comment updated (protection is on now); Q-014 items 1 (protection part) and 5 marked answered.
- **Files changed:** `.github/workflows/ci.yml`, `.github/workflows/main-guard.yml`, `agents/state/{task-board,current-status,open-questions,handoff-log}.md`.
- **Verified (Linux, CMake 3.28):** with `_deps` kept but `*-build`, `CMakeCache.txt` and `CMakeFiles` deleted, `cmake --preset headless` re-configures in 1.1 s instead of 5.6 s without cloning, then builds and passes 2/2 tests. The `python` step, run locally with bash `-eo pipefail`, passes a sample `ai-services/` package (ruff clean, 1 test) and prints "nothing to check" without one. YAML parses.
- **Not verified:** the cache on real runners (first run fills it; the second PR run should show a cache hit and a shorter Windows configure). Python 3.12 is an assumption until PB-006 picks the version; PB-006 may change the install line (e.g. a `dev` extra).
- **Next:** human pushes and marks the PR ready; PB-006 adds `ai-services/pyproject.toml` with ruff and pytest settings, and the `python` job starts checking it.
- **Traps:** a pull request that changes no C++/CMake file skips `build` (and its cache). `pytest` exits with code 5 when it finds no tests, which fails the job; PB-006 needs at least one test.

---

### HND-014 — Box2D + raylib falling-box spike (PB-008)
- **Date:** 2026-11-01
- **Agent:** claude (PB-008 session), working with Zeynep Ertürk
- **Task:** TASK-016 (ClickUp PB-008); TASK-008 closed by DEC-036
- **Goal:** Ground + falling box simulated by Box2D and drawn by raylib; units decided; findings for the team.
- **Result:** Done, ready for review. Box2D code in `sandbox/src/falling_box_scene.{hpp,cpp}` (header uses only our types), raylib code in `sandbox/src/box2d_raylib.cpp`, conversion in `sandbox/src/world_to_screen.hpp`. Fixed 1/60 s step with 4 substeps driven by a frame-time accumulator. Zeynep decided the units (DEC-036) and chose a separate spike target over splitting it into the engine targets (DEC-037, `Proposed` until Işık confirms; Q-017).
- **Files changed:** `sandbox/CMakeLists.txt`, `sandbox/src/{falling_box_scene.hpp,falling_box_scene.cpp,world_to_screen.hpp,box2d_raylib.cpp}`, `tests/CMakeLists.txt`, `tests/unit/sandbox/*` (new), `docs/research/box2d.md` (new), `docs/components.md` (new), `docs/README.md`, `docs/guidelines/{coding-standards,documentation-rules}.md`, `docs/design/{physics-adapter-design,ecs-design}.md`, `docs/troubleshooting/common-errors.md`, `docs/history/decision-log.md`, `docs/agents/state/{task-board,current-status,open-questions,handoff-log}.md`.
- **Verified (Linux, GCC 13, CMake 3.28, `ENGINE_WARNINGS_AS_ERRORS=ON`, raylib on):** builds with no warnings; `ctest` 9/9 pass; with `enableContactEvents` removed, `BoxLands_ContactEventNamesGroundAndBox` fails (so the flag is needed); `cmake --workflow --preset headless` builds and passes 2/2 (sandbox not built there); clang-format 19.1.7 `--dry-run -Werror` clean; `box2d_raylib_spike --frames 330` under Xvfb exits 0 and shows the box landing and resting at y = 1.50 m, asleep, with 1 contact.
- **Verified by Zeynep Ertürk (macOS):** `cmake --workflow --preset macos-debug` passes 9/9; the spike window works.
- **Verified on Windows (claude, on Işık Dönger's machine):** the first CI runs failed to build `engine_tests` because `falling_box_scene_test.cpp` used `std::minmax` without `<algorithm>`; fixed in `0b2a5c9`. With Visual Studio 2026 and `ENGINE_WARNINGS_AS_ERRORS=ON`, the `windows-debug` preset builds with no errors and `ctest` passes 9/9. At that commit the `build (windows-latest, windows-debug)` CI job and `ci-ok` pass. `box2d_raylib_spike.exe` is built in `sandbox\Debug\`, the path given in `research/box2d.md` section 7.
- **Not verified:** the spike window on Windows (the exe was built but not run).
- **Next:** Işık answers Q-017; then the Physics Adapter design can use `research/box2d.md` section 8.
- **Traps:** contact events are off by default in v3.1.1 (E-026). The ClickUp text names `docs/components.md`, which did not exist; Zeynep chose to create it (units and axes only; the component list stays in `design/ecs-design.md`). The spike target is only built when raylib is on, so its tests do not run in the headless preset.
- **Records:** DEC-036 (Accepted), DEC-037 (Proposed), Q-017, E-026…E-028.

---

### HND-015 — Python `ai-services` project skeleton (PB-006)
- **Date:** 2026-11-04
- **Agent:** codex/gpt-5/pb006
- **Task:** TASK-017 (ClickUp PB-006), GitHub issue #20
- **Goal:** Create a ready Python project structure so RL, graph and assistant work can begin without setup friction.
- **Result:** Ready for review. Arda selected Python 3.12 (DEC-039). Added an isolated `ai-services/` src-layout package, `.python-version`, package metadata, pinned setuptools/Ruff/pytest versions, Ruff and pytest settings, environment instructions and a package-metadata smoke test. Python CI now installs each package's pinned `dev` extra.
- **Files changed:** `ai-services/{.python-version,pyproject.toml,README.md,src/aegis_ai_services/__init__.py,tests/test_package.py}`, `.github/workflows/ci.yml`, and decision/state/environment/repository documentation.
- **Verified (macOS):** Python 3.12.13 fresh virtual environment; editable install with pinned tools succeeds; `python -m ruff check ai-services` passes; `python -m pytest` passes 1/1; `python -m pip check` reports no broken requirements; repository docs links pass.
- **Not verified:** GitHub Actions execution and Windows-local setup commands; CI uses the same Python 3.12 install and test flow.
- **Next:** Arda reviews the diff and creates the PR; CI should run the Python and docs jobs. Future AI capabilities remain separate tasks, and the engine-facing protocol is still deferred.
- **Traps:** Run the package checks from `ai-services/` after installing `.[dev]`. Python 3.13+ is intentionally rejected by package metadata. A fresh dependency install needs PyPI access (PRB-003).
- **Records:** DEC-039, Q-018 answered, PRB-003.

---

### HND-016 — C++/Python bridge research and decision (PB-065)
- **Date:** 2026-11-04
- **Agent:** claude (PR #23 review), on top of the PB-065 work by Codex
- **Task:** ClickUp PB-065, GitHub issue #22, PR #23
- **Result:** Işık Dönger accepted DEC-038 ("I accept dec-38"): the engine-to-Python bridge is gRPC/protobuf behind `engine::bridge::AgentBridge`. DEC-011 is superseded by DEC-038 and DEC-039. ADR-002, tech-stack, research docs and state files updated to match.
- **Next:** a production bridge task (pinned gRPC/protobuf, `.proto` contract, Windows build check). End-to-end RL benchmark before revisiting the transport.
- **Traps:** gRPC on Windows has not been built yet. The benchmark harness under `benchmarks/bridge/` is research code, not the production bridge.
- **Records:** DEC-038 accepted, DEC-011 superseded, DEC-039 and PRB-003 renumbered during the merge, PRB-002.

---

### HND-017 — Record the user-supplied LLM provider decision
- **Date:** 2026-11-06
- **Agent:** claude (use cases thread)
- **Task:** related to ClickUp PB-068 (does not complete it)
- **Result:** Işık Dönger decided that Aegis builds no model of its own, users supply their own provider account and API key, and only free-tier or local use is guaranteed. Recorded as DEC-045, with matching updates to `architecture/tech-stack.md` and `architecture/ai-assistant-layer.md`.
- **Next:** PB-068 picks which free-tier and local providers to support and test. The SRS changes (Sections 4 and 5) are handled in the course docs repo by the SRS-A review.
- **Traps:** The rationale was not given; ask before citing one.
- **Records:** DEC-045.

---

### HND-018 — Application window and game loop (PB-011)
- **Date:** 2026-11-10
- **Agent:** claude-code (PB-011 session), working with Arda Karabulut
- **Task:** TASK-006 (ClickUp PB-011)
- **Goal:** The engine opens a window and runs an update/draw loop; closing it exits with code 0 and no leaks.
- **Result:** Done, ready for review. `engine/core`: `Application` (`init`, `run`, `shutdown`), `IWindow` + `WindowConfig` + `WindowFactory`, `ISimulation`, `IFrameRenderer`, `enableLeakReportAtExit()`. `engine/rendering-raylib`: `RaylibWindow` (the only caller of `InitWindow`, `WindowShouldClose`, `CloseWindow`) and `StatusTextRenderer`; `smoke_window.*` became `raylib_info.*` and `runSmokeWindow` is gone. `sandbox` runs on `Application` with `--headless` and `--max-ticks N`. Arda chose the structure (DEC-046, `Proposed` until Işık confirms; Q-019).
- **Files changed:** `engine/core/{CMakeLists.txt,include/engine/core/{application,window,simulation,frame_renderer,leak_check}.hpp,src/{application,leak_check}.cpp}`, `engine/rendering-raylib/{CMakeLists.txt,include/engine/rendering_raylib/{raylib_info,raylib_window,status_text_renderer}.hpp,src/{raylib_info,raylib_window,status_text_renderer}.cpp}`, `sandbox/{CMakeLists.txt,src/main.cpp}`, `tests/{CMakeLists.txt,unit/core/application_test.cpp}`, `docs/architecture/engine-layer.md`, `docs/troubleshooting/{environment-setup,common-errors}.md`, `docs/history/decision-log.md`, `docs/agents/state/{task-board,current-status,open-questions,handoff-log}.md`.
- **Verified (Windows 11, Visual Studio Build Tools 2022, MSVC 19.44, CMake 4.2, `ENGINE_WARNINGS_AS_ERRORS=ON`):** `cmake --build --preset windows-debug` with no warnings; `ctest --preset windows-debug` 24/24 (13 new `ApplicationTest` tests and `Sandbox.HeadlessRun_ExitsCleanly`); `headless` preset builds without raylib, 16/16; clang-format 19.1.7 `--dry-run -Werror` clean. `sandbox.exe --max-ticks 120` opened the window, printed `ticks executed: 120`, exit code 0, standard error empty (no leak report). `sandbox.exe --headless` ran 10 000 ticks without any raylib start-up line, exit code 0. Mutations: a deliberate `new int[16]` gave "Detected memory leaks! ... 64 bytes long" and failed the sandbox test; moving the tick after present failed 2 `ApplicationTest` tests.
- **Not verified:** macOS and Linux builds (CI will run them after a push); closing the window by hand with the close button or Escape (the runs above ended through `--max-ticks`; the close path is covered by the fake-window tests).
- **Repo state:** builds, tests pass; committed on `feat/pb-011-feat-open-app-win-n-run-main-loop`, the branch the Issue start workflow made for issue #34 (created from ClickUp). The workflow could not open the draft PR ("GitHub Actions is not permitted to create or approve pull requests", HTTP 403), so the PR is opened by hand.
- **Next:** 1. Draft PR with `Closes #34`; CI then builds on Windows, macOS and Linux. 2. Işık answers Q-019. 3. PB-012 replaces the one-tick-per-frame loop with the fixed-timestep accumulator.
- **Traps:** `RaylibWindow::shouldClose()` is also true after Escape (raylib's default exit key). `Application` holds references to the simulation and the renderer; both must outlive it. A headless run needs `maxTicks`, or `init` refuses it. CTest ignores the exit code when `PASS_REGULAR_EXPRESSION` is set, so the sandbox test uses only `FAIL_REGULAR_EXPRESSION`. `engine/rendering-raylib` is Zeynep Ertürk's area: ask her to look at the window wrapper too.
- **Records:** DEC-046 (Proposed), Q-019, E-029, E-030.

---

### HND-019 — Fixed 1/60 s timestep loop (PB-012)
- **Date:** 2026-11-10
- **Agent:** claude-code (PB-012 session), working with Arda Karabulut
- **Task:** TASK-006 (ClickUp PB-012), on top of HND-018
- **Goal:** The simulation advances in fixed 1/60 s ticks whatever the render frame rate is.
- **Result:** Done, ready for review. `engine/core`: `tick.hpp` (`Tick`, `ticksPerSecond`, `fixedDeltaSeconds`, `simulatedSeconds()`), `FrameClock` (PB-012.1), `FixedTimestep` (PB-012.2), the tick counter in `Application` and `ISimulation::runTick(Tick)` (PB-012.3), unit tests (PB-012.4). The loop rules follow srs.fr.1.2 to 1.6 and are recorded as DEC-047 (`Proposed`, Q-020).
- **Files changed:** `engine/core/{CMakeLists.txt,include/engine/core/{application,simulation,tick,frame_clock,fixed_timestep}.hpp,src/{application,frame_clock,fixed_timestep}.cpp}`, `sandbox/src/main.cpp`, `tests/{CMakeLists.txt,unit/core/{application_test,fixed_timestep_test,frame_clock_test}.cpp,unit/core/even_frame_times.hpp}`, `docs/architecture/engine-layer.md`, `docs/troubleshooting/{environment-setup,common-errors}.md`, `docs/testing/test-checklists.md`, `docs/history/decision-log.md`, `docs/agents/state/{task-board,current-status,open-questions,handoff-log}.md`.
- **Verified (Windows 11, MSVC 19.44, CMake 4.2, `ENGINE_WARNINGS_AS_ERRORS=ON`):** `windows-debug` builds with no warnings, `ctest` 49/49; `headless` preset 41/41; clang-format 19.1.7 clean. `sandbox.exe --max-ticks 300 --fps N` for N = 30, 60, 144 and 0: 300 ticks (5.00 s simulated) took 5.00, 5.01, 5.00 and 5.00 s of real time in 151, 301, 702 and 15 396 render frames; exit code 0 and an empty standard error each time. Mutations: `unitsPerTick` set to 1 000 000 020 (a tick of 16 666 667 ns) failed 7 tests; incrementing the counter before `runTick` failed 7 tests.
- **Not verified:** macOS and Linux builds (CI after a push). The slow-simulation warning in a real window (covered by `FixedTimestep` tests; dragging the window should trigger it).
- **Repo state:** builds, tests pass; committed on `feat/pb-012-feat-implement-fixed-timestep-1-60-s` (issue #36), which merges the PB-011 branch first, so its PR's base is the PB-011 branch and the diff shows only PB-012 (Işık asked for the PR before PB-011 is merged).
- **Next:** 1. Merge PB-011 (#35) first. Then switch this PR's base to `main`; Işık resolves the conflicts that the squash merge of #35 causes, taking the PB-012 side in files both tasks touch. 2. Işık answers Q-020. 3. Later tasks: `RunError` in the loop (srs.fr.1.11), pause and step (srs.fr.1.16 to 1.18), headless run settings (srs.fr.7.x), a logger for the warning.
- **Traps:** 1/60 s is not a whole number of nanoseconds, so tests use frame times such as 50 ms (exactly 3 ticks) or `evenFrameTimes()`. The first render frame of a run always runs 0 ticks. `FrameClock` is read once per render frame by `Application`; a second reading in the same frame would steal time from the accumulator, so the sandbox keeps its own `FrameClock` for the read-out. Ticks beyond 5 per frame are dropped, so a debugger break or a dragged window loses simulated time by design.
- **Records:** DEC-047 (Proposed), Q-020, E-031; E-007 updated.

### HND-020 — Accept DEC-046 and DEC-047; the editor hosts the application (PB-122)
- **Date:** 2026-11-11
- **Agent:** claude (PB-122 thread), working with Işık Dönger
- **Task:** TASK-018 (ClickUp PB-122); closes TASK-006
- **Goal:** Record Işık's acceptance of the PB-011 and PB-012 decisions, and make the editor the application that opens the main window.
- **Result:** DEC-046 and DEC-047 `Accepted` (Q-019, Q-020 answered). DEC-048 records that the editor hosts `Application`, `sandbox` is a test host and games never host the engine. `editor/` now runs `Application` with `RaylibWindow`; it is no longer built in the `headless` preset. Rule S-6 no longer lists editor tooling as out of scope.
- **Verified (Linux, GCC 13.3, CMake 3.28, `ENGINE_WARNINGS_AS_ERRORS=ON`):** `headless` preset builds, `ctest` 41/41, no `editor` target; a Debug build with raylib builds with no warnings, `ctest` 49/49; `editor --max-ticks 120` under Xvfb opened a 1280 x 720 window, closed it and exited with code 0, nothing on stderr; `editor --bogus` exits with code 2. clang-format 19.1.7 clean.
- **Not verified:** Windows and macOS (CI after a push); closing the window by hand.
- **Repo state:** builds, tests pass; committed on `feat/pb-122-make-the-editor-host-the-application-and` (issue #38).
- **Next:** PB-110 (editor shell with dockable panels) builds on this window. The Issue start workflow failed to open the draft PR ("GitHub Actions is not permitted to create or approve pull requests"): the repository setting under Settings > Actions > General needs turning on.
- **Records:** DEC-046, DEC-047 (accepted), DEC-048, Q-019, Q-020.

---

---

### HND-021 — Physics adapter interface (PB-025)
- **Date:** 2026-11-10
- **Agent:** claude (PB-025 session), working with Zeynep Ertürk
- **Task:** TASK-019 (ClickUp PB-025)
- **Result:** Ready for review. `engine/physics/` (INTERFACE target `engine::physics`, links only `engine::core`): `body_desc.hpp`, `physics_types.hpp`, `physics_adapter.hpp`. `engine/core`: `vec2.hpp`, `entity_id.hpp` (copied from `ecs-api-design.md` §4 so the ECS reuses it). `tools/check_includes.py` added to the CI `format` job. `physics-adapter-design.md` §2 rewritten to the real interface.
- **Decided by Zeynep:** name `IPhysicsAdapter` (not the ClickUp `IPhysicsWorld`); bodies addressed by `BodyId` as the design doc says, plus `findBody(EntityId)`; `Vec2` and `EntityId` live in `engine/core`.
- **Verified (Linux, GCC 13, `ENGINE_WARNINGS_AS_ERRORS=ON`, raylib on):** builds with no warnings; `ctest` 14/14; clang-format 19.1.7 clean; `check_includes.py` reports 0, and reports 1 when a Box2D include is added to `body_desc.hpp`.
- **Not verified:** Windows and macOS builds (CI covers them); the headless preset.
- **Next:** PB-026 implements `Box2DPhysicsAdapter` in `engine/physics-box2d/` (also needs PB-012, the fixed-timestep loop).
- **Traps:** `BodyId` is 64-bit on purpose so the Box2D implementation can pack a `b2BodyId` into it. Collision events and raycasts return `EntityId`, not `BodyId`.
- **Records:** DEC-049 (Accepted by Işık Dönger on 2026-11-11 during review of PR #33).

---

### HND-022 — Render frame phase order (US-1.1)
- **Date:** 2026-11-12
- **Agent:** claude (US-1.1 thread), working with Işık Dönger
- **Task:** US-1.1 (#41), srs.fr.1.1 to 1.6
- **Result:** srs.fr.1.2 to 1.6 were already met by PB-011/PB-012. The gap was srs.fr.1.1: input was polled at the end of the previous frame inside raylib's `EndDrawing()`. `IWindow` now has `collectInput()`, `beginFrame()`, `presentFrame()` (was `endFrame()`); `Application` calls collect input, ticks, render, present. raylib is built with `SUPPORT_CUSTOM_FRAME_CONTROL=1`, so `RaylibWindow` polls, swaps and caps the frame rate itself. The PB-008 spike was adapted the same way.
- **Verified (Windows 11, MSVC, `windows-debug` preset):** builds with no new warnings; `ctest` 53/53; `editor --max-ticks 120` opened the window, ran 120 ticks in about 2 s (frame cap works) and exited with code 0; `box2d_raylib_spike --frames 120` exited with code 0. clang-format 19.1.7 clean.
- **Not verified:** macOS and headless Linux (CI after push); typing in the window, since nothing reads input yet.
- **Next:** US-1.3 (tick phases, srs.fr.1.12 to 1.15) and US-1.4 (pause).
- **Traps:** with custom frame control raylib's `GetFrameTime()` and `GetFPS()` stay 0; measure time with `FrameClock` or `GetTime()`. Raylib code outside `RaylibWindow` must call `PollInputEvents()` and `SwapScreenBuffer()` itself.
- **Records:** DEC-051 (Accepted by Işık Dönger on 2026-11-12).
