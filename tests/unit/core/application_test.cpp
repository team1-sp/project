// Game loop tests (PB-011, PB-012, US-1.1 to US-1.4, US-14.1; srs.fr.1.1 to 1.18, 7.1, 7.7, 14.2,
// 14.3). A fake
// window, simulation and renderer write to one call log, and the fake window moves a scripted
// clock, so the tests check the order of calls and the ticks per render frame without a real window
// or real time.
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "engine/core/application.hpp"
#include "engine/core/frame_renderer.hpp"
#include "engine/core/input_actions.hpp"
#include "engine/core/input_source.hpp"
#include "engine/core/run_error.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/core/window.hpp"
#include "even_frame_times.hpp"

namespace {

using namespace std::chrono_literals;
using std::chrono::nanoseconds;

using CallLog = std::vector<std::string>;
using FrameTimes = std::vector<nanoseconds>;

using engine::InputAction;
using engine::InputTransition;
using engine::TickPhase;

// One tick phase as the simulation saw it.
struct PhaseRecord {
    engine::Tick tick = 0;
    TickPhase phase = TickPhase::InputApplication;
    engine::InputActionState input;

    friend bool operator==(const PhaseRecord&, const PhaseRecord&) = default;
};

// Writes "tick N" to the call log when tick N starts, and keeps every phase it runs.
class FakeSimulation final : public engine::ISimulation {
public:
    explicit FakeSimulation(CallLog& log)
        : log_(log) {}

    void runPhase(TickPhase phase, const engine::TickContext& context) override {
        if (phase == TickPhase::InputApplication) {
            log_.push_back("tick " + std::to_string(context.tick));
        }
        phases.push_back({context.tick, phase, context.input});
        if (phase == TickPhase::InputApplication && onTick) {
            onTick();
        }
        if (onPhase) {
            onPhase(phase);
        }
    }

    // Called in the first phase of every tick, for tests that look at the application while a
    // tick runs.
    std::function<void()> onTick;
    // Called in every phase, after it is recorded.
    std::function<void(TickPhase)> onPhase;
    std::vector<PhaseRecord> phases;

private:
    CallLog& log_;
};

// An input source with a fixed list of transitions per tick, like a recording.
class ScriptedInputSource final : public engine::IInputSource {
public:
    explicit ScriptedInputSource(std::map<engine::Tick, std::vector<InputTransition>> script)
        : script_(std::move(script)) {}

    void takeTransitions(engine::Tick tick, std::vector<InputTransition>& transitions) override {
        const auto entry = script_.find(tick);
        if (entry != script_.end()) {
            transitions.insert(transitions.end(), entry->second.begin(), entry->second.end());
        }
    }

private:
    std::map<engine::Tick, std::vector<InputTransition>> script_;
};

class FakeFrameRenderer final : public engine::IFrameRenderer {
public:
    explicit FakeFrameRenderer(CallLog& log)
        : log_(log) {}

    void renderFrame() override {
        log_.push_back("render");
        if (onRender) {
            onRender();
        }
    }
    void releaseGraphics() override { log_.push_back("release"); }

    std::function<void()> onRender;

private:
    CallLog& log_;
};

// A window that plays a script of render frame times. Presenting frame k moves the scripted
// clock on by frameTimes[k], as if the frame had taken that long, so the loop sees that time
// at the start of frame k + 1. The user closes the window during the frame after the script,
// which makes a run frameTimes.size() + 1 render frames long.
class FakeWindow final : public engine::IWindow {
public:
    // `onCollectInput`, if set, is called with the render frame's index in collectInput(), where
    // a real window would push the frame's keyboard transitions.
    FakeWindow(CallLog& log, nanoseconds& realTime, FrameTimes frameTimes,
               std::function<void(std::size_t)> onCollectInput = {})
        : log_(log)
        , realTime_(realTime)
        , frameTimes_(std::move(frameTimes))
        , onCollectInput_(std::move(onCollectInput)) {}

    ~FakeWindow() override { log_.push_back("close"); }

    [[nodiscard]] bool shouldClose() const override {
        return framesPresented_ > frameTimes_.size();
    }
    void collectInput() override {
        log_.push_back("input");
        if (onCollectInput_) {
            onCollectInput_(framesPresented_);
        }
    }
    void beginFrame() override { log_.push_back("begin"); }
    void setTitle(const std::string& title) override { log_.push_back("title " + title); }
    void presentFrame() override {
        log_.push_back("present");
        if (framesPresented_ < frameTimes_.size()) {
            realTime_ += frameTimes_[framesPresented_];
        }
        ++framesPresented_;
    }

private:
    CallLog& log_;
    nanoseconds& realTime_;
    FrameTimes frameTimes_;
    std::function<void(std::size_t)> onCollectInput_;
    std::size_t framesPresented_ = 0;
};

// The fakes, the call log and the scripted clock every test needs.
class ApplicationTest : public testing::Test {
protected:
    engine::WindowFactory windowFactory(FrameTimes frameTimes) {
        return [this, frameTimes = std::move(frameTimes)](
                   const engine::WindowConfig&) -> std::unique_ptr<engine::IWindow> {
            log_.push_back("open");
            return std::make_unique<FakeWindow>(log_, realTime_, frameTimes,
                                                [this](std::size_t frame) {
                                                    if (onCollectInput) {
                                                        onCollectInput(frame);
                                                    }
                                                });
        };
    }

