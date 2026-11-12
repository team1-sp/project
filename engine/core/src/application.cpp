#include "engine/core/application.hpp"

#include <cstdio>
#include <utility>

#include "engine/core/fixed_timestep.hpp"
#include "engine/core/frame_renderer.hpp"
#include "engine/core/simulation.hpp"

namespace engine {

namespace {

constexpr int exitCodeOk = 0;
constexpr int exitCodeError = 1;

// The engine has no logger yet, so errors and warnings go straight to the standard error
// output.
void logError(const char* message) {
    std::fprintf(stderr, "error: %s\n", message);
}

void logWarning(const char* message) {
    std::fprintf(stderr, "warning: %s\n", message);
}

} // namespace

Application::Application(ISimulation& simulation)
    : simulation_(simulation) {
}

Application::Application(ISimulation& simulation, IFrameRenderer& frameRenderer,
                         WindowFactory createWindow, FrameClock::TimeSource realTime)
    : simulation_(simulation)
    , frameRenderer_(&frameRenderer)
    , createWindow_(std::move(createWindow))
    , frameClock_(std::move(realTime)) {
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

    // The tick counter is 0 when a run starts (srs.fr.1.5).
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

Tick Application::ticksExecuted() const {
    return ticksExecuted_;
}

double Application::simulatedSeconds() const {
    return engine::simulatedSeconds(ticksExecuted_);
}

void Application::runHeadless() {
    // Simulated time: ticks run back to back, with no clock and no waiting (srs.fr.7.4).
    while (!reachedMaxTicks()) {
        runOneTick();
    }
}

void Application::runNormal() {
    FixedTimestep timestep;
    frameClock_.start();

    // The close request is read only here, between render frames, so the frame in which the
    // user closes the window is always finished (srs.fr.1.10).
    // Every render frame runs the four steps of srs.fr.1.1 in this order.
    while (!window_->shouldClose() && !reachedMaxTicks()) {
        // (1) Collect the keyboard input this frame's ticks use.
        window_->collectInput();

        // (2) One tick for each whole 1/60 s of real time not simulated yet, at most 5
        // (srs.fr.1.3). The clock is read here and nowhere else: ticks never see real time (R-6).
        const FrameTicks frame = timestep.advance(frameClock_.measureFrame());
        if (frame.warnSlow) {
            logWarning("Simulation is running slower than real time");
        }
        for (int i = 0; i < frame.ticksToRun && !reachedMaxTicks(); ++i) {
            runOneTick();
        }

        // (3) Render the state the last completed tick left and (4) present it, also when this
        // frame ran no tick (srs.fr.1.15).
        window_->beginFrame();
        frameRenderer_->renderFrame();
        window_->presentFrame();
    }
}

void Application::runOneTick() {
    // The tick's number is the counter's value at its start; the counter goes up only after the
    // tick has completed (srs.fr.1.5).
    simulation_.runTick(ticksExecuted_);
    ++ticksExecuted_;
}

bool Application::reachedMaxTicks() const {
    return config_.maxTicks != 0 && ticksExecuted_ >= config_.maxTicks;
}

} // namespace engine
