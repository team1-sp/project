#pragma once

#include <cstdint>
#include <string_view>

#include "engine/core/vec2.hpp"

namespace engine {

class TypeRegistry;

// The components that give an entity a physics body (design/ecs-design.md §2, US-3.1). An entity
// with at least one collider gets one body; its RigidBody, if any, sets how it moves, and without
// one the body is static (srs.fr.3.2). Plain data (R-8): PhysicsSyncIn turns them into bodies and
// shapes through the Physics Adapter (DEC-041). Units: meters, kilograms, y up (DEC-036).

// How a body moves. The values match engine::physics::BodyType, which the ECS cannot include.
enum class RigidBodyType : std::uint8_t {
    Static,    // never moves: floors, walls
    Kinematic, // moves only at its Velocity: moving platforms
    Dynamic,   // moved by gravity, forces and collisions
};

// How the entity's body moves (srs.fr.3.2, srs.fr.3.6).
struct RigidBodyComponent {
    static constexpr std::string_view typeName = "RigidBody";
    RigidBodyType bodyType = RigidBodyType::Dynamic;
    float massKilograms = 1.0f; // dynamic bodies only (srs.fr.3.46)
    float gravityScale = 1.0f;
    float linearDamping = 0.0f;
    bool fixedRotation = false;
};

// A box shape on the entity's body (srs.fr.3.3, srs.fr.3.7).
struct BoxColliderComponent {
    static constexpr std::string_view typeName = "BoxCollider";
    float widthMeters = 1.0f;
    float heightMeters = 1.0f;
    float offsetXMeters = 0.0f;
    float offsetYMeters = 0.0f;
    float friction = 0.6f;
    float restitution = 0.0f;
    bool isSensor = false;
};

// A circle shape on the entity's body (srs.fr.3.4, srs.fr.3.7).
struct CircleColliderComponent {
    static constexpr std::string_view typeName = "CircleCollider";
    float radiusMeters = 0.5f;
    float offsetXMeters = 0.0f;
    float offsetYMeters = 0.0f;
    float friction = 0.6f;
    float restitution = 0.0f;
    bool isSensor = false;
};

// A convex polygon shape on the entity's body, 3 to 8 vertices (srs.fr.3.5, srs.fr.3.7). The
// vertices are separate fields because the ECS has no array field type yet; only the first
// `vertexCount` are used. The default is a 1 m square.
struct PolygonColliderComponent {
    static constexpr std::string_view typeName = "PolygonCollider";
    std::uint8_t vertexCount = 4;
    Vec2 vertex0{-0.5f, -0.5f};
    Vec2 vertex1{0.5f, -0.5f};
    Vec2 vertex2{0.5f, 0.5f};
    Vec2 vertex3{-0.5f, 0.5f};
    Vec2 vertex4{};
    Vec2 vertex5{};
    Vec2 vertex6{};
    Vec2 vertex7{};
    float offsetXMeters = 0.0f;
    float offsetYMeters = 0.0f;
    float friction = 0.6f;
    float restitution = 0.0f;
    bool isSensor = false;
};

// Registers RigidBodyType and the four components above with their descriptions.
void registerPhysicsComponents(TypeRegistry& types);

} // namespace engine
