// The gameplay random number generator (US-2.3; srs.fr.7.5, D-28).
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "engine/ecs/gameplay_random.hpp"

// The first outputs of the PCG reference demo for pcg32_srandom(42, 54).
TEST(GameplayRandom, Seed42_GivesThePcg32ReferenceSequence) {
    engine::GameplayRandom random(42);

    const std::vector<std::uint32_t> expected = {0xa15c02b7u, 0x7b47f409u, 0xba1d3330u,
                                                 0x83d2f293u, 0xbfa4784bu, 0xcbed606eu};
    for (const std::uint32_t value : expected) {
        EXPECT_EQ(random.next(), value);
    }
}

TEST(GameplayRandom, SameSeed_GivesTheSameSequenceAfterReseed) {
    engine::GameplayRandom random(7);
    const std::uint32_t first = random.next();
    const std::uint32_t second = random.next();

    random.reseed(7);

    EXPECT_EQ(random.next(), first);
    EXPECT_EQ(random.next(), second);
    EXPECT_NE(engine::GameplayRandom(8).next(), first);
}

TEST(GameplayRandom, NextBelowAndNextFloat_StayInRange) {
    engine::GameplayRandom random(1);
    for (int i = 0; i < 1000; ++i) {
        EXPECT_LT(random.nextBelow(6), 6u);
        const float value = random.nextFloat();
        EXPECT_GE(value, 0.0f);
        EXPECT_LT(value, 1.0f);
    }
    EXPECT_EQ(random.nextBelow(0), 0u);
}
