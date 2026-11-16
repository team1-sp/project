#pragma once

struct ImFont;

namespace editor {

// The fonts of the Aegis Editor UI design: Inter for the interface, Inter SemiBold for headings
// and JetBrains Mono for numbers (DEC-064). Null until applyTheme() has run.
struct ThemeFonts {
    ImFont* regular = nullptr;
    ImFont* semiBold = nullptr;
    ImFont* mono = nullptr;
};

// Loads the embedded fonts and sets Dear ImGui's colours and sizes to those of the Figma design
// (B-01, C-01). Call it once, when the Dear ImGui context has been created and before the first
// frame; `uiScale` multiplies fonts and sizes for high-DPI displays.
void applyTheme(float uiScale);

[[nodiscard]] const ThemeFonts& themeFonts();

// Colours of the design that widgets use directly, as 0xAABBGGRR values for ImDrawList and
// ImGui::PushStyleColor.
namespace themeColor {
inline constexpr unsigned int accent = 0xFFBB793A;    // #3a79bb: selection, active tab line
inline constexpr unsigned int aiButton = 0xFFD04F6A;  // #6a4fd0: AI Assistant button
inline constexpr unsigned int mutedText = 0xFF8A8A8A; // #8a8a8a
// Status bar (B-01, B-04).
inline constexpr unsigned int statusBarBg = 0xFF2A2A2A;     // #2a2a2a
inline constexpr unsigned int statusBarBorder = 0xFF1A1A1A; // #1a1a1a: line above it
inline constexpr unsigned int statusText = 0xFFA8A8A8;      // #a8a8a8
inline constexpr unsigned int statusHover = 0xFF3A3A3A; // #3a3a3a: a clickable item under the mouse
inline constexpr unsigned int warningText = 0xFF3CB0E0; // #e0b03c: active debug flags
inline constexpr unsigned int onlineDot = 0xFF3CCB6B;   // #6bcb3c: AI service connected
inline constexpr unsigned int offlineDot = 0xFF6A6A6A;  // #6a6a6a: AI service offline
inline constexpr unsigned int recordingDot = 0xFF4A4AE0; // #e04a4a: telemetry recording
} // namespace themeColor

} // namespace editor
