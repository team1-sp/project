# GoogleTest suite. Tests mirror the engine folders and never open a window, so the
# test target does not link raylib and runs in the headless preset too.
include(GoogleTest)

add_executable(engine_tests
    unit/core/application_test.cpp
    unit/core/fixed_timestep_test.cpp
    unit/core/frame_clock_test.cpp
    unit/core/version_test.cpp
    unit/bridge/agent_bridge_test.cpp
    unit/physics-box2d/box2d_info_test.cpp
)
target_link_libraries(engine_tests PRIVATE
    engine::core
    engine::bridge
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
endif()
