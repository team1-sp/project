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
    // Called once, right after the Dear ImGui context is created, to load the fonts and set the
    // style. `uiScale` is the display's scale factor (1 at 100 %); fonts and sizes are multiplied
    // by it. The backend merges its Font Awesome icons into the last font loaded. Empty: Dear
    // ImGui's default font and dark style.
    using SetupUi = std::function<void(float uiScale)>;

    explicit ImGuiRenderer(DrawUi drawUi, SetupUi setupUi = {});
    ~ImGuiRenderer() override;

    ImGuiRenderer(const ImGuiRenderer&) = delete;
    ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;

    void renderFrame() override;
    void releaseGraphics() override;

private:
    void start();

    DrawUi drawUi_;
    SetupUi setupUi_;
    bool started_ = false;
    // raylib time, in seconds, of the previous render frame; Dear ImGui animates with the
    // difference.
    double lastFrameSeconds_ = 0.0;
};

} // namespace engine::rendering_raylib
