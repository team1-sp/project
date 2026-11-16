#include "main_window_view.hpp"

#include <array>
#include <cstddef>
#include <cstdio>
#include <utility>

#include <imgui.h>
#include <imgui_internal.h>

#include "theme.hpp"
#include <IconsFontAwesome6.h>

namespace editor {

namespace {

constexpr const char* dockSpaceName = "MainDockSpace";

// Indexed by Key, in the order of its enumerators.
constexpr std::array<ImGuiKey, 31> imguiKeys = {
    ImGuiKey_A,         ImGuiKey_B,  ImGuiKey_C,  ImGuiKey_D,      ImGuiKey_E,      ImGuiKey_F,
    ImGuiKey_H,         ImGuiKey_N,  ImGuiKey_O,  ImGuiKey_P,      ImGuiKey_Q,      ImGuiKey_R,
    ImGuiKey_S,         ImGuiKey_T,  ImGuiKey_V,  ImGuiKey_W,      ImGuiKey_X,      ImGuiKey_Y,
    ImGuiKey_Z,         ImGuiKey_1,  ImGuiKey_2,  ImGuiKey_3,      ImGuiKey_4,      ImGuiKey_5,
    ImGuiKey_6,         ImGuiKey_F2, ImGuiKey_F4, ImGuiKey_Delete, ImGuiKey_Escape, ImGuiKey_Comma,
    ImGuiKey_Backslash,
};

// Dear ImGui's Ctrl stands for Cmd on macOS (ConfigMacOSXBehaviors), as srs.fr.14.6 asks.
ImGuiKeyChord keyChord(const Shortcut& shortcut) {
    ImGuiKeyChord chord = imguiKeys.at(static_cast<std::size_t>(shortcut.key));
    if (shortcut.ctrl) {
        chord |= ImGuiMod_Ctrl;
    }
    if (shortcut.shift) {
        chord |= ImGuiMod_Shift;
    }
    if (shortcut.alt) {
        chord |= ImGuiMod_Alt;
    }
    return chord;
}

// The Default layout of srs.fr.14.1 and B-01, built in the dock space `dockSpaceId`.
void buildDefaultLayout(ImGuiID dockSpaceId, const ImGuiViewport& viewport) {
    constexpr float sidePanelShare = 0.2F;
    constexpr float inspectorShare = 0.25F;
    constexpr float bottomShare = 0.3F;

    ImGui::DockBuilderRemoveNode(dockSpaceId);
    ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport.WorkSize);

    ImGuiID centre = dockSpaceId;
    const ImGuiID bottom =
        ImGui::DockBuilderSplitNode(centre, ImGuiDir_Down, bottomShare, nullptr, &centre);
    const ImGuiID left =
        ImGui::DockBuilderSplitNode(centre, ImGuiDir_Left, sidePanelShare, nullptr, &centre);
    const ImGuiID right =
        ImGui::DockBuilderSplitNode(centre, ImGuiDir_Right, inspectorShare, nullptr, &centre);

    ImGui::DockBuilderDockWindow(panelName(Panel::Hierarchy), left);
    ImGui::DockBuilderDockWindow(panelName(Panel::Scene), centre);
    ImGui::DockBuilderDockWindow(panelName(Panel::Game), centre);
    ImGui::DockBuilderDockWindow(panelName(Panel::Inspector), right);
    ImGui::DockBuilderDockWindow(panelName(Panel::Project), bottom);
    ImGui::DockBuilderDockWindow(panelName(Panel::Console), bottom);
    ImGui::DockBuilderFinish(dockSpaceId);
}

} // namespace

MainWindowView::MainWindowView(EditorSession& session, CloseEditor closeEditor, SetTitle setTitle)
    : session_(session)
    , closeEditor_(std::move(closeEditor))
    , setTitle_(std::move(setTitle))
    , platform_(currentPlatform())
    , menus_(buildMainMenu(*this))
    , toolbar_(buildToolbar()) {
}

void MainWindowView::draw() {
    if (frameIndex_ == 0) {
        // Saved layouts come with the layout stories; until then every start shows the Default
        // layout, so Dear ImGui writes no imgui.ini.
        ImGui::GetIO().IniFilename = nullptr;
    }

    updateTitle();
    for (const Menu& menu : menus_) {
        runShortcuts(menu.entries);
    }
    drawMenuBar();
    // The toolbar and the status bar take their height off the main viewport's work area first,
    // so the dock space fills what is left between them.
    drawToolbar();
    drawStatusBar();
    drawPanels();
    if (frameIndex_ < 2) {
        ++frameIndex_;
    }
}

void MainWindowView::exit() {
    if (session_.confirmClose() && closeEditor_) {
        closeEditor_();
    }
}

void MainWindowView::showPanel(Panel panel) {
    panelToFocus_ = panel;
}

void MainWindowView::updateTitle() {
    std::string title = session_.mainWindowTitle();
    if (title != shownTitle_) {
        shownTitle_ = std::move(title);
        if (setTitle_) {
            setTitle_(shownTitle_);
        }
    }
}

