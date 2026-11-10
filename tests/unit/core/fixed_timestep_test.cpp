// Fixed-timestep accumulator tests (PB-012.4; srs.fr.1.2 to 1.4). Real time is passed in as
// numbers, so nothing here reads a clock.
#include <chrono>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "engine/core/fixed_timestep.hpp"
#include "engine/core/tick.hpp"
#include "even_frame_times.hpp"

namespace {

using namespace std::chrono_literals;
using std::chrono::nanoseconds;

struct RunTotals {
    engine::Tick ticksRun = 0;
    engine::Tick ticksDropped = 0;
    int warnings = 0;
};

// Feeds the frame times one after the other, as the game loop does.
RunTotals feed(engine::FixedTimestep& timestep, const std::vector<nanoseconds>& frameTimes) {
    RunTotals totals;
    for (const nanoseconds frameTime : frameTimes) {
        const engine::FrameTicks frame = timestep.advance(frameTime);
        totals.ticksRun += static_cast<engine::Tick>(frame.ticksToRun);
        totals.ticksDropped += frame.ticksDropped;
        totals.warnings += frame.warnSlow ? 1 : 0;
    }
    return totals;
}

} // namespace

// PB-012.4: 1.0 s of real time in 1/120 s chunks runs exactly 60 ticks.
TEST(FixedTimestep, OneSecondIn120thSecondChunks_RunsExactly60Ticks) {
    engine::FixedTimestep timestep;

    const RunTotals totals = feed(timestep, engine_tests::evenFrameTimes(1s, 120));

    EXPECT_EQ(totals.ticksRun, 60u);
    EXPECT_EQ(totals.ticksDropped, 0u);
}

// 144 frames per second is where a double accumulator runs 59 ticks instead of 60.
TEST(FixedTimestep, OneSecondAtSteadyFrameRates_RunsExactly60Ticks) {
    for (const std::int64_t framesPerSecond : {30, 60, 75, 120, 144, 240, 1000}) {
        SCOPED_TRACE(framesPerSecond);
        engine::FixedTimestep timestep;

        const RunTotals totals = feed(timestep, engine_tests::evenFrameTimes(1s, framesPerSecond));

        EXPECT_EQ(totals.ticksRun, 60u);
        EXPECT_EQ(totals.ticksDropped, 0u);
    }
}

