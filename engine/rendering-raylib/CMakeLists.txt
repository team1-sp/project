# The only target that links raylib (R-4). Never built in the headless preset.
add_library(engine_rendering_raylib STATIC
    src/raylib_info.cpp
    src/raylib_window.cpp
    src/status_text_renderer.cpp
)
add_library(engine::rendering_raylib ALIAS engine_rendering_raylib)
target_include_directories(engine_rendering_raylib PUBLIC include)
target_link_libraries(engine_rendering_raylib
    PUBLIC engine::core
    PRIVATE raylib
)
engine_set_warnings(engine_rendering_raylib)
