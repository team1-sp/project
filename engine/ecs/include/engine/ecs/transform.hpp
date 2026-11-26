#pragma once

#include <cmath>

#include "engine/core/vec2.hpp"
#include "engine/ecs/core_components.hpp"

namespace engine {

// An entity's transform in world space: position, rotation and per-axis scale, no shear
// (srs.fr.3.42). Meters, radians counter-clockwise, y up (DEC-036).
struct WorldTransform {
    Vec2 position{};
    float rotationRadians = 0.0f;
    Vec2 scale{1.0f, 1.0f};
};

// The transform a root entity's Position is relative to (srs.fr.3.49).
inline constexpr WorldTransform worldOrigin{};

// Whether exactly one axis of a scale is negative: such a parent mirrors its children, which
// turns their rotation the other way (srs.fr.3.49).
[[nodiscard]] inline bool mirrors(Vec2 scale) {
    return (scale.x < 0.0f) != (scale.y < 0.0f);
}

[[nodiscard]] inline Vec2 rotate(float angleRadians, Vec2 v) {
    const float c = std::cos(angleRadians);
    const float s = std::sin(angleRadians);
    return Vec2{c * v.x - s * v.y, s * v.x + c * v.y};
}

// srs.fr.3.49: the world transform of an entity whose Position is `local`, under a parent whose
// world transform is `parent` (worldOrigin for a root entity).
[[nodiscard]] inline WorldTransform compose(const WorldTransform& parent,
                                            const PositionComponent& local) {
    const Vec2 scaled{parent.scale.x * local.xMeters, parent.scale.y * local.yMeters};
    const Vec2 offset = rotate(parent.rotationRadians, scaled);
    return WorldTransform{Vec2{parent.position.x + offset.x, parent.position.y + offset.y},
                          mirrors(parent.scale) ? parent.rotationRadians - local.rotationRadians
                                                : parent.rotationRadians + local.rotationRadians,
                          Vec2{parent.scale.x * local.scaleX, parent.scale.y * local.scaleY}};
}

// The inverse of compose() for position and rotation (srs.fr.3.45, 3.51): the Position that puts
// an entity at `worldPosition` and `worldRotationRadians` under `parent`, keeping its local scale.
// The parent's scale is never 0 on an axis (srs.fr.3.44).
[[nodiscard]] inline PositionComponent localFromWorld(const WorldTransform& parent,
                                                      Vec2 worldPosition,
                                                      float worldRotationRadians, Vec2 localScale) {
    const Vec2 unrotated =
        rotate(-parent.rotationRadians,
               Vec2{worldPosition.x - parent.position.x, worldPosition.y - parent.position.y});
    return PositionComponent{unrotated.x / parent.scale.x, unrotated.y / parent.scale.y,
                             mirrors(parent.scale) ? parent.rotationRadians - worldRotationRadians
                                                   : worldRotationRadians - parent.rotationRadians,
                             localScale.x, localScale.y};
}

} // namespace engine
