#pragma once

#include <array>
#include <cstdint>

#include "engine/core/vec2.hpp"

namespace engine::physics {

// How a body moves (ecs-design.md, RigidBodyComponent; srs.fr.3.2).
enum class BodyType {
    Static,    // never moves: floors, walls
    Kinematic, // moves only at the velocity we give it: moving platforms
    Dynamic,   // moved by gravity, forces and collisions: the player, crates
};

// Everything the adapter needs to create one body, without its shapes (shapes are added with
// IPhysicsAdapter::addShape). Units: kilograms, meters, radians, y up (DEC-036). Fields mirror
// RigidBodyComponent; an entity without one gets a default, static BodyDesc (srs.fr.3.2).
struct BodyDesc {
    BodyType type = BodyType::Static;
    Vec2 positionMeters{};
    float angleRadians = 0.0f;

    float massKilograms = 1.0f; // dynamic bodies only (srs.fr.3.6, srs.fr.3.46)
    float gravityScale = 1.0f;
    float linearDamping = 0.0f; // slows the body down over time, like air resistance
    bool fixedRotation = false; // true = never rotates (typical for a player character)
};

enum class ShapeType {
    Box,     // srs.fr.3.3
    Circle,  // srs.fr.3.4
    Polygon, // srs.fr.3.5: convex, 3 to maxPolygonVertices vertices
};

// The most vertices Box2D v3.1.1 polygons allow (srs.fr.3.5).
inline constexpr int maxPolygonVertices = 8;

// One collider shape on a body. Mirrors Box/Circle/PolygonColliderComponent. Positions are
// relative to the body position, in meters.
struct ShapeDesc {
    ShapeType type = ShapeType::Box;
    Vec2 boxSizeMeters{1.0f, 1.0f};                               // width and height; Box only
    float circleRadiusMeters = 0.5f;                              // Circle only
    std::array<Vec2, maxPolygonVertices> polygonVerticesMeters{}; // Polygon only
    std::uint8_t polygonVertexCount = 0;                          // Polygon only
    Vec2 offsetMeters{}; // shape centre relative to the body; moves polygon vertices too

    float friction = 0.6f;    // srs.fr.3.7
    float restitution = 0.0f; // bounciness: 0 = no bounce, 1 = bounces back fully
    bool isSensor = false;    // detects overlaps but pushes nothing
};

} // namespace engine::physics
