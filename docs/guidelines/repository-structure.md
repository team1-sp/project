# Repository Structure

**Current reality `[VERIFIED 2026-11-22]`:** `project/` contains `README.md`, `CONTRIBUTING.md`, `.github/`, `tools/`, `docs/`, repo config files and the build skeleton from PB-002. PB-006 adds the `ai-services/` Python 3.12 package skeleton. PB-025 adds `engine/physics/` and `tools/check_includes.py`. US-11.2 adds the `assistant/` core and production AgentBridge proto/optional gRPC target. US-11.1 adds the grounded-answer pipeline under `assistant/{include/aegis/assistant,src}/grounded/`. US-24.1 adds the canonical structured log contract in `engine/core` and the window-free Console model, platform launch and entity-navigation seams in `editor_core`. US-33.5 adds the window-free AI/privacy settings controller and per-user project settings store in `editor_core`, plus health/model discovery on AgentBridge. US-33.2 adds the window-free AI Assistant chat state to `editor_core`. The other folders below are created as their PRs land.

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
│   └── bridge/               transport-neutral AgentBridge; optional production gRPC client
├── assistant/                LAYER 3 window-free C++ client core: read-only orchestration + chat storage
├── editor/                   C++ editor app; hosts engine::Application and owns the main window (DEC-048)
├── sandbox/                  developer and CI test host on engine::Application; not the product, not the demo game
├── games/<name>/             demo game, levels, bug-injected variants — `[UNDECIDED]`: the team has not chosen the game (Q-016)
├── tests/                    GoogleTest, mirrors engine/ (unit/, integration/, scenarios/, benchmarks/)
├── proto/                    production AgentBridge gRPC/protobuf contract
├── semantic/                 LAYER 2, own package and pipeline (DEC-026); language open, Python assumed
├── ai-services/              LAYER 3, Python 3.12 src-layout package (DEC-039); uses semantic/ when available
├── assets/                   small, license-recorded assets
├── tools/                    repo scripts (PR checks, link check, label sync, include check `check_includes.py`)
├── .githooks/                git hooks: record-id check before commits and merge commits
├── docs/                     this documentation
└── .github/                  workflows, CODEOWNERS, issue forms, PR template, labels
```

CMake targets: each engine folder builds `engine_<folder>` (alias `engine::<folder>`, `-` becomes `_`) with public headers under `include/engine/<folder>/`; executables are `editor`, `sandbox` and `engine_tests`. Options: `ENGINE_WITH_RAYLIB` (off in `headless`: no raylib fetch, no `rendering-raylib`, no `editor`, no `sandbox`), `ENGINE_BUILD_TESTS`, `ENGINE_WARNINGS_AS_ERRORS` (on in CI). Build output goes to `build/<preset>/` (git-ignored).

`assistant/` builds `aegis_assistant` (`aegis::assistant`) and depends only on the transport-neutral
`engine::bridge` seam and the value types of `engine::core` (`EntityId`, `Tick`, `Vec2`; DEC-070).
`AEGIS_WITH_GRPC_BRIDGE` adds `engine::bridge_grpc`, generated from
`proto/agent_bridge.proto`; it is off in the default build so gRPC/protobuf remain optional
(DEC-038, DEC-067).

Dependency checks: only `physics-box2d` includes Box2D and only `rendering-raylib` includes raylib (compile errors elsewhere, and `tools/check_includes.py` in CI since PB-025); `engine/` never depends on `semantic/`, `ai-services/` or `editor/` (CMake targets + a CI script); the `headless` preset builds without raylib (R-6).

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
| 2026-10-27 | `games/demo-maze/` removed (the team never chose it as the demo); its smoke window moved to `sandbox/`; demo game undecided (Q-016) | claude-code |
| 2026-11-04 | Added the Python 3.12 `ai-services/` package skeleton (PB-006, DEC-039) | codex/gpt-5/pb006 |
| 2026-11-10 | `engine/physics/` and `tools/check_includes.py` exist (PB-025) | claude (PB-025 session) |
| 2026-11-22 | Added the US-11.2 `assistant/` core and production AgentBridge proto/optional gRPC target (DEC-067) | codex/gpt-5/us112 |
| 2026-11-22 | Added the US-11.1 grounded-answer pipeline in `assistant/.../grounded/` (DEC-070) | claude-code/opus-5.5/us111 |
| 2026-11-22 | Structured logging lives in `engine/core`; the Console model and navigation/launcher seams live in `editor_core` (US-24.1, DEC-068) | codex/gpt-5/104-console |
| 2026-11-22 | AI/privacy settings state and user-local persistence live in `editor_core`; health/model discovery stays behind AgentBridge (US-33.5, DEC-069) | codex/gpt-5/121-ai-privacy |
| 2026-11-22 | The AI Assistant chat state lives in `editor_core`, which links `aegis::assistant` (US-33.2, DEC-071) | claude-code/opus-5.5/119-chat |
