// Frame clock tests (PB-012.1). All but the last use a scripted time source instead of the
// real clock.
#include <chrono>

#include <gtest/gtest.h>

#include "engine/core/frame_clock.hpp"

namespace {

using namespace std::chrono_literals;
using std::chrono::nanoseconds;

} // namespace

TEST(FrameClock, BeforeFirstFrame_FrameTimeAndTotalTimeAreZero) {
    nanoseconds now = 5s;
    const engine::FrameClock clock([&now] { return now; });

    EXPECT_EQ(clock.frameTime(), 0ns);
    EXPECT_EQ(clock.totalTime(), 0ns);
}

TEST(FrameClock, MeasureFrame_ReturnsRealTimeSinceThePreviousFrame) {
    nanoseconds now = 5s;
    engine::FrameClock clock([&now] { return now; });

    now += 16ms;
    const nanoseconds first = clock.measureFrame();
    now += 33ms;
    const nanoseconds second = clock.measureFrame();

    EXPECT_EQ(first, 16ms);
    EXPECT_EQ(second, 33ms);
    EXPECT_EQ(clock.frameTime(), 33ms);
}

TEST(FrameClock, TotalTime_IsTheSumOfTheFrameTimes) {
    nanoseconds now = 5s;
    engine::FrameClock clock([&now] { return now; });

    now += 16ms;
    clock.measureFrame();
    now += 33ms;
    clock.measureFrame();

    EXPECT_EQ(clock.totalTime(), 49ms);
}

TEST(FrameClock, Start_MeasuresFromThatMomentAndClearsTheTotal) {
    nanoseconds now = 5s;
    engine::FrameClock clock([&now] { return now; });
    now += 2s;
    clock.measureFrame();

    now += 1s; // time that passes before the restart is not part of the next frame
    clock.start();
    now += 16ms;
    const nanoseconds frame = clock.measureFrame();

    EXPECT_EQ(frame, 16ms);
    EXPECT_EQ(clock.totalTime(), 16ms);
}

TEST(FrameClock, SourceStepsBackwards_FrameTimeIsZero) {
    nanoseconds now = 5s;
    engine::FrameClock clock([&now] { return now; });

    now -= 1s;
    const nanoseconds frame = clock.measureFrame();

    EXPECT_EQ(frame, 0ns);
    EXPECT_EQ(clock.totalTime(), 0ns);
}

// The default source is std::chrono::steady_clock. Its values are not predictable, only that
// it never runs backwards.
TEST(FrameClock, SteadyClock_FrameTimesAreNeverNegative) {
    engine::FrameClock clock;

    for (int frame = 0; frame < 100; ++frame) {
        EXPECT_GE(clock.measureFrame(), 0ns);
    }
    EXPECT_GE(clock.totalTime(), 0ns);
}
