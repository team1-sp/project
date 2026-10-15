# Environment Setup

**Status: skeleton.** The stack is decided — C++, Box2D v3.1.1, Raylib, custom ECS, 2D only (DEC-013…016, 018, 020). Platforms (DEC-029): development on Windows and macOS, Windows priority; games built for Windows only. Compilers: whatever each platform needs (DEC-030). Both the engine application and the dev tooling must run on macOS (DEC-029). C++20 (DEC-019), Raylib v6.0 (DEC-021) and CMake + GoogleTest (DEC-023) are decided. Exact commands arrive with the skeleton (TASK-005 / PB-002). TASK-007 fills this in after TASK-005. Until then, do not invent exact commands.

## 1. Goal

A new team member (or agent with shell access) can build, run and test from a clean machine using **only this file**.

## 2. Known facts `[VERIFIED 2026-10-12]`

- Development on Windows and macOS, Windows priority; games target Windows only (DEC-029). Known paths are Windows (paths like `D:\Projects\CTIS\senior-project`, `E:\StaringLight\Documents\School\CTIS\Senior Project`). `[ASSUMPTION]` other members may use other OSes — ask.
- The repo `project/` has `.gitignore` and `README.md` only.
- Course tooling: ClickUp (WBS, Gantt), Moodle (course materials).
- Optional physics visualisation (learning aid, not part of the engine): Python on PATH, then `pip install "box2d-python[testbed]"`, `pip install glfw pillow` (from the team guide; Python Box2D may be a different version from the one the engine uses).

## 3. To fill in once decisions are `Accepted`

| Section | Content | Depends on |
| --- | --- | --- |
| Prerequisites | A compiler per OS that supports C++20 (DEC-019, DEC-030; record here when the build is set up), IDE (optional), git | Q-007 |
| Dependencies | Box2D v3.1.1, Raylib v6.0, GoogleTest — pinned with CMake FetchContent | DEC-020, DEC-021, DEC-023 |
| Build | Exact commands (CMake presets: configure, build, test, clean), Windows and macOS | DEC-023, DEC-029 |
| Run | Normal mode and headless mode commands/flags | TASK-006 |
| Test | Command to run all tests; to run one suite | TASK-005 |
| Troubleshooting | Platform-specific problems and fixes (link `PRB-###`) | experience |

## 4. Rules

- Commands must be copy-pasteable and tool-neutral (no "click the Run button").
- Pin versions; record the date verified.
- When a setup problem is solved, add it to `history/problem-log.md` (area `Build`) **and** link it here.
- Never commit machine-specific paths into build files; use relative paths or configuration.

## 5. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Skeleton | claude-code/sonnet-5.5/012r |
| 2026-10-13 | Stack decided; open items re-pointed to Q-007/008/009 | claude-code/sonnet-5.5/012r |
| 2026-10-13 | Box2D version recorded; C++20/platforms proposed only | claude-code/sonnet-5.5/012r |
| 2026-10-15 | C++20, Raylib v6.0, platforms and compilers decided | claude-code/sonnet-5.5/012r |
