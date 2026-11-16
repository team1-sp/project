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
inline constexpr unsigned int accent = 0xFFBB793A;     // #3a79bb: selection, active tab line
inline constexpr unsigned int aiButton = 0xFFD04F6A;   // #6a4fd0: AI Assistant button
inline constexpr unsigned int mutedText = 0xFF8A8A8A;  // #8a8a8a
inline constexpr unsigned int offlineDot = 0xFF7A7A7A; // #7a7a7a: AI service offline
} // namespace themeColor

} // namespace editor
