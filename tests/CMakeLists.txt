# GoogleTest suite. Tests mirror the engine folders and never open a window, so the
# test target does not link raylib and runs in the headless preset too.
include(GoogleTest)

add_executable(engine_tests
    unit/core/version_test.cpp
    unit/physics-box2d/box2d_info_test.cpp
)
target_link_libraries(engine_tests PRIVATE
    engine::core
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
