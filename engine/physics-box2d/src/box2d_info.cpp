#include "engine/physics_box2d/box2d_info.hpp"

#include <box2d/box2d.h>

namespace engine::physics_box2d {

Version linkedBox2dVersion() {
    const b2Version version = b2GetVersion();
    return Version{version.major, version.minor, version.revision};
}

} // namespace engine::physics_box2d