    engine::FrameClock::TimeSource timeSource() {
        return [this] { return realTime_; };
    }

    // Called in step 1 of every render frame with the frame's index.
    std::function<void(std::size_t)> onCollectInput;
    CallLog log_;
    nanoseconds realTime_{0};
    FakeSimulation simulation_{log_};
    FakeFrameRenderer renderer_{log_};
};

engine::ApplicationConfig headlessConfig(engine::Tick maxTicks) {
    engine::ApplicationConfig config;
    config.headless = true;
    config.maxTicks = maxTicks;
    return config;
}

} // namespace

// srs.fr.1.1: every render frame collects input, runs the due ticks, renders, then presents.
// 50 ms is exactly 3 ticks.
TEST_F(ApplicationTest, NormalModeFrame_CollectsInputThenRunsDueTicksThenRendersThenPresents) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms}), timeSource());
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    const int exitCode = app.run();

    EXPECT_EQ(exitCode, 0);
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present", "input", "tick 0",
                             "tick 1", "tick 2", "begin", "render", "present"}));
}

// srs.fr.1.15: a render frame that runs no tick still renders and presents. 10 ms is 0.6 of a
// tick, so the first tick is due in the third frame.
TEST_F(ApplicationTest, FrameWithNoTickDue_StillRendersAndPresents) {
    engine::Application app(simulation_, renderer_, windowFactory({10ms, 10ms}), timeSource());
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(log_,
              (CallLog{"open", "input", "begin", "render", "present", "input", "begin", "render",
                       "present", "input", "tick 0", "begin", "render", "present"}));
}

// srs.fr.1.3, 1.4: at most 5 ticks per render frame, however long the frame before it took.
TEST_F(ApplicationTest, FiveSecondFrame_RunsFiveTicksInTheNextFrame) {
    engine::Application app(simulation_, renderer_, windowFactory({5s}), timeSource());
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(app.ticksExecuted(), 5u);
}

// PB-012: 60 ticks are 1.0 s, at any render frame rate (srs.fr.1.2, 1.6).
TEST_F(ApplicationTest, OneSecondAtSteadyFrameRates_RunsSixtyTicks) {
    for (const std::int64_t framesPerSecond : {30, 60, 144}) {
        SCOPED_TRACE(framesPerSecond);
        engine::Application app(simulation_, renderer_,
                                windowFactory(engine_tests::evenFrameTimes(1s, framesPerSecond)),
                                timeSource());
        ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

        ASSERT_EQ(app.run(), 0);

        EXPECT_EQ(app.ticksExecuted(), 60u);
        EXPECT_DOUBLE_EQ(app.simulatedSeconds(), 1.0);
    }
}

// A tick limit ends a normal-mode run inside the render frame, which is still rendered.
TEST_F(ApplicationTest, NormalModeWithMaxTicks_StopsAfterThatManyTicks) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms, 50ms, 50ms}),
                            timeSource());
    engine::ApplicationConfig config;
    config.maxTicks = 2;
    ASSERT_TRUE(app.init(config));

    const int exitCode = app.run();

    EXPECT_EQ(exitCode, 0);
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present", "input", "tick 0",
                             "tick 1", "begin", "render", "present"}));
}

// srs.fr.1.10: closing the window ends the run with exit code 0 and releases the window.
TEST_F(ApplicationTest, WindowClosedByUser_RunReturnsZeroAndShutdownClosesWindow) {
    engine::Application app(simulation_, renderer_, windowFactory({}), timeSource());
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    const int exitCode = app.run();
    app.shutdown();

    EXPECT_EQ(exitCode, 0);
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present", "release", "close"}));
}

// The renderer releases what it loaded through the window right before the window closes.
TEST_F(ApplicationTest, ShutdownCalledTwice_ClosesWindowOnce) {
    engine::Application app(simulation_, renderer_, windowFactory({}), timeSource());
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    app.shutdown();
    app.shutdown();

    EXPECT_EQ(log_, (CallLog{"open", "release", "close"}));
}

TEST_F(ApplicationTest, DestroyedWithoutShutdown_ClosesWindow) {
    {
        engine::Application app(simulation_, renderer_, windowFactory({}), timeSource());
        ASSERT_TRUE(app.init(engine::ApplicationConfig{}));
    }

    EXPECT_EQ(log_, (CallLog{"open", "release", "close"}));
}

TEST_F(ApplicationTest, WindowConfigGiven_FactoryReceivesIt) {
    engine::WindowConfig received;
    engine::Application app(
        simulation_, renderer_,
        [&](const engine::WindowConfig& config) -> std::unique_ptr<engine::IWindow> {
            received = config;
            return std::make_unique<FakeWindow>(log_, realTime_, FrameTimes{});
        });
    engine::ApplicationConfig config;
    config.window.widthPixels = 1280;
    config.window.heightPixels = 720;
    config.window.title = "test window";
    config.window.targetFramesPerSecond = 144;
    config.window.fullscreen = true;

    ASSERT_TRUE(app.init(config));

    EXPECT_EQ(received.widthPixels, 1280);
    EXPECT_EQ(received.heightPixels, 720);
    EXPECT_EQ(received.title, "test window");
    EXPECT_EQ(received.targetFramesPerSecond, 144);
    EXPECT_TRUE(received.fullscreen);
}

