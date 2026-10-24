#pragma once

#include "engine/core/version.hpp"

namespace engine::physics_box2d {

// The Box2D version this target was linked against. Lets tests check the pin (DEC-020)
// without any Box2D type leaving this target.
Version linkedBox2dVersion();

} // namespace engine::physics_box2d
