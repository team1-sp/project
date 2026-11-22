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
| PRB-003 | Clean Python environment could not download pinned build dependencies in the sandbox | Tooling | Solved | 2026-11-04 |
| PRB-004 | Bridge benchmark did not build or run on Windows (vcpkg ZeroMQ name, gRPC C4996, pybind11 output folder, missing .exe lookup) | Build | Solved | 2026-11-04 |
| PRB-005 | Generated protobuf code failed AppleClang warnings-as-errors | Build / AgentBridge | Solved | 2026-11-22 |
| PRB-006 | Test fixture wrote a chat file into the build folder: store built from a path member before it was set | Tests / Assistant | Solved | 2026-11-22 |
| PRB-007 | Chip sizes missing or zero: empty question refused, then diagnosis text cancelled the item's bytes | Editor / Assistant | Solved | 2026-11-22 |

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
  four CTest targets passed. The Ubuntu `bridge-benchmark` job on PR #23 then passed.
- **Prevention:** Optional benchmark dependencies must support both upstream/Homebrew CMake config
  packages and the packaging layout provided by the Ubuntu CI image.
- **Keywords:** FlatBuffers, flatbuffersConfig.cmake, Ubuntu, CMake, imported target, PB-065
- **Related:** PB-065, ADR-002, DEC-038
- **Occurrences:** GitHub Actions PR #23, Ubuntu 24.04

---

### PRB-003 — Clean Python environment could not download pinned build dependencies in the sandbox
- **Date:** 2026-11-04
- **Reported by:** codex/gpt-5/pb006
- **Area:** Tooling
- **Status:** Solved
- **Symptom:** Installing `./ai-services[dev]` in a fresh Python 3.12 virtual environment failed with `ERROR: Could not find a version that satisfies the requirement setuptools==84.2.1 (from versions: none)` and `ERROR: No matching distribution found for setuptools==84.2.1`.
- **Context:** Local PB-006 verification inside the restricted command sandbox.
- **Root cause:** The sandbox could not resolve or connect to PyPI; pip's preceding retries reported `Failed to establish a new connection: [Errno 8] nodename nor servname provided, or not known`. The package version exists and was independently verified on PyPI.
- **Solution:** Re-ran the clean-environment install with approved network access and `set -e`, then ran Ruff, pytest and `pip check` successfully.
- **How verified:** Python 3.12.13 installed setuptools 84.2.1, Ruff 0.17.3 and pytest 9.2.0; Ruff passed, pytest passed 1/1 and `pip check` found no broken requirements.
- **Prevention:** Expect fresh dependency installs to need network approval in restricted sessions, and use `set -e` for multi-command verification so the first failure controls the exit status.
- **Keywords:** pip, PyPI, setuptools 84.2.1, no matching distribution, nodename nor servname, sandbox, network
- **Related:** TASK-017, PB-006, DEC-039
- **Occurrences:** 2026-11-22 — codex/gpt-5/104-console — CMake FetchContent for the fresh US-24.1 worktree failed with `fatal: unable to access 'https://github.com/erincatto/box2d.git/': Could not resolve host: github.com`; rerunning the headless workflow with approved network access succeeded. The full macOS configure later stalled while fetching; it completed using the same pinned sources already present in the sibling `project-us-11-2` build through `FETCHCONTENT_SOURCE_DIR_*` cache variables. 2026-11-22 — codex/gpt-5/121-ai-privacy — the fresh headless and macOS US-33.5 configurations hit the same DNS error; approved access completed headless, and the macOS build reused the pinned sibling sources through the same cache variables.
- **Occurrence:** 2026-11-22 — codex/gpt-5/118-assistant — the first windowed Editor
  build failed while fetching pinned ImGui with `fatal: unable to access
  'https://github.com/ocornut/imgui.git/': Could not resolve host: github.com` and `Failed to
  clone repository`; rerunning the same build with approved network access succeeded.