// srs.fr.1.7, 7.1, 7.7: a headless run opens no window, renders nothing and stops at maxTicks.
// srs.fr.1.5: tick numbers start at 0 and go up by one.
TEST_F(ApplicationTest, HeadlessMode_RunsMaxTicksWithoutWindowOrRendering) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms}), timeSource());
    ASSERT_TRUE(app.init(headlessConfig(3)));

    const int exitCode = app.run();
    app.shutdown();

    EXPECT_EQ(exitCode, 0);
    EXPECT_EQ(log_, (CallLog{"tick 0", "tick 1", "tick 2"}));
    EXPECT_EQ(app.ticksExecuted(), 3u);
}

TEST_F(ApplicationTest, HeadlessOnlyApplication_RunsTicks) {
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(headlessConfig(2)));

    EXPECT_EQ(app.run(), 0);
    EXPECT_EQ(log_, (CallLog{"tick 0", "tick 1"}));
}

// srs.fr.1.6: simulated time is the tick counter times 1/60 s.
TEST_F(ApplicationTest, HeadlessSixtyTicks_SimulatedTimeIsOneSecond) {
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(headlessConfig(60)));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(app.ticksExecuted(), 60u);
    EXPECT_DOUBLE_EQ(app.simulatedSeconds(), 1.0);
}

// srs.fr.1.5: the counter goes up after a tick has completed, so during tick N it reads N.
TEST_F(ApplicationTest, DuringATick_TickCounterIsThatTicksNumber) {
    engine::Application app(simulation_);
    std::vector<engine::Tick> counterDuringTick;
    simulation_.onTick = [&] { counterDuringTick.push_back(app.ticksExecuted()); };
    ASSERT_TRUE(app.init(headlessConfig(3)));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(counterDuringTick, (std::vector<engine::Tick>{0, 1, 2}));
}

// srs.fr.1.5: the counter is 0 when a run starts, also for a second run.
TEST_F(ApplicationTest, SecondRun_TickCounterStartsAtZeroAgain) {
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(headlessConfig(2)));

    ASSERT_EQ(app.run(), 0);
    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(log_, (CallLog{"tick 0", "tick 1", "tick 0", "tick 1"}));
}

TEST_F(ApplicationTest, HeadlessWithoutMaxTicks_InitFails) {
    engine::Application app(simulation_);

    EXPECT_FALSE(app.init(headlessConfig(0)));
}

TEST_F(ApplicationTest, NormalModeWithoutWindowFactory_InitFails) {
    engine::Application app(simulation_);

    EXPECT_FALSE(app.init(engine::ApplicationConfig{}));
}

TEST_F(ApplicationTest, WindowCannotBeOpened_InitFails) {
    engine::Application app(
        simulation_, renderer_,
        [](const engine::WindowConfig&) -> std::unique_ptr<engine::IWindow> { return nullptr; });

    EXPECT_FALSE(app.init(engine::ApplicationConfig{}));
}

TEST_F(ApplicationTest, InitCalledTwice_SecondCallFails) {
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(headlessConfig(1)));

    EXPECT_FALSE(app.init(headlessConfig(1)));
}

TEST_F(ApplicationTest, RunBeforeInit_ReturnsErrorCodeAndRunsNoTick) {
    engine::Application app(simulation_);

    EXPECT_NE(app.run(), 0);
    EXPECT_TRUE(log_.empty());
}

// srs.fr.1.11: a run error ends a headless run at the failed tick's boundary with a non-zero exit
// code and the outcome Error. The failed tick is not counted as executed (srs.fr.1.5).
TEST_F(ApplicationTest, HeadlessTickThrowsRunError_RunStopsAtThatTickWithOutcomeError) {
    engine::Application app(simulation_);
    simulation_.onTick = [&] {
        if (app.ticksExecuted() == 2) {
            throw engine::RunError("invalid entity handle");
        }
    };
    ASSERT_TRUE(app.init(headlessConfig(10)));

    const int exitCode = app.run();

    EXPECT_NE(exitCode, 0);
    EXPECT_EQ(app.outcome(), engine::RunOutcome::Error);
    EXPECT_EQ(log_, (CallLog{"tick 0", "tick 1", "tick 2"}));
    EXPECT_EQ(app.ticksExecuted(), 2u);
}

// srs.fr.1.11: the log gets the error message and the number of the tick that was running.
TEST_F(ApplicationTest, RunError_LogsMessageAndTickNumber) {
    engine::Application app(simulation_);
    simulation_.onTick = [&] {
        if (app.ticksExecuted() == 7) {
            throw engine::RunError("PlayerMovement: entity 12 has no Velocity component");
        }
    };
    ASSERT_TRUE(app.init(headlessConfig(10)));

    testing::internal::CaptureStderr();
    (void)app.run();
    const std::string logged = testing::internal::GetCapturedStderr();

    EXPECT_EQ(logged, "error: tick 7: PlayerMovement: entity 12 has no Velocity component\n");
}

// srs.fr.1.11: in normal mode the run also ends at the failed tick's boundary. The frame is not
// rendered or presented; shutdown() still closes the window.
TEST_F(ApplicationTest, NormalModeTickThrowsRunError_StopsWithoutRenderingThatFrame) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms, 50ms}), timeSource());
    simulation_.onTick = [&] {
        if (app.ticksExecuted() == 1) {
            throw engine::RunError("access check failed");
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    const int exitCode = app.run();
    app.shutdown();

    EXPECT_NE(exitCode, 0);
    EXPECT_EQ(app.outcome(), engine::RunOutcome::Error);
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present", "input", "tick 0",
                             "tick 1", "release", "close"}));
}

