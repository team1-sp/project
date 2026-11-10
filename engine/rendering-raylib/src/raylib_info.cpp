#include "engine/rendering_raylib/raylib_info.hpp"

#include <raylib.h>

namespace engine::rendering_raylib {

Version linkedRaylibVersion() {
    return Version{RAYLIB_VERSION_MAJOR, RAYLIB_VERSION_MINOR, RAYLIB_VERSION_PATCH};
}

} // namespace engine::rendering_raylib
