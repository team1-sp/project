#pragma once

namespace engine {

// A 2D vector: a position, a velocity or a direction. In the simulation it is in meters
// (or meters per second), with y pointing up (DEC-036).
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    friend bool operator==(Vec2, Vec2) = default;
};

} // namespace engine
