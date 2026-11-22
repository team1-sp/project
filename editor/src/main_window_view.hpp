#pragma once

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "assistant_chat_panel.hpp"
#include "editor/ai_privacy_settings.hpp"
#include "editor/assistant_chat.hpp"
#include "editor/assistant_entry_points.hpp"
#include "editor/command.hpp"
#include "editor/console_model.hpp"
#include "editor/editor_session.hpp"
#include "editor/main_menu.hpp"
#include "editor/status_bar.hpp"

namespace editor {

// Draws the Editor's main window with Dear ImGui each render frame (srs.fr.14.1): the menu bar
// and the toolbar at the top, Hierarchy on the left, Scene and Game in the centre, Inspector on
// the right, Project and Console at the bottom and the status bar at the very bottom. The panels
// are empty until their stories fill them.
class MainWindowView final : public IEditorActions {
public:
    // Ends the Editor's run; called once the unsaved-changes check has passed.
    using CloseEditor = std::function<void()>;
    // Shows a new title in the main window's title bar.
    using SetTitle = std::function<void(const std::string&)>;

    // `session` and `status` must outlive the view.
    MainWindowView(EditorSession& session, EditorStatus& status, ConsoleModel& console,
                   AssistantEntryPoints& assistant, AssistantChat& assistantChat,
                   ProjectSettingsState& projectSettings, IExternalCodeEditor& externalEditor,
                   IEntityNavigator& entityNavigator, CloseEditor closeEditor, SetTitle setTitle,
                   AiPrivacySettingsController* aiSettings = nullptr);

    // Builds this render frame's interface. Call it between ImGui::NewFrame() and ImGui::Render().
    void draw();

    void exit() override;
    void showPanel(Panel panel) override;
    void openAssistant() override;
    void toggleAssistant() override;
    [[nodiscard]] bool canAskAboutSelection() const override;
    void askAboutSelection() override;
    [[nodiscard]] bool canDiagnoseLastRun() const override;
    void diagnoseLastRun() override;
    [[nodiscard]] bool canOpenAiPrivacySettings() const override;
    void openAiPrivacySettings() override;

private:
    void updateTitle();
    void runShortcuts(const std::vector<MenuEntry>& entries);
    void drawMenuBar();
    void drawMenuEntries(const std::vector<MenuEntry>& entries);
    void drawToolbar();
    void drawStatusBar();
    void drawPanels();
    void drawAssistantPanel();
    void drawConsolePanel();
    void drawProjectSettings();
    // Turns an AI-and-Privacy request from the assistant entry points into the settings window.
    void openRequestedProjectSettings();

    EditorSession& session_;
    EditorStatus& status_;
    ConsoleModel& console_;
    AssistantEntryPoints& assistant_;
    AssistantChat& assistantChat_;
    AssistantChatPanel assistantChatPanel_;
    ProjectSettingsState& projectSettings_;
    IExternalCodeEditor& externalEditor_;
    IEntityNavigator& entityNavigator_;
    AiPrivacySettingsController* aiSettings_;
    CloseEditor closeEditor_;
    SetTitle setTitle_;
    Platform platform_;
    std::vector<Menu> menus_;
    std::vector<Command> toolbar_;
    std::string shownTitle_;
    // The panel that showPanel() asked to bring to the front in the next frame.
    std::optional<Panel> panelToFocus_;
    std::array<char, 256> consoleSearch_{};
    std::array<char, 512> serviceAddress_{};
    bool projectSettingsOpen_ = false;
    // Render frames drawn so far, counted up to 2.
    int frameIndex_ = 0;
};

} // namespace editor
