# Problems

Every problem actually hit, with its root cause and solution, so it takes minutes the second time. One file per area keeps each file small. **Agents do not read this folder at boot.** They open it when they hit a problem (an error, a failing build or test, behaviour they cannot explain) and before they debug.

## How to use

1. **Before debugging**, find the area below and search its file (and this index) for the exact error text, the component, the library and the symptom. Search the whole folder (`grep -ri "<error text>" docs/history/problems/`) when the area is unclear.
2. **After solving**, append the entry at the bottom of its area file with [the template](../../agents/templates/problem-entry.md) and add its row at the bottom of the index below, in the same PR. `tools/check_ids.py` fails a PR when an entry and its row do not match.
3. **If it happens again**, add an `Occurrences` line to the existing entry. Do not create a duplicate.
4. When an area file passes about 30 KB, or a new area gets its first few entries, split it into a new file and move its rows' links. Moving entries between files is allowed; editing their text is not.
5. *Anticipated* pitfalls that have not happened yet live in [../../troubleshooting/common-errors.md](../../troubleshooting/common-errors.md).

## Areas

| File | Covers |
| --- | --- |
| [build-and-tooling.md](build-and-tooling.md) | CMake, dependencies (vcpkg, FlatBuffers, gRPC, protobuf), compilers and warnings, Python environments, CI, scripts. |
| [engine.md](engine.md) | ECS, physics and the Physics Adapter, rendering, input, gameplay, telemetry, headless runs. |
| [ai.md](ai.md) | AgentBridge, `ai-services`, the assistant core, semantic layer, AI tests. |
| [editor.md](editor.md) | Editor panels and models, Dear ImGui, the assistant UI. |

## Index

| ID | Title | File | Status | Date |
| --- | --- | --- | --- | --- |
| PRB-001 | Raylib sprite shader shows edge artefacts: texture wrap defaults to repeat | [engine](engine.md) | Solved (documented in spec) | 2026-10-12 (recorded) |
| PRB-002 | Ubuntu FlatBuffers package has no CMake config package | [build-and-tooling](build-and-tooling.md) | Solved | 2026-11-04 |
| PRB-003 | Clean Python environment could not download pinned build dependencies in the sandbox | [build-and-tooling](build-and-tooling.md) | Solved | 2026-11-04 |
| PRB-004 | Bridge benchmark did not build or run on Windows (vcpkg ZeroMQ name, gRPC C4996, pybind11 output folder, missing .exe lookup) | [build-and-tooling](build-and-tooling.md) | Solved | 2026-11-04 |
| PRB-005 | Generated protobuf code failed AppleClang warnings-as-errors | [build-and-tooling](build-and-tooling.md) | Solved | 2026-11-22 |
| PRB-006 | Test fixture wrote a chat file into the build folder: store built from a path member before it was set | [ai](ai.md) | Solved | 2026-11-22 |
| PRB-007 | Chip sizes missing or zero: empty question refused, then diagnosis text cancelled the item's bytes | [editor](editor.md) | Solved | 2026-11-22 |
