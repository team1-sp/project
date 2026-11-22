#include "main_window_view.hpp"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
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

MainWindowView::MainWindowView(EditorSession& session, EditorStatus& status, ConsoleModel& console,
                               IExternalCodeEditor& externalEditor,
                               IEntityNavigator& entityNavigator, CloseEditor closeEditor,
                               SetTitle setTitle, AiPrivacySettingsController* aiSettings)
    : session_(session)
    , status_(status)
    , console_(console)
    , externalEditor_(externalEditor)
    , entityNavigator_(entityNavigator)
    , aiSettings_(aiSettings)
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

    if (aiSettings_ != nullptr) {
        aiSettings_->poll();
        status_.aiService = aiSettings_->connectionState() == ConnectionState::Connected
                                ? AiServiceState::Connected
                                : AiServiceState::Offline;
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
    drawProjectSettings();
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

bool MainWindowView::canOpenAiPrivacySettings() const {
    return aiSettings_ != nullptr;
}

void MainWindowView::openAiPrivacySettings() {
    if (aiSettings_ == nullptr) {
        return;
    }
    projectSettingsOpen_ = true;
    aiSettings_->openTab();
    std::snprintf(serviceAddress_.data(), serviceAddress_.size(), "%s",
                  aiSettings_->addressDraft().c_str());
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
    constexpr ImGuiWindowFlags barFlags = ImGuiWindowFlags_NoScrollbar |
                                          ImGuiWindowFlags_NoSavedSettings |
                                          ImGuiWindowFlags_NoScrollWithMouse;
    const ImGuiStyle& style = ImGui::GetStyle();
    // Each item has this much room on both sides for its hover background; items are this far
    // apart (B-01: about 16 px from one text to the next).
    const float itemPadding = style.ItemInnerSpacing.x;
    const float itemGap = style.ItemSpacing.x;
    const float dotDiameter = ImGui::GetFontSize() * 0.6F;
    const float dotGap = style.ItemInnerSpacing.x;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, themeColor::statusBarBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.WindowPadding.x, 0.0F));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0F, 0.0F));
    const bool open = ImGui::BeginViewportSideBar("##StatusBar", viewport, ImGuiDir_Down,
                                                  ImGui::GetFrameHeight(), barFlags);
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
    if (!open) {
        ImGui::End();
        return;
    }

    const StatusBar bar =
        buildStatusBar(status_, static_cast<double>(ImGui::GetIO().Framerate), *this);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();
    drawList->AddLine(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y),
                      themeColor::statusBarBorder);

    const auto dotColour = [](StatusDot dot) {
        switch (dot) {
        case StatusDot::Green:
            return themeColor::onlineDot;
        case StatusDot::Red:
            return themeColor::recordingDot;
        case StatusDot::Grey:
        case StatusDot::None:
            break;
        }
        return themeColor::offlineDot;
    };
    // The width of the item's text and dot, without the padding.
    const auto contentWidth = [&](const StatusItem& item, const std::string& text) {
        return ImGui::CalcTextSize(text.c_str()).x +
               (item.dot != StatusDot::None ? dotDiameter + dotGap : 0.0F);
    };
    // Draws one item at the cursor, at most `maxWidth` wide; text that does not fit ends in "...".
    // A clickable item highlights under the mouse, shows its target as a tooltip and runs it when
    // clicked (srs.fr.14.7, 14.9, 14.10).
    const auto drawItem = [&](const StatusItem& item, const std::string& text, const char* id,
                              float maxWidth) {
        const float width = std::min(contentWidth(item, text), maxWidth);
        if (width <= 0.0F) {
            return;
        }
        const ImVec2 size(width + 2.0F * itemPadding, windowSize.y);
        const ImVec2 min = ImGui::GetCursorScreenPos();
        const ImVec2 max(min.x + size.x, min.y + size.y);
        bool hovered = false;
        if (item.isClickable()) {
            if (ImGui::InvisibleButton(id, size)) {
                item.click.run();
            }
            hovered = ImGui::IsItemHovered();
            if (hovered) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                ImGui::SetTooltip("%s", item.click.label.c_str());
            }
        } else {
            ImGui::Dummy(size);
        }
        if (hovered) {
            drawList->AddRectFilled(ImVec2(min.x, min.y + 2.0F), ImVec2(max.x, max.y - 1.0F),
                                    themeColor::statusHover, style.FrameRounding);
        }
        float x = min.x + itemPadding;
        const float centreY = min.y + size.y * 0.5F;
        if (item.dot != StatusDot::None) {
            drawList->AddCircleFilled(ImVec2(x + dotDiameter * 0.5F, centreY), dotDiameter * 0.5F,
                                      dotColour(item.dot));
            x += dotDiameter + dotGap;
        }
        const ImU32 textColour = item.colour == StatusTextColour::Warning ? themeColor::warningText
                                                                          : themeColor::statusText;
        const float textTop = centreY - ImGui::GetFontSize() * 0.5F;
        const ImVec2 textMax(min.x + itemPadding + width, max.y);
        ImGui::PushStyleColor(ImGuiCol_Text, textColour);
        if (width < contentWidth(item, text)) {
            ImGui::RenderTextEllipsis(drawList, ImVec2(x, textTop), textMax, textMax.x,
                                      text.c_str(), nullptr, nullptr);
        } else {
            drawList->AddText(ImVec2(x, textTop), textColour, text.c_str());
        }
        ImGui::PopStyleColor();
    };

    // The right-hand items first, so the message knows how much room is left for it.
    float rightWidth = 0.0F;
    for (const StatusItem& item : bar.right) {
        rightWidth += contentWidth(item, item.text) + 2.0F * itemPadding;
    }
    if (!bar.right.empty()) {
        rightWidth += itemGap * static_cast<float>(bar.right.size() - 1);
    }
    const float contentLeft = ImGui::GetCursorPosX();
    const float rightX =
        std::max(contentLeft, windowSize.x - style.WindowPadding.x + itemPadding - rightWidth);

    // Left: the last Console message (srs.fr.14.7).
    if (!bar.message.text.empty()) {
        ImGui::SetCursorPosX(contentLeft - itemPadding);
        const std::string text = ICON_FA_CIRCLE_INFO " " + bar.message.text;
        const float maxWidth = rightX - itemGap - contentLeft - itemPadding;
        drawItem(bar.message, text, "##StatusMessage", maxWidth);
    }

    // Right: debug flags, REC, mode, FPS, entities and the AI service.
    ImGui::SetCursorPos(ImVec2(rightX, 0.0F));
    for (std::size_t i = 0; i < bar.right.size(); ++i) {
        if (i > 0) {
            ImGui::SameLine(0.0F, itemGap);
        }
        const std::string id = "##StatusItem" + std::to_string(i);
        ImGui::SetCursorPosY(0.0F);
        drawItem(bar.right[i], bar.right[i].text, id.c_str(), FLT_MAX);
    }
    ImGui::End();
}