TEST_F(ApplicationTest, RunErrorInFirstRun_SecondRunStartsAgainAndCanEndNormally) {
    engine::Application app(simulation_);
    bool failNextTick = true;
    simulation_.onTick = [&] {
        if (failNextTick) {
            failNextTick = false;
            throw engine::RunError("failed once");
        }
    };
    ASSERT_TRUE(app.init(headlessConfig(2)));

    EXPECT_NE(app.run(), 0);
    EXPECT_EQ(app.run(), 0);

    EXPECT_EQ(app.outcome(), engine::RunOutcome::MaxTicks);
    EXPECT_EQ(log_, (CallLog{"tick 0", "tick 0", "tick 1"}));
}

TEST_F(ApplicationTest, BeforeRun_OutcomeIsNotRun) {
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(headlessConfig(1)));

    EXPECT_EQ(app.outcome(), engine::RunOutcome::NotRun);
}

// srs.fr.7.7: a headless run that reaches its maximum ends with the outcome Max ticks.
TEST_F(ApplicationTest, HeadlessRunReachesMaxTicks_OutcomeIsMaxTicks) {
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(headlessConfig(3)));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(app.outcome(), engine::RunOutcome::MaxTicks);
}

// srs.fr.1.10: closing the window is a normal end, with its own outcome.
TEST_F(ApplicationTest, WindowClosedByUser_OutcomeIsWindowClosed) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms}), timeSource());
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(app.outcome(), engine::RunOutcome::WindowClosed);
}

TEST_F(ApplicationTest, NormalModeWithMaxTicks_OutcomeIsMaxTicks) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms, 50ms}), timeSource());
    engine::ApplicationConfig config;
    config.maxTicks = 2;
    ASSERT_TRUE(app.init(config));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(app.outcome(), engine::RunOutcome::MaxTicks);
}

TEST(RunOutcomeName, EveryOutcome_HasItsReportName) {
    EXPECT_STREQ(engine::runOutcomeName(engine::RunOutcome::NotRun), "Not run");
    EXPECT_STREQ(engine::runOutcomeName(engine::RunOutcome::WindowClosed), "Window closed");
    EXPECT_STREQ(engine::runOutcomeName(engine::RunOutcome::MaxTicks), "Max ticks");
    EXPECT_STREQ(engine::runOutcomeName(engine::RunOutcome::Error), "Error");
}

// US-1.3 (srs.fr.1.12 to 1.15): the tick phases and the input of every tick.

namespace {

constexpr InputTransition down(InputAction action) {
    return {action, true};
}

constexpr InputTransition up(InputAction action) {
    return {action, false};
}

// The phases tick `tick` runs, in order.
std::vector<TickPhase> phasesOfTick(const std::vector<PhaseRecord>& records, engine::Tick tick) {
    std::vector<TickPhase> phases;
    for (const PhaseRecord& record : records) {
        if (record.tick == tick) {
            phases.push_back(record.phase);
        }
    }
    return phases;
}

const std::vector<TickPhase> allPhasesInOrder(engine::tickPhases.begin(), engine::tickPhases.end());

} // namespace

// srs.fr.1.12: every tick runs the seven tick phases, in their fixed order, one tick after the
// other.
TEST_F(ApplicationTest, HeadlessTick_RunsTheSevenPhasesInOrder) {
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(headlessConfig(2)));

    ASSERT_EQ(app.run(), 0);

    ASSERT_EQ(simulation_.phases.size(), 2 * engine::tickPhaseCount);
    EXPECT_EQ(phasesOfTick(simulation_.phases, 0), allPhasesInOrder);
    EXPECT_EQ(phasesOfTick(simulation_.phases, 1), allPhasesInOrder);
    EXPECT_EQ(simulation_.phases[engine::tickPhaseCount - 1].tick, 0u);
    EXPECT_EQ(simulation_.phases[engine::tickPhaseCount].tick, 1u);
}

TEST_F(ApplicationTest, NormalModeTick_RunsTheSevenPhasesInOrder) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms}), timeSource());
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    ASSERT_EQ(simulation_.phases.size(), 3 * engine::tickPhaseCount);
    for (const engine::Tick tick : {0u, 1u, 2u}) {
        EXPECT_EQ(phasesOfTick(simulation_.phases, tick), allPhasesInOrder);
    }
}

// The goal of US-1.3: with the same input source, a headless run and a normal-mode run give the
// simulation exactly the same phases, ticks and input states, whatever the render frames do.
TEST_F(ApplicationTest, SameInputSource_HeadlessAndNormalModeRunIdenticalPhases) {
    const std::map<engine::Tick, std::vector<InputTransition>> script = {
        {0, {down(InputAction::MoveRight)}},
        {3, {down(InputAction::Interact), up(InputAction::Interact)}},
        {4, {up(InputAction::MoveRight), down(InputAction::MoveUp)}},
        {8, {up(InputAction::MoveUp)}},
    };
    constexpr engine::Tick ticks = 10;

    ScriptedInputSource headlessInput(script);
    engine::ApplicationConfig headless = headlessConfig(ticks);
    headless.inputSource = &headlessInput;
    engine::Application headlessApp(simulation_);
    ASSERT_TRUE(headlessApp.init(headless));
    ASSERT_EQ(headlessApp.run(), 0);
    const std::vector<PhaseRecord> headlessPhases = simulation_.phases;

    // Uneven render frames: 0, 1, 3 and 5 ticks per frame.
    simulation_.phases.clear();
    ScriptedInputSource normalInput(script);
    engine::ApplicationConfig normal;
    normal.maxTicks = ticks;
    normal.inputSource = &normalInput;
    engine::Application normalApp(simulation_, renderer_, windowFactory({10ms, 10ms, 50ms, 5s, 5s}),
                                  timeSource());
    ASSERT_TRUE(normalApp.init(normal));
    ASSERT_EQ(normalApp.run(), 0);

    EXPECT_EQ(simulation_.phases.size(), ticks * engine::tickPhaseCount);
    EXPECT_EQ(simulation_.phases, headlessPhases);
}

