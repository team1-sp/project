#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "engine/core/frame_clock.hpp"
#include "engine/core/input_actions.hpp"
#include "engine/core/input_source.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/window.hpp"

namespace engine {

class FixedTimestep;
class IFrameRenderer;
class ILogSink;
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
    // Normal mode: asked each time the user asks to close the window (srs.fr.1.10). It returns
    // true to close the window and end the run, or false to keep running; the Editor applies its
    // unsaved-changes check here (srs.fr.14.3). Empty: the window always closes.
    std::function<bool()> confirmClose;
    // Optional structured logging destination. Not owned; it must outlive init() and run().
    // Without one, Application preserves the command-line stderr output used by headless runs.
    ILogSink* logSink = nullptr;
};

// Owns the application window and runs the game loop (srs.fr.1.1 to 1.18).
// Use: construct, init(), run(), shutdown(). The simulation and the frame renderer are not
// owned and must outlive the Application.
//
// Normal mode runs render frames in the order of srs.fr.1.1: each one collects the keyboard
// input, runs the ticks that are due, at most 5, then renders and presents the state the last
// completed tick left. While paused, a render frame runs only the steps asked for and adds no
// real time to the accumulator (srs.fr.1.16 to 1.18). Headless mode runs one tick after the
// other without a clock. Both modes run every tick through the same function, runOneTick(),
// which builds the tick's input action state and runs the seven tick phases of srs.fr.1.12 in
// order (srs.fr.1.13, 1.14).
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

    // Pause, resume and step (srs.fr.1.16 to 1.18, 17.12 to 17.14). They act only on a running
    // normal-mode run, in which Play mode's Pause and Step and Error Pause call them, from the
    // render frame's input collection or from inside a tick; they return false and change nothing
    // in headless mode or outside run(). A new run starts unpaused.
    //
    // Asks the game loop to pause at the next tick boundary (srs.fr.1.16): asked from inside a
    // tick, the pause takes effect when that tick has completed, even when the render frame had
    // more ticks to run. Then no further tick starts, the whole ticks of real time not simulated
    // yet are discarded and the part of a tick left over is kept. Render frames go on and keep
    // showing the last completed tick. Returns true when the run is paused or will pause.
    bool requestPause();
    // Ends a pause, or cancels a pause request that has not taken effect yet (srs.fr.1.18). Real
    // time is measured again from this moment and the kept part of a tick carries on, so no
    // paused time is simulated and the pause causes no "Simulation is running slower than real
    // time" warning. Returns false when the run was neither paused nor about to pause.
    bool resume();
    // While paused, asks for one more tick (srs.fr.17.14): the next render frame runs it, without
    // reading or changing the real-time accumulator (srs.fr.1.17), and renders its result. Each
    // call is one tick. Returns false, asking for nothing, when the run is not paused.
    bool requestStep();
    // True from the tick boundary at which a pause took effect until resume().
    [[nodiscard]] bool isPaused() const;

    // Ends a normal-mode run after the current render frame, as closing the window does, without
    // asking confirmClose: the caller has made its checks (File > Exit, srs.fr.15.15). The
    // outcome is WindowClosed. Returns false, changing nothing, outside a normal-mode run.
    bool requestClose();
    // Changes the title of the application window (srs.fr.14.2). Returns false when no window is
    // open: in headless mode or before init().
    bool setWindowTitle(const std::string& title);

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
    // Step (2) of a running and of a paused render frame.
    [[nodiscard]] bool runDueTicks(FixedTimestep& timestep, std::chrono::nanoseconds frameTime);
    [[nodiscard]] bool runRequestedSteps();
    // Makes a pending pause request take effect at this tick boundary.
    void enterPause();

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
    // True while run() runs the normal-mode loop: only then can it pause.
    bool runningNormal_ = false;
    bool closeRequested_ = false;
    bool pauseRequested_ = false;
    bool paused_ = false;
    // Steps asked for while paused and not run yet.
    int stepsRequested_ = 0;
    // The real time between the last frame measurement and resume(): it was paused, so it is
    // taken off the next frame time before the accumulator gets it (srs.fr.1.18).
    std::chrono::nanoseconds pausedSinceFrameStart_{};
};

} // namespace engine
