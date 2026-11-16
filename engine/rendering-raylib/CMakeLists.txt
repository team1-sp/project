# The only target that links raylib (R-4). Never built in the headless preset.
add_library(engine_rendering_raylib STATIC
    src/imgui_renderer.cpp
    src/raylib_info.cpp
    src/raylib_window.cpp
    src/status_text_renderer.cpp
)
add_library(engine::rendering_raylib ALIAS engine_rendering_raylib)
target_include_directories(engine_rendering_raylib PUBLIC include)
target_link_libraries(engine_rendering_raylib
    PUBLIC engine::core
    PRIVATE raylib imgui rlimgui
)
engine_set_warnings(engine_rendering_raylib)

# rlImGui, the raylib backend of Dear ImGui (DEC-063). It includes raylib.h, so it is built
# here and linked only by this target (R-4). It is third-party code and keeps its own warnings.
# It merges the Font Awesome 6 icons into the last font loaded; `rlimgui_icons` gives other
# targets the icon names (ICON_FA_...) without raylib (DEC-064).
add_library(rlimgui STATIC ${rlimgui_SOURCE_DIR}/rlImGui.cpp)
target_include_directories(rlimgui SYSTEM PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PUBLIC raylib imgui)

add_library(rlimgui_icons INTERFACE)
target_include_directories(rlimgui_icons SYSTEM INTERFACE ${rlimgui_SOURCE_DIR}/extras)