// srs.fr.1.14: the input state is built before phase 1 and is the same in all seven phases.
TEST_F(ApplicationTest, InputState_IsTheSameInEveryPhaseOfATick) {
    ScriptedInputSource input({{0, {down(InputAction::MoveLeft)}}});
    engine::ApplicationConfig config = headlessConfig(1);
    config.inputSource = &input;
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(config));

    ASSERT_EQ(app.run(), 0);

    ASSERT_EQ(simulation_.phases.size(), engine::tickPhaseCount);
    for (const PhaseRecord& record : simulation_.phases) {
        EXPECT_TRUE(record.input.pressed(InputAction::MoveLeft));
        EXPECT_TRUE(record.input.held(InputAction::MoveLeft));
    }
}

// srs.fr.1.14, 5.6, 5.7: pressed and released last one tick; held lasts until an up transition.
TEST_F(ApplicationTest, HeldAction_StaysHeldInTicksWithoutTransitions) {
    ScriptedInputSource input(
        {{0, {down(InputAction::MoveDown)}}, {3, {up(InputAction::MoveDown)}}});
    engine::ApplicationConfig config = headlessConfig(5);
    config.inputSource = &input;
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(config));

    ASSERT_EQ(app.run(), 0);

    std::vector<bool> pressed;
    std::vector<bool> held;
    std::vector<bool> released;
    for (const PhaseRecord& record : simulation_.phases) {
        if (record.phase == TickPhase::InputApplication) {
            pressed.push_back(record.input.pressed(InputAction::MoveDown));
            held.push_back(record.input.held(InputAction::MoveDown));
            released.push_back(record.input.released(InputAction::MoveDown));
        }
    }
    EXPECT_EQ(pressed, (std::vector<bool>{true, false, false, false, false}));
    EXPECT_EQ(held, (std::vector<bool>{true, true, true, false, false}));
    EXPECT_EQ(released, (std::vector<bool>{false, false, false, true, false}));
}

// srs.fr.1.14 with srs.fr.5.5: in normal mode the keyboard transitions of render frames that run
// no tick stay queued, and the first tick afterwards consumes all of them, in order. 10 ms is
// 0.6 of a tick, so the first tick runs in the third frame.
TEST_F(ApplicationTest, KeyboardTransitionsOfFramesWithoutTick_GoToTheNextTick) {
    engine::Application app(simulation_, renderer_, windowFactory({10ms, 10ms, 10ms}),
                            timeSource());
    onCollectInput = [&](std::size_t frame) {
        if (frame == 0) {
            app.keyboardQueue().push(down(InputAction::Interact));
        } else if (frame == 1) {
            app.keyboardQueue().push(up(InputAction::Interact));
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    // Tick 0 consumed down, up: pressed and released but not held (srs.fr.5.6).
    ASSERT_FALSE(simulation_.phases.empty());
    const engine::InputActionState& tick0 = simulation_.phases.front().input;
    EXPECT_EQ(simulation_.phases.front().tick, 0u);
    EXPECT_TRUE(tick0.pressed(InputAction::Interact));
    EXPECT_TRUE(tick0.released(InputAction::Interact));
    EXPECT_FALSE(tick0.held(InputAction::Interact));
    EXPECT_TRUE(app.keyboardQueue().empty());
}

// srs.fr.5.5: when one render frame runs several ticks, only the first consumes the keyboard
// transitions.
TEST_F(ApplicationTest, FrameWithSeveralTicks_OnlyTheFirstTickGetsTheKeyboardTransitions) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms}), timeSource());
    onCollectInput = [&](std::size_t frame) {
        if (frame == 1) {
            app.keyboardQueue().push(down(InputAction::MoveUp));
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    std::vector<bool> pressed;
    std::vector<bool> held;
    for (const PhaseRecord& record : simulation_.phases) {
        if (record.phase == TickPhase::InputApplication) {
            pressed.push_back(record.input.pressed(InputAction::MoveUp));
            held.push_back(record.input.held(InputAction::MoveUp));
        }
    }
    EXPECT_EQ(pressed, (std::vector<bool>{true, false, false}));
    EXPECT_EQ(held, (std::vector<bool>{true, true, true}));
}

// srs.fr.5.11: while another input source is active, the keyboard is ignored.
TEST_F(ApplicationTest, NormalModeWithAnotherSource_IgnoresTheKeyboard) {
    ScriptedInputSource input({});
    engine::Application app(simulation_, renderer_, windowFactory({10ms, 50ms}), timeSource());
    onCollectInput = [&](std::size_t /*frame*/) {
        app.keyboardQueue().push(down(InputAction::MoveRight));
    };
    engine::ApplicationConfig config;
    config.inputSource = &input;
    ASSERT_TRUE(app.init(config));

    ASSERT_EQ(app.run(), 0);

    ASSERT_FALSE(simulation_.phases.empty());
    for (const PhaseRecord& record : simulation_.phases) {
        EXPECT_FALSE(record.input.held(InputAction::MoveRight));
    }
    EXPECT_TRUE(app.keyboardQueue().empty());
}

// srs.fr.7.3: a headless run without a source uses None, never the keyboard.
TEST_F(ApplicationTest, HeadlessWithoutSource_TicksGetNoInput) {
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(headlessConfig(2)));
    app.keyboardQueue().push(down(InputAction::MoveLeft));
    simulation_.onTick = [&] { app.keyboardQueue().push(down(InputAction::MoveLeft)); };

    ASSERT_EQ(app.run(), 0);

    for (const PhaseRecord& record : simulation_.phases) {
        EXPECT_EQ(record.input, engine::InputActionState{});
    }
}

