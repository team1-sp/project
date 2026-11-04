add_library(engine_bridge INTERFACE)
add_library(engine::bridge ALIAS engine_bridge)
target_include_directories(engine_bridge INTERFACE include)
