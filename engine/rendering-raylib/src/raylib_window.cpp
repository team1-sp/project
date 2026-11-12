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

    const double targetFrameSeconds =
        config.targetFramesPerSecond > 0 ? 1.0 / config.targetFramesPerSecond : 0.0;
    // Not make_unique: the constructor is private.
    return std::unique_ptr<IWindow>(new RaylibWindow(targetFrameSeconds));
}

RaylibWindow::RaylibWindow(double targetFrameSeconds)
    : targetFrameSeconds_(targetFrameSeconds)
    , lastPresentSeconds_(GetTime()) {
}

RaylibWindow::~RaylibWindow() {
    CloseWindow();
}

bool RaylibWindow::shouldClose() const {
    return WindowShouldClose();
}

void RaylibWindow::collectInput() {
    // raylib is built with SUPPORT_CUSTOM_FRAME_CONTROL (DEC-051), so EndDrawing() polls nothing
    // and this is the only place input is collected.
    PollInputEvents();
}

void RaylibWindow::beginFrame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
}

void RaylibWindow::presentFrame() {
    // With custom frame control EndDrawing() only draws what is batched; presenting and the
    // frame-rate cap are done here, as EndDrawing() would do them without it.
    EndDrawing();
    SwapScreenBuffer();

    if (targetFrameSeconds_ > 0.0) {
        const double frameSeconds = GetTime() - lastPresentSeconds_;
        if (frameSeconds < targetFrameSeconds_) {
            WaitTime(targetFrameSeconds_ - frameSeconds);
        }
    }
    lastPresentSeconds_ = GetTime();
}

} // namespace engine::rendering_raylib
