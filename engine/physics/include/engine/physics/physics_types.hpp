#pragma once

#include <cstdint>

#include "engine/core/entity_id.hpp"
#include "engine/core/vec2.hpp"

namespace engine::physics {

// Our handle for one body (physics-adapter-design.md section 2). Opaque: only the adapter
// implementation knows what the number means; 64 bits so it can hold a library's own handle.
enum class BodyId : std::uint64_t {};
inline constexpr BodyId noBody{0};

// Where a body is and how it moves, read back after a step (written to the ECS by
// PhysicsSyncOut). Meters, radians, y up (DEC-036).
struct BodyState {
    Vec2 positionMeters{};
    float angleRadians = 0.0f;
    Vec2 linearVelocityMetersPerSecond{};
    float angularVelocityRadiansPerSecond = 0.0f;
};

enum class ContactPhase {
    Begin, // the two bodies started touching in this step
    End,   // they stopped touching in this step
};

// One contact change in the last step, as entities, never library body handles (ip.fr.3.6).
struct CollisionEvent {
    EntityId entityA = noEntity; // always the smaller id of the two
    EntityId entityB = noEntity;
    ContactPhase phase = ContactPhase::Begin;
};

// The first body a ray hits (ip.fr.3.7).
struct RaycastHit {
    EntityId entity = noEntity;
    Vec2 pointMeters{};
    Vec2 normal{}; // unit vector pointing out of the surface that was hit
};

} // namespace engine::physics
