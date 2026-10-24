#pragma once

#include <string_view>

#include "engine/core/version.hpp"

namespace engine::rendering_raylib {

// The raylib version this target was compiled against (DEC-033).
Version linkedRaylibVersion();

// Opens a window, draws a status line each frame and closes it again. Proves the build
// links raylib and can open a window; the real renderer replaces it.
// maxFrames == 0 runs until the window is closed or Escape is pressed.
void runSmokeWindow(std::string_view title, int maxFrames);

} // namespace engine::rendering_raylib
