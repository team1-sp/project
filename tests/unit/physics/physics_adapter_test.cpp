#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "engine/physics/physics_adapter.hpp"

namespace {

using engine::EntityId;
using engine::Vec2;
using namespace engine::physics;

// The smallest possible adapter: no simulation, it only remembers bodies. It proves the
// interface can be implemented without any physics library, and shows the contract.
class FakePhysicsAdapter final : public IPhysicsAdapter {
public:
    BodyId createBody(EntityId entity, const BodyDesc& desc) override {
        const BodyId body{nextBody_++};
        bodies_[body] = Body{entity, BodyState{desc.positionMeters, desc.angleRadians, {}, 0.0f}};
        return body;
    }
    void destroyBody(BodyId body) override { bodies_.erase(body); }
    std::optional<BodyId> findBody(EntityId entity) const override {
        for (const auto& [body, data] : bodies_) {
            if (data.entity == entity) {
                return body;
            }
        }
        return std::nullopt;
    }
    // at() throws for a destroyed body; the result itself is not needed.
    void setBodyType(BodyId body, BodyType) override { static_cast<void>(bodies_.at(body)); }
    ShapeId addShape(BodyId body, const ShapeDesc&) override {
        static_cast<void>(bodies_.at(body)); // throws for a destroyed body
        return ShapeId{nextShape_++};
    }
    void removeShape(ShapeId) override {}
    void setTransform(BodyId body, Vec2 positionMeters, float angleRadians) override {
        bodies_.at(body).state.positionMeters = positionMeters;
        bodies_.at(body).state.angleRadians = angleRadians;
    }
    void setLinearVelocity(BodyId body, Vec2 velocityMetersPerSecond) override {
        bodies_.at(body).state.linearVelocityMetersPerSecond = velocityMetersPerSecond;
    }
    void applyForce(BodyId, Vec2) override {}
    void applyImpulse(BodyId, Vec2) override {}
    void step(float) override {}
    BodyState bodyState(BodyId body) const override { return bodies_.at(body).state; }
    std::span<const CollisionEvent> collisionEvents() const override { return events_; }
    std::optional<RaycastHit> raycast(Vec2, Vec2, float) const override { return std::nullopt; }

private:
    struct Body {
        EntityId entity;
        BodyState state;
    };
    std::uint64_t nextBody_ = 1;  // 0 is noBody
    std::uint64_t nextShape_ = 1; // 0 is noShape
    std::map<BodyId, Body> bodies_;
    std::vector<CollisionEvent> events_;
};

constexpr EntityId player{42};

} // namespace

TEST(BodyDesc, Default_IsStatic) {
    EXPECT_EQ(BodyDesc{}.type, BodyType::Static);
}

TEST(ShapeDesc, Default_IsOneMeterSolidBox) {
    const ShapeDesc desc;
    EXPECT_EQ(desc.type, ShapeType::Box);
    EXPECT_EQ(desc.boxSizeMeters, (Vec2{1.0f, 1.0f}));
    EXPECT_FALSE(desc.isSensor);
}

TEST(IPhysicsAdapter, AddShape_ReturnsAShape) {
    FakePhysicsAdapter physics;
    const BodyId body = physics.createBody(player, BodyDesc{});

    EXPECT_NE(physics.addShape(body, ShapeDesc{}), noShape);
}

TEST(IPhysicsAdapter, CreateBody_FindBodyReturnsIt) {
    FakePhysicsAdapter physics;
    const BodyId body = physics.createBody(player, BodyDesc{});

    EXPECT_NE(body, noBody);
    EXPECT_EQ(physics.findBody(player), body);
}

TEST(IPhysicsAdapter, DestroyBody_FindBodyReturnsNothing) {
    FakePhysicsAdapter physics;
    physics.destroyBody(physics.createBody(player, BodyDesc{}));

    EXPECT_EQ(physics.findBody(player), std::nullopt);
}

TEST(IPhysicsAdapter, SetTransform_BodyStateReportsIt) {
    FakePhysicsAdapter physics;
    const BodyId body = physics.createBody(player, BodyDesc{});
    physics.setTransform(body, Vec2{3.0f, 5.0f}, 0.5f);

    EXPECT_EQ(physics.bodyState(body).positionMeters, (Vec2{3.0f, 5.0f}));
    EXPECT_FLOAT_EQ(physics.bodyState(body).angleRadians, 0.5f);
}
