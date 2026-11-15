// Game loop tests (PB-011, PB-012, US-1.1, US-1.2; srs.fr.1.1 to 1.11, 1.15, 7.1, 7.7). A fake
// window, simulation and renderer write to one call log, and the fake window moves a scripted
// clock, so the tests check the order of calls and the ticks per render frame without a real window
// or real time.
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "engine/core/application.hpp"
#include "engine/core/frame_renderer.hpp"
#include "engine/core/run_error.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/window.hpp"
#include "even_frame_times.hpp"

namespace {

using namespace std::chrono_literals;
using std::chrono::nanoseconds;

using CallLog = std::vector<std::string>;
using FrameTimes = std::vector<nanoseconds>;

class FakeSimulation final : public engine::ISimulation {
public:
    explicit FakeSimulation(CallLog& log)
        : log_(log) {}

    void runTick(engine::Tick tick) override {
        log_.push_back("tick " + std::to_string(tick));
        if (onTick) {
            onTick();
        }
    }

    // Called inside every tick, for tests that look at the application while a tick runs.
    std::function<void()> onTick;

private:
    CallLog& log_;
};

class FakeFrameRenderer final : public engine::IFrameRenderer {
public:
    explicit FakeFrameRenderer(CallLog& log)
        : log_(log) {}

    void renderFrame() override { log_.push_back("render"); }

private:
    CallLog& log_;
};

// A window that plays a script of render frame times. Presenting frame k moves the scripted
// clock on by frameTimes[k], as if the frame had taken that long, so the loop sees that time
// at the start of frame k + 1. The user closes the window during the frame after the script,
// which makes a run frameTimes.size() + 1 render frames long.
class FakeWindow final : public engine::IWindow {
public:
    FakeWindow(CallLog& log, nanoseconds& realTime, FrameTimes frameTimes)
        : log_(log)
        , realTime_(realTime)
        , frameTimes_(std::move(frameTimes)) {}

    ~FakeWindow() override { log_.push_back("close"); }

    [[nodiscard]] bool shouldClose() const override {
        return framesPresented_ > frameTimes_.size();
    }
    void collectInput() override { log_.push_back("input"); }
    void beginFrame() override { log_.push_back("begin"); }
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
    std::size_t framesPresented_ = 0;
};

// The fakes, the call log and the scripted clock every test needs.
class ApplicationTest : public testing::Test {
protected:
    engine::WindowFactory windowFactory(FrameTimes frameTimes) {
        return [this, frameTimes = std::move(frameTimes)](
                   const engine::WindowConfig&) -> std::unique_ptr<engine::IWindow> {
            log_.push_back("open");
            return std::make_unique<FakeWindow>(log_, realTime_, frameTimes);
        };
    }

    engine::FrameClock::TimeSource timeSource() {
        return [this] { return realTime_; };
    }

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
    EXPECT_EQ(log_, (CallLog{"open", "input", "begin", "render", "present", "close"}));
}

TEST_F(ApplicationTest, ShutdownCalledTwice_ClosesWindowOnce) {
    engine::Application app(simulation_, renderer_, windowFactory({}), timeSource());
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    app.shutdown();
    app.shutdown();

    EXPECT_EQ(log_, (CallLog{"open", "close"}));
}

TEST_F(ApplicationTest, DestroyedWithoutShutdown_ClosesWindow) {
    {
        engine::Application app(simulation_, renderer_, windowFactory({}), timeSource());
        ASSERT_TRUE(app.init(engine::ApplicationConfig{}));
    }

    EXPECT_EQ(log_, (CallLog{"open", "close"}));
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
                             "tick 1", "close"}));
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
