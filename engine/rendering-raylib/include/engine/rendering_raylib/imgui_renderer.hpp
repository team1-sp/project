#pragma once

#include <functional>

#include "engine/core/frame_renderer.hpp"

namespace engine::rendering_raylib {

// Draws a Dear ImGui user interface each render frame (DEC-063). It owns the Dear ImGui context
// and its raylib backend, rlImGui, so the rest of the application uses only imgui.h and never
// raylib (R-4). The Editor's main window is drawn through it (srs.fr.14.1).
//
// The context is created in the first render frame, once the window and its graphics context
// exist, and released in releaseGraphics(), before the window closes.
class ImGuiRenderer final : public IFrameRenderer {
public:
    // Called once per render frame, between ImGui::NewFrame() and ImGui::Render(), to build the
    // frame's interface with the imgui.h functions.
    using DrawUi = std::function<void()>;

    explicit ImGuiRenderer(DrawUi drawUi);
    ~ImGuiRenderer() override;

    ImGuiRenderer(const ImGuiRenderer&) = delete;
    ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;

    void renderFrame() override;
    void releaseGraphics() override;

private:
    DrawUi drawUi_;
    bool started_ = false;
    // raylib time, in seconds, of the previous render frame; Dear ImGui animates with the
    // difference.
    double lastFrameSeconds_ = 0.0;
};

} // namespace engine::rendering_raylib
