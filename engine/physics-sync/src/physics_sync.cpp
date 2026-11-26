#include "engine/physics_sync/physics_sync.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <set>
#include <vector>

#include "engine/core/tick.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/ecs_simulation.hpp"
#include "engine/ecs/physics_components.hpp"
#include "engine/ecs/system_context.hpp"
#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/transform.hpp"
#include "engine/ecs/world.hpp"

namespace engine::physics_sync {

using physics::BodyDesc;
using physics::BodyType;
using physics::ShapeDesc;
using physics::ShapeType;

namespace {

BodyType toBodyType(RigidBodyType type) {
    switch (type) {
    case RigidBodyType::Static:
        return BodyType::Static;
    case RigidBodyType::Kinematic:
        return BodyType::Kinematic;
    case RigidBodyType::Dynamic:
        return BodyType::Dynamic;
    }
    return BodyType::Static;
}

// srs.fr.3.2: the RigidBody sets the body; without one the body is static. Box2D gets world
// transforms (srs.fr.3.39).
BodyDesc bodyDescOf(const WorldTransform& world, const RigidBodyComponent* rigidBody) {
    BodyDesc desc;
    desc.positionMeters = world.position;
    desc.angleRadians = world.rotationRadians;
    if (rigidBody != nullptr) {
        desc.type = toBodyType(rigidBody->bodyType);
        desc.massKilograms = rigidBody->massKilograms;
        desc.gravityScale = rigidBody->gravityScale;
        desc.linearDamping = rigidBody->linearDamping;
        desc.fixedRotation = rigidBody->fixedRotation;
    }
    return desc;
}

// The entity's parent, if it has one.
std::optional<Entity> parentOf(SystemContext& ctx, Entity e) {
    const EntityId parent = ctx.get<const HierarchyComponent>(e).parent;
    return parent == noEntity ? std::nullopt : ctx.find(parent);
}

// srs.fr.3.37, 3.49: the parent's world transform combined with the local one, from the root
// down. An entity without a Position counts as one at the parent's origin.
WorldTransform worldTransformOf(SystemContext& ctx, std::optional<Entity> e) {
    std::vector<Entity> chain;
    for (; e; e = parentOf(ctx, *e)) {
        chain.push_back(*e);
    }
    WorldTransform world = worldOrigin;
    for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
        if (const auto* position = ctx.tryGet<const PositionComponent>(*it)) {
            world = compose(world, *position);
        }
    }
    return world;
}

template <typename C>
ShapeDesc commonShapeDesc(ShapeType type, const C& collider) {
    ShapeDesc desc;
    desc.type = type;
    desc.offsetMeters = Vec2{collider.offsetXMeters, collider.offsetYMeters};
    desc.friction = collider.friction;
    desc.restitution = collider.restitution;
    desc.isSensor = collider.isSensor;
    return desc;
}

ShapeDesc shapeDescOf(const BoxColliderComponent& box) {
    ShapeDesc desc = commonShapeDesc(ShapeType::Box, box);
    desc.boxSizeMeters = Vec2{box.widthMeters, box.heightMeters};
    return desc;
}

ShapeDesc shapeDescOf(const CircleColliderComponent& circle) {
    ShapeDesc desc = commonShapeDesc(ShapeType::Circle, circle);
    desc.circleRadiusMeters = circle.radiusMeters;
    return desc;
}

ShapeDesc shapeDescOf(const PolygonColliderComponent& polygon) {
    ShapeDesc desc = commonShapeDesc(ShapeType::Polygon, polygon);
    desc.polygonVerticesMeters = {polygon.vertex0, polygon.vertex1, polygon.vertex2,
                                  polygon.vertex3, polygon.vertex4, polygon.vertex5,
                                  polygon.vertex6, polygon.vertex7};
    desc.polygonVertexCount = polygon.vertexCount; // the adapter refuses fewer than 3 or above 8
    return desc;
}

// The collider of kind `kind` on the entity as a shape, if it has one.
std::optional<ShapeDesc> colliderShape(SystemContext& ctx, Entity e, ColliderKind kind) {
    switch (kind) {
    case ColliderKind::Box:
        if (const auto* box = ctx.tryGet<const BoxColliderComponent>(e)) {
            return shapeDescOf(*box);
        }
        break;
    case ColliderKind::Circle:
        if (const auto* circle = ctx.tryGet<const CircleColliderComponent>(e)) {
            return shapeDescOf(*circle);
        }
        break;
    case ColliderKind::Polygon:
        if (const auto* polygon = ctx.tryGet<const PolygonColliderComponent>(e)) {
            return shapeDescOf(*polygon);
        }
        break;
    }
    return std::nullopt;
}

constexpr std::array<ColliderKind, colliderKindCount> colliderKinds{
    ColliderKind::Box, ColliderKind::Circle, ColliderKind::Polygon};

} // namespace

