# Tests

Unit tests use GoogleTest v1.17.0 (DEC-023), fetched by `cmake/Dependencies.cmake`, and run through CTest. Everything here builds into one executable, `engine_tests`. What we test and why: [docs/testing/test-strategy.md](../docs/testing/test-strategy.md).

## Run the tests

From the repository root, with your platform's preset (`windows-debug`, `macos-debug` or `headless`):

```
cmake --workflow --preset windows-debug             # configure, build and run all tests
ctest --preset windows-debug                        # run all tests (after a build)
ctest --preset windows-debug -R EngineVersion       # run tests whose name matches a regex
ctest --preset windows-debug -R EngineVersion -V    # same, with GoogleTest's own output
```

CTest only sees tests that existed at the last build, so rebuild (`cmake --build --preset <preset>`) after adding one. You can also run the executable directly, which is handy in a debugger:

```
build\windows-debug\tests\Debug\engine_tests.exe --gtest_filter=EngineVersion.*
build/macos-debug/tests/engine_tests --gtest_filter=EngineVersion.*
```

CI runs `ctest` on Windows, macOS and Linux (headless) for every PR.

## Add a test

1. **Pick the folder.** Tests mirror the engine folders: a test for `engine/core/` goes in `tests/unit/core/`, one for `engine/physics-box2d/` in `tests/unit/physics-box2d/`. Name the file after the unit it tests: `<unit>_test.cpp`. Other levels (`integration/`, `scenarios/`, `benchmarks/`) get their folders when their first test lands.
2. **Write the test.** `<Unit>_<Condition>_<Expected>` from [coding-standards.md](../docs/guidelines/coding-standards.md) becomes `TEST(<Unit>, <Condition>_<Expected>)`. Worked example: [`unit/core/version_test.cpp`](unit/core/version_test.cpp).

   ```cpp
   #include <gtest/gtest.h>

   #include "engine/core/version.hpp"
   #include "engine/core/version_config.hpp"

   TEST(EngineVersion, Called_MatchesCMakeProjectVersion) {
       const engine::Version version = engine::engineVersion();
       EXPECT_EQ(version.major, engine::build::versionMajor);
   }
   ```

   CTest shows it as `EngineVersion.Called_MatchesCMakeProjectVersion`.
3. **Register the file** in [`CMakeLists.txt`](CMakeLists.txt): add it to `add_executable(engine_tests ...)`. If the test uses an engine library that is not linked yet, add its `engine::<folder>` target to `target_link_libraries`. `gtest_discover_tests` picks up every `TEST` automatically.
4. **Build and run** with the commands above. Check that the test fails when the behaviour is broken (change one value once and watch it go red).

## Rules

- **Headless.** Tests never open a window or touch the GPU, so `engine_tests` does not link raylib and runs in the `headless` preset. Test rendering through render data, not pixels.
- **Deterministic.** Fixed timestep, scripted input, seeded randomness, no wall-clock time. A flaky test is a bug.
- **One behaviour per test**, Arrange-Act-Assert, no shared global state between tests.
- **Don't weaken a failing test to get green.** Fix the cause.

More: [write-tests playbook](../docs/agents/playbooks/write-tests.md), [test checklists](../docs/testing/test-checklists.md).
