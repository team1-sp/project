#pragma once

#include <memory>

#include "engine/core/window.hpp"

namespace engine::rendering_raylib {

// The application window (PB-011.1). It wraps raylib's InitWindow, WindowShouldClose and
// CloseWindow; nothing else in the engine calls them. raylib keeps one global window, so at
// most one RaylibWindow exists at a time.
class RaylibWindow final : public IWindow {
public:
    // Opens the window. Returns nullptr when a window is already open or raylib cannot create
    // the window and its graphics context. Usable as an engine::WindowFactory.
    [[nodiscard]] static std::unique_ptr<IWindow> open(const WindowConfig& config);

    // Closes the window. Everything loaded through raylib must be unloaded before this.
    ~RaylibWindow() override;

    RaylibWindow(const RaylibWindow&) = delete;
    RaylibWindow& operator=(const RaylibWindow&) = delete;

    // True after the user clicked the close button or pressed Escape, raylib's default exit key.
    [[nodiscard]] bool shouldClose() const override;
    void beginFrame() override;
    void endFrame() override;

private:
    // Only open() constructs one, after raylib reports that the window is ready.
    RaylibWindow() = default;
};

} // namespace engine::rendering_raylib
