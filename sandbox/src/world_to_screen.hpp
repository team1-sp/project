#pragma once

#include <numbers>

// PB-008 spike: meters (world) to pixels (screen). Decided in PB-008.3: the world is in meters
// with y up, as Box2D expects; the screen is raylib's, in pixels with y down and the origin at
// the top-left. This conversion happens only on the rendering side, never in physics code.
namespace sandbox {

inline constexpr float pixelsPerMeter = 32.0f;

struct ScreenPoint {
    float xPixels;
    float yPixels;
};

constexpr float metersToPixels(float meters) {
    return meters * pixelsPerMeter;
}

// World (0, 0) is drawn at the bottom-left corner of the window.
constexpr ScreenPoint worldToScreen(float xMeters, float yMeters, float screenHeightPixels) {
    return ScreenPoint{metersToPixels(xMeters), screenHeightPixels - metersToPixels(yMeters)};
}

// Box2D angles are radians, counter-clockwise in a y-up world. raylib takes degrees and, because
// its y axis points down, a positive angle turns clockwise on screen. So the sign flips.
constexpr float worldAngleToScreenDegrees(float angleRadians) {
    return -angleRadians * 180.0f / std::numbers::pi_v<float>;
}

} // namespace sandbox
