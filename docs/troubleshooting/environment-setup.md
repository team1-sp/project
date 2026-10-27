# Environment Setup

**Status: first version (PB-002, 2026-10-24).** Commands match the presets in [`CMakePresets.json`](../../CMakePresets.json). Stack: C++20 (DEC-019), Box2D v3.1.1 (DEC-020), raylib 6.2 (DEC-033), CMake + GoogleTest (DEC-023). Platforms (DEC-029): develop on Windows and macOS, Windows first; games are built for Windows only. Compilers: whatever each platform needs (DEC-030), recorded in section 3.

`[VERIFIED 2026-10-24]` only on Linux (GCC 13, CMake 3.28) by the agent that wrote the skeleton: the `headless` workflow and a full build with raylib, and the window app (now `sandbox`) opened and closed a window under Xvfb. `[VERIFIED 2026-10-24]` on Windows by Işık Dönger: `cmake --workflow --preset windows-debug` with Visual Studio 2026 (MSVC 19.51, CMake 4.4.3), and the window app (now `sandbox.exe`) opened its window. macOS is checked by the CI `build` job and by the Mac users (PB-002.6); until then treat the macOS steps as `[NOT YET VERIFIED]`.

## 1. Goal

A new team member (or agent with shell access) can build, run and test from a clean machine using **only this file**.

## 2. Prerequisites

| Tool | Windows | macOS |
| --- | --- | --- |
| C++20 compiler | Visual Studio 2022 (17.x) or newer with the **Desktop development with C++** workload (MSVC). It also installs CMake | Xcode Command Line Tools (Apple Clang): `xcode-select --install` |
| CMake 3.25 or newer | Comes with Visual Studio; or `winget install Kitware.CMake` | `brew install cmake` |
| git | `winget install Git.Git` | Comes with the Command Line Tools |
| clang-format 19 (DEC-031) | `pip install clang-format==19.1.7` | `pip install clang-format==19.1.7` |

Optional physics visualisation (learning aid, not part of the engine; may be a different Box2D version): `pip install "box2d-python[testbed]" glfw pillow`.

Raylib, Box2D and GoogleTest are not installed by hand: CMake downloads them at the pinned versions on the first configure (`cmake/Dependencies.cmake`), so the first configure needs internet access and takes a few minutes.

## 3. Compilers in use (DEC-030)

| OS | Compiler | Status |
| --- | --- | --- |
| Windows | MSVC from Visual Studio 2022 or newer (CMake picks the newest Visual Studio installed) | `[VERIFIED 2026-10-24]` Visual Studio 2026 (generator `Visual Studio 18 2026`), MSVC 19.51, Windows SDK 10.0.26100 |
| macOS | Apple Clang from the Xcode Command Line Tools | `[ASSUMPTION — confirm in PB-002.6]` |

## 4. Build and test

Run from the repository root. One command configures, builds and runs the tests:

```
cmake --workflow --preset windows-debug     # Windows
cmake --workflow --preset macos-debug       # macOS
cmake --workflow --preset headless          # any OS, without raylib
```

The steps separately (replace the preset name):

```
cmake --preset windows-debug                # configure into build/windows-debug
cmake --build --preset windows-debug        # build
ctest --preset windows-debug                # run all tests
ctest --preset windows-debug -R Box2d       # run tests whose name matches
```

| Preset | What it builds |
| --- | --- |
| `windows-debug`, `windows-release` | Everything, Visual Studio generator (Windows only) |
| `macos-debug`, `macos-release` | Everything, Unix Makefiles (macOS only) |
| `headless` | No raylib: engine libraries, `editor`, `engine_tests`. No `sandbox` (R-4, R-6) |

Clean: delete `build/<preset>/` (or all of `build/`). Opening the folder in Visual Studio, CLion or VS Code with the CMake Tools extension picks up the same presets.

## 5. Run

| Program | Windows | macOS |
| --- | --- | --- |
| Sandbox (opens a window; Escape closes it) | `build\windows-debug\sandbox\Debug\sandbox.exe` | `build/macos-debug/sandbox/sandbox` |
| Sandbox, closes itself after N frames | add `--frames 120` | add `--frames 120` |
| Editor (placeholder, prints its version) | `build\windows-debug\editor\Debug\editor.exe` | `build/macos-debug/editor/editor` |

Normal and headless run modes of the engine come with TASK-006.

## 6. Troubleshooting

| Problem | Fix |
| --- | --- |
| `No such preset` on configure | The Windows presets only exist on Windows and the macOS presets only on macOS. Use `cmake --list-presets` |
| First configure seems stuck after `Box2D on MSVC` | It is downloading raylib (a large repository); the first configure can take several minutes. Later configures reuse `build/<preset>/_deps` |
| First configure fails while cloning | It needs github.com. Retry; behind a proxy set `HTTPS_PROXY` |
| Linux (not a supported platform) | Needs the X11 and OpenGL dev packages for raylib; on a machine without Wayland libraries add `-DGLFW_BUILD_WAYLAND=OFF` |

Solved setup problems also go into `history/problem-log.md` (area `Build`) and get a line here.

## 7. Rules

- Commands must be copy-pasteable and tool-neutral (no "click the Run button").
- Pin versions; record the date verified.
- When a setup problem is solved, add it to `history/problem-log.md` (area `Build`) **and** link it here.
- Never commit machine-specific paths into build files; use relative paths or configuration.

## 8. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Skeleton | claude-code/sonnet-5.5/012r |
| 2026-10-13 | Stack decided; open items re-pointed to Q-007/008/009 | claude-code/sonnet-5.5/012r |
| 2026-10-13 | Box2D version recorded; C++20/platforms proposed only | claude-code/sonnet-5.5/012r |
| 2026-10-15 | C++20, Raylib v6.0, platforms and compilers decided | claude-code/sonnet-5.5/012r |
| 2026-10-24 | Exact prerequisites, build, test and run commands for the PB-002 skeleton | claude-code (PB-002 session) |
| 2026-10-24 | Windows build verified with Visual Studio 2026 / MSVC 19.51; first configure takes several minutes (raylib download) | claude-code (PB-002 session) |
