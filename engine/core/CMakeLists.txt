# The version header is generated from the root project() version.
configure_file(version_config.hpp.in include/engine/core/version_config.hpp @ONLY)

add_library(engine_core STATIC
    src/version.cpp
)
add_library(engine::core ALIAS engine_core)
target_include_directories(engine_core PUBLIC include "${CMAKE_CURRENT_BINARY_DIR}/include")
engine_set_warnings(engine_core)
