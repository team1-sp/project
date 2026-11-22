#pragma once

#include <functional>
#include <string>

#include "engine/core/frame_renderer.hpp"

namespace engine::rendering_raylib {

// Draws one piece of status text in the top-left corner each render frame. It stands in for the
// real renderer until that exists, so an application can show that its loop is running.
class StatusTextRenderer final : public IFrameRenderer {
public:
    // Called once per render frame for the text to draw.
    using TextSource = std::function<std::string()>;

    explicit StatusTextRenderer(TextSource textSource);

    void renderFrame() override;

private:
    TextSource textSource_;
};

} // namespace engine::rendering_raylib
