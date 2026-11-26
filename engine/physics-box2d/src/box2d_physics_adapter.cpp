#include "engine/physics_box2d/box2d_physics_adapter.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <box2d/box2d.h>

namespace engine::physics_box2d {

using physics::BodyDesc;
using physics::BodyId;
using physics::BodyState;
using physics::BodyType;
using physics::CollisionEvent;
using physics::ContactPhase;
using physics::RaycastHit;
using physics::ShapeDesc;
using physics::ShapeId;
using physics::ShapeType;

namespace {

b2Vec2 toB2(Vec2 v) {
    return b2Vec2{v.x, v.y};
}

Vec2 fromB2(b2Vec2 v) {
    return Vec2{v.x, v.y};
}

b2BodyType toB2(BodyType type) {
    switch (type) {
    case BodyType::Static:
        return b2_staticBody;
    case BodyType::Kinematic:
        return b2_kinematicBody;
    case BodyType::Dynamic:
        return b2_dynamicBody;
    }
    return b2_staticBody;
}

// Box2D's own ids fit in our 64-bit handles, and a valid id is never 0 (noBody, noShape).
BodyId toBodyId(b2BodyId id) {
    return BodyId{b2StoreBodyId(id)};
}

b2BodyId toB2(BodyId id) {
    return b2LoadBodyId(static_cast<std::uint64_t>(id));
}

ShapeId toShapeId(b2ShapeId id) {
    return ShapeId{b2StoreShapeId(id)};
}

b2ShapeId toB2(ShapeId id) {
    return b2LoadShapeId(static_cast<std::uint64_t>(id));
}

} // namespace

struct Box2DPhysicsAdapter::World {
    struct Body {
        EntityId entity;
        BodyType type;
        float massKilograms;
    };

    b2WorldId id{};
    int subStepCount = 4;
    ILogSink* log = nullptr;
    std::map<EntityId, BodyId> bodyOfEntity;
    std::map<BodyId, Body> bodies;
    std::map<ShapeId, BodyId> shapes;
    std::vector<CollisionEvent> events;

    void error(const std::string& message) const {
        if (log != nullptr) {
            log->write(LogEntry{LogSeverity::Error, "Physics Adapter: " + message, {}, {}, {}, {}});
        }
    }

    const Body* find(BodyId body, const char* operation) const {
        const auto it = bodies.find(body);
        if (it == bodies.end()) {
            error(std::string(operation) + " was given a body that does not exist");
            return nullptr;
        }
        return &it->second;
    }

    std::optional<EntityId> entityOfShape(b2ShapeId shape) const {
        if (!b2Shape_IsValid(shape)) {
            return std::nullopt;
        }
        const auto it = bodies.find(toBodyId(b2Shape_GetBody(shape)));
        if (it == bodies.end()) {
            return std::nullopt;
        }
        return it->second.entity;
    }

