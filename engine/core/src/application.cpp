#include "engine/core/application.hpp"

#include <cstdio>
#include <utility>

#include "engine/core/fixed_timestep.hpp"
#include "engine/core/frame_renderer.hpp"
#include "engine/core/run_error.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick_phase.hpp"

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

// srs.fr.1.11: the message and the number of the tick that was running.
void logRunError(Tick tick, const char* message) {
    std::fprintf(stderr, "error: tick %llu: %s\n", static_cast<unsigned long long>(tick), message);
}

} // namespace

const char* runOutcomeName(RunOutcome outcome) {
    switch (outcome) {
    case RunOutcome::NotRun:
        return "Not run";
    case RunOutcome::WindowClosed:
        return "Window closed";
    case RunOutcome::MaxTicks:
        return "Max ticks";
    case RunOutcome::Error:
        return "Error";
    }
    return "Unknown";
}

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

    // The tick counter is 0 when a run starts (srs.fr.1.5), no action is held and nothing is
    // queued from an earlier run.
    ticksExecuted_ = 0;
    inputState_ = InputActionState{};
    keyboardQueue_.clear();
    outcome_ = RunOutcome::NotRun;
    if (config_.headless) {
        runHeadless();
    } else {
        runNormal();
    }
    return outcome_ == RunOutcome::Error ? exitCodeError : exitCodeOk;
}

void Application::shutdown() {
    window_.reset();
    initialised_ = false;
}

RunOutcome Application::outcome() const {
    return outcome_;
}

KeyboardQueue& Application::keyboardQueue() {
    return keyboardQueue_;
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
        if (!runOneTick()) {
            return;
        }
    }
    outcome_ = RunOutcome::MaxTicks;
}

void Application::runNormal() {
    FixedTimestep timestep;
    frameClock_.start();

    // The close request is read only here, between render frames, so the frame in which the
    // user closes the window is always finished (srs.fr.1.10).
    // Every render frame runs the four steps of srs.fr.1.1 in this order.
    while (!window_->shouldClose()) {
        if (reachedMaxTicks()) {
            outcome_ = RunOutcome::MaxTicks;
            return;
        }

        // (1) Collect the keyboard input this frame's ticks use. While another input source is
        // active the keyboard is ignored (srs.fr.5.11), so nothing stays queued for later.
        window_->collectInput();
        if (&inputSource() != &keyboardQueue_) {
            keyboardQueue_.clear();
        }

        // (2) One tick for each whole 1/60 s of real time not simulated yet, at most 5
        // (srs.fr.1.3). The clock is read here and nowhere else: ticks never see real time (R-6).
        const FrameTicks frame = timestep.advance(frameClock_.measureFrame());
        if (frame.warnSlow) {
            logWarning("Simulation is running slower than real time");
        }
        for (int i = 0; i < frame.ticksToRun && !reachedMaxTicks(); ++i) {
            // A run error ends the run at this tick boundary; the frame is not rendered, so
            // nothing presents state the failed tick left half-changed (srs.fr.1.11).
            if (!runOneTick()) {
                return;
            }
        }

        // (3) Render the state the last completed tick left and (4) present it, also when this
        // frame ran no tick (srs.fr.1.15).
        window_->beginFrame();
        frameRenderer_->renderFrame();
        window_->presentFrame();
    }
    outcome_ = RunOutcome::WindowClosed;
}

bool Application::runOneTick() {
    // The tick's number is the counter's value at its start; the counter goes up only after the
    // tick has completed (srs.fr.1.5), so a tick that fails is not counted.
    const Tick tick = ticksExecuted_;
    try {
        // The tick's input action state, from the transitions its source gives it, by the rules
        // of srs.fr.5.6 (srs.fr.1.14). It is built before phase 1 and does not change during the
        // tick.
        tickTransitions_.clear();
        inputSource().takeTransitions(tick, tickTransitions_);
        inputState_.applyTick(tickTransitions_);

        // The seven tick phases of srs.fr.1.12, in order. Headless and normal mode both come
        // here, so they run the same phases in the same order.
        const TickContext context{tick, inputState_};
        for (const TickPhase phase : tickPhases) {
            simulation_.runPhase(phase, context);
        }
    } catch (const RunError& error) {
        // The game loop is the only place RunError is caught (DEC-042).
        logRunError(tick, error.what());
        outcome_ = RunOutcome::Error;
        return false;
    }
    ++ticksExecuted_;
    return true;
}

IInputSource& Application::inputSource() {
    if (config_.inputSource != nullptr) {
        return *config_.inputSource;
    }
    // srs.fr.7.3: the keyboard is not a source in headless mode.
    if (config_.headless) {
        return noInput_;
    }
    return keyboardQueue_;
}

bool Application::reachedMaxTicks() const {
    return config_.maxTicks != 0 && ticksExecuted_ >= config_.maxTicks;
}

} // namespace engine
