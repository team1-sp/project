#pragma once

#include <string_view>

namespace engine {

class TypeRegistry;

// The engine's own components that physics, gameplay and rendering share (design/ecs-design.md
// §2). Neither has an enabled flag of its own (srs.fr.2.11): disabling the entity hides them.
// Units: meters, radians, seconds, y up (DEC-036).

// Where the entity is in the world.
struct PositionComponent {
    static constexpr std::string_view typeName = "Position";
    float xMeters = 0.0f;
    float yMeters = 0.0f;
    float rotationRadians = 0.0f; // counter-clockwise
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

// How fast the entity moves. For a dynamic body the physics body owns it and PhysicsSyncOut
// writes it back (DEC-040).
struct VelocityComponent {
    static constexpr std::string_view typeName = "Velocity";
    float xMetersPerSecond = 0.0f;
    float yMetersPerSecond = 0.0f;
};

// Registers Position and Velocity with their descriptions.
void registerCoreComponents(TypeRegistry& types);

} // namespace engine
