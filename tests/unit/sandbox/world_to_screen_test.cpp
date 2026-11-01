#include <numbers>

#include <gtest/gtest.h>

#include "world_to_screen.hpp"

TEST(WorldToScreen, WorldOrigin_IsBottomLeftOfWindow) {
    const sandbox::ScreenPoint point = sandbox::worldToScreen(0.0f, 0.0f, 450.0f);
    EXPECT_FLOAT_EQ(point.xPixels, 0.0f);
    EXPECT_FLOAT_EQ(point.yPixels, 450.0f);
}

TEST(WorldToScreen, OneMeterUpAndRight_Is32PixelsRightAndUp) {
    const sandbox::ScreenPoint point = sandbox::worldToScreen(1.0f, 1.0f, 450.0f);
    EXPECT_FLOAT_EQ(point.xPixels, 32.0f);
    EXPECT_FLOAT_EQ(point.yPixels, 418.0f);
}

TEST(WorldAngleToScreenDegrees, CounterClockwiseQuarterTurn_IsMinus90) {
    EXPECT_FLOAT_EQ(sandbox::worldAngleToScreenDegrees(std::numbers::pi_v<float> / 2.0f), -90.0f);
}
