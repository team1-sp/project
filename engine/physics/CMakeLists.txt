# The Physics Adapter interface (PB-025): our types and IPhysicsAdapter, no physics library.
# Header-only for now. It must never link Box2D; engine/physics-box2d implements it (R-3).
add_library(engine_physics INTERFACE)
add_library(engine::physics ALIAS engine_physics)
target_include_directories(engine_physics INTERFACE include)
target_link_libraries(engine_physics INTERFACE engine::core)
