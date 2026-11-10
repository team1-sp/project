#pragma once

#include <optional>
#include <span>

#include "engine/core/entity_id.hpp"
#include "engine/core/vec2.hpp"
#include "engine/physics/body_desc.hpp"
#include "engine/physics/physics_types.hpp"

namespace engine::physics {

// The only way the engine talks to physics (R-3, DEC-003). No physics library type appears
// here, so the library behind it can be replaced. Each body belongs to one entity; the
// implementation keeps the entity <-> body mapping in both directions (ip.fr.3.4).
//
// Units: meters, kilograms, seconds, radians, y up (DEC-036).
// Errors: no exceptions cross this interface. Using a destroyed BodyId, or creating a second
// body for the same entity, is a programming error the implementation reports
// loudly (coding standards rule 9).
// Threading: single-threaded; only PhysicsSyncIn, the game loop's PhysicsStep and
// PhysicsSyncOut call it (DEC-041).
class IPhysicsAdapter {
public:
    virtual ~IPhysicsAdapter() = default;

    // Bodies
    virtual BodyId createBody(EntityId entity, const BodyDesc& desc) = 0;
    virtual void
    destroyBody(BodyId body) = 0; // must be called when the entity is destroyed (E-001)
    virtual std::optional<BodyId> findBody(EntityId entity) const = 0;

    // Changes from the ECS, applied before the next step
    virtual void setTransform(BodyId body, Vec2 positionMeters, float angleRadians) = 0;
    virtual void setLinearVelocity(BodyId body, Vec2 velocityMetersPerSecond) = 0;
    virtual void applyForce(BodyId body, Vec2 forceNewtons) = 0;           // for this step
    virtual void applyImpulse(BodyId body, Vec2 impulseNewtonSeconds) = 0; // instant

    // Simulation: always called with the fixed 1/60 s (R-5)
    virtual void step(float dtSeconds) = 0;

    // Results of the last step
    virtual BodyState bodyState(BodyId body) const = 0;
    // Sorted by (entityA, entityB) so the order is the same every run. Valid until the next step.
    virtual std::span<const CollisionEvent> collisionEvents() const = 0;

    // Queries
    virtual std::optional<RaycastHit> raycast(Vec2 originMeters, Vec2 direction,
                                              float maxDistanceMeters) const = 0;

protected:
    IPhysicsAdapter() = default;
    IPhysicsAdapter(const IPhysicsAdapter&) = delete;
    IPhysicsAdapter& operator=(const IPhysicsAdapter&) = delete;
};

} // namespace engine::physics
