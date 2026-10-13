# Repository Structure

**Current reality `[VERIFIED 2026-10-12]`:** `project/` contains `README.md`, `.gitignore`, `docs/`. No source yet.

Related repos on the team's machines (separate git repos):

| Path | Purpose |
| --- | --- |
| `senior-project/docs/` | Course deliverables (Initial Plan, business model canvas). **Not authoritative (the decision log is); agents do not edit.** |
| `senior-project/shader-test/` | Graphics benchmark (OpenGL, Raylib, SFML) — evidence only; Raylib is decided (DEC-015) |
| `senior-project/project/` | **This repo:** engine and tools |

## Proposed layout `[PROPOSED — finalise in TASK-005 after the build system is decided (Q-009)]`

Folder names mirror the architecture so the structure teaches the design.

```
project/
├── README.md
├── docs/                      this documentation
├── src/
│   ├── engine/                LAYER 1 (no dependency on semantic/ or ai/)
│   │   ├── core/                game loop, time, scene, config, run modes
│   │   ├── ecs/                 world, entity, component storage, query, system scheduler
│   │   ├── components/          pure-data components
│   │   ├── systems/             input, movement, collision, gameplay, telemetry, render
│   │   ├── physics/             IPhysicsAdapter + Box2D implementation (the ONLY place Box2D types appear)
│   │   ├── rendering/           rendering abstraction + Raylib implementation (the ONLY place Raylib types appear)
│   │   ├── input/               action interface (keyboard + agent)
│   │   ├── telemetry/           recording + export
│   │   └── ui/                  engine/test-tool UI
│   ├── semantic/              LAYER 2 (depends on engine export formats only)
│   │   ├── data/  graph/  relationships/
│   ├── ai/                    LAYER 3
│   │   ├── reader/  llm/  assistant/
│   ├── playtest/              Automated Playtest Agent (uses engine via action interface)
│   └── demo-game/             demo game + injectable bug flags
├── tests/
│   ├── unit/  adapter/  integration/  scenarios/  benchmarks/
├── assets/                    sprites, shaders, fonts (small, license-clear)
├── tools/                     scripts (build helpers, telemetry viewers)
└── third_party/               pinned dependencies if vendored (Raylib v6.0 proposed — DEC-021; Box2D v3.1.1 — DEC-020)
```

Dependency check (enforce in review, later by a script): files under `src/engine/` must not reference `src/semantic/` or `src/ai/`.

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