void MainWindowView::drawPanels() {
    const ImGuiViewport& viewport = *ImGui::GetMainViewport();
    const ImGuiID dockSpaceId = ImGui::GetID(dockSpaceName);
    const std::optional<Panel> focusThisFrame = panelToFocus_;
    panelToFocus_.reset();
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
        if (focusThisFrame == panel || (frameIndex_ == 1 && startsInFront)) {
            ImGui::SetNextWindowFocus();
        }
        const std::string windowName =
            panel == Panel::Console ? console_.tabLabel() : std::string(panelName(panel));
        if (ImGui::Begin(windowName.c_str())) {
            if (panel == Panel::Console) {
                drawConsolePanel();
            } else {
                ImGui::TextDisabled("%s", session_.isProjectOpen() ? "" : "No project open");
            }
        }
        ImGui::End();
    }
}

void MainWindowView::drawConsolePanel() {
    if (ImGui::Button("Clear")) {
        console_.clear();
    }
    ImGui::SameLine();
    bool clearOnPlay = console_.clearOnPlay();
    if (ImGui::Checkbox("Clear on Play", &clearOnPlay)) {
        console_.setClearOnPlay(clearOnPlay);
    }
    ImGui::SameLine();
    bool collapse = console_.collapse();
    if (ImGui::Checkbox("Collapse", &collapse)) {
        console_.setCollapse(collapse);
    }
    ImGui::SameLine();
    bool errorPause = console_.errorPause();
    if (ImGui::Checkbox("Error Pause", &errorPause)) {
        console_.setErrorPause(errorPause);
    }

    const editor::ConsoleCounts counts = console_.counts();
    const auto severityToggle = [&](engine::LogSeverity severity, const char* icon,
                                    std::size_t count, const char* id) {
        bool visible = console_.isSeverityVisible(severity);
        ImGui::SameLine();
        const std::string label = std::string(icon) + " " + std::to_string(count) + id;
        if (ImGui::Checkbox(label.c_str(), &visible)) {
            console_.setSeverityVisible(severity, visible);
        }
    };
    severityToggle(engine::LogSeverity::Info, ICON_FA_CIRCLE_INFO, counts.info, "##Info");
    severityToggle(engine::LogSeverity::Warning, ICON_FA_TRIANGLE_EXCLAMATION, counts.warning,
                   "##Warning");
    severityToggle(engine::LogSeverity::Error, ICON_FA_CIRCLE_XMARK, counts.error, "##Error");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(std::max(140.0F, ImGui::GetContentRegionAvail().x));
    if (ImGui::InputTextWithHint("##ConsoleSearch", "Search messages", consoleSearch_.data(),
                                 consoleSearch_.size())) {
        console_.setSearch(consoleSearch_.data());
    }

    const engine::LogEntry* selected = console_.selectedEntry();
    const float detailHeight =
        selected != nullptr ? ImGui::GetContentRegionAvail().y * 0.38F : 0.0F;
    const float rowsHeight = selected != nullptr ? ImGui::GetContentRegionAvail().y - detailHeight
                                                 : ImGui::GetContentRegionAvail().y;
    constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                           ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;
    if (ImGui::BeginTable("##ConsoleRows", 3, tableFlags, ImVec2(0.0F, rowsHeight))) {
        ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 58.0F);
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Tick", ImGuiTableColumnFlags_WidthFixed, 72.0F);
        ImGui::TableHeadersRow();
        for (const ConsoleRow& row : console_.visibleRows()) {
            const engine::LogEntry& log = console_.entries()[row.entryIndex];
            const char* icon = ICON_FA_CIRCLE_INFO;
            if (log.severity == engine::LogSeverity::Warning) {
                icon = ICON_FA_TRIANGLE_EXCLAMATION;
            } else if (log.severity == engine::LogSeverity::Error) {
                icon = ICON_FA_CIRCLE_XMARK;
            }
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            const std::string selectableId = "##ConsoleRow" + std::to_string(row.entryIndex);
            const bool isSelected = console_.selectedIndex() == row.entryIndex;
            if (ImGui::Selectable(selectableId.c_str(), isSelected,
                                  ImGuiSelectableFlags_SpanAllColumns |
                                      ImGuiSelectableFlags_AllowDoubleClick)) {
                console_.select(row.entryIndex);
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    const ConsoleActivation activation =
                        console_.activate(row.entryIndex, externalEditor_, entityNavigator_);
                    if (activation.entityNavigationRequested) {
                        panelToFocus_ = Panel::Hierarchy;
                    }
                }
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(icon);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(log.message.c_str());
            if (row.count > 1) {
                ImGui::SameLine();
                ImGui::TextDisabled("x%zu", row.count);
            }
            ImGui::TableSetColumnIndex(2);
            if (log.tick) {
                ImGui::Text("%llu", static_cast<unsigned long long>(*log.tick));
            } else {
                ImGui::TextDisabled("—");
            }
        }
        ImGui::EndTable();
    }

    selected = console_.selectedEntry();
    if (selected == nullptr) {
        return;
    }
    ImGui::SeparatorText("Details");
    ImGui::Text("%s  ·  Tick ", engine::logSeverityName(selected->severity));
    ImGui::SameLine(0.0F, 0.0F);
    if (selected->tick) {
        ImGui::Text("%llu", static_cast<unsigned long long>(*selected->tick));
    } else {
        ImGui::TextDisabled("—");
    }
    ImGui::TextWrapped("%s", selected->message.c_str());
    if (selected->stackTrace) {
        ImGui::Spacing();
        ImGui::TextDisabled("Stack trace");
        ImGui::TextUnformatted(selected->stackTrace->c_str());
    }
}

