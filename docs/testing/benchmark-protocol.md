# Benchmark Protocol — Graphics Framework / Shader

**Status: historical.** Raylib was chosen on 2026-10-13 (DEC-015), superseding DEC-012, so this benchmark no longer decides anything. **Outcome (reported by Işık Dönger, 2026-10-13):** the benchmark finished; no notable difference between the frameworks was seen, as expected; the team therefore decided on code complexity and ease of use. The results table below was not filled in (numbers not provided); fill it only if the team wants the evidence. Summary of `senior-project/shader-test/test-guideline.md` (the spec for the benchmark scene; if this file and the spec differ, the spec wins for the *benchmark*). Purpose was to compare OpenGL, Raylib and SFML on visual fidelity, custom shaders and pipelines before choosing. Task: TASK-001.

## 1. Scene (identical across implementations)

| Item | Value |
| --- | --- |
| Window | 1920×1080, not resizable, VSync **off**, title `<Library> - FPS: <n>` updated once per second |
| Background | pure green (0,255,0) |
| Square | 300×300 px, centred (480,540), horizontal gradient green→red, **no shader** |
| Sprite | `joint-files/sample-sprite.png` (820×1054 RGBA), 300 px tall, centred (1440,540), rotating **clockwise 45°/s** about its centre, linear filter + mipmaps, **clamp-to-edge** wrap, alpha blending (SRC_ALPHA, ONE_MINUS_SRC_ALPHA) |
| Shader | `joint-files/sample-shader.frag` applied to the **sprite only**; uniforms `time` (seconds since start), `texture0`; inputs `fragTexCoord`, `fragColor` = (1,1,1,1) |
| UI/Text | `joint-files/ARLRDBD.TTF`, 48 px, white, two centred lines; top edge 20 px from top; gap 10 px; line 1 local time `HH:MM:SS`; line 2 library name exactly `OpenGL`, `Raylib` or `SFML` |

Origin top-left, pixel units. Assets are loaded at runtime from `../joint-files/` (relative path); do not copy or modify them.

## 2. Assignments (from the spec)

| Library | Owner | Focus |
| --- | --- | --- |
| Raw OpenGL (GLFW + GLAD/GLEW) | Işık Dönger | Total control vs boilerplate; shaders, VAO/VBO, `stb_image` by hand |
| Raylib | Arda Sarıtaş | Built-in abstraction (`LoadShader`, `BeginShaderMode`); optionally a 3D cube |
| SFML | Elifnur Özçelik | `sf::Sprite` + `sf::Shader`; uniform handling |

## 3. Evaluation criteria

| Metric | Question |
| --- | --- |
| Visual fidelity | Shader looks as intended? Artefacts/tearing? |
| Shader integration | How easy: load GLSL, pass `time`, apply to object? |
| Code complexity | Lines of code to reach textured object + custom shader |
| Performance (FPS) | FPS with the unoptimised fragment shader every frame |
| Engine fit | Headroom for ECS + AI microservices vs manual management; headless friendliness; telemetry |

Add for engine decision: **headless support** (can the simulation run with no window? — see `testing/headless-testing.md`), and licence/maintenance.

## 4. Results table (fill in; keep raw numbers)

| Metric | OpenGL | Raylib | SFML |
| --- | --- | --- | --- |
| Visual fidelity | | | |
| Shader integration | | | |
| LOC (scene + shader) | | | |
| FPS (avg, hardware: ___) | | | |
| Engine fit / headless | | | |
| Notes / problems (link PRB-###) | | | |

## 5. Known gotcha

Raylib defaults to texture wrap *repeat*; the shader samples slightly outside 0–1, so set clamp-to-edge — PRB-001.

## 6. How agents may help

If asked: review an implementation against §1, measure LOC, fill the table from the owners' numbers. Do not edit owners' code without asking. The Raylib scene is also a handy smoke test for the rendering layer's shader/texture path (wrap mode: PRB-001).
