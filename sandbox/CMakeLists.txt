# Sandbox app. It only opens and closes a window to prove the build links raylib and
# Box2D (PB-002). It is not the demo game, which the team has not chosen yet (Q-016).
add_executable(sandbox
    src/main.cpp
)
target_link_libraries(sandbox PRIVATE
    engine::core
    engine::physics_box2d
    engine::rendering_raylib
)
engine_set_warnings(sandbox)
