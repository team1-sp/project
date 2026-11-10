#include "engine/core/application.hpp"

#include <cstdio>
#include <utility>

#include "engine/core/frame_renderer.hpp"
#include "engine/core/simulation.hpp"

namespace engine {

namespace {

constexpr int exitCodeOk = 0;
constexpr int exitCodeError = 1;

// The engine has no logger yet, so errors go straight to the standard error output.
void logError(const char* message) {
    std::fprintf(stderr, "error: %s\n", message);
}

} // namespace

Application::Application(ISimulation& simulation)
    : simulation_(simulation) {
}

Application::Application(ISimulation& simulation, IFrameRenderer& frameRenderer,
                         WindowFactory createWindow)
    : simulation_(simulation)
    , frameRenderer_(&frameRenderer)
    , createWindow_(std::move(createWindow)) {
}

Application::~Application() {
    shutdown();
}

bool Application::init(const ApplicationConfig& config) {
    if (initialised_) {
        logError("Application::init was called twice without a shutdown");
        return false;
    }

    if (config.headless) {
        if (config.maxTicks == 0) {
            logError("a headless run needs a maximum number of ticks, or it never ends");
            return false;
        }
    } else {
        if (frameRenderer_ == nullptr || !createWindow_) {
            logError("normal mode needs a frame renderer and a window factory");
            return false;
        }
        window_ = createWindow_(config.window);
        if (window_ == nullptr) {
            logError("the application window could not be opened");
            return false;
        }
    }

    config_ = config;
    initialised_ = true;
    return true;
}

int Application::run() {
    if (!initialised_) {
        logError("Application::run was called before a successful init");
        return exitCodeError;
    }

    ticksExecuted_ = 0;
    if (config_.headless) {
        runHeadless();
    } else {
        runNormal();
    }
    return exitCodeOk;
}

void Application::shutdown() {
    window_.reset();
    initialised_ = false;
}

void Application::runHeadless() {
    // Simulated time: ticks run back to back, with no clock and no waiting (srs.fr.7.4).
    while (!reachedMaxTicks()) {
        runOneTick();
    }
}

void Application::runNormal() {
    // The close request is read only here, between render frames, so the frame in which the
    // user closes the window is always finished (srs.fr.1.10).
    while (!window_->shouldClose() && !reachedMaxTicks()) {
        // One tick per render frame until the fixed-timestep accumulator replaces it (PB-012).
        runOneTick();

        // Render and present the state the last completed tick left (srs.fr.1.1). endFrame()
        // also collects the input that the next frame's ticks will use.
        window_->beginFrame();
        frameRenderer_->renderFrame();
        window_->endFrame();
    }
}

void Application::runOneTick() {
    simulation_.runTick();
    ++ticksExecuted_;
}

bool Application::reachedMaxTicks() const {
    return config_.maxTicks != 0 && ticksExecuted_ >= config_.maxTicks;
}

} // namespace engine
