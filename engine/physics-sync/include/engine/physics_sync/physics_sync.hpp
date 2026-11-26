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

namespace engine::physics_sync {

// The kinds of collider component, in a fixed order (the order shapes are added).
enum class ColliderKind { Box, Circle, Polygon };
inline constexpr std::size_t colliderKindCount = 3;

// What one World's physics bodies are, as PhysicsSyncIn last built them, and the entities
// destroyed since. One per World, next to its adapter (physics-adapter-design.md section 7).
// Register it as the World's observer before the first tick, and keep it alive as long as the
// World: World::addObserver(sync). It never calls the adapter itself (DEC-041).
class PhysicsSync final : public IWorldObserver {
public:
    explicit PhysicsSync(physics::IPhysicsAdapter& adapter)
        : adapter_(adapter) {}

    // srs.fr.2.116, 3.58: the body is destroyed in the next synchronization-in.
    void onEntityDestroyed(Entity e, EntityId id) override;

    // The body PhysicsSyncIn made for an entity, if any; for tests and PhysicsSyncOut.
    [[nodiscard]] std::optional<physics::BodyId> bodyOf(EntityId entity) const;

private:
    friend class PhysicsSyncInSystem;

    struct Tracked {
        physics::BodyId body;
        std::array<std::optional<physics::ShapeId>, colliderKindCount> shapes;
        bool hasRigidBody;
    };

    physics::IPhysicsAdapter& adapter_;
    std::map<EntityId, Tracked> bodies_; // ascending EntityId (srs.fr.3.26)
    std::vector<EntityId> destroyed_;
};

// The PhysicsSyncIn system (TickPhase::PhysicsSyncIn, srs.fr.3.63). This part (US-3.1) keeps
// bodies and shapes in step with the components:
// (1) destroys the bodies of destroyed entities;
// (2) creates a body for each entity that has a collider and no body, in ascending EntityId,
//     typed by its RigidBody or static without one, with one shape per collider (srs.fr.3.2);
// (3) adds and removes shapes as colliders are added and removed, destroys the body when the
//     last collider is gone, and makes it static when the RigidBody is removed (srs.fr.3.8).
// Pending Position and Velocity writes, runtime property changes and disabled components join
// with US-3.2, US-3.4 and US-3.6.
class PhysicsSyncInSystem final : public ISystem {
public:
    explicit PhysicsSyncInSystem(PhysicsSync& sync)
        : sync_(sync) {}

    void describe(SystemDescription& d) const override;
    void update(SystemContext& ctx) override;

private:
    PhysicsSync& sync_;
};

} // namespace engine::physics_sync
