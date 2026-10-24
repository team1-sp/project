# Repository Structure

**Current reality `[VERIFIED 2026-10-24]`:** `project/` contains `README.md`, `CONTRIBUTING.md`, `.github/`, `tools/`, `docs/`, repo config files and the build skeleton from PB-002: `CMakeLists.txt`, `CMakePresets.json`, `cmake/`, `engine/core/`, `engine/physics-box2d/`, `engine/rendering-raylib/`, `editor/`, `games/demo-maze/` and `tests/unit/`. The other folders below are created as their PRs land.

Related repos on the team's machines (separate git repos):

| Path | Purpose |
| --- | --- |
| `senior-project/docs/` (`team1-sp/docs`) | Course deliverables only (DEC-022). **Not authoritative (the decision log is); agents do not edit.** |
| `senior-project/shader-test/` | Graphics benchmark (OpenGL, Raylib, SFML) — evidence only; Raylib is decided (DEC-015) |
| `senior-project/project/` | **This repo:** engine and tools |

## Layout `[ACCEPTED — DEC-022, DEC-023, DEC-026; folders are created as the skeleton PRs land]`

One repository for all code and engineering docs (DEC-022); `team1-sp/docs` holds course deliverables only. Top-level folders split by toolchain; inside `engine/` each folder is its own CMake target, so the build enforces the layer rules.

```
project/
├── CMakeLists.txt            root build, C++20 (DEC-019), options (ENGINE_WITH_RAYLIB, ENGINE_BUILD_TESTS)
├── CMakePresets.json         windows-debug/-release, macos-debug/-release, headless (no raylib, any OS)
├── cmake/                    Dependencies.cmake: FetchContent pins (raylib 6.2 — DEC-033, Box2D v3.1.1 — DEC-020, GoogleTest v1.17.0 — DEC-023); CompilerWarnings.cmake
├── engine/                   LAYER 1, C++ static libraries, one CMake target per folder
│   ├── core/                 game loop, time, config, run modes, logging
│   ├── ecs/                  world, entities, storage, queries, scheduler, component metadata
│   ├── physics/              IPhysicsAdapter interface (no Box2D types)
│   ├── physics-box2d/        the ONLY target that links Box2D (PRIVATE)
│   ├── rendering/            renderer interface + null renderer
│   ├── rendering-raylib/     the ONLY target that links raylib (PRIVATE)
│   ├── input/                action layer, scripted replay, agent action source
│   ├── telemetry/            recording + export (what the semantic layer reads)
│   ├── headless/             headless runner, limits, exit codes
│   └── bridge/               engine server for the Python client
├── editor/                   C++ editor app
├── games/demo-maze/          demo game, levels, bug-injected variants
├── tests/                    GoogleTest, mirrors engine/ (unit/, integration/, scenarios/, benchmarks/)
├── proto/                    engine <-> Python contract
├── semantic/                 LAYER 2, own package and pipeline (DEC-026); language open, Python assumed
├── ai-services/              LAYER 3, Python package (RL, bug detection, assistant, narrative); uses semantic/
├── assets/                   small, license-recorded assets
├── tools/                    repo scripts (PR checks, link check, label sync, architecture check)
├── docs/                     this documentation
└── .github/                  workflows, CODEOWNERS, issue forms, PR template, labels
```

CMake targets: each engine folder builds `engine_<folder>` (alias `engine::<folder>`, `-` becomes `_`) with public headers under `include/engine/<folder>/`; executables are `editor`, `demo-maze` and `engine_tests`. Options: `ENGINE_WITH_RAYLIB` (off in `headless`: no raylib fetch, no `rendering-raylib`, no `demo-maze`), `ENGINE_BUILD_TESTS`, `ENGINE_WARNINGS_AS_ERRORS` (on in CI). Build output goes to `build/<preset>/` (git-ignored).

Dependency checks: only `physics-box2d` includes Box2D and only `rendering-raylib` includes raylib (compile errors elsewhere); `engine/` never depends on `semantic/`, `ai-services/` or `editor/` (CMake targets + a CI script); the `headless` preset builds without raylib (R-6).

## Rules

1. One concept per file; names follow `coding-standards.md`.
2. Tests mirror source folders.
3. Assets: only what is needed; record license/source in `assets/README.md`. Do not commit large binaries casually (check `.gitignore`).
4. Generated files and build output are git-ignored.
5. Moving or renaming a folder = a task + doc update (this file and any link).

## Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Proposed layout | claude-code/sonnet-5.5/012r |
| 2026-10-15 | Accepted layout: one repo, CMake targets per engine module, separate `semantic/` (DEC-022, DEC-023, DEC-026) | claude-code (session 0171) |
| 2026-10-24 | Build skeleton landed (PB-002): presets incl. macOS, target names, options; raylib 6.2 (DEC-033) | claude-code (PB-002 session) |
