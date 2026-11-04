# Problem Log

**Append-only.** Every problem actually encountered, with root cause and solution, so that if it happens again we solve it in minutes. Template: [../agents/templates/problem-entry.md](../agents/templates/problem-entry.md).

## How to use

1. **Before debugging**, search this file (Ctrl-F / grep) for: the exact error text, the component name, the library name, the symptom.
2. **After solving**, append an entry at the bottom and add a row to the index.
3. If it happens again, add an `Occurrences` line to the existing entry — do not create a duplicate.
4. *Anticipated* pitfalls that have not yet happened live in [../troubleshooting/common-errors.md](../troubleshooting/common-errors.md). When one actually happens, create the `PRB-###` here and reference it from there.

Areas: ECS · Physics · Adapter · Rendering · Input · Gameplay · Telemetry · Headless · Semantic · AI · Build · Tooling · Docs · Process.

## Index

| ID | Title | Area | Status | Date |
| --- | --- | --- | --- | --- |
| PRB-001 | Raylib sprite shader shows edge artefacts: texture wrap defaults to repeat | Rendering | Solved (documented in spec) | 2026-10-12 (recorded) |
| PRB-002 | Ubuntu FlatBuffers package has no CMake config package | Build | Solved | 2026-11-04 |

---

### PRB-001 — Raylib sprite shader shows edge artefacts: texture wrap defaults to repeat
- **Date:** 2026-10-12 (recorded from the benchmark spec; originally found by the team)
- **Reported by:** team via `shader-test/test-guideline.md`; recorded by claude-code/sonnet-5.5/012r
- **Area:** Rendering
- **Status:** Solved
- **Symptom:** The shared fragment shader samples slightly outside the 0–1 texture-coordinate range. With Raylib's default texture wrap mode (repeat), the sprite's edge samples pixels from the opposite side, giving visible artefacts and making output differ from OpenGL/SFML.
- **Context:** Shader benchmark; sprite `sample-sprite.png` (820×1054 RGBA); shader `sample-shader.frag` (wave, pulsing glow, CRT scanlines).
- **Root cause:** Raylib defaults to repeat wrap; the other implementations were set to clamp-to-edge. `[VERIFIED by spec statement; not re-run by the recording agent]`
- **Solution:** Set texture wrap to clamp-to-edge on both axes (`CLAMP_TO_EDGE` / Raylib `TEXTURE_WRAP_CLAMP`) so all three implementations match.
- **How verified:** Side-by-side comparison with the OpenGL baseline (benchmark protocol).
- **Prevention:** The wrap mode is a mandatory line in the shared scene specification (`testing/benchmark-protocol.md`). Apply the same check to any new sprite/shader path in the engine's rendering layer.
- **Keywords:** raylib, texture wrap, repeat, CLAMP_TO_EDGE, sprite edge artefact, shader sampling outside 0-1, fragTexCoord
- **Related:** TASK-001, DEC-012
- **Occurrences:** —

---

### PRB-002 — Ubuntu FlatBuffers package has no CMake config package
- **Date:** 2026-11-04
- **Reported by:** GitHub Actions `bridge-benchmark` job for PB-065
- **Area:** Build
- **Status:** Solved
- **Symptom:** CMake stopped during the Ubuntu 24.04 bridge configuration with
  `Could not find a package configuration file provided by "flatbuffers"` and listed
  `flatbuffersConfig.cmake` and `flatbuffers-config.cmake` as missing.
- **Context:** `libflatbuffers-dev`, `flatbuffers-compiler` and `flatbuffers-compiler-dev` 2.0.8 were
  installed successfully from Ubuntu packages. The same CMake project configured successfully with
  Homebrew FlatBuffers 25.12.19 on macOS.
- **Root cause:** Ubuntu's FlatBuffers development package supplies the headers, library and `flatc`
  executable but does not supply the CMake config package required by
  `find_package(flatbuffers CONFIG REQUIRED)`.
- **Solution:** Prefer the FlatBuffers config package when available, then fall back to locating the
  distro headers, library and `flatc` executable and expose the same imported CMake targets used by
  the benchmark.
- **How verified:** Forced the fallback locally with
  `-DCMAKE_DISABLE_FIND_PACKAGE_flatbuffers=ON`; configuration and compilation succeeded and all
  four CTest targets passed. The Ubuntu GitHub Actions rerun is pending.
- **Prevention:** Optional benchmark dependencies must support both upstream/Homebrew CMake config
  packages and the packaging layout provided by the Ubuntu CI image.
- **Keywords:** FlatBuffers, flatbuffersConfig.cmake, Ubuntu, CMake, imported target, PB-065
- **Related:** PB-065, ADR-002, DEC-038
- **Occurrences:** GitHub Actions PR #23, Ubuntu 24.04

---
