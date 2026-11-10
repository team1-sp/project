#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

namespace engine_tests {

// The frame times of a steady frame rate as a nanosecond clock measures them: `frames` whole
// nanosecond durations that differ by at most 1 ns and add up to exactly `total`.
inline std::vector<std::chrono::nanoseconds> evenFrameTimes(std::chrono::nanoseconds total,
                                                            std::int64_t frames) {
    std::vector<std::chrono::nanoseconds> frameTimes;
    frameTimes.reserve(static_cast<std::size_t>(frames));
    for (std::int64_t i = 0; i < frames; ++i) {
        const std::int64_t frameStart =
            total.count() / frames * i + total.count() % frames * i / frames;
        const std::int64_t frameEnd =
            total.count() / frames * (i + 1) + total.count() % frames * (i + 1) / frames;
        frameTimes.emplace_back(frameEnd - frameStart);
    }
    return frameTimes;
}

} // namespace engine_tests