void MainWindowView::runShortcuts(const std::vector<MenuEntry>& entries) {
    for (const MenuEntry& entry : entries) {
        runShortcuts(entry.submenu);
        const std::optional<Shortcut> shortcut = entry.command.shortcutOn(platform_);
        if (shortcut && entry.command.isEnabled() &&
            ImGui::Shortcut(keyChord(*shortcut), ImGuiInputFlags_RouteGlobal)) {
            entry.command.run();
        }
    }
}

void MainWindowView::drawMenuBar() {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }
    for (const Menu& menu : menus_) {
        if (ImGui::BeginMenu(menu.title.c_str())) {
            drawMenuEntries(menu.entries);
            ImGui::EndMenu();
        }
    }
    ImGui::EndMainMenuBar();
}

void MainWindowView::drawMenuEntries(const std::vector<MenuEntry>& entries) {
    for (const MenuEntry& entry : entries) {
        if (entry.separatorBefore) {
            ImGui::Separator();
        }
        const char* label = entry.command.label.c_str();
        if (entry.isSubmenu) {
            if (ImGui::BeginMenu(label, !entry.submenu.empty())) {
                drawMenuEntries(entry.submenu);
                ImGui::EndMenu();
            }
            continue;
        }
        const std::optional<Shortcut> shortcut = entry.command.shortcutOn(platform_);
        const std::string shortcutLabel = shortcut ? shortcutText(*shortcut, platform_) : "";
        // A disabled item is drawn in muted text and does not highlight under the mouse
        // (srs.fr.14.5).
        if (ImGui::MenuItem(label, shortcutLabel.c_str(), false, entry.command.isEnabled())) {
            entry.command.run();
        }
    }
}

void MainWindowView::drawToolbar() {
    constexpr ImGuiWindowFlags barFlags = ImGuiWindowFlags_NoScrollbar |
                                          ImGuiWindowFlags_NoSavedSettings |
                                          ImGuiWindowFlags_NoScrollWithMouse;
    // Buttons of C-01 by their index in buildToolbar(), with the design's icons.
    constexpr std::array<const char*, 12> icons = {
        ICON_FA_HAND,          ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT,
        ICON_FA_ROTATE,        ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER,
        ICON_FA_VECTOR_SQUARE, ICON_FA_CROSSHAIRS,
        ICON_FA_GLOBE,         ICON_FA_TABLE_CELLS,
        ICON_FA_PLAY,          ICON_FA_PAUSE,
        ICON_FA_FORWARD_STEP,  ICON_FA_WAND_MAGIC,
    };
    // Pivot, Global, Snap and AI Assistant show their name beside the icon.
    constexpr std::size_t firstToggle = 5;
    constexpr std::size_t firstRunButton = 8;
    constexpr std::size_t aiButton = 11;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImGuiStyle& style = ImGui::GetStyle();
    const float height = ImGui::GetFrameHeight() + 2.0F * style.WindowPadding.y;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
    const bool open =
        ImGui::BeginViewportSideBar("##Toolbar", viewport, ImGuiDir_Up, height, barFlags);
    ImGui::PopStyleColor();
    if (open && toolbar_.size() == icons.size()) {
        const auto button = [this, &icons](std::size_t index) {
            const Command& command = toolbar_[index];
            const bool named =
                (index >= firstToggle && index < firstRunButton) || index == aiButton;
            const std::string label = std::string(icons.at(index)) +
                                      (named ? " " + command.label : std::string()) + "##" +
                                      command.label;
            ImGui::BeginDisabled(!command.isEnabled());
            if (index == aiButton) {
                ImGui::PushStyleColor(ImGuiCol_Button, themeColor::aiButton);
            }
            if (ImGui::Button(label.c_str())) {
                command.run();
            }
            if (index == aiButton) {
                ImGui::PopStyleColor();
            }
            ImGui::EndDisabled();
            const std::optional<Shortcut> shortcut = command.shortcutOn(platform_);
            if (shortcut && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled |
                                                 ImGuiHoveredFlags_ForTooltip)) {
                ImGui::SetTooltip("%s (%s)", command.label.c_str(),
                                  shortcutText(*shortcut, platform_).c_str());
            }
        };
        const auto groupGap = [&style] { ImGui::SameLine(0.0F, style.ItemSpacing.x * 3.0F); };

        // Left: transform tools, then the Pivot, Global and Snap toggles.
        for (std::size_t i = 0; i < firstRunButton; ++i) {
            button(i);
            if (i + 1 == firstToggle) {
                groupGap();
            } else {
                ImGui::SameLine(0.0F, 2.0F);
            }
        }

        // Centre: the Normal / Headless mode switch, Play, Pause, Step and the tick counter
        // (srs.fr.17.15, before the first tick).
        const char* tickText = "Tick \xE2\x80\x94 \xC2\xB7 0 executed \xC2\xB7 0.00 s";
        ImGui::PushFont(themeFonts().mono, 0.0F);
        const float tickWidth = ImGui::CalcTextSize(tickText).x;
        ImGui::PopFont();
        const float centreWidth = ImGui::CalcTextSize("NormalHeadless").x +
                                  4.0F * style.FramePadding.x + 3.0F * ImGui::GetFrameHeight() +
                                  tickWidth + 8.0F * style.ItemSpacing.x;
        const float centreX = (ImGui::GetWindowWidth() - centreWidth) * 0.5F;
        if (centreX > ImGui::GetCursorPosX()) {
            ImGui::SetCursorPosX(centreX);
        }
        ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_Header));
        ImGui::Button("Normal");
        ImGui::PopStyleColor();
        ImGui::SameLine(0.0F, 0.0F);
        ImGui::Button("Headless");
        ImGui::EndDisabled();
        groupGap();
        for (std::size_t i = firstRunButton; i < aiButton; ++i) {
            button(i);
            ImGui::SameLine(0.0F, 2.0F);
        }
        groupGap();
        ImGui::PushFont(themeFonts().mono, 0.0F);
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(themeColor::mutedText), "%s", tickText);
        ImGui::PopFont();

        // Right: AI Assistant and the layout menu.
        const std::string layoutLabel = std::string("Layout: Default ") + ICON_FA_CARET_DOWN;
        const float rightWidth =
            ImGui::CalcTextSize((std::string(icons.at(aiButton)) + " AI Assistant").c_str()).x +
            ImGui::CalcTextSize(layoutLabel.c_str()).x + 4.0F * style.FramePadding.x +
            style.ItemSpacing.x;
        ImGui::SameLine(ImGui::GetWindowWidth() - rightWidth - style.WindowPadding.x);
        button(aiButton);
        ImGui::SameLine();
        ImGui::BeginDisabled();
        ImGui::Button(layoutLabel.c_str());
        ImGui::EndDisabled();
    }
    ImGui::End();
}

