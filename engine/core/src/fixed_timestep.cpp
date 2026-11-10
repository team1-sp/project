#include "engine/core/fixed_timestep.hpp"

#include <algorithm>

namespace engine {

namespace {

using std::chrono::nanoseconds;
using std::chrono::seconds;

// The accumulator counts nanoseconds times ticksPerSecond, so one tick is a whole number.
constexpr std::uint64_t unitsPerNanosecond = ticksPerSecond;
constexpr std::uint64_t unitsPerTick = 1'000'000'000;
constexpr Tick ticksPerWholeSecond = ticksPerSecond;
// srs.fr.1.4: the warning is logged at most once per second.
constexpr nanoseconds warningInterval = seconds(1);

} // namespace

FrameTicks FixedTimestep::advance(nanoseconds realDelta) {
    const nanoseconds delta = std::max(realDelta, nanoseconds::zero());

    // Whole seconds are converted on their own, so that no frame duration, however long, can
    // overflow the accumulator.
    const seconds wholeSeconds = std::chrono::duration_cast<seconds>(delta);
    const nanoseconds rest = delta - wholeSeconds;
    carriedUnits_ += static_cast<std::uint64_t>(rest.count()) * unitsPerNanosecond;
    const Tick ticksDue = static_cast<Tick>(wholeSeconds.count()) * ticksPerWholeSecond +
                          carriedUnits_ / unitsPerTick;
    // Only the part of a tick is kept: the whole ticks either run now or are dropped.
    carriedUnits_ %= unitsPerTick;

    FrameTicks frame;
    frame.ticksToRun =
        static_cast<int>(std::min(ticksDue, static_cast<Tick>(maxTicksPerRenderFrame)));
    frame.ticksDropped = ticksDue - static_cast<Tick>(frame.ticksToRun);

    sinceWarning_ =
        delta >= warningInterval - sinceWarning_ ? warningInterval : sinceWarning_ + delta;
    if (frame.ticksDropped > 0 && sinceWarning_ >= warningInterval) {
        frame.warnSlow = true;
        sinceWarning_ = nanoseconds::zero();
    }
    return frame;
}

} // namespace engine
