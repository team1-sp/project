# Placeholder editor app, so the target exists from the start (PB-002).
add_executable(editor
    src/main.cpp
)
target_link_libraries(editor PRIVATE engine::core)
engine_set_warnings(editor)
