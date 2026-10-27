# One CMake target per engine module, so the build enforces the layer rules.
# Only physics-box2d links Box2D and only rendering-raylib links raylib (R-3, R-4).
add_subdirectory(core)
add_subdirectory(physics-box2d)

if(ENGINE_WITH_RAYLIB)
    add_subdirectory(rendering-raylib)
endif()
