#include "engine/physics_sync/physics_sync.hpp"

#include <algorithm>
#include <set>

#include "engine/ecs/core_components.hpp"
#include "engine/ecs/physics_components.hpp"
#include "engine/ecs/system_context.hpp"

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

// srs.fr.3.2: the RigidBody sets the body; without one the body is static.
BodyDesc bodyDescOf(const PositionComponent* position, const RigidBodyComponent* rigidBody) {
    BodyDesc desc;
    if (position != nullptr) {
        desc.positionMeters = Vec2{position->xMeters, position->yMeters};
        desc.angleRadians = position->rotationRadians;
    }
    if (rigidBody != nullptr) {
        desc.type = toBodyType(rigidBody->bodyType);
        desc.massKilograms = rigidBody->massKilograms;
        desc.gravityScale = rigidBody->gravityScale;
        desc.linearDamping = rigidBody->linearDamping;
        desc.fixedRotation = rigidBody->fixedRotation;
    }
    return desc;
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
        .reads<PositionComponent, RigidBodyComponent, BoxColliderComponent, CircleColliderComponent,
               PolygonColliderComponent>();
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
        const physics::BodyId body =
            adapter.createBody(id, bodyDescOf(ctx.tryGet<const PositionComponent>(e),
                                              ctx.tryGet<const RigidBodyComponent>(e)));
        if (body == physics::noBody) {
            continue; // the adapter logged why
        }
        PhysicsSync::Tracked tracked{body, {}, ctx.has<const RigidBodyComponent>(e)};
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
            adapter.setBodyType(tracked.body, rigidBody != nullptr ? toBodyType(rigidBody->bodyType)
                                                                   : BodyType::Static);
            tracked.hasRigidBody = rigidBody != nullptr;
        }
        ++it;
    }
}

} // namespace engine::physics_sync
