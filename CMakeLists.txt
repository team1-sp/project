# Root build for the engine, editor, sandbox and tests (PB-002).
# Layout and layer rules: docs/guidelines/repository-structure.md
cmake_minimum_required(VERSION 3.25)

# Aegis is the product name (DEC-034).
project(aegis VERSION 0.1.0 LANGUAGES C CXX)

# C++20 everywhere, no compiler extensions (DEC-019, DEC-030).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The headless preset turns raylib off, so nothing that needs a window is built (R-6).
option(ENGINE_WITH_RAYLIB "Build the raylib renderer and the targets that open a window" ON)
option(ENGINE_BUILD_TESTS "Build the GoogleTest suite" ON)
option(ENGINE_WARNINGS_AS_ERRORS "Treat compiler warnings in our own targets as errors" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Dependencies)

add_subdirectory(engine)
add_subdirectory(editor)

if(ENGINE_WITH_RAYLIB)
    add_subdirectory(sandbox)
endif()

if(ENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
