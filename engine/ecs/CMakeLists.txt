# The custom ECS (DEC-018, docs/design/ecs-api-design.md): entities, their IDs and hierarchy,
# component storage and deferred structural changes. It links only engine::core, so physics and
# rendering reach each other only through it (R-2).
add_library(engine_ecs STATIC
    src/command_buffer.cpp
    src/entity_table.cpp
    src/world.cpp
)
add_library(engine::ecs ALIAS engine_ecs)
target_include_directories(engine_ecs PUBLIC include)
target_link_libraries(engine_ecs PUBLIC engine::core)
engine_set_warnings(engine_ecs)
