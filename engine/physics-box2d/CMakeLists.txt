# The only target that links Box2D. PRIVATE keeps Box2D headers out of every target that
# links this one (R-3).
add_library(engine_physics_box2d STATIC
    src/box2d_info.cpp
    src/box2d_physics_adapter.cpp
)
add_library(engine::physics_box2d ALIAS engine_physics_box2d)
target_include_directories(engine_physics_box2d PUBLIC include)
target_link_libraries(engine_physics_box2d
    PUBLIC engine::core engine::physics
    PRIVATE box2d::box2d
)
engine_set_warnings(engine_physics_box2d)
