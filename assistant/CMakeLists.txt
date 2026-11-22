add_library(aegis_assistant STATIC
    src/chat_store.cpp
    src/read_only_assistant.cpp
    src/grounded/answer_format.cpp
    src/grounded/citations.cpp
    src/grounded/content_hash.cpp
    src/grounded/context_assembly.cpp
    src/grounded/diagnosis.cpp
    src/grounded/evidence.cpp
    src/grounded/grounded_agent_bridge.cpp
    src/grounded/grounded_assistant.cpp
    src/grounded/scripts_folder_reader.cpp
)
add_library(aegis::assistant ALIAS aegis_assistant)
target_include_directories(aegis_assistant PUBLIC include)
# engine::core supplies only the value types EntityId, Tick and Vec2 (R-1: AI -> Engine).
target_link_libraries(aegis_assistant PUBLIC engine::bridge engine::core)
engine_set_warnings(aegis_assistant)