// A second run starts with no action held.
TEST_F(ApplicationTest, SecondRun_StartsWithNoActionHeld) {
    ScriptedInputSource input({{0, {down(InputAction::MoveRight)}}});
    engine::ApplicationConfig config = headlessConfig(1);
    config.inputSource = &input;
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(config));
    ASSERT_EQ(app.run(), 0);
    ASSERT_TRUE(simulation_.phases.back().input.held(InputAction::MoveRight));

    // A source with no transitions: anything still held would have come from the first run.
    ScriptedInputSource none({});
    config.inputSource = &none;
    app.shutdown();
    ASSERT_TRUE(app.init(config));
    simulation_.phases.clear();
    ASSERT_EQ(app.run(), 0);

    EXPECT_FALSE(simulation_.phases.back().input.held(InputAction::MoveRight));
}

// srs.fr.1.11 with srs.fr.1.12: a phase that throws a run error ends the tick there; the later
// phases of that tick do not run and the tick is not counted.
TEST_F(ApplicationTest, PhaseThrowsRunError_LaterPhasesOfThatTickDoNotRun) {
    engine::Application app(simulation_);
    simulation_.onPhase = [&](TickPhase phase) {
        if (app.ticksExecuted() == 1 && phase == TickPhase::PhysicsSyncOut) {
            throw engine::RunError("PhysicsSyncOut: body of entity 3 is missing");
        }
    };
    ASSERT_TRUE(app.init(headlessConfig(5)));

    testing::internal::CaptureStderr();
    EXPECT_NE(app.run(), 0);
    const std::string logged = testing::internal::GetCapturedStderr();

    EXPECT_EQ(logged, "error: tick 1: PhysicsSyncOut: body of entity 3 is missing\n");
    EXPECT_EQ(app.ticksExecuted(), 1u);
    EXPECT_EQ(phasesOfTick(simulation_.phases, 1),
              (std::vector<TickPhase>{TickPhase::InputApplication, TickPhase::GameplayInput,
                                      TickPhase::PhysicsSyncIn, TickPhase::PhysicsStep,
                                      TickPhase::PhysicsSyncOut}));
}

