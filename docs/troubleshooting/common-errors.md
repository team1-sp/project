# Common Errors — Anticipated Pitfalls

These are **anticipated** (well-known in engine/ECS/physics work) and **not yet encountered** in this project, unless a `PRB-###` link says so. When one actually occurs, create a `PRB-###` in `history/problem-log.md` with the real error text and link it here. Search `problem-log.md` for *actual* history.

| ID | Pitfall | Symptom | Prevention / fix | Seen? |
| --- | --- | --- | --- | --- |
| E-001 | **Entity destroyed but physics body not** | Invisible walls, ghost collisions, memory growth | Adapter destroys body on entity destruction; assertion "no body without live entity" each step | no |
| E-002 | **Stale entity↔body mapping / ID reuse** | Collision events name the wrong entity | Use generation-counted entity IDs; remove mapping on DestroyBody; test it | no |
| E-003 | **Structural change during iteration** (add/remove component or destroy entity inside a query loop) | Crash, skipped entities, undefined behaviour | Defer changes via a command buffer applied after the system; or collect IDs first | no |
| E-004 | **Coordinate mismatch: screen y-down vs physics y-up** | Sprite upside down, gravity "up", rotations inverted | One world convention; convert only at the render boundary; unit test with a known position | no |
| E-005 | **Unit mismatch (pixels vs meters)** | Objects too slow/fast/floaty; library ignores tiny bodies | Define pixels-per-meter once (TASK-008); never mix raw pixels into the adapter | no |
| E-006 | **Variable timestep in physics** | Non-deterministic runs, tunnelling, jitter | Fixed 1/60 s accumulator (R-5); cap steps per frame (spiral of death) | no |
| E-007 | **Spiral of death** | Frame time grows → more steps per frame → slower | Clamp frame delta (e.g. ≤ 0.25 s) and max steps per frame | no |
| E-008 | **Tunnelling** (fast body passes through thin wall) | Player escapes map | Thicker walls, smaller step, continuous collision, clamp max speed | no |
| E-009 | **Reading render state in simulation** | Headless ≠ normal | Simulation reads ECS only (R-6); equivalence test | no |
| E-010 | **Wall-clock time in simulation** | Different runs differ; headless too slow/fast | Use fixed `dt` only; inject time source | no |
| E-011 | **Unordered iteration affects results** (hash maps, parallel loops) | Heisenbugs, flaky tests | Iterate in ascending entity ID; sort events | no |
| E-012 | **Gameplay logic inside physics callbacks** | Door logic duplicated/hidden, AI can't diagnose | Callbacks only record raw collisions; gameplay systems decide (R-7) | no |
| E-013 | **Events cleared before consumers read them** | "No collision event" despite collision | Define event lifetime (per frame, cleared at frame start); consumers run after producers | no |
| E-014 | **Library types leaking out of adapter** | Can't swap library; compile errors across modules | Only adapter includes library headers (R-3); review check | no |
| E-015 | ~~GC pauses in hot paths~~ | — | **obsolete:** language is C++ (DEC-013). Equivalent C++ risk: per-frame heap allocation → see E-019 | n/a |
| E-016 | **Dangling pointers/references to components** | Crash after add/remove | Store entity IDs, not pointers; re-query | no |
| E-017 | **Texture wrap mode differences between frameworks** | Edge artefacts with shaders | Set clamp-to-edge explicitly — **PRB-001** | **yes** |
| E-018 | **Library version mismatch** (Box2D v2.4 tutorials vs our v3.1.1 C API; Raylib version differences) | Missing/changed functions, doesn't compile | Versions pinned: Box2D v3.1.1 (DEC-020), Raylib 6.2 (DEC-033); copy examples only for the pinned version; compile a smoke test early | no |
| E-019 | **Telemetry too expensive** | FPS/step rate misses ip.nfr.1.* | Record deltas, pre-allocate, async/ring buffer; measure with telemetry off/on | no |
| E-020 | **Window created in tests** | CI/headless crash, slow tests | Tests use headless entry point only | no |
| E-023 | **Raylib called in headless mode or from simulation code** (`InitWindow`, `LoadTexture`, `LoadShader`, `LoadFont` need a window/GL context) | Crash or hang without a display; headless ≠ normal | Only the rendering implementation calls Raylib; simulation uses asset IDs; headless never initialises it (R-4) | no |
| E-024 | **Raylib resources unloaded after `CloseWindow`** (or never) | Crash on exit, leaks | RAII wrappers; unload before closing the window | no |
| E-025 | **Mixing Box2D (y-up, meters) with Raylib (y-down, pixels)** | See E-004/E-005 | One world convention; convert at the render boundary | no |
| E-021 | **LLM answers without evidence** | Confident wrong diagnosis | Require citations; abstain when evidence missing (ip.fr.11.5) | no |
| E-022 | **Sending code/telemetry to an external AI by default** | Privacy violation (ip.nfr.2.1) | External calls off by default; explicit config | no |

## Adding an entry

Use the next `E-###`, keep it to one table row, link `PRB-###` when seen. Don't delete rows; mark obsolete ones `obsolete`.
