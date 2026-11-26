#include <cmath>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "engine/core/logging.hpp"
#include "engine/physics_box2d/box2d_physics_adapter.hpp"

namespace {

using engine::EntityId;
using engine::Vec2;
using engine::physics_box2d::Box2DPhysicsAdapter;
using namespace engine::physics;

constexpr float dt = 1.0f / 60.0f;
constexpr EntityId ground{1};
constexpr EntityId crate{2};

class LogCollector final : public engine::ILogSink {
public:
    void write(engine::LogEntry entry) override { entries.push_back(std::move(entry)); }
    std::vector<engine::LogEntry> entries;
};

ShapeDesc box(float width, float height) {
    ShapeDesc desc;
    desc.boxSizeMeters = Vec2{width, height};
    return desc;
}

ShapeDesc polygon(std::initializer_list<Vec2> vertices) {
    ShapeDesc desc;
    desc.type = ShapeType::Polygon;
    for (const Vec2 vertex : vertices) {
        desc.polygonVerticesMeters[desc.polygonVertexCount++] = vertex;
    }
    return desc;
}

BodyDesc dynamicAt(Vec2 position, float massKilograms = 1.0f) {
    BodyDesc desc;
    desc.type = BodyType::Dynamic;
    desc.positionMeters = position;
    desc.massKilograms = massKilograms;
    return desc;
}

void stepFor(Box2DPhysicsAdapter& physics, int steps) {
    for (int i = 0; i < steps; ++i) {
        physics.step(dt);
    }
}

// A 20 m wide static floor whose top is at y = 0, and a 1 m crate dropped from `heightMeters`.
struct FloorAndCrate {
    explicit FloorAndCrate(float heightMeters, BodyDesc crateDesc = dynamicAt({}),
                           ShapeDesc crateShape = box(1.0f, 1.0f)) {
        BodyDesc floorDesc;
        floorDesc.positionMeters = Vec2{0.0f, -0.5f};
        floor = physics.createBody(ground, floorDesc);
        physics.addShape(floor, box(20.0f, 1.0f));
        crateDesc.positionMeters = Vec2{0.0f, heightMeters};
        body = physics.createBody(crate, crateDesc);
        physics.addShape(body, crateShape);
    }
    Box2DPhysicsAdapter physics;
    BodyId floor{};
    BodyId body{};
};

} // namespace

TEST(Box2DPhysicsAdapter, CreateBody_FindBodyReturnsIt) {
    Box2DPhysicsAdapter physics;
    const BodyId body = physics.createBody(crate, BodyDesc{});

    EXPECT_NE(body, noBody);
    EXPECT_EQ(physics.findBody(crate), body);
}

TEST(Box2DPhysicsAdapter, SecondBodyForSameEntity_LogsErrorAndReturnsNoBody) {
    LogCollector log;
    Box2DPhysicsAdapter physics({}, &log);
    physics.createBody(crate, BodyDesc{});

    EXPECT_EQ(physics.createBody(crate, BodyDesc{}), noBody);
    ASSERT_EQ(log.entries.size(), 1u);
    EXPECT_EQ(log.entries[0].severity, engine::LogSeverity::Error);
}

TEST(Box2DPhysicsAdapter, DestroyBody_FindBodyReturnsNothingAndLaterUseLogsError) {
    LogCollector log;
    Box2DPhysicsAdapter physics({}, &log);
    const BodyId body = physics.createBody(crate, BodyDesc{});
    physics.addShape(body, box(1.0f, 1.0f));
    physics.destroyBody(body);

    EXPECT_EQ(physics.findBody(crate), std::nullopt);
    physics.setLinearVelocity(body, Vec2{1.0f, 0.0f});
    EXPECT_EQ(log.entries.size(), 1u);
}

TEST(Box2DPhysicsAdapter, DynamicBox_LandsOnStaticFloor) {
    FloorAndCrate scene(5.0f);
    stepFor(scene.physics, 180);

    EXPECT_NEAR(scene.physics.bodyState(scene.body).positionMeters.y, 0.5f, 0.02f);
    EXPECT_EQ(scene.physics.bodyState(scene.floor).positionMeters, (Vec2{0.0f, -0.5f}));
}

TEST(Box2DPhysicsAdapter, KinematicBody_MovesAtItsVelocityAndIgnoresGravity) {
    BodyDesc platform;
    platform.type = BodyType::Kinematic;
    FloorAndCrate scene(5.0f, platform);
    scene.physics.setLinearVelocity(scene.body, Vec2{1.0f, 0.0f});
    stepFor(scene.physics, 60);

    EXPECT_NEAR(scene.physics.bodyState(scene.body).positionMeters.x, 1.0f, 0.001f);
    EXPECT_FLOAT_EQ(scene.physics.bodyState(scene.body).positionMeters.y, 5.0f);
}

TEST(Box2DPhysicsAdapter, DynamicBody_KeepsRigidBodyMassWhenShapesChange) {
    Box2DPhysicsAdapter physics;
    const BodyId body = physics.createBody(crate, dynamicAt({}, 3.0f));
    EXPECT_FLOAT_EQ(physics.bodyMassKilograms(body), 3.0f); // no shapes yet

    physics.addShape(body, box(2.0f, 2.0f)); // Box2D alone would give 4 kg
    EXPECT_FLOAT_EQ(physics.bodyMassKilograms(body), 3.0f);

    ShapeDesc head;
    head.type = ShapeType::Circle;
    head.offsetMeters = Vec2{0.0f, 1.5f};
    const ShapeId headShape = physics.addShape(body, head);
    EXPECT_FLOAT_EQ(physics.bodyMassKilograms(body), 3.0f);

    physics.removeShape(headShape);
    EXPECT_FLOAT_EQ(physics.bodyMassKilograms(body), 3.0f);
}

