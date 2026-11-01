#include <algorithm>
#include <utility>

#include <gtest/gtest.h>

#include "falling_box_scene.hpp"

namespace {

constexpr int threeSecondsOfSteps = 180;

void stepFor(sandbox::FallingBoxScene& scene, int steps) {
    for (int i = 0; i < steps; ++i) {
        scene.step();
    }
}

} // namespace

TEST(FallingBoxScene, ThreeSeconds_BoxRestsOnGround) {
    sandbox::FallingBoxScene scene;
    stepFor(scene, threeSecondsOfSteps);

    // Ground top is at 1 m and the box is 1 m tall, so its centre rests at 1.5 m.
    EXPECT_NEAR(scene.boxPose().yMeters, 1.5f, 0.02f);
    EXPECT_LT(scene.boxSpeedMetersPerSecond(), 0.01f);
}

TEST(FallingBoxScene, Stepped_GroundDoesNotMove) {
    sandbox::FallingBoxScene scene;
    stepFor(scene, threeSecondsOfSteps);

    EXPECT_FLOAT_EQ(scene.groundPose().xMeters, 12.5f);
    EXPECT_FLOAT_EQ(scene.groundPose().yMeters, 0.5f);
    EXPECT_FLOAT_EQ(scene.groundPose().angleRadians, 0.0f);
}

TEST(FallingBoxScene, BoxLands_ContactEventNamesGroundAndBox) {
    sandbox::FallingBoxScene scene;
    stepFor(scene, threeSecondsOfSteps);

    ASSERT_FALSE(scene.contactBegins().empty());
    const sandbox::ContactBegin first = scene.contactBegins().front();
    const auto pair = std::minmax(first.entityA, first.entityB);
    EXPECT_EQ(pair.first, sandbox::groundEntity);
    EXPECT_EQ(pair.second, sandbox::boxEntity);
}

TEST(FallingBoxScene, TwoRuns_IdenticalTrajectory) {
    sandbox::FallingBoxScene first;
    sandbox::FallingBoxScene second;
    for (int i = 0; i < threeSecondsOfSteps; ++i) {
        first.step();
        second.step();
        ASSERT_EQ(first.boxPose().xMeters, second.boxPose().xMeters) << "step " << i;
        ASSERT_EQ(first.boxPose().yMeters, second.boxPose().yMeters) << "step " << i;
        ASSERT_EQ(first.boxPose().angleRadians, second.boxPose().angleRadians) << "step " << i;
    }
}
