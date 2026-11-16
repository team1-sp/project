#pragma once

#include <functional>
#include <memory>
#include <string>

namespace engine {

// Size, fullscreen state, title and frame-rate cap of the application window (srs.fr.1.9). A
// standalone game takes the size and the fullscreen state from the project's Rendering settings
// (srs.fr.27.23) once they exist. Pixels are a rendering unit; the simulation works in meters
// (DEC-036).
struct WindowConfig {
    int widthPixels = 800;
    int heightPixels = 450;
    // Fullscreen at widthPixels x heightPixels instead of a window on the desktop.
    bool fullscreen = false;
    std::string title = "Aegis";
    // Render frames per second the window waits for; 0 means no cap.
    int targetFramesPerSecond = 60;
    // Pressing Escape asks to close the window, as the close button does. The Editor turns this
    // off: Escape has other uses there (srs.fr.14.3).
    bool escapeClosesWindow = true;
};

// The application window of normal mode (srs.fr.1.7). engine/core knows only this interface;
// the implementation is in engine/rendering-raylib, the one target that may use the graphics
// framework (R-4). An IWindow object is an open window: destroying it closes the window.
// Use it only from the thread that opened it.
//
// The game loop calls the three frame steps in the order of srs.fr.1.1, once per render frame:
// collectInput(), then the ticks, then beginFrame(), the drawing and presentFrame().
class IWindow {
public:
    virtual ~IWindow() = default;

    // True when the user asked to close the window during the last collectInput(). The request
    // is not kept: when the application decides not to close, the next collectInput() clears it.
    [[nodiscard]] virtual bool shouldClose() const = 0;
    // Step 1 of a render frame: collects the keyboard input and window events the operating
    // system has queued since the previous call (srs.fr.1.1, 5.1).
    virtual void collectInput() = 0;
    // Starts drawing a render frame with a cleared image. Drawing is valid until presentFrame().
    virtual void beginFrame() = 0;
    // Step 4 of a render frame: presents the image, then waits for the frame-rate cap. It
    // collects no input.
    virtual void presentFrame() = 0;
    // Changes the title shown in the window's title bar (srs.fr.14.2).
    virtual void setTitle(const std::string& title) = 0;
};

// Opens a window, or returns nullptr when it cannot be created. Application calls it once in
// normal mode and never in headless mode.
using WindowFactory = std::function<std::unique_ptr<IWindow>(const WindowConfig&)>;

} // namespace engine
