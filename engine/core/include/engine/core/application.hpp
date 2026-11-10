#pragma once

#include <cstdint>
#include <memory>

#include "engine/core/window.hpp"

namespace engine {

class IFrameRenderer;
class ISimulation;

// Settings of one run.
struct ApplicationConfig {
    // Headless mode opens no window and runs ticks back to back (srs.fr.1.7, 7.1, 7.4).
    bool headless = false;
    // The run ends once this many ticks have been executed (srs.fr.7.7). 0 means no limit, which
    // only normal mode accepts: a headless run has no window to close.
    std::uint64_t maxTicks = 0;
    // Used in normal mode only.
    WindowConfig window;
};

// Owns the application window and runs the game loop (srs.fr.1.1, 1.7 to 1.10).
// Use: construct, init(), run(), shutdown(). The simulation and the frame renderer are not
// owned and must outlive the Application.
class Application {
public:
    // An application that can only run headless: it cannot reach a window or a renderer (R-6).
    explicit Application(ISimulation& simulation);
    // An application for normal mode. A headless run of it never calls `createWindow` or
    // `frameRenderer`, so the graphics framework is not initialised (R-4).
    Application(ISimulation& simulation, IFrameRenderer& frameRenderer, WindowFactory createWindow);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Checks the settings and, in normal mode, opens the window. Returns false, after writing
    // the reason to the standard error output, when the run cannot start.
    [[nodiscard]] bool init(const ApplicationConfig& config);

    // Runs the game loop until the user closes the window or `maxTicks` is reached. The render
    // frame in which the window is closed is finished first. Returns the process exit code:
    // 0 after a normal end (srs.fr.1.10), non-zero when init() has not succeeded.
    [[nodiscard]] int run();

    // Closes the window and releases everything init() created. Calling it again does nothing;
    // the destructor calls it too.
    void shutdown();

private:
    void runHeadless();
    void runNormal();
    void runOneTick();
    [[nodiscard]] bool reachedMaxTicks() const;

    ISimulation& simulation_;
    IFrameRenderer* frameRenderer_ = nullptr;
    WindowFactory createWindow_;
    ApplicationConfig config_;
    std::unique_ptr<IWindow> window_;
    std::uint64_t ticksExecuted_ = 0;
    bool initialised_ = false;
};

} // namespace engine
