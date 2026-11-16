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

    // The tick counter is 0 when a run starts (srs.fr.1.5), no action is held, nothing is
    // queued from an earlier run and the run is not paused.
    ticksExecuted_ = 0;
    inputState_ = InputActionState{};
    keyboardQueue_.clear();
    outcome_ = RunOutcome::NotRun;
    closeRequested_ = false;
    pauseRequested_ = false;
    paused_ = false;
    stepsRequested_ = 0;
    pausedSinceFrameStart_ = std::chrono::nanoseconds::zero();
    if (config_.headless) {
        runHeadless();
    } else {
        runningNormal_ = true;
        runNormal();
        runningNormal_ = false;
    }
    return outcome_ == RunOutcome::Error ? exitCodeError : exitCodeOk;
}

void Application::shutdown() {
    if (window_ && frameRenderer_ != nullptr) {
        frameRenderer_->releaseGraphics();
    }
    window_.reset();
    initialised_ = false;
}

RunOutcome Application::outcome() const {
    return outcome_;
}

KeyboardQueue& Application::keyboardQueue() {
    return keyboardQueue_;
}

bool Application::requestClose() {
    if (!runningNormal_) {
        return false;
    }
    closeRequested_ = true;
    return true;
}

bool Application::setWindowTitle(const std::string& title) {
    if (!window_) {
        return false;
    }
    window_->setTitle(title);
    return true;
}

bool Application::requestPause() {
    if (!runningNormal_) {
        return false;
    }
    if (!paused_) {
        pauseRequested_ = true;
    }
    return true;
}

bool Application::resume() {
    if (pauseRequested_) {
        // The pause has not taken effect yet: no tick boundary has passed, nothing was discarded.
        pauseRequested_ = false;
        return true;
    }
    if (!paused_) {
        return false;
    }
    paused_ = false;
    stepsRequested_ = 0;
    // srs.fr.1.18: real time counts again from now. The next frame time also holds the time
    // since the last measurement, which was paused; it is taken off before the accumulator.
    pausedSinceFrameStart_ = frameClock_.sinceFrameStart();
    return true;
}

bool Application::requestStep() {
    if (!paused_) {
        return false;
    }
    ++stepsRequested_;
    return true;
}

bool Application::isPaused() const {
    return paused_;
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

    // The close requests are read only here, between render frames, so the frame in which the
    // user closes the window is always finished (srs.fr.1.10). A close the application does not
    // confirm is dropped, and the window stays open (srs.fr.14.3).
    // Every render frame runs the four steps of srs.fr.1.1 in this order.
    while (!closeRequested_) {
        if (window_->shouldClose() && (!config_.confirmClose || config_.confirmClose())) {
            break;
        }
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

        // (2) Run the ticks of this frame. The clock is read here and nowhere else: ticks never
        // see real time (R-6). It is read while paused too, so that the frame times stay true.
        // A run error ends the run at that tick boundary; the frame is not rendered, so nothing
        // presents state the failed tick left half-changed (srs.fr.1.11).
        const std::chrono::nanoseconds frameTime = frameClock_.measureFrame();
        if (!(paused_ ? runRequestedSteps() : runDueTicks(timestep, frameTime))) {
            return;
        }

        // (3) Render the state the last completed tick left and (4) present it, also when this
        // frame ran no tick (srs.fr.1.15).
        window_->beginFrame();
        frameRenderer_->renderFrame();
        window_->presentFrame();
    }
    outcome_ = RunOutcome::WindowClosed;
}

bool Application::runDueTicks(FixedTimestep& timestep, std::chrono::nanoseconds frameTime) {
    // srs.fr.1.18: the part of the frame time before resume() was paused and is not simulated.
    const std::chrono::nanoseconds realDelta = frameTime - pausedSinceFrameStart_;
    pausedSinceFrameStart_ = std::chrono::nanoseconds::zero();

    // One tick for each whole 1/60 s of real time not simulated yet, at most 5 (srs.fr.1.3).
    const FrameTicks frame = timestep.advance(realDelta);
    if (frame.warnSlow) {
        logWarning("Simulation is running slower than real time");
    }
    for (int i = 0; i < frame.ticksToRun && !reachedMaxTicks(); ++i) {
        // srs.fr.1.16: a pause request takes effect at the tick boundary. The frame's ticks that
        // have not started are whole ticks of the accumulator and are dropped with this loop;
        // the timestep keeps only the part of a tick left over.
        if (pauseRequested_) {
            break;
        }
        if (!runOneTick()) {
            return false;
        }
    }
    // A request made during the frame's last tick, or before a frame that runs no tick, takes
    // effect at this boundary as well.
    if (pauseRequested_) {
        enterPause();
    }
    return true;
}

bool Application::runRequestedSteps() {
    // srs.fr.1.17, 17.14: each step runs one tick and leaves the accumulator as it is, so the
    // timestep is not called at all while paused.
    while (stepsRequested_ > 0 && !reachedMaxTicks()) {
        --stepsRequested_;
        if (!runOneTick()) {
            return false;
        }
    }
    return true;
}

void Application::enterPause() {
    pauseRequested_ = false;
    paused_ = true;
    stepsRequested_ = 0;
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
