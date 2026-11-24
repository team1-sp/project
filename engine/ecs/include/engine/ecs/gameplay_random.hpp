#pragma once

#include <cstdint>

namespace engine {

// The one gameplay random number generator of a World (srs.fr.7.5, D-28): PCG32 (XSH RR, 64-bit
// state), implemented here so every build and platform draws the same sequence for a seed.
// Systems reach it only through SystemContext::random() and only when they declare usesRandom();
// draws happen in system run order, so the run's seed fixes the whole sequence.
class GameplayRandom {
public:
    explicit GameplayRandom(std::uint64_t seed = 0) { reseed(seed); }

    // Starts the sequence of `seed` again (pcg32_srandom with the fixed stream 54).
    void reseed(std::uint64_t seed);

    // The next 32 random bits.
    std::uint32_t next();
    // A value in [0, bound), without modulo bias; bound 0 gives 0.
    std::uint32_t nextBelow(std::uint32_t bound);
    // A value in [0, 1) with 24 random bits.
    float nextFloat();

private:
    std::uint64_t state_ = 0;
    std::uint64_t increment_ = 0;
};

} // namespace engine
