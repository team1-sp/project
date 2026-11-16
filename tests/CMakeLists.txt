# GoogleTest suite. Tests mirror the engine folders and never open a window, so the
# test target does not link raylib and runs in the headless preset too.
include(GoogleTest)

add_executable(engine_tests
    unit/core/application_test.cpp
    unit/core/fixed_timestep_test.cpp
    unit/core/frame_clock_test.cpp
    unit/core/input_actions_test.cpp
    unit/core/input_source_test.cpp
    unit/core/run_options_test.cpp
    unit/core/tick_phase_test.cpp
    unit/core/version_test.cpp
    unit/editor/main_menu_test.cpp
    unit/bridge/agent_bridge_test.cpp
    unit/physics/physics_adapter_test.cpp
    unit/physics-box2d/box2d_info_test.cpp
)
target_link_libraries(engine_tests PRIVATE
    editor::core
    engine::core
    engine::bridge
    engine::physics
    engine::physics_box2d
    GTest::gtest_main
)
engine_set_warnings(engine_tests)

# PB-008 spike. The scene uses Box2D only, so these tests need no window; the target only
# exists when raylib is on, because sandbox/ is not built in the headless preset.
if(TARGET sandbox_falling_box)
    target_sources(engine_tests PRIVATE
        unit/sandbox/falling_box_scene_test.cpp
        unit/sandbox/world_to_screen_test.cpp
    )
    target_link_libraries(engine_tests PRIVATE sandbox_falling_box)
endif()
gtest_discover_tests(engine_tests)

# PB-011.3: a headless sandbox run must exit with code 0 and, in MSVC Debug builds, without a C
# runtime leak report (engine/core/leak_check.hpp). It opens no window, so it also runs in CI.
if(TARGET sandbox)
    add_test(NAME Sandbox.HeadlessRun_ExitsCleanly COMMAND sandbox --headless --max-ticks 600)
    set_tests_properties(Sandbox.HeadlessRun_ExitsCleanly PROPERTIES
        FAIL_REGULAR_EXPRESSION "Detected memory leaks")

    # US-1.3, srs.fr.1.12: a headless run runs the seven tick phases of every tick.
    add_test(NAME Sandbox.HeadlessRun_RunsSevenPhasesPerTick
             COMMAND sandbox --headless --max-ticks 600)
    set_tests_properties(Sandbox.HeadlessRun_RunsSevenPhasesPerTick PROPERTIES
        PASS_REGULAR_EXPRESSION "tick phases run: 4200\n")

    # US-1.2, srs.fr.7.33: a headless run without --max-ticks stops at 10 000 ticks.
    add_test(NAME Sandbox.HeadlessWithoutMaxTicks_RunsTenThousandTicks
             COMMAND sandbox --headless --seed 42)
    set_tests_properties(Sandbox.HeadlessWithoutMaxTicks_RunsTenThousandTicks PROPERTIES
        PASS_REGULAR_EXPRESSION "outcome: Max ticks.*ticks executed: 10000 ")

    # srs.fr.7.33: an invalid option runs no tick and exits with a non-zero code.
    add_test(NAME Sandbox.InvalidSeed_ExitsNonZeroWithoutTicks
             COMMAND sandbox --headless --seed 4294967296)
    set_tests_properties(Sandbox.InvalidSeed_ExitsNonZeroWithoutTicks PROPERTIES WILL_FAIL TRUE)

    # srs.fr.1.11: a run error is logged with its tick number and the process exits non-zero.
    # Two tests, because CTest ignores the exit code of a test that matches its output.
    add_test(NAME Sandbox.RunError_ExitsNonZero
             COMMAND sandbox --headless --max-ticks 100 --fail-at-tick 41)
    set_tests_properties(Sandbox.RunError_ExitsNonZero PROPERTIES WILL_FAIL TRUE)
    add_test(NAME Sandbox.RunError_LogsMessageAndTick
             COMMAND sandbox --headless --max-ticks 100 --fail-at-tick 41)
    set_tests_properties(Sandbox.RunError_LogsMessageAndTick PROPERTIES
        PASS_REGULAR_EXPRESSION "error: tick 41: sandbox: --fail-at-tick")
endif()
