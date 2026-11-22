# Editor logic that needs no window: the menu model, the shortcuts and the session state
# (US-14.1), the status bar's items (US-14.2) and the AI Assistant chat state (US-33.2). It
# links neither raylib nor Dear ImGui, so the tests use it in every preset.
add_library(editor_core STATIC
    src/assistant_chat.cpp
    src/assistant_chat_context.cpp
    src/assistant_chat_services.cpp
    src/assistant_entry_points.cpp
    src/command.cpp
    src/console_model.cpp
    src/editor_session.cpp
    src/entity_navigation.cpp
    src/external_code_editor.cpp
    src/main_menu.cpp
    src/status_bar.cpp
)
add_library(editor::core ALIAS editor_core)
target_include_directories(editor_core PUBLIC include)
# The AI Assistant chat (US-33.2) uses US-11.1/US-11.2's window-free assistant core.
find_package(Threads REQUIRED)
target_link_libraries(editor_core PUBLIC engine::core aegis::assistant PRIVATE Threads::Threads)
engine_set_warnings(editor_core)

# Editor app: hosts engine::Application and owns the main window (DEC-048). It opens a window,
# so it is built only when raylib is on, like sandbox/ (R-4, R-6). It draws with Dear ImGui
# (DEC-063) and reaches raylib only through engine::rendering_raylib.
if(ENGINE_WITH_RAYLIB)
    add_executable(editor
        src/assistant_chat_panel.cpp
        src/main.cpp
        src/main_window_view.cpp
        src/theme.cpp
    )
    target_link_libraries(editor PRIVATE
        editor::core
        engine::core
        engine::rendering_raylib
        imgui
        rlimgui_icons
    )
    engine_set_warnings(editor)
    # The fonts of the UI design live inside the executable (DEC-064).
    embed_file(editor interRegularTtf ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Inter-Regular.ttf)
    embed_file(editor interSemiBoldTtf ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Inter-SemiBold.ttf)
    embed_file(editor jetBrainsMonoTtf
               ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/JetBrainsMono-Regular.ttf)
endif()
