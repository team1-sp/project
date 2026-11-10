#include "engine/rendering_raylib/raylib_window.hpp"

#include <raylib.h>

namespace engine::rendering_raylib {

std::unique_ptr<IWindow> RaylibWindow::open(const WindowConfig& config) {
    if (IsWindowReady()) {
        return nullptr;
    }

    InitWindow(config.widthPixels, config.heightPixels, config.title.c_str());
    // InitWindow reports a failure only through this flag. CloseWindow must not run for a window
    // that never became ready, so the object is created only after the check.
    if (!IsWindowReady()) {
        return nullptr;
    }
    SetTargetFPS(config.targetFramesPerSecond);

    // Not make_unique: the constructor is private.
    return std::unique_ptr<IWindow>(new RaylibWindow());
}

RaylibWindow::~RaylibWindow() {
    CloseWindow();
}

bool RaylibWindow::shouldClose() const {
    return WindowShouldClose();
}

void RaylibWindow::beginFrame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
}

void RaylibWindow::endFrame() {
    // Swaps the buffers, waits for the frame-rate cap and then polls the input events.
    EndDrawing();
}

} // namespace engine::rendering_raylib
