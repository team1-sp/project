// Editor main window tests (US-14.1; srs.fr.14.2 to 14.6). They use the window-free menu model
// and session, so they need neither raylib nor Dear ImGui.
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/command.hpp"
#include "editor/editor_session.hpp"
#include "editor/main_menu.hpp"

namespace {

using editor::Key;
using editor::Menu;
using editor::MenuEntry;
using editor::Panel;
using editor::Platform;
using editor::Shortcut;

class RecordingActions final : public editor::IEditorActions {
public:
    void exit() override { calls.push_back("exit"); }
    void showPanel(Panel panel) override {
        calls.push_back(std::string("show ") + editor::panelName(panel));
    }
    [[nodiscard]] bool canOpenAiPrivacySettings() const override { return settingsAvailable; }
    void openAiPrivacySettings() override { calls.push_back("open AI settings"); }

    std::vector<std::string> calls;
    bool settingsAvailable = false;
};

const Menu& menuTitled(const std::vector<Menu>& menus, const char* title) {
    for (const Menu& menu : menus) {
        if (menu.title == title) {
            return menu;
        }
    }
    ADD_FAILURE() << "no menu " << title;
    return menus.front();
}

const MenuEntry& entryLabelled(const std::vector<MenuEntry>& entries, const char* label) {
    for (const MenuEntry& entry : entries) {
        if (entry.command.label == label) {
            return entry;
        }
    }
    ADD_FAILURE() << "no entry " << label;
    return entries.front();
}

// Every command in `entries` and their submenus, depth first.
void collectCommands(const std::vector<MenuEntry>& entries,
                     std::vector<const editor::Command*>& commands) {
    for (const MenuEntry& entry : entries) {
        if (entry.isSubmenu) {
            collectCommands(entry.submenu, commands);
        } else {
            commands.push_back(&entry.command);
        }
    }
}

} // namespace

// srs.fr.14.4: the menu bar holds these nine menus, in this order (D-01 to D-09).
TEST(MainMenuTest, MenuBar_HoldsTheNineMenusInOrder) {
    RecordingActions actions;

    const std::vector<Menu> menus = editor::buildMainMenu(actions);

    std::vector<std::string> titles;
    for (const Menu& menu : menus) {
        titles.push_back(menu.title);
        EXPECT_FALSE(menu.entries.empty()) << menu.title;
    }
    EXPECT_EQ(titles, (std::vector<std::string>{"File", "Edit", "Assets", "Entity", "Component",
                                                "Run", "AI", "Window", "Help"}));
}

// srs.fr.14.5: only the items whose feature is built are enabled: File > Exit and the six
// panels of Window > General. Every other item is disabled.
TEST(MainMenuTest, ItemsNotBuiltYet_AreDisabled) {
    RecordingActions actions;
    const std::vector<Menu> menus = editor::buildMainMenu(actions);

    std::vector<std::string> enabled;
    for (const Menu& menu : menus) {
        std::vector<const editor::Command*> commands;
        collectCommands(menu.entries, commands);
        for (const editor::Command* command : commands) {
            if (command->isEnabled()) {
                enabled.push_back(menu.title + " > " + command->label);
            }
        }
    }

    EXPECT_EQ(enabled, (std::vector<std::string>{"File > Exit", "Window > Scene", "Window > Game",
                                                 "Window > Hierarchy", "Window > Inspector",
                                                 "Window > Project", "Window > Console"}));
}

// File > Exit and Window > General call the Editor's actions.
TEST(MainMenuTest, EnabledItems_CallTheirActions) {
    RecordingActions actions;
    const std::vector<Menu> menus = editor::buildMainMenu(actions);
    const Menu& window = menuTitled(menus, "Window");
    const MenuEntry& general = entryLabelled(window.entries, "General");

    entryLabelled(menuTitled(menus, "File").entries, "Exit").command.run();
    entryLabelled(general.submenu, "Console").command.run();
    entryLabelled(general.submenu, "Scene").command.run();

    EXPECT_EQ(actions.calls, (std::vector<std::string>{"exit", "show Console", "show Scene"}));
}

TEST(MainMenuTest, ProjectAndAiSettingsUseTheSameAvailableAction) {
    RecordingActions actions;
    actions.settingsAvailable = true;
    const std::vector<Menu> menus = editor::buildMainMenu(actions);

    const editor::Command& projectSettings =
        entryLabelled(menuTitled(menus, "Edit").entries, "Project Settings...").command;
    const editor::Command& aiSettings =
        entryLabelled(menuTitled(menus, "AI").entries, "AI Settings...").command;
    ASSERT_TRUE(projectSettings.isEnabled());
    ASSERT_TRUE(aiSettings.isEnabled());
    projectSettings.run();
    aiSettings.run();
    EXPECT_EQ(actions.calls, (std::vector<std::string>{"open AI settings", "open AI settings"}));
}

