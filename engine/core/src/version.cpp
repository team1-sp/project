#include "engine/core/version.hpp"

#include "engine/core/version_config.hpp"

namespace engine {

Version engineVersion() {
    return Version{build::versionMajor, build::versionMinor, build::versionPatch};
}

} // namespace engine