void PhysicsSync::onEntityDestroyed(Entity /*e*/, EntityId id) {
    destroyed_.push_back(id);
}

std::optional<physics::BodyId> PhysicsSync::bodyOf(EntityId entity) const {
    const auto it = bodies_.find(entity);
    if (it == bodies_.end()) {
        return std::nullopt;
    }
    return it->second.body;
}

void PhysicsSyncInSystem::describe(SystemDescription& d) const {
    d.name("PhysicsSyncIn")
        .purpose("Creates, changes and destroys physics bodies and shapes to match the "
                 "entities' RigidBody and collider components.")
        .phase(TickPhase::PhysicsSyncIn)
        .reads<HierarchyComponent, PositionComponent, VelocityComponent, RigidBodyComponent,
               BoxColliderComponent, CircleColliderComponent, PolygonColliderComponent>();
}

void PhysicsSyncInSystem::update(SystemContext& ctx) {
    physics::IPhysicsAdapter& adapter = sync_.adapter_;
    auto& bodies = sync_.bodies_;

    // (1) Bodies of destroyed entities (srs.fr.3.8, srs.fr.3.58 step 1).
    std::sort(sync_.destroyed_.begin(), sync_.destroyed_.end());
    for (const EntityId id : sync_.destroyed_) {
        if (const auto it = bodies.find(id); it != bodies.end()) {
            adapter.destroyBody(it->second.body);
            bodies.erase(it);
        }
    }
    sync_.destroyed_.clear();

    // (2) New entities with a collider, ascending EntityId (srs.fr.3.2, 3.26; 3.58 step 2).
    std::set<EntityId> candidates;
    const auto collect = [&](Entity e, const auto&) {
        if (!bodies.contains(ctx.idOf(e))) {
            candidates.insert(ctx.idOf(e));
        }
    };
    ctx.each<const BoxColliderComponent>(collect);
    ctx.each<const CircleColliderComponent>(collect);
    ctx.each<const PolygonColliderComponent>(collect);
    for (const EntityId id : candidates) {
        const Entity e = *ctx.find(id);
        const BodyDesc desc =
            bodyDescOf(worldTransformOf(ctx, e), ctx.tryGet<const RigidBodyComponent>(e));
        const physics::BodyId body = adapter.createBody(id, desc);
        if (body == physics::noBody) {
            continue; // the adapter logged why
        }
        if (const auto* velocity = ctx.tryGet<const VelocityComponent>(e);
            velocity != nullptr && desc.type != BodyType::Static) {
            adapter.setLinearVelocity(body,
                                      Vec2{velocity->xMetersPerSecond, velocity->yMetersPerSecond});
        }
        PhysicsSync::Tracked tracked{body, desc.type, {}, ctx.has<const RigidBodyComponent>(e)};
        for (const ColliderKind kind : colliderKinds) {
            if (const std::optional<ShapeDesc> shape = colliderShape(ctx, e, kind)) {
                tracked.shapes[static_cast<std::size_t>(kind)] = adapter.addShape(body, *shape);
            }
        }
        bodies.emplace(id, tracked);
    }

    // (3) Colliders and RigidBodies added or removed on entities that already had a body
    // (srs.fr.3.8; 3.58 steps 3 and 4).
    for (auto it = bodies.begin(); it != bodies.end();) {
        const EntityId id = it->first;
        PhysicsSync::Tracked& tracked = it->second;
        const std::optional<Entity> e = ctx.find(id);
        if (!e || candidates.contains(id)) {
            ++it; // destroyed entities were handled in (1); new bodies are up to date
            continue;
        }
        bool anyCollider = false;
        for (const ColliderKind kind : colliderKinds) {
            auto& shapeId = tracked.shapes[static_cast<std::size_t>(kind)];
            const std::optional<ShapeDesc> shape = colliderShape(ctx, *e, kind);
            anyCollider = anyCollider || shape.has_value();
            if (shape && !shapeId) {
                shapeId = adapter.addShape(tracked.body, *shape);
            } else if (!shape && shapeId) {
                adapter.removeShape(*shapeId);
                shapeId.reset();
            }
        }
        if (!anyCollider) {
            adapter.destroyBody(tracked.body); // the last collider was removed
            it = bodies.erase(it);
            continue;
        }
        const RigidBodyComponent* rigidBody = ctx.tryGet<const RigidBodyComponent>(*e);
        if (tracked.hasRigidBody != (rigidBody != nullptr)) {
            tracked.type =
                rigidBody != nullptr ? toBodyType(rigidBody->bodyType) : BodyType::Static;
            adapter.setBodyType(tracked.body, tracked.type);
            tracked.hasRigidBody = rigidBody != nullptr;
        }
        ++it;
    }

    // Pending writes (srs.fr.3.11, 3.22, 3.55), then marks cleared; children's static and
    // kinematic bodies follow their parent (srs.fr.3.39). Only entities with a body.
    for (const auto& [id, tracked] : bodies) {
        const Entity e = *ctx.find(id);
        const bool positionWritten = ctx.hasPendingWrite<PositionComponent>(e);
        const bool followsParent =
            tracked.type != BodyType::Dynamic && parentOf(ctx, e).has_value();
        if (positionWritten || followsParent) {
            const WorldTransform world = worldTransformOf(ctx, e);
            const physics::BodyState body = adapter.bodyState(tracked.body);
            if (positionWritten || body.positionMeters != world.position ||
                std::abs(body.angleRadians - world.rotationRadians) > 1e-4f) {
                adapter.setTransform(tracked.body, world.position, world.rotationRadians);
            }
            ctx.clearPendingWrite<PositionComponent>(e);
        }
        if (ctx.hasPendingWrite<VelocityComponent>(e)) {
            if (tracked.type != BodyType::Static) { // srs.fr.3.18: ignored for a static body
                const VelocityComponent& velocity = ctx.get<const VelocityComponent>(e);
                adapter.setLinearVelocity(
                    tracked.body, Vec2{velocity.xMetersPerSecond, velocity.yMetersPerSecond});
            }
            ctx.clearPendingWrite<VelocityComponent>(e);
        }
    }
}

