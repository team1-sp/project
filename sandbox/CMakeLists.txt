# Sandbox app: the smallest application on the engine's game loop (PB-011). It opens the
# window, or runs headless, and proves the build links raylib and Box2D (PB-002). It is not
# the demo game, which the team has not chosen yet (Q-016).
add_executable(sandbox
    src/main.cpp
)
target_link_libraries(sandbox PRIVATE
    engine::core
    engine::physics_box2d
    engine::rendering_raylib
)
engine_set_warnings(sandbox)

# PB-008 spike: Box2D + raylib falling box, written before the Physics Adapter exists.
# Research code that links Box2D and raylib directly, an agreed exception to R-3/R-4 for this
# spike only; engine code must never copy it. Box2D stays inside falling_box_scene.cpp and
# raylib inside box2d_raylib.cpp, so the scene's test runs without a window.
add_library(sandbox_falling_box STATIC
    src/falling_box_scene.cpp
)
target_include_directories(sandbox_falling_box PUBLIC src)
target_link_libraries(sandbox_falling_box PRIVATE box2d::box2d)
engine_set_warnings(sandbox_falling_box)

add_executable(box2d_raylib_spike
    src/box2d_raylib.cpp
)
target_link_libraries(box2d_raylib_spike PRIVATE
    sandbox_falling_box
    raylib
)
engine_set_warnings(box2d_raylib_spike)