void MainWindowView::drawProjectSettings() {
    if (!projectSettingsOpen_ || aiSettings_ == nullptr) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(620.0F, 480.0F), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Project Settings", &projectSettingsOpen_)) {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("AI and Privacy");
    ImGui::TextUnformatted("Service address");
    ImGui::SetNextItemWidth(-130.0F);
    if (ImGui::InputText("##AiServiceAddress", serviceAddress_.data(), serviceAddress_.size())) {
        aiSettings_->setAddressDraft(serviceAddress_.data());
    }
    const bool addressLostFocus = ImGui::IsItemDeactivatedAfterEdit();
    ImGui::SameLine();
    if (ImGui::Button("Test Connection")) {
        aiSettings_->setAddressDraft(serviceAddress_.data());
        aiSettings_->testConnection();
    } else if (addressLostFocus) {
        aiSettings_->setAddressDraft(serviceAddress_.data());
        if (aiSettings_->commitAddressOnFocusLoss()) {
            aiSettings_->testConnection();
        }
    }
    if (!aiSettings_->addressError().empty()) {
        ImGui::TextColored(ImVec4(0.92F, 0.28F, 0.28F, 1.0F), "%s",
                           aiSettings_->addressError().c_str());
    }

    const ConnectionState connection = aiSettings_->connectionState();
    if (connection == ConnectionState::Checking) {
        ImGui::TextDisabled("Checking connection...");
    } else if (!aiSettings_->connectionText().empty()) {
        const ImVec4 colour = connection == ConnectionState::Connected
                                  ? ImVec4(0.28F, 0.82F, 0.45F, 1.0F)
                                  : ImVec4(0.92F, 0.28F, 0.28F, 1.0F);
        ImGui::TextColored(colour, "%s", aiSettings_->connectionText().c_str());
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("Model");
    const std::string modelText = aiSettings_->modelFieldText();
    const bool modelsAvailable = aiSettings_->modelListState() == ModelListState::Available;
    ImGui::BeginDisabled(!modelsAvailable);
    ImGui::SetNextItemWidth(-1.0F);
    if (ImGui::BeginCombo("##AiModel", modelText.c_str())) {
        for (const engine::bridge::AvailableModel& model : aiSettings_->models()) {
            const bool selected = model.identifier == aiSettings_->settings().modelIdentifier;
            const std::string label = model.displayName + "##" + model.identifier;
            if (ImGui::Selectable(label.c_str(), selected)) {
                (void)aiSettings_->selectModel(model.identifier);
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
    if (aiSettings_->modelListState() == ModelListState::Loading) {
        ImGui::TextDisabled("Loading models...");
    }
    if (!aiSettings_->modelsError().empty()) {
        ImGui::TextColored(ImVec4(0.92F, 0.28F, 0.28F, 1.0F), "%s",
                           aiSettings_->modelsError().c_str());
    }
    if (!aiSettings_->sendingEnabled()) {
        ImGui::TextDisabled("Sending disabled: %s", aiSettings_->sendDisabledReason().data());
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Data sharing");
    bool sourceCode = aiSettings_->settings().allowSourceCode;
    if (ImGui::Checkbox("Allow sending source code", &sourceCode)) {
        aiSettings_->requestAllow(ProjectDataCategory::SourceCode, sourceCode);
    }
    bool telemetry = aiSettings_->settings().allowTelemetry;
    if (ImGui::Checkbox("Allow sending telemetry", &telemetry)) {
        aiSettings_->requestAllow(ProjectDataCategory::Telemetry, telemetry);
    }
    bool askEveryTime = aiSettings_->settings().askBeforeEverySend;
    if (ImGui::Checkbox("Ask before every send", &askEveryTime)) {
        aiSettings_->setAskBeforeEverySend(askEveryTime);
    }

    if (aiSettings_->pendingConfirmation() != PrivacyConfirmation::None) {
        ImGui::OpenPopup("Confirm data sharing");
    }
    if (ImGui::BeginPopupModal("Confirm data sharing", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", aiSettings_->confirmationText().c_str());
        if (ImGui::Button("Enable")) {
            aiSettings_->confirmPrivacyChange();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            aiSettings_->cancelPrivacyChange();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (!aiSettings_->persistenceError().empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.92F, 0.28F, 0.28F, 1.0F), "Could not save settings: %s",
                           aiSettings_->persistenceError().c_str());
    }
    ImGui::End();
}

} // namespace editor
