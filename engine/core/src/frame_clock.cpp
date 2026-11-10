#include "engine/core/frame_clock.hpp"

#include <algorithm>
#include <utility>

namespace engine {

namespace {

std::chrono::nanoseconds steadyClockNow() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch());
}

} // namespace

FrameClock::FrameClock()
    : FrameClock(TimeSource{}) {
}

FrameClock::FrameClock(TimeSource now)
    : now_(now ? std::move(now) : TimeSource{steadyClockNow}) {
    start();
}

void FrameClock::start() {
    startedAt_ = now_();
    frameStartedAt_ = startedAt_;
    frameTime_ = std::chrono::nanoseconds::zero();
}

std::chrono::nanoseconds FrameClock::measureFrame() {
    // A source that steps backwards would give a negative frame time; treat it as no time.
    const std::chrono::nanoseconds now = std::max(now_(), frameStartedAt_);
    frameTime_ = now - frameStartedAt_;
    frameStartedAt_ = now;
    return frameTime_;
}

std::chrono::nanoseconds FrameClock::frameTime() const {
    return frameTime_;
}

std::chrono::nanoseconds FrameClock::totalTime() const {
    return frameStartedAt_ - startedAt_;
}

} // namespace engine
