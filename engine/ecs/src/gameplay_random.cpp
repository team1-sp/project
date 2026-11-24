#include "engine/ecs/gameplay_random.hpp"

namespace engine {

namespace {

// The stream of the PCG reference demo; one fixed stream, because a run has one generator.
constexpr std::uint64_t stream = 54;
constexpr std::uint64_t multiplier = 6364136223846793005ull;

} // namespace

void GameplayRandom::reseed(std::uint64_t seed) {
    state_ = 0;
    increment_ = (stream << 1u) | 1u;
    next();
    state_ += seed;
    next();
}

std::uint32_t GameplayRandom::next() {
    const std::uint64_t old = state_;
    state_ = old * multiplier + increment_;
    const auto xorShifted = static_cast<std::uint32_t>(((old >> 18u) ^ old) >> 27u);
    const auto rotation = static_cast<std::uint32_t>(old >> 59u);
    return (xorShifted >> rotation) | (xorShifted << ((0u - rotation) & 31u));
}

std::uint32_t GameplayRandom::nextBelow(std::uint32_t bound) {
    if (bound == 0) {
        return 0;
    }
    // Reject the values below 2^32 mod bound, so every result is equally likely.
    const std::uint32_t threshold = (0u - bound) % bound;
    while (true) {
        const std::uint32_t value = next();
        if (value >= threshold) {
            return value % bound;
        }
    }
}

float GameplayRandom::nextFloat() {
    return static_cast<float>(next() >> 8u) * (1.0f / 16777216.0f);
}

} // namespace engine
