#include <gtest/gtest.h>

#include "engine/physics_box2d/box2d_info.hpp"

// Fails if the fetched Box2D drifts from the pinned v3.1.1 (DEC-020).
TEST(LinkedBox2dVersion, Called_IsPinnedVersion) {
    const engine::Version version = engine::physics_box2d::linkedBox2dVersion();
    EXPECT_EQ(version.major, 3);
    EXPECT_EQ(version.minor, 1);
    EXPECT_EQ(version.patch, 1);
}
