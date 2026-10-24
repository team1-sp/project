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
gtest_discover_tests(engine_tests)