void PhysicsSyncOutSystem::describe(SystemDescription& d) const {
    d.name("PhysicsSyncOut")
        .purpose("Writes the positions and velocities physics computed back to the entities.")
        .phase(TickPhase::PhysicsSyncOut)
        .reads<HierarchyComponent>()
        .writesUntracked<PositionComponent, VelocityComponent>();
}

void PhysicsSyncOutSystem::update(SystemContext& ctx) {
    physics::IPhysicsAdapter& adapter = sync_.adapter_;
    for (const auto& [id, tracked] : sync_.bodies_) {
        if (tracked.type == BodyType::Static) {
            continue; // physics never moves a static body (srs.fr.3.18)
        }
        const std::optional<Entity> e = ctx.find(id);
        if (!e) {
            continue; // destroyed this tick; its body goes in the next synchronization-in
        }
        const physics::BodyState body = adapter.bodyState(tracked.body);
        const auto* position = ctx.tryGet<const PositionComponent>(*e);
        if (position != nullptr && !ctx.hasPendingWrite<PositionComponent>(*e)) {
            ctx.writeUntracked(*e, localFromWorld(worldTransformOf(ctx, parentOf(ctx, *e)),
                                                  body.positionMeters, body.angleRadians,
                                                  Vec2{position->scaleX, position->scaleY}));
        }
        // srs.fr.3.13: a kinematic body's Velocity is the ECS's own (srs.fr.3.19).
        if (tracked.type == BodyType::Dynamic && ctx.has<const VelocityComponent>(*e) &&
            !ctx.hasPendingWrite<VelocityComponent>(*e)) {
            ctx.writeUntracked(*e, VelocityComponent{body.linearVelocityMetersPerSecond.x,
                                                     body.linearVelocityMetersPerSecond.y});
        }
    }
}

void connectPhysics(World& world, EcsSimulation& simulation, PhysicsSync& sync,
                    physics::IPhysicsAdapter& adapter) {
    world.addObserver(sync);
    world.systems().add([&sync] { return std::make_unique<PhysicsSyncInSystem>(sync); });
    world.systems().add([&sync] { return std::make_unique<PhysicsSyncOutSystem>(sync); });
    simulation.setPhysicsStep(
        [&adapter](const TickContext&) { adapter.step(static_cast<float>(fixedDeltaSeconds)); });
}

} // namespace engine::physics_sync
