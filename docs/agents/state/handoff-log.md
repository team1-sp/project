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

