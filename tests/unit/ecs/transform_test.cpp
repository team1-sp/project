// srs.fr.3.49 composition and its inverse (srs.fr.3.45).
#include <numbers>

#include <gtest/gtest.h>

#include "engine/ecs/transform.hpp"

using engine::compose;
using engine::localFromWorld;
using engine::PositionComponent;
using engine::Vec2;
using engine::WorldTransform;

namespace {
constexpr float quarterTurn = std::numbers::pi_v<float> / 2.0f;
}

TEST(Transform, RootEntity_WorldEqualsLocal) {
    const WorldTransform world = compose(engine::worldOrigin, PositionComponent{2, 3, 0.5f, 2, 1});

    EXPECT_EQ(world.position, (Vec2{2, 3}));
    EXPECT_FLOAT_EQ(world.rotationRadians, 0.5f);
    EXPECT_EQ(world.scale, (Vec2{2, 1}));
}

TEST(Transform, ChildOfTurnedScaledParent_IsRotatedScaledAndMoved) {
    const WorldTransform parent{Vec2{10, 0}, quarterTurn, Vec2{2, 2}};
    const WorldTransform child = compose(parent, PositionComponent{1, 0, 0.25f, 1, 1});

    // (1, 0) scaled by 2 is (2, 0), turned a quarter is (0, 2), plus (10, 0).
    EXPECT_NEAR(child.position.x, 10.0f, 1e-5f);
    EXPECT_NEAR(child.position.y, 2.0f, 1e-5f);
    EXPECT_FLOAT_EQ(child.rotationRadians, quarterTurn + 0.25f);
    EXPECT_EQ(child.scale, (Vec2{2, 2}));
}

TEST(Transform, MirroringParent_TurnsChildRotationTheOtherWay) {
    const WorldTransform parent{Vec2{}, 0.1f, Vec2{-1, 1}};
    const WorldTransform child = compose(parent, PositionComponent{1, 0, 0.3f, 1, 1});

    EXPECT_NEAR(child.position.x, -std::cos(0.1f), 1e-5f);
    EXPECT_FLOAT_EQ(child.rotationRadians, 0.1f - 0.3f);
}

TEST(Transform, LocalFromWorld_UndoesCompose) {
    for (const Vec2 parentScale : {Vec2{2, 0.5f}, Vec2{-1, 1}, Vec2{-2, -3}}) {
        const WorldTransform parent{Vec2{4, -1}, 0.7f, parentScale};
        const PositionComponent local{1.5f, -2, 0.4f, 3, 1};
        const WorldTransform world = compose(parent, local);
        const PositionComponent back =
            localFromWorld(parent, world.position, world.rotationRadians, Vec2{3, 1});

        EXPECT_NEAR(back.xMeters, local.xMeters, 1e-4f);
        EXPECT_NEAR(back.yMeters, local.yMeters, 1e-4f);
        EXPECT_NEAR(back.rotationRadians, local.rotationRadians, 1e-5f);
        EXPECT_EQ(back.scaleX, 3.0f);
    }
}
