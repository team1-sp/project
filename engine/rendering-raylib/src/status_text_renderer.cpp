#include "engine/rendering_raylib/status_text_renderer.hpp"

#include <utility>

#include <raylib.h>

namespace engine::rendering_raylib {

StatusTextRenderer::StatusTextRenderer(TextSource textSource)
    : textSource_(std::move(textSource)) {
}

void StatusTextRenderer::renderFrame() {
    constexpr int marginPixels = 20;
    constexpr int fontSizePixels = 20;

    const std::string text = textSource_();
    DrawText(text.c_str(), marginPixels, marginPixels, fontSizePixels, DARKGRAY);
}

} // namespace engine::rendering_raylib
