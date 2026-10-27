#include <cstdio>
#include <cstdlib>
#include <string_view>

#include "engine/core/version.hpp"
#include "engine/physics_box2d/box2d_info.hpp"
#include "engine/rendering_raylib/smoke_window.hpp"

namespace {

void printVersion(const char* name, engine::Version version) {
    std::printf("%s %d.%d.%d\n", name, version.major, version.minor, version.patch);
}

} // namespace

// Usage: demo-maze [--frames N]   (N = 0, the default, runs until the window is closed)
int main(int argc, char** argv) {
    int maxFrames = 0;
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--frames" && i + 1 < argc) {
            maxFrames = std::atoi(argv[++i]);
        }
    }

    printVersion("engine", engine::engineVersion());
    printVersion("box2d", engine::physics_box2d::linkedBox2dVersion());
    printVersion("raylib", engine::rendering_raylib::linkedRaylibVersion());

    engine::rendering_raylib::runSmokeWindow("demo-maze", maxFrames);
    return 0;
}