TEST(FixedTimestep, OneHourAt144FramesPerSecond_RunsExactly216000Ticks) {
    engine::FixedTimestep timestep;

    const RunTotals totals = feed(timestep, engine_tests::evenFrameTimes(1h, 144 * 3600));

    EXPECT_EQ(totals.ticksRun, 216'000u);
    EXPECT_EQ(totals.ticksDropped, 0u);
}

// PB-012.4: a 5 s spike runs at most the maximum number of ticks; the rest is dropped.
TEST(FixedTimestep, FiveSecondSpike_RunsFiveTicksAndDropsTheRest) {
    engine::FixedTimestep timestep;

    const engine::FrameTicks spike = timestep.advance(5s);
    const engine::FrameTicks next = timestep.advance(0ns);

    EXPECT_EQ(spike.ticksToRun, engine::maxTicksPerRenderFrame);
    EXPECT_EQ(spike.ticksToRun, 5);
    EXPECT_EQ(spike.ticksDropped, 295u);
    EXPECT_EQ(next.ticksToRun, 0);
}

// srs.fr.1.3: a part of a tick is carried over to the next render frame.
TEST(FixedTimestep, PartOfATick_IsCarriedToTheNextFrame) {
    engine::FixedTimestep timestep;
    std::vector<int> ticksPerFrame;

    // 10 ms is 0.6 of a tick: 0.6, 1.2, 0.8, 1.4.
    for (int frame = 0; frame < 4; ++frame) {
        ticksPerFrame.push_back(timestep.advance(10ms).ticksToRun);
    }

    EXPECT_EQ(ticksPerFrame, (std::vector<int>{0, 1, 0, 1}));
}

TEST(FixedTimestep, OneNanosecondShortOfATick_RunsTheTickWhenTheRestArrives) {
    engine::FixedTimestep timestep;

    const engine::FrameTicks shortOfATick = timestep.advance(16'666'666ns);
    const engine::FrameTicks rest = timestep.advance(1ns);

    EXPECT_EQ(shortOfATick.ticksToRun, 0);
    EXPECT_EQ(rest.ticksToRun, 1);
}

// srs.fr.1.4: with more than 5 ticks due, the further whole ticks are dropped and the part of a
// tick is kept. 110 ms is 6.6 ticks: 5 run, 1 is dropped, 0.6 stays.
TEST(FixedTimestep, MoreThanFiveTicksDue_DropsWholeTicksAndKeepsThePart) {
    engine::FixedTimestep timestep;

    const engine::FrameTicks spike = timestep.advance(110ms);
    const engine::FrameTicks stillShort = timestep.advance(6ms); // 0.6 + 0.36 = 0.96 of a tick
    const engine::FrameTicks complete = timestep.advance(1ms);   // 1.02 ticks

    EXPECT_EQ(spike.ticksToRun, 5);
    EXPECT_EQ(spike.ticksDropped, 1u);
    EXPECT_EQ(stillShort.ticksToRun, 0);
    EXPECT_EQ(complete.ticksToRun, 1);
}

TEST(FixedTimestep, ExactlyFiveTicksDue_RunsFiveWithoutDropOrWarning) {
    engine::FixedTimestep timestep;

    const engine::FrameTicks frame = timestep.advance(84ms); // 5.04 ticks

    EXPECT_EQ(frame.ticksToRun, 5);
    EXPECT_EQ(frame.ticksDropped, 0u);
    EXPECT_FALSE(frame.warnSlow);
}

TEST(FixedTimestep, NegativeDuration_CountsAsZero) {
    engine::FixedTimestep timestep;

    const engine::FrameTicks negative = timestep.advance(-1s);
    const engine::FrameTicks next = timestep.advance(50ms); // 3 ticks

    EXPECT_EQ(negative.ticksToRun, 0);
    EXPECT_EQ(negative.ticksDropped, 0u);
    EXPECT_EQ(next.ticksToRun, 3);
}

TEST(FixedTimestep, LongestPossibleDuration_DoesNotOverflow) {
    engine::FixedTimestep timestep;

    const engine::FrameTicks frame = timestep.advance(nanoseconds::max());

    // 9 223 372 036 whole seconds are 553 402 322 160 ticks; the last 0.854 775 807 s add 51.
    EXPECT_EQ(frame.ticksToRun, 5);
    EXPECT_EQ(frame.ticksDropped, 553'402'322'211u - 5u);
}

// srs.fr.1.4: the warning is due when ticks are dropped, at most once per second of real time.
TEST(FixedTimestep, TicksDroppedEveryFrame_WarnsOncePerSecond) {
    engine::FixedTimestep timestep;
    std::vector<bool> warned;

    // Each 200 ms frame has 12 ticks due, so every frame drops 7.
    for (int frame = 0; frame < 11; ++frame) {
        warned.push_back(timestep.advance(200ms).warnSlow);
    }

    EXPECT_EQ(warned, (std::vector<bool>{true, false, false, false, false, true, false, false,
                                         false, false, true}));
}

TEST(FixedTimestep, SecondDropAfterOneSecondOfSmoothFrames_WarnsAgain) {
    engine::FixedTimestep timestep;

    const bool first = timestep.advance(200ms).warnSlow;
    const bool tooSoon = timestep.advance(200ms).warnSlow;
    const RunTotals smooth = feed(timestep, std::vector<nanoseconds>(50, 16ms)); // 0.8 s, no drops
    const bool afterOneSecond = timestep.advance(200ms).warnSlow;

    EXPECT_TRUE(first);
    EXPECT_FALSE(tooSoon);
    EXPECT_EQ(smooth.warnings, 0);
    EXPECT_TRUE(afterOneSecond);
}

TEST(FixedTimestep, NoTicksDropped_NeverWarns) {
    engine::FixedTimestep timestep;

    const RunTotals totals = feed(timestep, engine_tests::evenFrameTimes(10s, 600));

    EXPECT_EQ(totals.ticksRun, 600u);
    EXPECT_EQ(totals.warnings, 0);
}
