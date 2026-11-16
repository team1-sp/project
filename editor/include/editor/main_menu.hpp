#pragma once

#include <string>
#include <vector>

#include "editor/command.hpp"

namespace editor {

// The panels of the Default layout (srs.fr.14.1), in the order of Window > General and of the
// shortcuts Ctrl+1 to Ctrl+6 (srs.fr.14.22, 14.23).
enum class Panel {
    Scene,
    Game,
    Hierarchy,
    Inspector,
    Project,
    Console,
};

inline constexpr int panelCount = 6;

// The panel's name as its tab and the Window menu show it: "Scene", "Game", ...
[[nodiscard]] const char* panelName(Panel panel);

// What the menu bar and the toolbar can ask the Editor to do. More actions join as their
// stories are built.
class IEditorActions {
public:
    virtual ~IEditorActions() = default;

    // File > Exit: applies the unsaved-changes check, then closes the Editor (srs.fr.15.15).
    virtual void exit() = 0;
    // Opens the panel, or brings it to the front if it is open (srs.fr.14.22).
    virtual void showPanel(Panel panel) = 0;
};

// One line of a menu: a command, or a submenu of further entries.
struct MenuEntry {
    Command command;
    // True for a submenu; only its label is used from `command`. A submenu with no entries, such
    // as File > Open Recent with no recent scene, is disabled.
    bool isSubmenu = false;
    std::vector<MenuEntry> submenu;
    // Draws a separator line above the entry.
    bool separatorBefore = false;
};

// One menu of the menu bar.
struct Menu {
    std::string title;
    std::vector<MenuEntry> entries;
};

// The menu bar: File, Edit, Assets, Entity, Component, Run, AI, Window and Help, with the items of
// D-01 to D-09 (srs.fr.14.4). An item whose feature is not built yet is disabled (srs.fr.14.5).
// `actions` must outlive the menus.
[[nodiscard]] std::vector<Menu> buildMainMenu(IEditorActions& actions);

// The toolbar controls of C-01, left to right. None of their features are built yet, so all of
// them are disabled (srs.fr.14.5); they come with the Scene view and Play mode stories.
[[nodiscard]] std::vector<Command> buildToolbar();

} // namespace editor
