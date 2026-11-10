# Editor app: hosts engine::Application and owns the main window (DEC-048). It opens a window,
# so it is built only when raylib is on, like sandbox/ (R-4, R-6).
add_executable(editor
    src/main.cpp
)
target_link_libraries(editor PRIVATE
    engine::core
    engine::rendering_raylib
)
engine_set_warnings(editor)
