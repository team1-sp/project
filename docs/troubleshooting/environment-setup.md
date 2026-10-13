# Environment Setup

**Status: skeleton.** The stack is decided — C++, Box2D, Raylib, 2D only (DEC-013…016) — with Box2D v3.1.1 (DEC-020) and a custom ECS (DEC-018). Not decided: C++ standard (C++20 proposed), platforms (dev on Windows+macOS, Windows-only target proposed), Raylib version (v6.0 proposed) — Q-007; build system and test framework (human: later, Q-009); compilers/toolchain. TASK-007 fills this in after TASK-005. Until then, do not invent exact commands.

## 1. Goal

A new team member (or agent with shell access) can build, run and test from a clean machine using **only this file**.

## 2. Known facts `[VERIFIED 2026-10-12]`

- `[PROPOSED]` Development on Windows and macOS; engine app and games target Windows only for now (DEC-019, not decided). Known paths are Windows (paths like `D:\Projects\CTIS\senior-project`, `E:\StaringLight\Documents\School\CTIS\Senior Project`). `[ASSUMPTION]` other members may use other OSes — ask.
- The repo `project/` has `.gitignore` and `README.md` only.
- Course tooling: ClickUp (WBS, Gantt), Moodle (course materials).
- Optional physics visualisation (learning aid, not part of the engine): Python on PATH, then `pip install "box2d-python[testbed]"`, `pip install glfw pillow` (from the team guide; Python Box2D may be a different version from the one the engine uses).

## 3. To fill in once decisions are `Accepted`

| Section | Content | Depends on |
| --- | --- | --- |
| Prerequisites | C++ compiler(s) and versions per OS (C++20 proposed), IDE (optional), git | Q-007 |
| Dependencies | Box2D v3.1.1, Raylib (v6.0 proposed), test framework (pinned) | DEC-020, DEC-021, Q-009 |
| Build | Exact commands (configure, build, clean) | Q-009 |
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
