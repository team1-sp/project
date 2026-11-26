#pragma once

#include <array>
#include <map>
#include <optional>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/world_observer.hpp"
#include "engine/physics/physics_adapter.hpp"

namespace engine {
class EcsSimulation;
class World;
} // namespace engine

namespace engine::physics_sync {

// The kinds of collider component, in a fixed order (the order shapes are added).
enum class ColliderKind { Box, Circle, Polygon };
inline constexpr std::size_t colliderKindCount = 3;

// What one World's physics bodies are, as PhysicsSyncIn last built them, and the entities
// destroyed since. One per World, next to its adapter (physics-adapter-design.md section 7).
// It is the World's observer and must outlive the World's systems. It never calls the adapter
// itself; only the two systems do (DEC-041, srs.fr.3.63).
class PhysicsSync final : public IWorldObserver {
public:
    explicit PhysicsSync(physics::IPhysicsAdapter& adapter)
        : adapter_(adapter) {}

    // srs.fr.2.116, 3.58: the body is destroyed in the next synchronization-in.
    void onEntityDestroyed(Entity e, EntityId id) override;

    // The body PhysicsSyncIn made for an entity, if any (srs.fr.3.10).
    [[nodiscard]] std::optional<physics::BodyId> bodyOf(EntityId entity) const;

private:
    friend class PhysicsSyncInSystem;
    friend class PhysicsSyncOutSystem;

    struct Tracked {
        physics::BodyId body;
        physics::BodyType type;
        std::array<std::optional<physics::ShapeId>, colliderKindCount> shapes;
        bool hasRigidBody;
    };

    physics::IPhysicsAdapter& adapter_;
    std::map<EntityId, Tracked> bodies_; // ascending EntityId (srs.fr.3.26)
    std::vector<EntityId> destroyed_;
};

// The PhysicsSyncIn system (TickPhase::PhysicsSyncIn, srs.fr.3.63), in the order of srs.fr.3.58:
// (1) destroys the bodies of destroyed entities;
// (2) creates a body for each entity with a collider and no body, in ascending EntityId, typed by
//     its RigidBody or static without one, at the entity's world transform, with its Velocity
//     and one shape per collider (srs.fr.3.2);
// (3, 4) adds and removes shapes as colliders come and go, destroys the body with the last
//     collider, makes it static when the RigidBody is removed (srs.fr.3.8);
// then applies pending writes (srs.fr.3.11, 3.22): a Position write moves the body to the
// entity's new world transform and a Velocity write sets the velocity of a kinematic or dynamic
// body; both marks are cleared. Static and kinematic bodies of child entities follow their
// parent's world transform (srs.fr.3.39). Physics never moves a static body (srs.fr.3.18).
// Runtime property changes, disabled components and forces join with US-3.4 and US-3.6.
class PhysicsSyncInSystem final : public ISystem {
public:
    explicit PhysicsSyncInSystem(PhysicsSync& sync)
        : sync_(sync) {}

    void describe(SystemDescription& d) const override;
    void update(SystemContext& ctx) override;

private:
    PhysicsSync& sync_;
};

// The PhysicsSyncOut system (TickPhase::PhysicsSyncOut, srs.fr.3.63): writes the world position
// and rotation of each kinematic and dynamic body to its entity's Position, converted to the
// entity's local transform (srs.fr.3.12, 3.37), and each dynamic body's linear velocity to its
// Velocity (srs.fr.3.13), without pending-write marks; it skips a component that has a pending
// write (srs.fr.3.48). Collision events join with US-3.3.
class PhysicsSyncOutSystem final : public ISystem {
public:
    explicit PhysicsSyncOutSystem(PhysicsSync& sync)
        : sync_(sync) {}

    void describe(SystemDescription& d) const override;
    void update(SystemContext& ctx) override;

private:
    PhysicsSync& sync_;
};

// Connects one World to its Physics Adapter: registers `sync` as the World's observer, adds
// PhysicsSyncIn and PhysicsSyncOut, and makes the PhysicsStep phase step the adapter by exactly
// 1/60 s (srs.fr.3.23). `sync` and `adapter` must outlive the World and the simulation.
void connectPhysics(World& world, EcsSimulation& simulation, PhysicsSync& sync,
                    physics::IPhysicsAdapter& adapter);

} // namespace engine::physics_sync
