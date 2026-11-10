#pragma once

#include "engine/core/vec2.hpp"

namespace engine::physics {

// How a body moves (ecs-design.md, RigidBodyComponent).
enum class BodyType {
    Static,    // never moves: floors, walls
    Kinematic, // moves only at the velocity we give it: moving platforms
    Dynamic,   // moved by gravity, forces and collisions: the player, crates
};

enum class ShapeType {
    Box,
    Circle,
};

// Everything the adapter needs to create one body. Units: meters, kilograms, radians, y up
// (DEC-036). Fields mirror RigidBodyComponent and Box/CircleColliderComponent.
struct BodyDesc {
    BodyType type = BodyType::Static;
    Vec2 positionMeters{};
    float angleRadians = 0.0f;

    ShapeType shape = ShapeType::Box;
    Vec2 boxSizeMeters{1.0f, 1.0f};  // width and height; used when shape is Box
    float circleRadiusMeters = 0.5f; // used when shape is Circle
    Vec2 shapeOffsetMeters{};        // shape centre relative to the body position

    float friction = 0.6f;
    float restitution = 0.0f; // bounciness: 0 = no bounce, 1 = bounces back fully
    bool isSensor = false;    // detects overlaps but nothing bumps into it

    float massKilograms = 1.0f; // dynamic bodies only
    float gravityScale = 1.0f;
    float linearDamping = 0.0f; // slows the body down over time, like air resistance
    bool fixedRotation = false; // true = never rotates (typical for a player character)
};

} // namespace engine::physics