// srs.fr.14.22, 14.23: Window > General lists the panels in order, on Ctrl+1 to Ctrl+6.
TEST(MainMenuTest, WindowGeneral_ListsPanelsOnCtrlOneToSix) {
    RecordingActions actions;
    const std::vector<Menu> menus = editor::buildMainMenu(actions);
    const MenuEntry& general = entryLabelled(menuTitled(menus, "Window").entries, "General");

    std::vector<std::string> items;
    for (const MenuEntry& entry : general.submenu) {
        items.push_back(entry.command.label + " " +
                        editor::shortcutText(*entry.command.shortcut, Platform::Windows));
    }
    EXPECT_EQ(items,
              (std::vector<std::string>{"Scene Ctrl+1", "Game Ctrl+2", "Hierarchy Ctrl+3",
                                        "Inspector Ctrl+4", "Project Ctrl+5", "Console Ctrl+6"}));
}

// File > Open Recent is a submenu with no entries while there is no recent scene, so it is drawn
// disabled.
TEST(MainMenuTest, OpenRecentWithoutRecentScenes_IsEmptySubmenu) {
    RecordingActions actions;
    const std::vector<Menu> menus = editor::buildMainMenu(actions);

    const MenuEntry& openRecent = entryLabelled(menuTitled(menus, "File").entries, "Open Recent");

    EXPECT_TRUE(openRecent.isSubmenu);
    EXPECT_TRUE(openRecent.submenu.empty());
}

// srs.fr.14.5: the toolbar's features are not built yet, so every control is disabled.
TEST(MainMenuTest, Toolbar_AllControlsDisabled) {
    const std::vector<editor::Command> toolbar = editor::buildToolbar();

    ASSERT_FALSE(toolbar.empty());
    for (const editor::Command& command : toolbar) {
        EXPECT_FALSE(command.isEnabled()) << command.label;
    }
}

// srs.fr.14.6: Ctrl on Windows, Cmd in its place on macOS.
TEST(ShortcutTest, CtrlShortcuts_ReadCtrlOnWindowsAndCmdOnMacOS) {
    const Shortcut saveAs{Key::S, true, true, false};
    const Shortcut rename{Key::F2, false, false, false};
    const Shortcut step{Key::P, true, false, true};

    EXPECT_EQ(editor::shortcutText(saveAs, Platform::Windows), "Ctrl+Shift+S");
    EXPECT_EQ(editor::shortcutText(saveAs, Platform::MacOS), "Cmd+Shift+S");
    EXPECT_EQ(editor::shortcutText(step, Platform::Windows), "Ctrl+Alt+P");
    EXPECT_EQ(editor::shortcutText(step, Platform::MacOS), "Cmd+Alt+P");
    EXPECT_EQ(editor::shortcutText(rename, Platform::MacOS), "F2");
}

// srs.fr.15.15: File > Exit is Alt+F4 on Windows and Cmd+Q on macOS.
TEST(ShortcutTest, Exit_IsAltF4OnWindowsAndCmdQOnMacOS) {
    RecordingActions actions;
    const std::vector<Menu> menus = editor::buildMainMenu(actions);
    const editor::Command& exit = entryLabelled(menuTitled(menus, "File").entries, "Exit").command;

    EXPECT_EQ(editor::shortcutText(*exit.shortcutOn(Platform::Windows), Platform::Windows),
              "Alt+F4");
    EXPECT_EQ(editor::shortcutText(*exit.shortcutOn(Platform::MacOS), Platform::MacOS), "Cmd+Q");
}

// srs.fr.14.2: the title shows the project and the active scene, then "*" while anything has
// unsaved changes.
TEST(EditorSessionTest, MainWindowTitle_ShowsProjectSceneAndUnsavedMark) {
    constexpr const char* dash = " \xE2\x80\x94 ";
    editor::EditorSession session;
    EXPECT_EQ(session.mainWindowTitle(), "Aegis Editor 0.1");

    session.openProject("DemoGame");
    EXPECT_EQ(session.mainWindowTitle(), std::string("Aegis Editor 0.1") + dash + "DemoGame");

    session.setActiveScene("DemoScene.scene");
    const std::string saved =
        std::string("Aegis Editor 0.1") + dash + "DemoGame" + dash + "DemoScene.scene";
    EXPECT_EQ(session.mainWindowTitle(), saved);

    session.setUnsavedItems({"DemoScene.scene", "Slime.asset"});
    EXPECT_EQ(session.mainWindowTitle(), saved + "*");

    session.setUnsavedItems({});
    EXPECT_EQ(session.mainWindowTitle(), saved);
}

// srs.fr.14.3, 15.22: with nothing unsaved the Editor closes at once.
TEST(EditorSessionTest, ConfirmCloseWithNothingUnsaved_AllowsClose) {
    editor::EditorSession session;
    session.openProject("DemoGame");

    EXPECT_TRUE(session.confirmClose());
    EXPECT_FALSE(session.isUnsavedChangesDialogRequested());
}

// srs.fr.14.3, 15.22: with unsaved items the Editor stays open and asks for the Unsaved Changes
// dialog (F-01).
TEST(EditorSessionTest, ConfirmCloseWithUnsavedItems_KeepsOpenAndRequestsDialog) {
    editor::EditorSession session;
    session.openProject("DemoGame");
    session.setUnsavedItems({"DemoScene.scene"});

    EXPECT_FALSE(session.confirmClose());
    EXPECT_TRUE(session.isUnsavedChangesDialogRequested());

    session.clearUnsavedChangesDialogRequest();
    EXPECT_FALSE(session.isUnsavedChangesDialogRequested());
}
