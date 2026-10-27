# Demo game. For now it only opens and closes a window to prove the build links
# raylib and Box2D (PB-002); the game itself comes later.
add_executable(demo-maze
    src/main.cpp
)
target_link_libraries(demo-maze PRIVATE
    engine::core
    engine::physics_box2d
    engine::rendering_raylib
)
engine_set_warnings(demo-maze)
