#pragma once

#include <cstdint>

namespace engine {

// A simulation tick number, or a number of ticks (srs.fr.1.5, 1.19). Tick numbers start at 0:
// a run that has executed N ticks completed ticks 0 to N - 1.
using Tick = std::uint64_t;

// The simulation advances only in ticks of exactly 1/60 s of simulated time (srs.fr.1.2, R-5).
inline constexpr int ticksPerSecond = 60;
inline constexpr double fixedDeltaSeconds = 1.0 / ticksPerSecond;

// Simulated time of a run that has executed `ticksExecuted` ticks (srs.fr.1.6).
[[nodiscard]] constexpr double simulatedSeconds(Tick ticksExecuted) {
    return static_cast<double>(ticksExecuted) / ticksPerSecond;
}

} // namespace engine
