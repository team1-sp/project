#include "main_window_view.hpp"

#include <array>
#include <cstddef>
#include <utility>

#include <imgui.h>
#include <imgui_internal.h>

namespace editor {

namespace {

constexpr const char* dockSpaceName = "MainDockSpace";

// Indexed by Key, in the order of its enumerators.
constexpr std::array<ImGuiKey, 30> imguiKeys = {
    ImGuiKey_A, ImGuiKey_B,  ImGuiKey_C,  ImGuiKey_D,      ImGuiKey_E,      ImGuiKey_F,
    ImGuiKey_H, ImGuiKey_N,  ImGuiKey_O,  ImGuiKey_P,      ImGuiKey_Q,      ImGuiKey_R,
    ImGuiKey_S, ImGuiKey_T,  ImGuiKey_V,  ImGuiKey_W,      ImGuiKey_X,      ImGuiKey_Y,
    ImGuiKey_Z, ImGuiKey_1,  ImGuiKey_2,  ImGuiKey_3,      ImGuiKey_4,      ImGuiKey_5,
    ImGuiKey_6, ImGuiKey_F2, ImGuiKey_F4, ImGuiKey_Delete, ImGuiKey_Escape, ImGuiKey_Comma,
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
    constexpr ImGuiWindowFlags barFlags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    // The tools, the gizmo toggles and the run controls form three groups (C-01).
    constexpr std::size_t lastToolIndex = 4;
    constexpr std::size_t lastToggleIndex = 6;
    constexpr std::size_t lastRunIndex = 9;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float height = ImGui::GetFrameHeight();
    if (ImGui::BeginViewportSideBar("##Toolbar", viewport, ImGuiDir_Up, height, barFlags)) {
        if (ImGui::BeginMenuBar()) {
            for (std::size_t i = 0; i < toolbar_.size(); ++i) {
                const Command& command = toolbar_[i];
                ImGui::BeginDisabled(!command.isEnabled());
                if (ImGui::Button(command.label.c_str())) {
                    command.run();
                }
                ImGui::EndDisabled();
                const std::optional<Shortcut> shortcut = command.shortcutOn(platform_);
                if (shortcut && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled |
                                                     ImGuiHoveredFlags_ForTooltip)) {
                    ImGui::SetTooltip("%s (%s)", command.label.c_str(),
                                      shortcutText(*shortcut, platform_).c_str());
                }
                if (i == lastToolIndex || i == lastToggleIndex || i == lastRunIndex) {
                    ImGui::Separator();
                }
                if (i == lastRunIndex) {
                    // The tick counter before the first tick (srs.fr.17.15). The default font has
                    // no em dash, so a hyphen stands in for it.
                    ImGui::TextDisabled("Tick - \xC2\xB7 0 executed \xC2\xB7 0.00 s");
                    ImGui::Separator();
                }
            }
            ImGui::EndMenuBar();
        }
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
            ImGui::TextUnformatted(session_.isProjectOpen() ? "Edit" : "No project open");
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
    ImGui::DockSpaceOverViewport(dockSpaceId, &viewport);

    for (int i = 0; i < panelCount; ++i) {
        const auto panel = static_cast<Panel>(i);
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
