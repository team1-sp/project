#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "editor/command.hpp"
#include "editor/editor_session.hpp"
#include "editor/main_menu.hpp"

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

    // `session` must outlive the view.
    MainWindowView(EditorSession& session, CloseEditor closeEditor, SetTitle setTitle);

    // Builds this render frame's interface. Call it between ImGui::NewFrame() and ImGui::Render().
    void draw();

    void exit() override;
    void showPanel(Panel panel) override;

private:
    void updateTitle();
    void runShortcuts(const std::vector<MenuEntry>& entries);
    void drawMenuBar();
    void drawMenuEntries(const std::vector<MenuEntry>& entries);
    void drawToolbar();
    void drawStatusBar();
    void drawPanels();

    EditorSession& session_;
    CloseEditor closeEditor_;
    SetTitle setTitle_;
    Platform platform_;
    std::vector<Menu> menus_;
    std::vector<Command> toolbar_;
    std::string shownTitle_;
    // The panel that showPanel() asked to bring to the front in the next frame.
    std::optional<Panel> panelToFocus_;
    // Render frames drawn so far, counted up to 2.
    int frameIndex_ = 0;
};

} // namespace editor
