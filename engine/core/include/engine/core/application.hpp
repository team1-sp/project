#pragma once

#include <memory>
#include <vector>

#include "engine/core/frame_clock.hpp"
#include "engine/core/input_actions.hpp"
#include "engine/core/input_source.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/window.hpp"

namespace engine {

class IFrameRenderer;
class ISimulation;

// Why a run ended (srs.fr.1.10, 1.11, 7.7). The names of the headless outcomes are those of
// srs.fr.7.16; Stopped, Failure and Objective reached come with the stop conditions of US-7.2.
enum class RunOutcome {
    // run() has not been called since init(), or init() did not succeed.
    NotRun,
    // Normal mode: the user closed the window (srs.fr.1.10).
    WindowClosed,
    // The tick counter reached the run's maximum number of ticks (srs.fr.7.7).
    MaxTicks,
    // A tick threw engine::RunError (srs.fr.1.11).
    Error,
};

// The outcome's name as reports and logs write it: "Not run", "Window closed", "Max ticks",
// "Error".
[[nodiscard]] const char* runOutcomeName(RunOutcome outcome);

// Settings of one run.
struct ApplicationConfig {
    // Headless mode opens no window and runs ticks back to back (srs.fr.1.7, 7.1, 7.4).
    bool headless = false;
    // The run ends once this many ticks have been executed (srs.fr.7.7). 0 means no limit, which
    // only normal mode accepts: a headless run has no window to close.
    Tick maxTicks = 0;
    // Used in normal mode only.
    WindowConfig window;
    // The run's input source (srs.fr.5.11): a recording or the agent. Not owned; it must outlive
    // the run. nullptr selects the default: the keyboard queue in normal mode, None in headless
    // mode, which has no keyboard (srs.fr.7.3). While another source is given, the keyboard is
    // ignored.
    IInputSource* inputSource = nullptr;
};

// Owns the application window and runs the game loop (srs.fr.1.1 to 1.11, 1.15).
// Use: construct, init(), run(), shutdown(). The simulation and the frame renderer are not
// owned and must outlive the Application.
//
// Normal mode runs render frames in the order of srs.fr.1.1: each one collects the keyboard
// input, runs the ticks that are due, at most 5, then renders and presents the state the last
// completed tick left. Headless mode runs one tick after the other without a clock. Both modes
// run every tick through the same function, runOneTick(), which builds the tick's input action
// state and runs the seven tick phases of srs.fr.1.12 in order (srs.fr.1.13, 1.14).
class Application {
public:
    // An application that can only run headless: it cannot reach a window or a renderer (R-6).
    explicit Application(ISimulation& simulation);
    // An application for normal mode. A headless run of it never calls `createWindow` or
    // `frameRenderer`, so the graphics framework is not initialised (R-4). `realTime` replaces
    // the steady clock that measures render frames; tests use it to script time.
    Application(ISimulation& simulation, IFrameRenderer& frameRenderer, WindowFactory createWindow,
                FrameClock::TimeSource realTime = {});
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Checks the settings and, in normal mode, opens the window. Returns false, after writing
    // the reason to the standard error output, when the run cannot start.
    [[nodiscard]] bool init(const ApplicationConfig& config);

    // Runs the game loop until the user closes the window, `maxTicks` is reached or a tick throws
    // engine::RunError. The render frame in which the window is closed is finished first.
    // Returns the process exit code: 0 after a normal end (srs.fr.1.10); non-zero when init()
    // has not succeeded or the run ended with a run error (srs.fr.1.11).
    //
    // A RunError ends the run at the boundary of the tick that threw it: the loop writes the
    // error message and that tick's number to the standard error output, runs no further tick
    // and renders no further frame. The failed tick does not count as executed. Any other
    // exception is a bug and is not caught.
    [[nodiscard]] int run();

    // Closes the window and releases everything init() created. Calling it again does nothing;
    // the destructor calls it too.
    void shutdown();

    // The tick counter (srs.fr.1.5): 0 when a run starts, one more after every completed tick.
    // While a tick runs, this is that tick's number.
    [[nodiscard]] Tick ticksExecuted() const;
    // Simulated time of the run: the tick counter times 1/60 s (srs.fr.1.6).
    [[nodiscard]] double simulatedSeconds() const;
    // Why the last run ended; NotRun before the first run.
    [[nodiscard]] RunOutcome outcome() const;

    // The keyboard queue of normal mode (srs.fr.5.5). Keyboard input collection, step 1 of a
    // render frame, pushes the frame's action transitions here; the next tick takes them all.
    // Turning keys into actions is the Input Map's job (srs.fr.5.1, US-5.1).
    [[nodiscard]] KeyboardQueue& keyboardQueue();

private:
    void runHeadless();
    void runNormal();
    // Runs one tick: builds its input action state from the run's input source, then runs the
    // tick phases in order. Returns false, after logging the error, when the tick threw RunError.
    [[nodiscard]] bool runOneTick();
    [[nodiscard]] IInputSource& inputSource();
    [[nodiscard]] bool reachedMaxTicks() const;

    ISimulation& simulation_;
    IFrameRenderer* frameRenderer_ = nullptr;
    WindowFactory createWindow_;
    FrameClock frameClock_;
    ApplicationConfig config_;
    std::unique_ptr<IWindow> window_;
    KeyboardQueue keyboardQueue_;
    NoInputSource noInput_;
    // The input action state of the running or last completed tick; held actions carry over
    // from tick to tick (srs.fr.5.7).
    InputActionState inputState_;
    // Reused by every tick, so a tick allocates nothing once it has grown.
    std::vector<InputTransition> tickTransitions_;
    Tick ticksExecuted_ = 0;
    RunOutcome outcome_ = RunOutcome::NotRun;
    bool initialised_ = false;
};

} // namespace engine