TEST(Box2DPhysicsAdapter, ConvexPolygon_IsAddedAndCollides) {
    FloorAndCrate scene(5.0f, dynamicAt({}),
                        polygon({{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}}));
    stepFor(scene.physics, 180);

    EXPECT_NEAR(scene.physics.bodyState(scene.body).positionMeters.y, 0.5f, 0.02f);
}

TEST(Box2DPhysicsAdapter, ConcaveOrTooLargePolygon_IsRefusedWithAnError) {
    LogCollector log;
    Box2DPhysicsAdapter physics({}, &log);
    const BodyId body = physics.createBody(crate, BodyDesc{});

    // An arrow head: (0, 0.2) lies inside the hull of the other points.
    EXPECT_EQ(physics.addShape(body, polygon({{-1, 0}, {0, 1}, {1, 0}, {0, 0.2f}})), noShape);
    ShapeDesc tooMany = polygon({{0, 0}, {1, 0}, {1, 1}});
    tooMany.polygonVertexCount = maxPolygonVertices + 1;
    EXPECT_EQ(physics.addShape(body, tooMany), noShape);
    EXPECT_EQ(log.entries.size(), 2u);
}

TEST(Box2DPhysicsAdapter, BoxOffset_MovesTheShapeAwayFromTheBody) {
    Box2DPhysicsAdapter physics;
    const BodyId body = physics.createBody(crate, BodyDesc{});
    ShapeDesc shifted = box(1.0f, 1.0f);
    shifted.offsetMeters = Vec2{3.0f, 0.0f};
    physics.addShape(body, shifted);

    const std::optional<RaycastHit> hit = physics.raycast({-10.0f, 0.0f}, {1.0f, 0.0f}, 20.0f);
    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(hit->entity, crate);
    EXPECT_NEAR(hit->pointMeters.x, 2.5f, 0.01f);
}

TEST(Box2DPhysicsAdapter, Restitution_MakesTheCrateBounce) {
    ShapeDesc bouncy = box(1.0f, 1.0f);
    bouncy.restitution = 1.0f;
    FloorAndCrate scene(3.0f, dynamicAt({}), bouncy);

    float highestAfterLanding = 0.0f;
    bool landed = false;
    for (int i = 0; i < 120; ++i) {
        scene.physics.step(dt);
        const float y = scene.physics.bodyState(scene.body).positionMeters.y;
        landed = landed || y < 0.6f;
        if (landed) {
            highestAfterLanding = std::max(highestAfterLanding, y);
        }
    }
    EXPECT_GT(highestAfterLanding, 2.0f);
}

TEST(Box2DPhysicsAdapter, SensorFloor_DoesNotStopTheCrate) {
    Box2DPhysicsAdapter physics;
    BodyDesc floorDesc;
    floorDesc.positionMeters = Vec2{0.0f, -0.5f};
    ShapeDesc sensor = box(20.0f, 1.0f);
    sensor.isSensor = true;
    physics.addShape(physics.createBody(ground, floorDesc), sensor);
    const BodyId body = physics.createBody(crate, dynamicAt({0.0f, 2.0f}));
    physics.addShape(body, box(1.0f, 1.0f));
    stepFor(physics, 120);

    EXPECT_LT(physics.bodyState(body).positionMeters.y, -2.0f);
}

TEST(Box2DPhysicsAdapter, GravityScaleZero_BodyFloats) {
    BodyDesc floating = dynamicAt({});
    floating.gravityScale = 0.0f;
    FloorAndCrate scene(5.0f, floating);
    stepFor(scene.physics, 60);

    EXPECT_FLOAT_EQ(scene.physics.bodyState(scene.body).positionMeters.y, 5.0f);
}

TEST(Box2DPhysicsAdapter, FixedRotation_TiltedCrateDoesNotTurnOnLanding) {
    BodyDesc tilted = dynamicAt({});
    tilted.angleRadians = 0.3f;
    tilted.fixedRotation = true;
    FloorAndCrate scene(3.0f, tilted);
    stepFor(scene.physics, 120);

    // b2Rot_GetAngle is an approximation (about 0.002 rad off); a turning crate lands flat (0).
    EXPECT_NEAR(scene.physics.bodyState(scene.body).angleRadians, 0.3f, 0.01f);
}

TEST(Box2DPhysicsAdapter, Landing_ReportsOneBeginEventWithBothEntities) {
    FloorAndCrate scene(2.0f);
    std::vector<CollisionEvent> begins;
    for (int i = 0; i < 120; ++i) {
        scene.physics.step(dt);
        for (const CollisionEvent& event : scene.physics.collisionEvents()) {
            if (event.phase == ContactPhase::Begin) {
                begins.push_back(event);
            }
        }
    }
    ASSERT_EQ(begins.size(), 1u);
    EXPECT_EQ(begins[0].entityA, ground); // the lower id first
    EXPECT_EQ(begins[0].entityB, crate);
}

TEST(Box2DPhysicsAdapter, TwoRuns_GiveIdenticalTrajectories) {
    BodyDesc tilted = dynamicAt({});
    tilted.angleRadians = 0.3f;
    FloorAndCrate first(5.0f, tilted);
    FloorAndCrate second(5.0f, tilted);
    for (int i = 0; i < 180; ++i) {
        first.physics.step(dt);
        second.physics.step(dt);
        ASSERT_EQ(first.physics.bodyState(first.body).positionMeters,
                  second.physics.bodyState(second.body).positionMeters)
            << "step " << i;
    }
}