// srs.fr.1.15: a render frame draws the state of the last completed tick: every tick that ran
// before it has finished all seven phases, also in frames that run no tick.
TEST_F(ApplicationTest, RenderFrame_SeesOnlyCompletedTicks) {
    engine::Application app(simulation_, renderer_, windowFactory({10ms, 50ms, 10ms}),
                            timeSource());
    std::vector<std::size_t> phasesSeenByFrame;
    renderer_.onRender = [&] {
        phasesSeenByFrame.push_back(simulation_.phases.size());
        if (!simulation_.phases.empty()) {
            EXPECT_EQ(simulation_.phases.back().phase, TickPhase::TelemetryRecording);
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    // Frames run 0, 0, 3 and 1 ticks (10 ms carries over 0.6 of a tick).
    EXPECT_EQ(phasesSeenByFrame, (std::vector<std::size_t>{0, 0, 3 * engine::tickPhaseCount,
                                                           4 * engine::tickPhaseCount}));
}

// ---- Pause, resume and step (US-1.4; srs.fr.1.16 to 1.18, 17.12 to 17.14) ----

// srs.fr.1.16, 24.6: a pause asked for during a tick takes effect when that tick has completed,
// even though the render frame had more ticks to run. 80 ms is 4.8 ticks, so the second frame
// would run 4.
TEST_F(ApplicationTest, PauseRequestedDuringATick_NoFurtherTickStarts) {
    engine::Application app(simulation_, renderer_, windowFactory({80ms, 80ms, 80ms}),
                            timeSource());
    simulation_.onTick = [&] {
        if (app.ticksExecuted() == 1) {
            EXPECT_TRUE(app.requestPause());
            EXPECT_FALSE(app.isPaused());
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    EXPECT_TRUE(app.isPaused());
    EXPECT_EQ(app.ticksExecuted(), 2u);
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present", "input", "tick 0",
                             "tick 1", "begin", "render", "present", "input", "begin", "render",
                             "present", "input", "begin", "render", "present"}));
}

// srs.fr.1.16 to 1.18 together. The first frame leaves half a tick (25 ms is 1.5 ticks). The
// pause, asked for before the third frame's ticks, discards the 3 whole ticks due then and keeps
// the half. A second of paused time adds nothing and does not warn. After the resume, 10 ms
// (0.6 of a tick) plus the kept half make exactly one tick.
TEST_F(ApplicationTest, PauseAndResume_DiscardWholeTicksKeepTheFractionAndSimulateNoPausedTime) {
    engine::Application app(simulation_, renderer_, windowFactory({25ms, 50ms, 1s, 10ms}),
                            timeSource());
    onCollectInput = [&](std::size_t frame) {
        if (frame == 2) {
            EXPECT_TRUE(app.requestPause());
        }
        if (frame == 3) {
            EXPECT_TRUE(app.isPaused());
            EXPECT_TRUE(app.resume());
            EXPECT_FALSE(app.isPaused());
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    testing::internal::CaptureStderr();
    ASSERT_EQ(app.run(), 0);
    const std::string logged = testing::internal::GetCapturedStderr();

    EXPECT_EQ(logged, "");
    EXPECT_EQ(log_, (CallLog{"open",  "input",  "begin",  "render",  "present",    // frame 0
                             "input", "tick 0", "begin",  "render",  "present",    // frame 1
                             "input", "begin",  "render", "present",               // frame 2
                             "input", "begin",  "render", "present",               // frame 3
                             "input", "tick 1", "begin",  "render",  "present"})); // frame 4
}

// srs.fr.1.17: however long the pause lasts, no paused time reaches the accumulator; without
// the pause, 5 s would have run 5 ticks a frame and warned that the simulation is slow.
TEST_F(ApplicationTest, WhilePaused_LongFramesRunNoTickAndDoNotWarn) {
    engine::Application app(simulation_, renderer_, windowFactory({5s, 5s, 5s}), timeSource());
    onCollectInput = [&](std::size_t frame) {
        if (frame == 0) {
            EXPECT_TRUE(app.requestPause());
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    testing::internal::CaptureStderr();
    ASSERT_EQ(app.run(), 0);
    const std::string logged = testing::internal::GetCapturedStderr();

    EXPECT_EQ(logged, "");
    EXPECT_EQ(app.ticksExecuted(), 0u);
    EXPECT_TRUE(app.isPaused());
}

// srs.fr.1.18: the real time between the last frame measurement and the resume was paused. Here
// the resume comes during rendering, 1 s after the frame's measurement, and the frame after it
// takes 1 s plus 10 ms: only the 10 ms (0.6 of a tick) counts, so no tick is due yet.
TEST_F(ApplicationTest, ResumeDuringAFrame_PausedPartOfTheNextFrameTimeIsNotSimulated) {
    engine::Application app(simulation_, renderer_, windowFactory({10ms, 10ms, 10ms}),
                            timeSource());
    std::size_t framesRendered = 0;
    onCollectInput = [&](std::size_t frame) {
        if (frame == 0) {
            EXPECT_TRUE(app.requestPause());
        }
    };
    renderer_.onRender = [&] {
        if (framesRendered++ == 1) {
            realTime_ += 1s;
            EXPECT_TRUE(app.resume());
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    // Frame 2 counts 10 ms (0.6 of a tick), frame 3 another 10 ms: one tick, in frame 3.
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present",      // frame 0
                             "input", "begin", "render", "present",              // frame 1, resume
                             "input", "begin", "render", "present",              // frame 2
                             "input", "tick 0", "begin", "render", "present"})); // frame 3
}

// srs.fr.17.14, 1.17: Step while paused runs exactly one tick in the next render frame, renders
// it and leaves the accumulator as it is: after the resume, the half tick kept by the pause and
// 10 ms make one more tick, as without the step.
TEST_F(ApplicationTest, StepWhilePaused_RunsExactlyOneTickAndLeavesTheAccumulator) {
    engine::Application app(simulation_, renderer_, windowFactory({25ms, 1s, 1s, 1s, 10ms}),
                            timeSource());
    onCollectInput = [&](std::size_t frame) {
        if (frame == 1) {
            EXPECT_TRUE(app.requestPause());
        }
        if (frame == 2) {
            EXPECT_TRUE(app.requestStep());
        }
        if (frame == 4) {
            EXPECT_TRUE(app.resume());
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(log_, (CallLog{"open",  "input",  "begin",  "render",  "present",    // 0
                             "input", "begin",  "render", "present",               // 1
                             "input", "tick 0", "begin",  "render",  "present",    // 2
                             "input", "begin",  "render", "present",               // 3
                             "input", "begin",  "render", "present",               // 4
                             "input", "tick 1", "begin",  "render",  "present"})); // 5
    EXPECT_EQ(app.ticksExecuted(), 2u);
}

// srs.fr.17.14: each Step is one tick; two steps asked for in one frame run two ticks.
TEST_F(ApplicationTest, TwoStepsInOneFrame_RunTwoTicks) {
    engine::Application app(simulation_, renderer_, windowFactory({10ms, 10ms}), timeSource());
    onCollectInput = [&](std::size_t frame) {
        if (frame == 0) {
            EXPECT_TRUE(app.requestPause());
        }
        if (frame == 1) {
            EXPECT_TRUE(app.requestStep());
            EXPECT_TRUE(app.requestStep());
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(app.ticksExecuted(), 2u);
    EXPECT_TRUE(app.isPaused());
}

// Step and resume do nothing while the run is not paused; a resume before the pause takes
// effect cancels it, so the ticks run as if no pause had been asked for.
TEST_F(ApplicationTest, StepOrResumeWhileRunning_ChangeNothing) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms, 50ms}), timeSource());
    onCollectInput = [&](std::size_t frame) {
        EXPECT_FALSE(app.requestStep());
        if (frame == 1) {
            EXPECT_FALSE(app.resume());
            EXPECT_TRUE(app.requestPause());
            EXPECT_TRUE(app.resume());
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(app.ticksExecuted(), 6u);
    EXPECT_FALSE(app.isPaused());
}

// Pause belongs to Play mode, which is a normal-mode run: a headless run cannot be paused, and
// outside run() nothing can.
TEST_F(ApplicationTest, HeadlessRunOrNoRun_CannotBePaused) {
    engine::Application app(simulation_);
    simulation_.onTick = [&] { EXPECT_FALSE(app.requestPause()); };
    EXPECT_FALSE(app.requestPause());
    ASSERT_TRUE(app.init(headlessConfig(3)));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(app.ticksExecuted(), 3u);
    EXPECT_FALSE(app.isPaused());
    EXPECT_FALSE(app.requestPause());
}

// A step that reaches the run's maximum number of ticks ends the run like any other tick.
TEST_F(ApplicationTest, StepReachesMaxTicks_RunEndsWithMaxTicks) {
    engine::Application app(simulation_, renderer_, windowFactory({10ms, 10ms, 10ms}),
                            timeSource());
    onCollectInput = [&](std::size_t frame) {
        if (frame == 0) {
            EXPECT_TRUE(app.requestPause());
        }
        if (frame == 1) {
            EXPECT_TRUE(app.requestStep());
        }
    };
    engine::ApplicationConfig config;
    config.maxTicks = 1;
    ASSERT_TRUE(app.init(config));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(app.outcome(), engine::RunOutcome::MaxTicks);
    EXPECT_EQ(app.ticksExecuted(), 1u);
}

// A run that ended paused does not make the next run start paused.
TEST_F(ApplicationTest, SecondRun_StartsUnpaused) {
    engine::Application app(simulation_, renderer_, windowFactory({50ms}), timeSource());
    bool pauseInFirstRun = true;
    onCollectInput = [&](std::size_t frame) {
        if (frame == 0 && pauseInFirstRun) {
            EXPECT_TRUE(app.requestPause());
        }
    };
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));
    ASSERT_EQ(app.run(), 0);
    ASSERT_EQ(app.ticksExecuted(), 0u);
    app.shutdown();

    pauseInFirstRun = false;
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));
    ASSERT_EQ(app.run(), 0);

    EXPECT_FALSE(app.isPaused());
    EXPECT_EQ(app.ticksExecuted(), 3u);
}

// US-14.1, srs.fr.14.3: a close the application does not confirm is dropped and the run goes on;
// the window is asked again at the next close request. The fake window asks to close in every
// frame after its script, so two refusals add two render frames.
TEST_F(ApplicationTest, CloseNotConfirmed_KeepsRunningUntilConfirmed) {
    engine::Application app(simulation_, renderer_, windowFactory({10ms}), timeSource());
    engine::ApplicationConfig config;
    int confirmCalls = 0;
    config.confirmClose = [&confirmCalls] { return ++confirmCalls == 3; };
    ASSERT_TRUE(app.init(config));

    ASSERT_EQ(app.run(), 0);

    EXPECT_EQ(confirmCalls, 3);
    EXPECT_EQ(app.outcome(), engine::RunOutcome::WindowClosed);
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present", "input", "begin",
                             "render", "present", "input", "begin", "render", "present", "input",
                             "begin", "render", "present"}));
}

