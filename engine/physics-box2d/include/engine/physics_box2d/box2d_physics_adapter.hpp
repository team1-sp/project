#pragma once

#include <memory>
#include <optional>
#include <span>

#include "engine/core/entity_id.hpp"
#include "engine/core/logging.hpp"
#include "engine/core/vec2.hpp"
#include "engine/physics/physics_adapter.hpp"

namespace engine::physics_box2d {

// Settings for one Box2D world. Gravity and the sub-step count come from the project's Physics
// settings once those exist (srs.fr.3.9, srs.fr.3.23); these defaults match DEC-036 and the
// Box2D recommendation.
struct Box2DPhysicsConfig {
    Vec2 gravityMetersPerSecondSquared{0.0f, -10.0f};
    int subStepCount = 4;
};

// The Physics Adapter implemented with the Box2D v3.1.1 C API (srs.fr.3.1, DEC-014, DEC-020).
// This header has no Box2D type: the world and its ids stay in the .cpp file (R-3).
//
// One instance owns one Box2D world for one ECS World (physics-adapter-design.md section 7).
// Errors are logged to `log` (when given) and the call is ignored; see IPhysicsAdapter.
class Box2DPhysicsAdapter final : public physics::IPhysicsAdapter {
public:
    explicit Box2DPhysicsAdapter(Box2DPhysicsConfig config = {}, ILogSink* log = nullptr);
    ~Box2DPhysicsAdapter() override;

    physics::BodyId createBody(EntityId entity, const physics::BodyDesc& desc) override;
    void destroyBody(physics::BodyId body) override;
    std::optional<physics::BodyId> findBody(EntityId entity) const override;

    physics::ShapeId addShape(physics::BodyId body, const physics::ShapeDesc& desc) override;
    void removeShape(physics::ShapeId shape) override;

    void setTransform(physics::BodyId body, Vec2 positionMeters, float angleRadians) override;
    void setLinearVelocity(physics::BodyId body, Vec2 velocityMetersPerSecond) override;
    void applyForce(physics::BodyId body, Vec2 forceNewtons) override;
    void applyImpulse(physics::BodyId body, Vec2 impulseNewtonSeconds) override;

    void step(float dtSeconds) override;

    physics::BodyState bodyState(physics::BodyId body) const override;
    std::span<const physics::CollisionEvent> collisionEvents() const override;

    std::optional<physics::RaycastHit> raycast(Vec2 originMeters, Vec2 direction,
                                               float maxDistanceMeters) const override;

    // The mass Box2D uses for a body now (kilograms); for tests and diagnostics.
    float bodyMassKilograms(physics::BodyId body) const;

private:
    struct World;
    std::unique_ptr<World> world_;
};

} // namespace engine::physics_box2d
