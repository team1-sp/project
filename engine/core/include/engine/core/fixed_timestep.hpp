#pragma once

#include <chrono>
#include <cstdint>

#include "engine/core/tick.hpp"

namespace engine {

// The most ticks one render frame runs (srs.fr.1.3).
inline constexpr int maxTicksPerRenderFrame = 5;

// What one render frame has to run, as FixedTimestep::advance() decided it.
struct FrameTicks {
    // Ticks to run in this render frame: 0 to maxTicksPerRenderFrame.
    int ticksToRun = 0;
    // Whole ticks of real time dropped because more than the maximum were due (srs.fr.1.4).
    Tick ticksDropped = 0;
    // True when the warning "Simulation is running slower than real time" is due: ticks were
    // dropped and at least one second of real time has passed since the last such warning.
    bool warnSlow = false;
};

// Turns the real duration of render frames into a number of fixed 1/60 s ticks (srs.fr.1.2 to
// 1.4). It reads no clock: the caller passes each frame's real duration, so the game loop and
// the tests drive it the same way.
//
// The accumulator is exact. It counts nanoseconds times 60, a unit in which one tick is exactly
// 1 000 000 000, so no rounding error builds up: frame durations that add up to one second give
// exactly 60 ticks at any frame rate. A floating-point accumulator, or a tick rounded to whole
// nanoseconds, loses a tick at some frame rates.
class FixedTimestep {
public:
    // Adds the real time since the previous call and returns what this render frame runs. A
    // negative duration counts as zero. A part of a tick that is left over is carried to the
    // next call; whole ticks beyond the maximum are dropped.
    [[nodiscard]] FrameTicks advance(std::chrono::nanoseconds realDelta);

private:
    // Real time not simulated yet, in units of 1/60 ns. Less than one tick between calls.
    std::uint64_t carriedUnits_ = 0;
    // Real time since the last warning, counted only up to the warning interval. It starts at
    // the interval so that the first dropped tick warns at once.
    std::chrono::nanoseconds sinceWarning_{std::chrono::seconds(1)};
};

} // namespace engine
