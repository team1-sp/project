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
// Errors: no exceptions cross this interface. Misuse (a destroyed BodyId or ShapeId, a second
// body for the same entity, an invalid polygon) is a programming or data error: the
// implementation logs it as an error and ignores the call, or returns noBody / noShape.
// Threading: single-threaded; only PhysicsSyncIn, the game loop's PhysicsStep and
// PhysicsSyncOut call it (DEC-041).
class IPhysicsAdapter {
public:
    virtual ~IPhysicsAdapter() = default;

    // Bodies. A new body has no shapes; add one per collider component (srs.fr.3.2).
    virtual BodyId createBody(EntityId entity, const BodyDesc& desc) = 0;
    // Destroys the body and its shapes. Must be called when the entity is destroyed (E-001).
    virtual void destroyBody(BodyId body) = 0;
    virtual std::optional<BodyId> findBody(EntityId entity) const = 0;
    // Changes how the body moves; a dynamic body gets its mass set again (srs.fr.3.8, 3.46).
    virtual void setBodyType(BodyId body, BodyType type) = 0;

    // Shapes. After a shape of a dynamic body is added or removed, the body keeps the mass of
    // its BodyDesc (srs.fr.3.33, srs.fr.3.46).
    virtual ShapeId addShape(BodyId body, const ShapeDesc& desc) = 0;
    virtual void removeShape(ShapeId shape) = 0;

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
