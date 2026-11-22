add_library(aegis_assistant STATIC
    src/chat_store.cpp
    src/read_only_assistant.cpp
)
add_library(aegis::assistant ALIAS aegis_assistant)
target_include_directories(aegis_assistant PUBLIC include)
target_link_libraries(aegis_assistant PUBLIC engine::bridge)
engine_set_warnings(aegis_assistant)
