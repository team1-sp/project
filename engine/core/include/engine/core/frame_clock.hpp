#pragma once

#include <chrono>
#include <functional>

namespace engine {

// Measures real time for the game loop: how long each render frame took and how long the run
// has lasted (PB-012.1). Real time only decides how many ticks a render frame runs; it never
// reaches the simulation, which sees fixed 1/60 s ticks only (R-6, coding rule 5).
class FrameClock {
public:
    // The current time of a clock that never goes backwards, counted from any fixed moment.
    using TimeSource = std::function<std::chrono::nanoseconds()>;

    // Reads std::chrono::steady_clock.
    FrameClock();
    // Reads `now` instead, so tests can script real time. An empty source means the steady clock.
    explicit FrameClock(TimeSource now);

    // Starts the measurement again from this moment; the total time becomes zero. A new
    // FrameClock is already started.
    void start();
    // Ends the frame that is running and starts the next one. Returns the frame time: the real
    // time since the previous measureFrame(), or since start() for the first frame.
    std::chrono::nanoseconds measureFrame();

    // The frame time of the last measured frame; zero before the first.
    [[nodiscard]] std::chrono::nanoseconds frameTime() const;
    // Real time from start() to the last measureFrame().
    [[nodiscard]] std::chrono::nanoseconds totalTime() const;

private:
    TimeSource now_;
    std::chrono::nanoseconds startedAt_{};
    std::chrono::nanoseconds frameStartedAt_{};
    std::chrono::nanoseconds frameTime_{};
};

} // namespace engine
