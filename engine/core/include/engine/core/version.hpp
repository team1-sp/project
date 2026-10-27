#pragma once

namespace engine {

// A library or engine version, independent of how each library spells it.
struct Version {
    int major = 0;
    int minor = 0;
    int patch = 0;
};

// The engine's own version, taken from the root CMake project.
Version engineVersion();

} // namespace engine
