#include "theme.hpp"

#include <cstddef>

#include <imgui.h>

// The fonts, embedded by cmake/EmbedFile.cmake.
extern const unsigned char interRegularTtf[];
extern const std::size_t interRegularTtfSize;
extern const unsigned char interSemiBoldTtf[];
extern const std::size_t interSemiBoldTtfSize;
extern const unsigned char jetBrainsMonoTtf[];
extern const std::size_t jetBrainsMonoTtfSize;

namespace editor {

namespace {

ThemeFonts fonts;

// The design's interface text is 12 to 13 px.
constexpr float fontSizePixels = 13.0F;

ImVec4 rgb(unsigned int hex, float alpha = 1.0F) {
    constexpr float channelMax = 255.0F;
    return ImVec4(static_cast<float>((hex >> 16) & 0xFFU) / channelMax,
                  static_cast<float>((hex >> 8) & 0xFFU) / channelMax,
                  static_cast<float>(hex & 0xFFU) / channelMax, alpha);
}

ImFont* addFont(const unsigned char* data, std::size_t size, float sizePixels) {
    ImFontConfig config;
    // The bytes are part of the executable, so the atlas must not free them.
    config.FontDataOwnedByAtlas = false;
    config.OversampleH = 2;
    return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(data),
                                                      static_cast<int>(size), sizePixels, &config);
}

// Colours from the palette of the Figma design (aegis-figma-update-6).
void setColours(ImGuiStyle& style) {
    ImVec4* c = style.Colors;
    c[ImGuiCol_Text] = rgb(0xE6E6E6);
    c[ImGuiCol_TextDisabled] = rgb(0x6A6A6A);
    c[ImGuiCol_WindowBg] = rgb(0x282828);
    c[ImGuiCol_ChildBg] = rgb(0x282828);
    c[ImGuiCol_PopupBg] = rgb(0x2D2D2D);
    c[ImGuiCol_Border] = rgb(0x1A1A1A);
    c[ImGuiCol_BorderShadow] = rgb(0x000000, 0.0F);
    c[ImGuiCol_FrameBg] = rgb(0x1E1E1E);
    c[ImGuiCol_FrameBgHovered] = rgb(0x2A2A2A);
    c[ImGuiCol_FrameBgActive] = rgb(0x303030);
    c[ImGuiCol_TitleBg] = rgb(0x1F1F1F);
    c[ImGuiCol_TitleBgActive] = rgb(0x1F1F1F);
    c[ImGuiCol_TitleBgCollapsed] = rgb(0x1F1F1F);
    c[ImGuiCol_MenuBarBg] = rgb(0x232323);
    c[ImGuiCol_ScrollbarBg] = rgb(0x232323);
    c[ImGuiCol_ScrollbarGrab] = rgb(0x4A4A4A);
    c[ImGuiCol_ScrollbarGrabHovered] = rgb(0x555555);
    c[ImGuiCol_ScrollbarGrabActive] = rgb(0x5E5E5E);
    c[ImGuiCol_CheckMark] = rgb(0x6AA8E8);
    c[ImGuiCol_SliderGrab] = rgb(0x6AA8E8);
    c[ImGuiCol_SliderGrabActive] = rgb(0x7FB2E5);
    c[ImGuiCol_Button] = rgb(0x3A3A3A);
    c[ImGuiCol_ButtonHovered] = rgb(0x4A4A4A);
    c[ImGuiCol_ButtonActive] = rgb(0x3A79BB);
    c[ImGuiCol_Header] = rgb(0x2C5D87);
    c[ImGuiCol_HeaderHovered] = rgb(0x3A3A3A);
    c[ImGuiCol_HeaderActive] = rgb(0x3A79BB);
    c[ImGuiCol_Separator] = rgb(0x3A3A3A);
    c[ImGuiCol_SeparatorHovered] = rgb(0x3A79BB);
    c[ImGuiCol_SeparatorActive] = rgb(0x3A79BB);
    c[ImGuiCol_ResizeGrip] = rgb(0x000000, 0.0F);
    c[ImGuiCol_ResizeGripHovered] = rgb(0x3A79BB, 0.6F);
    c[ImGuiCol_ResizeGripActive] = rgb(0x3A79BB);
    c[ImGuiCol_InputTextCursor] = rgb(0xE6E6E6);
    c[ImGuiCol_Tab] = rgb(0x2D2D2D);
    c[ImGuiCol_TabHovered] = rgb(0x3A3A3A);
    c[ImGuiCol_TabSelected] = rgb(0x3C3C3C);
    c[ImGuiCol_TabSelectedOverline] = rgb(0x3A79BB);
    c[ImGuiCol_TabDimmed] = rgb(0x2D2D2D);
    c[ImGuiCol_TabDimmedSelected] = rgb(0x353535);
    c[ImGuiCol_TabDimmedSelectedOverline] = rgb(0x3A79BB);
    c[ImGuiCol_DockingPreview] = rgb(0x3A79BB, 0.6F);
    c[ImGuiCol_DockingEmptyBg] = rgb(0x1B1B1B);
    c[ImGuiCol_TableHeaderBg] = rgb(0x2D2D2D);
    c[ImGuiCol_TableBorderStrong] = rgb(0x1A1A1A);
    c[ImGuiCol_TableBorderLight] = rgb(0x2E2E2E);
    c[ImGuiCol_TextSelectedBg] = rgb(0x3A79BB, 0.4F);
    c[ImGuiCol_NavCursor] = rgb(0x3A79BB);
    c[ImGuiCol_ModalWindowDimBg] = rgb(0x000000, 0.41F);
}

// Sizes of the design: 3 px corners, compact paddings and 22 px tabs.
void setSizes(ImGuiStyle& style, float uiScale) {
    style.WindowRounding = 0.0F;
    style.ChildRounding = 0.0F;
    style.FrameRounding = 3.0F;
    style.PopupRounding = 3.0F;
    style.ScrollbarRounding = 3.0F;
    style.GrabRounding = 3.0F;
    style.TabRounding = 3.0F;
    style.WindowBorderSize = 1.0F;
    style.FrameBorderSize = 0.0F;
    style.PopupBorderSize = 1.0F;
    style.TabBorderSize = 0.0F;
    style.TabBarBorderSize = 1.0F;
    style.TabBarOverlineSize = 2.0F;
    style.WindowPadding = ImVec2(8.0F, 6.0F);
    style.FramePadding = ImVec2(8.0F, 4.0F);
    style.ItemSpacing = ImVec2(6.0F, 4.0F);
    style.ItemInnerSpacing = ImVec2(4.0F, 4.0F);
    style.ScrollbarSize = 12.0F;
    style.DockingSeparatorSize = 2.0F;
    style.DisabledAlpha = 0.45F;
    style.ScaleAllSizes(uiScale);
    style.FontScaleDpi = uiScale;
}

} // namespace

void applyTheme(float uiScale) {
    // Inter Regular is loaded last: the backend merges the toolbar icons into the last font.
    fonts.mono = addFont(jetBrainsMonoTtf, jetBrainsMonoTtfSize, fontSizePixels);
    fonts.semiBold = addFont(interSemiBoldTtf, interSemiBoldTtfSize, fontSizePixels);
    fonts.regular = addFont(interRegularTtf, interRegularTtfSize, fontSizePixels);
    ImGui::GetIO().FontDefault = fonts.regular;

    ImGuiStyle& style = ImGui::GetStyle();
    style = ImGuiStyle();
    setColours(style);
    setSizes(style, uiScale);
}

const ThemeFonts& themeFonts() {
    return fonts;
}

} // namespace editor