- **Occurrence:** 2026-11-22 — codex/gpt-5/118-assistant — configuring the isolated worktree while
  fixing the AI Settings entry point failed while fetching pinned Box2D with `fatal: unable to
  access 'https://github.com/erincatto/box2d.git/': Could not resolve host: github.com`; rerunning
  the same headless configure/build with approved network access succeeded.
- **Amendments:** 2026-11-04 — renumbered from PRB-002 by Işık Dönger while merging `main` into the PB-065 branch, because both branches had created a PRB-002.

---

### PRB-004 — Bridge benchmark did not build or run on Windows
- **Date:** 2026-11-04
- **Reported by:** claude (PR #23 review), running the PB-065 benchmark on Işık Dönger's Windows 11 machine
- **Area:** Build
- **Status:** Solved
- **Symptom:** four separate failures at commit `df60602`:
  1. configure: `Could not find ZEROMQ_LIBRARY using the following names: zmq, libzmq`;
  2. build with `ENGINE_WARNINGS_AS_ERRORS=ON`: `grpc_bridge_benchmark` failed with `C2220` from `C4996: 'grpc::experimental::IdentityKeyCertPair::~IdentityKeyCertPair': Use IdentityKeyOrSignerCertPair instead`;
  3. ctest: 4 of 14 pytest cases failed with `ModuleNotFoundError: No module named 'aegis_bridge_pybind'`;
  4. `run_benchmarks.py`: `FileNotFoundError: missing benchmark executable: ...\grpc_bridge_benchmark`.
- **Context:** Windows 11, VS 2022 / MSVC 14.44, Visual Studio generator, vcpkg (bundled with VS) x64-windows, gRPC 1.84.0, Python 3.12.0.
- **Root cause:** (1) vcpkg names the ZeroMQ import library `libzmq-mt-4_3_5.lib`; (2) the deprecation warning comes from gRPC's own headers; (3) the Visual Studio generator puts the `.pyd` in `python/Release/`; (4) the runner looked only for a binary without `.exe` directly in the build folder.
- **Solution:** (1) use the `libzmq` target that cppzmq's config package provides, with the old `find_library` as fallback; (2) `/wd4996` on `grpc_bridge_benchmark` only, under `if(MSVC)`; (3) `LIBRARY_OUTPUT_DIRECTORY "$<1:...>"` so no config subfolder is added; (4) `run_benchmarks.py` uses the same `Release`/`Debug` `.exe` lookup as `test_integration.py`.
- **How verified:** on the Windows machine with equivalent local fixes: build passed with warnings as errors, ctest 4/4, and the full 40-case matrix passed with 0 errors. The committed versions of fixes 1 and 4 differ slightly from the local workarounds; Ubuntu CI covers fix 1's fallback path, and Windows needs a re-run to confirm the committed form.
- **Prevention:** README now documents the Windows setup. A Windows CI job for the bridge was not added because the uncached gRPC build takes about 20–40 minutes.
- **Keywords:** Windows, MSVC, vcpkg, libzmq-mt-4_3_5, ZEROMQ_LIBRARY, C4996, C2220, IdentityKeyCertPair, aegis_bridge_pybind, ModuleNotFoundError, .pyd, Release folder, run_benchmarks.py
- **Related:** PB-065, DEC-029, DEC-038, PRB-002
- **Occurrences:** —

---

### PRB-005 — Generated protobuf code failed AppleClang warnings-as-errors
- **Date:** 2026-11-22
- **Reported by:** codex/gpt-5/us112
- **Area:** Build / AgentBridge
- **Status:** Solved
- **Symptom:** the optional production gRPC bridge failed with
  `error: type nullability specifier '_Nullable' is a Clang extension [-Werror,-Wnullability-extension]`
  and the equivalent `_Nonnull` error while compiling generated `agent_bridge.pb.cc`.
- **Context:** macOS, AppleClang 17, Homebrew gRPC 1.85.1 / protobuf 36.2,
  `ENGINE_WARNINGS_AS_ERRORS=ON`, `AEGIS_WITH_GRPC_BRIDGE=ON`.
- **Root cause:** protobuf-generated checks expand Abseil's `absl_nullable` and `absl_nonnull` to
  Clang nullability extensions. The project warning policy promoted the generated-code diagnostic
  to an error; no project source triggered it.
- **Solution:** add `-Wno-nullability-extension` to `engine_bridge_grpc` for Clang compilers. The
  suppression is specific to this extension and optional target; project warnings remain errors.
- **How verified:** reconfigured and built `build/grpc-debug` with the options above; generated
  protobuf/gRPC sources, `grpc_agent_bridge.cpp` and all other targets compiled successfully.
- **Prevention:** keep generated protocol sources inside the optional transport target and scope
  third-party warning suppressions to that target/compiler.
- **Keywords:** AppleClang, protobuf 36.2, Abseil, _Nullable, _Nonnull,
  -Wnullability-extension, warnings as errors, agent_bridge.pb.cc
- **Related:** DEC-038, DEC-067, TASK-020
- **Occurrences:** US-11.2 local gRPC build. US-33.5's warnings-as-errors gRPC build first failed with `error: enumeration value 'DO_NOT_USE' not handled in switch [-Werror,-Wswitch]`; adding the sentinel case to the exhaustive status-name mapping made the health/model bridge build pass.

---

### PRB-006 — Test fixture wrote a chat file into the build folder
- **Date:** 2026-11-22
- **Area:** Tests / Assistant
- **Status:** Solved
- **Symptom:** `GroundedAssistantTest.Ask_UnauthorizedContextSendsNothingAndLeavesChatUnchanged` failed with
  `Value of: std::filesystem::exists(store.chatFilePath("grounded")) Actual: true Expected: false`,
  and `build/headless/tests/assistant/chats/grounded.chat` appeared.
- **Context:** US-11.1 tests, headless preset. The fixture declared `std::filesystem::path root;`,
  set it in `SetUp()`, and built `FixedUserProjectPaths rootPaths{root}` and `ChatStore` as members.
- **Root cause:** members are initialised before `SetUp()` runs, so the store copied an empty path
  and wrote chats relative to the test's working directory, shared by all tests.
- **Solution:** initialise `root` in its declaration from the current test name, before the paths
  and store members, and only clear it in `SetUp()`.
- **How verified:** removed the stray folder, rebuilt; all 207 tests pass and no chat file appears
  under `build/`.
- **Prevention:** in fixtures, give any member another member depends on its value in its
  declaration, never in `SetUp()`.
- **Keywords:** GoogleTest fixture, member initialisation order, SetUp, ChatStore, userLocalArea,
  temp_directory_path
- **Related:** TASK-023, DEC-067
- **Occurrences:** US-11.1 tests

---

### PRB-007 — Chip sizes were missing or zero when measured through GroundedAssistant::prepare()
- **Date:** 2026-11-22
- **Area:** Editor / Assistant
- **Status:** Solved
- **Symptom:** `AssistantChatGroundedTest.ChipSizesAreTheBytesTheItemAddsToTheContextDocument` failed
  first with `Value of: source.has_value() Actual: false Expected: true`, then with
  `Expected: (*source) >= (std::string("bool unlocked = false;\n").size()), actual: 0 vs 23`.
- **Context:** US-33.2 `GroundedContextSizeEstimator`, headless preset.
- **Root cause:** `assembleContext()` refuses an empty question (`question is empty`); after giving
  it a text, the diagnosis in the context document shortened when evidence was attached, cancelling
  the source file's bytes.
- **Solution:** measure with a fixed placeholder question and render both documents with
  `renderContextDocument(context, Diagnosis{})`, so only the item's evidence differs.
- **How verified:** the test passes; headless 258/258 and macOS Debug 271/271 with warnings as errors.
- **Prevention:** measure one item's contribution against an otherwise identical document.
- **Keywords:** context chip size, assembleContext, question is empty, renderContextDocument, diagnosis
- **Related:** TASK-025, DEC-071, DEC-070
- **Occurrences:** US-33.2 tests

---
