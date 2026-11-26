# Where the ECS and the Physics Adapter meet (DEC-041): the PhysicsSyncIn system and, later,
# PhysicsSyncOut. Links the ECS and the adapter interface, never Box2D (R-3).
add_library(engine_physics_sync STATIC
    src/physics_sync.cpp
)
add_library(engine::physics_sync ALIAS engine_physics_sync)
target_include_directories(engine_physics_sync PUBLIC include)
target_link_libraries(engine_physics_sync PUBLIC engine::ecs engine::physics)
engine_set_warnings(engine_physics_sync)