    // srs.fr.3.46: Box2D computes the mass data from the shapes, then the RigidBody mass
    // replaces the mass and scales the rotational inertia by the same ratio; the centre of mass
    // stays where the shapes put it. A body without shapes has no area: its centre is the body
    // origin and its rotational inertia that of a box of zero size, 0.
    void applyMass(BodyId body) const {
        const Body& record = bodies.at(body);
        if (record.type != BodyType::Dynamic) {
            return;
        }
        const b2BodyId b2Body = toB2(body);
        b2Body_ApplyMassFromShapes(b2Body);
        b2MassData mass = b2Body_GetMassData(b2Body);
        if (mass.mass > 0.0f) {
            mass.rotationalInertia *= record.massKilograms / mass.mass;
        } else {
            mass.center = b2Vec2{0.0f, 0.0f};
            mass.rotationalInertia = 0.0f;
        }
        mass.mass = record.massKilograms;
        b2Body_SetMassData(b2Body, mass);
    }
};

Box2DPhysicsAdapter::Box2DPhysicsAdapter(Box2DPhysicsConfig config, ILogSink* log)
    : world_(std::make_unique<World>()) {
    world_->log = log;
    world_->subStepCount = config.subStepCount;
    if (config.subStepCount < 1) {
        world_->error("the sub-step count must be at least 1; using 4");
        world_->subStepCount = 4;
    }
    b2WorldDef def = b2DefaultWorldDef();
    def.gravity = toB2(config.gravityMetersPerSecondSquared);
    world_->id = b2CreateWorld(&def);
}

Box2DPhysicsAdapter::~Box2DPhysicsAdapter() {
    // Destroying the world destroys every body and shape in it.
    b2DestroyWorld(world_->id);
}

BodyId Box2DPhysicsAdapter::createBody(EntityId entity, const BodyDesc& desc) {
    if (entity == noEntity) {
        world_->error("createBody was given no entity");
        return physics::noBody;
    }
    if (world_->bodyOfEntity.contains(entity)) {
        world_->error("createBody: entity " + std::to_string(static_cast<std::uint64_t>(entity)) +
                      " already has a body");
        return physics::noBody;
    }
    float mass = desc.massKilograms;
    if (desc.type == BodyType::Dynamic && !(mass > 0.0f && std::isfinite(mass))) {
        world_->error("createBody: a dynamic body needs a mass above 0; using 1 kg");
        mass = 1.0f;
    }

    b2BodyDef def = b2DefaultBodyDef();
    def.type = toB2(desc.type);
    def.position = toB2(desc.positionMeters);
    def.rotation = b2MakeRot(desc.angleRadians);
    def.gravityScale = desc.gravityScale;
    def.linearDamping = desc.linearDamping;
    def.fixedRotation = desc.fixedRotation;
    const BodyId body = toBodyId(b2CreateBody(world_->id, &def));

    world_->bodyOfEntity.emplace(entity, body);
    world_->bodies.emplace(body, World::Body{entity, desc.type, mass});
    world_->applyMass(body);
    return body;
}

void Box2DPhysicsAdapter::destroyBody(BodyId body) {
    const World::Body* record = world_->find(body, "destroyBody");
    if (record == nullptr) {
        return;
    }
    b2DestroyBody(toB2(body)); // also destroys its shapes
    std::erase_if(world_->shapes, [body](const auto& entry) { return entry.second == body; });
    world_->bodyOfEntity.erase(record->entity);
    world_->bodies.erase(body);
}

std::optional<BodyId> Box2DPhysicsAdapter::findBody(EntityId entity) const {
    const auto it = world_->bodyOfEntity.find(entity);
    if (it == world_->bodyOfEntity.end()) {
        return std::nullopt;
    }
    return it->second;
}

void Box2DPhysicsAdapter::setBodyType(BodyId body, BodyType type) {
    if (world_->find(body, "setBodyType") == nullptr) {
        return;
    }
    world_->bodies.at(body).type = type;
    b2Body_SetType(toB2(body), toB2(type));
    world_->applyMass(body);
}

ShapeId Box2DPhysicsAdapter::addShape(BodyId body, const ShapeDesc& desc) {
    if (world_->find(body, "addShape") == nullptr) {
        return physics::noShape;
    }

    b2ShapeDef def = b2DefaultShapeDef();
    def.density = 1.0f; // any value above 0: applyMass replaces the mass (srs.fr.3.46)
    def.material.friction = desc.friction;
    def.material.restitution = desc.restitution;
    def.isSensor = desc.isSensor;
    // Off by default in v3.1.1 (research/box2d.md E-026); on for every shape (srs.fr.3.15-3.16).
    def.enableContactEvents = true;
    def.enableSensorEvents = true;
    def.updateBodyMass = false; // applyMass runs once below

    const b2BodyId b2Body = toB2(body);
    b2ShapeId shape{};
    switch (desc.type) {
    case ShapeType::Box: {
        const Vec2 size = desc.boxSizeMeters;
        if (!(size.x > 0.0f && size.y > 0.0f)) {
            world_->error("addShape: a box needs a width and height above 0");
            return physics::noShape;
        }
        const b2Polygon box =
            b2MakeOffsetBox(size.x / 2.0f, size.y / 2.0f, toB2(desc.offsetMeters), b2MakeRot(0.0f));
        shape = b2CreatePolygonShape(b2Body, &def, &box);
        break;
    }
    case ShapeType::Circle: {
        if (!(desc.circleRadiusMeters > 0.0f)) {
            world_->error("addShape: a circle needs a radius above 0");
            return physics::noShape;
        }
        const b2Circle circle{toB2(desc.offsetMeters), desc.circleRadiusMeters};
        shape = b2CreateCircleShape(b2Body, &def, &circle);
        break;
    }
    case ShapeType::Polygon: {
        const int count = desc.polygonVertexCount;
        if (count < 3 || count > physics::maxPolygonVertices) {
            world_->error("addShape: a polygon needs 3 to 8 vertices");
            return physics::noShape;
        }
        std::array<b2Vec2, physics::maxPolygonVertices> points{};
        for (int i = 0; i < count; ++i) {
            const Vec2 vertex = desc.polygonVerticesMeters[static_cast<std::size_t>(i)];
            points[static_cast<std::size_t>(i)] =
                toB2(Vec2{vertex.x + desc.offsetMeters.x, vertex.y + desc.offsetMeters.y});
        }
        // The hull keeps every vertex only for a convex polygon without repeated points.
        const b2Hull hull = b2ComputeHull(points.data(), count);
        if (hull.count != count) {
            world_->error("addShape: the polygon is not convex or has repeated vertices");
            return physics::noShape;
        }
        const b2Polygon polygon = b2MakePolygon(&hull, 0.0f);
        shape = b2CreatePolygonShape(b2Body, &def, &polygon);
        break;
    }
    }

    const ShapeId id = toShapeId(shape);
    world_->shapes.emplace(id, body);
    world_->applyMass(body); // srs.fr.3.33
    return id;
}

void Box2DPhysicsAdapter::removeShape(ShapeId shape) {
    const auto it = world_->shapes.find(shape);
    if (it == world_->shapes.end()) {
        world_->error("removeShape was given a shape that does not exist");
        return;
    }
    const BodyId body = it->second;
    b2DestroyShape(toB2(shape), false);
    world_->shapes.erase(it);
    world_->applyMass(body); // srs.fr.3.33
}

void Box2DPhysicsAdapter::setTransform(BodyId body, Vec2 positionMeters, float angleRadians) {
    if (world_->find(body, "setTransform") != nullptr) {
        b2Body_SetTransform(toB2(body), toB2(positionMeters), b2MakeRot(angleRadians));
    }
}

void Box2DPhysicsAdapter::setLinearVelocity(BodyId body, Vec2 velocityMetersPerSecond) {
    if (world_->find(body, "setLinearVelocity") != nullptr) {
        b2Body_SetLinearVelocity(toB2(body), toB2(velocityMetersPerSecond));
    }
}

void Box2DPhysicsAdapter::applyForce(BodyId body, Vec2 forceNewtons) {
    if (world_->find(body, "applyForce") != nullptr) {
        b2Body_ApplyForceToCenter(toB2(body), toB2(forceNewtons), true);
    }
}

void Box2DPhysicsAdapter::applyImpulse(BodyId body, Vec2 impulseNewtonSeconds) {
    if (world_->find(body, "applyImpulse") != nullptr) {
        b2Body_ApplyLinearImpulseToCenter(toB2(body), toB2(impulseNewtonSeconds), true);
    }
}

void Box2DPhysicsAdapter::step(float dtSeconds) {
    world_->events.clear();
    if (!(dtSeconds > 0.0f && std::isfinite(dtSeconds))) {
        world_->error("step needs a time step above 0");
        return;
    }
    b2World_Step(world_->id, dtSeconds, world_->subStepCount);

    // Contact events are only valid until the next step, so translate them now (ip.fr.3.6).
    // Sensor events, tick numbers and collider pairs come with US-3.3.
    const b2ContactEvents contacts = b2World_GetContactEvents(world_->id);
    const auto add = [this](b2ShapeId a, b2ShapeId b, ContactPhase phase) {
        const std::optional<EntityId> entityA = world_->entityOfShape(a);
        const std::optional<EntityId> entityB = world_->entityOfShape(b);
        if (entityA && entityB) {
            const auto [low, high] = std::minmax(*entityA, *entityB);
            world_->events.push_back(CollisionEvent{low, high, phase});
        }
    };
    for (int i = 0; i < contacts.beginCount; ++i) {
        add(contacts.beginEvents[i].shapeIdA, contacts.beginEvents[i].shapeIdB,
            ContactPhase::Begin);
    }
    for (int i = 0; i < contacts.endCount; ++i) {
        add(contacts.endEvents[i].shapeIdA, contacts.endEvents[i].shapeIdB, ContactPhase::End);
    }
    std::sort(world_->events.begin(), world_->events.end(),
              [](const CollisionEvent& x, const CollisionEvent& y) {
                  return std::tie(x.entityA, x.entityB, x.phase) <
                         std::tie(y.entityA, y.entityB, y.phase);
              });
}

BodyState Box2DPhysicsAdapter::bodyState(BodyId body) const {
    if (world_->find(body, "bodyState") == nullptr) {
        return BodyState{};
    }
    const b2BodyId b2Body = toB2(body);
    return BodyState{fromB2(b2Body_GetPosition(b2Body)), b2Rot_GetAngle(b2Body_GetRotation(b2Body)),
                     fromB2(b2Body_GetLinearVelocity(b2Body)), b2Body_GetAngularVelocity(b2Body)};
}

std::span<const CollisionEvent> Box2DPhysicsAdapter::collisionEvents() const {
    return world_->events;
}

std::optional<RaycastHit> Box2DPhysicsAdapter::raycast(Vec2 originMeters, Vec2 direction,
                                                       float maxDistanceMeters) const {
    const float length = std::hypot(direction.x, direction.y);
    if (!(length > 0.0f) || !(maxDistanceMeters > 0.0f && std::isfinite(maxDistanceMeters))) {
        world_->error("raycast needs a direction and a maximum distance above 0");
        return std::nullopt;
    }
    // Closest hit only; layers, sensors and ties come with US-3.3 (srs.fr.3.17).
    const b2Vec2 translation{direction.x / length * maxDistanceMeters,
                             direction.y / length * maxDistanceMeters};
    const b2RayResult result =
        b2World_CastRayClosest(world_->id, toB2(originMeters), translation, b2DefaultQueryFilter());
    if (!result.hit) {
        return std::nullopt;
    }
    const std::optional<EntityId> entity = world_->entityOfShape(result.shapeId);
    if (!entity) {
        return std::nullopt;
    }
    return RaycastHit{*entity, fromB2(result.point), fromB2(result.normal)};
}

float Box2DPhysicsAdapter::bodyMassKilograms(BodyId body) const {
    if (world_->find(body, "bodyMassKilograms") == nullptr) {
        return 0.0f;
    }
    return b2Body_GetMass(toB2(body));
}

} // namespace engine::physics_box2d
