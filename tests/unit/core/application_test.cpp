// Game loop tests (PB-011; srs.fr.1.1, 1.7, 1.10, 7.1, 7.7). A fake window, simulation and
// renderer write to one call log, so the tests check the order of calls without a real window.
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "engine/core/application.hpp"
#include "engine/core/frame_renderer.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/window.hpp"

namespace {

using CallLog = std::vector<std::string>;

class FakeSimulation final : public engine::ISimulation {
public:
    explicit FakeSimulation(CallLog& log)
        : log_(log) {}

    void runTick() override { log_.push_back("tick"); }

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

// A window whose user asks to close it once `framesUntilClose` frames have been presented.
class FakeWindow final : public engine::IWindow {
public:
    FakeWindow(CallLog& log, int framesUntilClose)
        : log_(log)
        , framesUntilClose_(framesUntilClose) {}

    ~FakeWindow() override { log_.push_back("close"); }

    [[nodiscard]] bool shouldClose() const override {
        return framesPresented_ >= framesUntilClose_;
    }
    void beginFrame() override { log_.push_back("begin"); }
    void endFrame() override {
        log_.push_back("present");
        ++framesPresented_;
    }

private:
    CallLog& log_;
    int framesUntilClose_;
    int framesPresented_ = 0;
};

engine::WindowFactory fakeWindowFactory(CallLog& log, int framesUntilClose) {
    return
        [&log, framesUntilClose](const engine::WindowConfig&) -> std::unique_ptr<engine::IWindow> {
            log.push_back("open");
            return std::make_unique<FakeWindow>(log, framesUntilClose);
        };
}

// The fakes and call log every test needs.
class ApplicationTest : public testing::Test {
protected:
    CallLog log_;
    FakeSimulation simulation_{log_};
    FakeFrameRenderer renderer_{log_};
};

engine::ApplicationConfig headlessConfig(std::uint64_t maxTicks) {
    engine::ApplicationConfig config;
    config.headless = true;
    config.maxTicks = maxTicks;
    return config;
}

} // namespace

// srs.fr.1.1: ticks, then render, then present, in every render frame.
TEST_F(ApplicationTest, NormalModeFrame_RunsTickThenRenderThenPresent) {
    engine::Application app(simulation_, renderer_, fakeWindowFactory(log_, 2));
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    const int exitCode = app.run();

    EXPECT_EQ(exitCode, 0);
    EXPECT_EQ(log_, (CallLog{"open", "tick", "begin", "render", "present", "tick", "begin",
                             "render", "present"}));
}

// srs.fr.1.10: closing the window ends the run with exit code 0 and releases the window.
TEST_F(ApplicationTest, WindowClosedByUser_RunReturnsZeroAndShutdownClosesWindow) {
    engine::Application app(simulation_, renderer_, fakeWindowFactory(log_, 1));
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    const int exitCode = app.run();
    app.shutdown();

    EXPECT_EQ(exitCode, 0);
    EXPECT_EQ(log_.back(), "close");
}

TEST_F(ApplicationTest, ShutdownCalledTwice_ClosesWindowOnce) {
    engine::Application app(simulation_, renderer_, fakeWindowFactory(log_, 1));
    ASSERT_TRUE(app.init(engine::ApplicationConfig{}));

    app.shutdown();
    app.shutdown();

    EXPECT_EQ(log_, (CallLog{"open", "close"}));
}

TEST_F(ApplicationTest, DestroyedWithoutShutdown_ClosesWindow) {
    {
        engine::Application app(simulation_, renderer_, fakeWindowFactory(log_, 1));
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
            return std::make_unique<FakeWindow>(log_, 0);
        });
    engine::ApplicationConfig config;
    config.window.widthPixels = 1280;
    config.window.heightPixels = 720;
    config.window.title = "test window";

    ASSERT_TRUE(app.init(config));

    EXPECT_EQ(received.widthPixels, 1280);
    EXPECT_EQ(received.heightPixels, 720);
    EXPECT_EQ(received.title, "test window");
}

// A normal-mode run with a tick limit ends after the render frame of its last tick.
TEST_F(ApplicationTest, NormalModeWithMaxTicks_EndsAfterThatManyTicks) {
    engine::Application app(simulation_, renderer_, fakeWindowFactory(log_, 100));
    engine::ApplicationConfig config;
    config.maxTicks = 2;
    ASSERT_TRUE(app.init(config));

    const int exitCode = app.run();

    EXPECT_EQ(exitCode, 0);
    EXPECT_EQ(log_, (CallLog{"open", "tick", "begin", "render", "present", "tick", "begin",
                             "render", "present"}));
}

// srs.fr.1.7, 7.1, 7.7: a headless run opens no window, renders nothing and stops at maxTicks.
TEST_F(ApplicationTest, HeadlessMode_RunsMaxTicksWithoutWindowOrRendering) {
    engine::Application app(simulation_, renderer_, fakeWindowFactory(log_, 100));
    ASSERT_TRUE(app.init(headlessConfig(3)));

    const int exitCode = app.run();
    app.shutdown();

    EXPECT_EQ(exitCode, 0);
    EXPECT_EQ(log_, (CallLog{"tick", "tick", "tick"}));
}

TEST_F(ApplicationTest, HeadlessOnlyApplication_RunsTicks) {
    engine::Application app(simulation_);
    ASSERT_TRUE(app.init(headlessConfig(2)));

    EXPECT_EQ(app.run(), 0);
    EXPECT_EQ(log_, (CallLog{"tick", "tick"}));
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
