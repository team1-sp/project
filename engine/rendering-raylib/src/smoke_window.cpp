#include "engine/rendering_raylib/smoke_window.hpp"

#include <string>

#include <raylib.h>

namespace engine::rendering_raylib {

Version linkedRaylibVersion() {
    return Version{RAYLIB_VERSION_MAJOR, RAYLIB_VERSION_MINOR, RAYLIB_VERSION_PATCH};
}

void runSmokeWindow(std::string_view title, int maxFrames) {
    constexpr int widthPixels = 800;
    constexpr int heightPixels = 450;

    // raylib needs a null-terminated title.
    const std::string titleText(title);
    InitWindow(widthPixels, heightPixels, titleText.c_str());
    SetTargetFPS(60);

    int frame = 0;
    while (!WindowShouldClose() && (maxFrames == 0 || frame < maxFrames)) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Engine skeleton: raylib and Box2D are linked.", 20, 20, 20, DARKGRAY);
        EndDrawing();
        ++frame;
    }

    CloseWindow();
}

} // namespace engine::rendering_raylib
