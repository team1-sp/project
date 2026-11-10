#pragma once

#include <functional>
#include <memory>
#include <string>

namespace engine {

// Size, title and frame-rate cap of the application window (srs.fr.1.9). Pixels are a rendering
// unit; the simulation works in meters (DEC-036).
struct WindowConfig {
    int widthPixels = 800;
    int heightPixels = 450;
    std::string title = "Aegis";
    // Render frames per second the window waits for; 0 means no cap.
    int targetFramesPerSecond = 60;
};

// The application window of normal mode (srs.fr.1.7). engine/core knows only this interface;
// the implementation is in engine/rendering-raylib, the one target that may use the graphics
// framework (R-4). An IWindow object is an open window: destroying it closes the window.
// Use it only from the thread that opened it.
class IWindow {
public:
    virtual ~IWindow() = default;

    // True once the user has asked to close the window.
    [[nodiscard]] virtual bool shouldClose() const = 0;
    // Starts a render frame with a cleared image. Drawing is valid until endFrame().
    virtual void beginFrame() = 0;
    // Presents the image, waits for the frame-rate cap, then collects the operating system's
    // input for the next render frame.
    virtual void endFrame() = 0;
};

// Opens a window, or returns nullptr when it cannot be created. Application calls it once in
// normal mode and never in headless mode.
using WindowFactory = std::function<std::unique_ptr<IWindow>(const WindowConfig&)>;

} // namespace engine
