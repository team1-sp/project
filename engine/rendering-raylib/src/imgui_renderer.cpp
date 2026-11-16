#include "engine/rendering_raylib/imgui_renderer.hpp"

#include <utility>

#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

namespace engine::rendering_raylib {

ImGuiRenderer::ImGuiRenderer(DrawUi drawUi)
    : drawUi_(std::move(drawUi)) {
}

ImGuiRenderer::~ImGuiRenderer() {
    releaseGraphics();
}

void ImGuiRenderer::renderFrame() {
    // 1/60 s for the first frame, as Dear ImGui needs a time step greater than zero.
    constexpr double firstFrameSeconds = 1.0 / 60.0;

    const double now = GetTime();
    double deltaSeconds = now - lastFrameSeconds_;
    if (!started_) {
        rlImGuiSetup(true);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        started_ = true;
        deltaSeconds = firstFrameSeconds;
    }
    lastFrameSeconds_ = now;
    if (deltaSeconds <= 0.0) {
        deltaSeconds = firstFrameSeconds;
    }

    // rlImGuiBegin() would read raylib's frame time, which custom frame control (DEC-051) does
    // not update, so the time step is measured here.
    rlImGuiBeginDelta(static_cast<float>(deltaSeconds));
    if (drawUi_) {
        drawUi_();
    }
    rlImGuiEnd();
}

void ImGuiRenderer::releaseGraphics() {
    if (started_) {
        rlImGuiShutdown();
        started_ = false;
    }
}

} // namespace engine::rendering_raylib
