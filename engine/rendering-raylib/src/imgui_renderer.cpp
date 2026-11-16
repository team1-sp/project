#include "engine/rendering_raylib/imgui_renderer.hpp"

#include <utility>

#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

namespace engine::rendering_raylib {

namespace {

// rlImGui takes the font loader as a plain function pointer, so the renderer that is starting
// leaves its loader and scale here for the moment rlImGui calls it.
const ImGuiRenderer::SetupUi* startingSetup = nullptr;
float startingScale = 1.0F;

void loadFonts() {
    if (startingSetup != nullptr && *startingSetup) {
        (*startingSetup)(startingScale);
    } else {
        ImGui::GetIO().Fonts->AddFontDefault();
        ImGui::StyleColorsDark();
    }
}

} // namespace

ImGuiRenderer::ImGuiRenderer(DrawUi drawUi, SetupUi setupUi)
    : drawUi_(std::move(drawUi))
    , setupUi_(std::move(setupUi)) {
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
        start();
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

void ImGuiRenderer::start() {
    // On macOS, and on windows opened with the high-DPI flag, raylib already works in scaled
    // pixels; elsewhere sizes follow the display's scale, as rlImGui does for its default font.
    float scale = 1.0F;
#if !defined(__APPLE__)
    if (!IsWindowState(FLAG_WINDOW_HIGHDPI)) {
        scale = GetWindowScaleDPI().y;
    }
#endif
    startingSetup = &setupUi_;
    startingScale = scale > 0.0F ? scale : 1.0F;
    rlImGuiSetLoadFontsCallback(loadFonts);
    // Creates the context and calls loadFonts(); the second half adds the icons and the backend.
    rlImGuiBeginInitImGui();
    rlImGuiEndInitImGui();
    rlImGuiSetLoadFontsCallback(nullptr);
    startingSetup = nullptr;

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    started_ = true;
}

void ImGuiRenderer::releaseGraphics() {
    if (started_) {
        rlImGuiShutdown();
        started_ = false;
    }
}

} // namespace engine::rendering_raylib
