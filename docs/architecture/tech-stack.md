# Technology Stack

**Status: core stack DECIDED (2026-10-13).** Source of truth: `history/decision-log.md`. Agents MUST NOT change or work around these; if you think one is wrong, raise it with the human (`AGENT-PROTOCOL.md`).

## 1. Decided

| Area | Decision | Entry |
| --- | --- | --- |
| Language | **C++** | DEC-013 |
| Physics library | **Box2D** — only behind the Physics Adapter | DEC-014, DEC-003 |
| Graphics framework | **Raylib** — only behind the rendering abstraction | DEC-015, DEC-007 |
| ECS | **Custom-built** (so components can be LLM-instructive through the semantic layer) | DEC-018 |
| Box2D version | **v3.1.1** (C API with id handles) | DEC-020 |
| Scope | **2D only.** 3D is future work; keep the seams (adapter, rendering abstraction, ECS) so it stays possible, build nothing 3D | DEC-016 |

## 2. Still open (ask the human; do not decide)

| Area | Candidates / notes | Question |
| --- | --- | --- |
| Build system | Not chosen — human: "to be decided later" (must work for dev on Windows and macOS) | Q-009 |
| Test framework | Not chosen — same, later | Q-009 |
| C++ standard / platforms | `[PROPOSED]` C++20; development on Windows and macOS; engine app and games target Windows only for now — human: "still open" | DEC-019, Q-007 |
| Compilers / toolchain | Not given | Q-007 |
| Does the engine have to build and run on macOS, or is macOS dev-only (editing/docs)? | Not given; affects platform-specific code and build system | Q-007 |
| Raylib version | `[PROPOSED]` v6.0 (DEC-021) — human wrote "still open"; confirm | Q-007 |
| What "LLM-instructive components" means concretely (metadata schema) | Not decided | Q-013 |
| AI service language / protocol | `[PROPOSED]` Python + gRPC/protobuf (DEC-011) | open |
| World units / coordinate convention | Meters vs pixels; y-up vs y-down | TASK-008 |

## 3. What the decisions mean in practice

- **C++:** entities are IDs, never raw pointers (lifetime safety); RAII for every resource; no allocations in per-frame hot paths. See `guidelines/coding-standards.md`.
- **Box2D v3.1.1:** a C API with id handles (not v2.4 objects/pointers); tutorials for v2.4 do not apply. Fits a 2D engine: static / kinematic / dynamic bodies, box/circle (and polygon) shapes, raycast, joints, restitution/friction, independent of rendering. 2D-only: a future 3D would need another library behind the same adapter, which is why DEC-003 stays.
- **Raylib:** simple window/input/drawing/shader API. It needs a window and an OpenGL context to load textures, shaders and fonts — so **headless mode must never initialise Raylib** and simulation uses asset IDs instead of loaded resources (`testing/headless-testing.md`). Default texture wrap is *repeat*; see PRB-001.
- **Custom ECS:** we own component definitions, so each component can carry structured semantics (what it means, units, who writes it) for the semantic layer and the LLM. See `design/ecs-design.md`.
- **2D only:** no 3D experiments, models, cameras or 3D physics (rule S-6).

## 4. Considered and not chosen (history)

Recorded so nobody re-opens them without a reason (summary from guide §6; full reasoning in the decision entries):

| Area | Not chosen | Why (as far as recorded) |
| --- | --- | --- |
| Language | C# (.NET) | Team chose C++ (DEC-013). Rationale beyond the decision not given |
| Physics | BEPUphysics | 3D-oriented; 2D-only scope (DEC-014, DEC-016) |
| Graphics | OpenGL (raw), SFML, MonoGame | Raylib chosen (DEC-015); benchmark compared OpenGL/Raylib/SFML |

## 5. Selection criteria used (guide §10)

Learning time · setup ease · team knowledge · 2D MVP suitability · physics integration · rendering integration · headless support · telemetry ease · semantic-graph data extraction · 3D potential · documentation quality · team experience. Use the same list for any future choice.

## 6. Principle that holds regardless

The engine is not directly coupled to Box2D or Raylib. The Physics Adapter and the rendering abstraction exist so libraries can be swapped (R-3, R-4).

## 7. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Created from guide §6 and Initial Plan | claude-code/sonnet-5.5/012r |
| 2026-10-13 | Rewritten after the human decided the stack (DEC-013…016) | claude-code/sonnet-5.5/012r |
| 2026-10-13 | Custom ECS, Box2D v3.1.1 recorded (DEC-018, DEC-020); C++20/platforms and Raylib v6.0 proposed only (DEC-019, DEC-021) | claude-code/sonnet-5.5/012r |
