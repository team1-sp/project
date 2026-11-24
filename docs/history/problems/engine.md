# Problems: Engine

**Append-only.** Scope: ECS, physics and the Physics Adapter, rendering, input, gameplay, telemetry, headless runs. How to use this folder: [README.md](README.md).

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