void MainWindowView::drawStatusBar() {
    constexpr ImGuiWindowFlags barFlags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float height = ImGui::GetFrameHeight();
    if (ImGui::BeginViewportSideBar("##StatusBar", viewport, ImGuiDir_Down, height, barFlags)) {
        if (ImGui::BeginMenuBar()) {
            const ImVec4 muted = ImGui::ColorConvertU32ToFloat4(themeColor::mutedText);
            ImGui::TextColored(muted, ICON_FA_CIRCLE_INFO " %s",
                               session_.isProjectOpen() ? "" : "No project open");

            // The mode, the frame rate and the AI service state, at the right (B-01).
            char fps[32];
            std::snprintf(fps, sizeof fps, "FPS %.0f",
                          static_cast<double>(ImGui::GetIO().Framerate));
            const char* mode = "Edit mode";
            const char* ai = ICON_FA_CIRCLE " AI service: offline";
            const ImGuiStyle& style = ImGui::GetStyle();
            const float rightWidth = ImGui::CalcTextSize(mode).x + ImGui::CalcTextSize(fps).x +
                                     ImGui::CalcTextSize(ai).x + 4.0F * style.ItemSpacing.x * 2.0F;
            ImGui::SameLine(ImGui::GetWindowWidth() - rightWidth);
            ImGui::TextColored(muted, "%s", mode);
            ImGui::SameLine(0.0F, style.ItemSpacing.x * 3.0F);
            ImGui::TextColored(muted, "%s", fps);
            ImGui::SameLine(0.0F, style.ItemSpacing.x * 3.0F);
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(themeColor::offlineDot),
                               ICON_FA_CIRCLE);
            ImGui::SameLine(0.0F, 4.0F);
            ImGui::TextColored(muted, "AI service: offline");
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}

void MainWindowView::drawPanels() {
    const ImGuiViewport& viewport = *ImGui::GetMainViewport();
    const ImGuiID dockSpaceId = ImGui::GetID(dockSpaceName);
    if (frameIndex_ == 0) {
        buildDefaultLayout(dockSpaceId, viewport);
    }
    // The design opens the panel tab menu (E-07) from a ⋮ button, which comes with the docking
    // stories, so Dear ImGui's own window menu button is hidden.
    ImGuiWindowClass panelClass;
    panelClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton;
    ImGui::DockSpaceOverViewport(dockSpaceId, &viewport, ImGuiDockNodeFlags_None, &panelClass);

    for (int i = 0; i < panelCount; ++i) {
        const auto panel = static_cast<Panel>(i);
        ImGui::SetNextWindowClass(&panelClass);
        // The Default layout starts with Scene and Project in front of Game and Console. Dear
        // ImGui docks the panels in the first frame, so they are brought to the front in the
        // second.
        const bool startsInFront = panel == Panel::Scene || panel == Panel::Project;
        if (panelToFocus_ == panel || (frameIndex_ == 1 && startsInFront)) {
            ImGui::SetNextWindowFocus();
        }
        if (ImGui::Begin(panelName(panel))) {
            ImGui::TextDisabled("%s", session_.isProjectOpen() ? "" : "No project open");
        }
        ImGui::End();
    }
    panelToFocus_.reset();
}

} // namespace editor