// US-14.1, srs.fr.15.15: requestClose() ends the run after the current render frame without
// asking confirmClose, which the caller has already applied.
TEST_F(ApplicationTest, RequestCloseDuringFrame_EndsRunAfterThatFrame) {
    engine::Application app(simulation_, renderer_, windowFactory({10ms, 10ms, 10ms}),
                            timeSource());
    engine::ApplicationConfig config;
    bool confirmAsked = false;
    config.confirmClose = [&confirmAsked] {
        confirmAsked = true;
        return true;
    };
    renderer_.onRender = [&app] { EXPECT_TRUE(app.requestClose()); };
    ASSERT_TRUE(app.init(config));

    ASSERT_EQ(app.run(), 0);

    EXPECT_FALSE(confirmAsked);
    EXPECT_EQ(app.outcome(), engine::RunOutcome::WindowClosed);
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present"}));
}

// requestClose() acts only during a normal-mode run.
TEST_F(ApplicationTest, RequestCloseOutsideNormalRun_ChangesNothing) {
    engine::Application app(simulation_, renderer_, windowFactory({10ms}), timeSource());
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));
    EXPECT_FALSE(app.requestClose());

    ASSERT_EQ(app.run(), 0);

    // The earlier call did not end the run early: both frames ran.
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present", "input", "begin",
                             "render", "present"}));
    EXPECT_FALSE(app.requestClose());
}

// US-14.1, srs.fr.14.2: the title reaches the window once it is open.
TEST_F(ApplicationTest, SetWindowTitle_ReachesOpenWindowOnly) {
    engine::Application app(simulation_, renderer_, windowFactory({}), timeSource());
    EXPECT_FALSE(app.setWindowTitle("before init"));
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    EXPECT_TRUE(app.setWindowTitle("Aegis Editor 0.1"));

    EXPECT_EQ(log_, (CallLog{"open", "title Aegis Editor 0.1"}));
}
