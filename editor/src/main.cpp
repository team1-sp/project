#include <cstdio>

#include "engine/core/version.hpp"

int main() {
    const engine::Version version = engine::engineVersion();
    std::printf("editor %d.%d.%d: not implemented yet\n", version.major, version.minor,
                version.patch);
    return 0;
}
