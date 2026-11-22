#include "editor/main_menu.hpp"

#include <array>
#include <cstddef>
#include <utility>

namespace editor {

namespace {

bool always() {
    return true;
}

Shortcut ctrl(Key key) {
    return Shortcut{key, true, false, false};
}

Shortcut ctrlShift(Key key) {
    return Shortcut{key, true, true, false};
}

Shortcut plain(Key key) {
    return Shortcut{key, false, false, false};
}

// An item whose feature is not built yet: it shows its label and shortcut and stays disabled.
MenuEntry notBuilt(std::string label, std::optional<Shortcut> shortcut = std::nullopt,
                   bool separatorBefore = false) {
    MenuEntry entry;
    entry.command.label = std::move(label);
    entry.command.shortcut = shortcut;
    entry.separatorBefore = separatorBefore;
    return entry;
}

MenuEntry submenu(std::string label, std::vector<MenuEntry> entries, bool separatorBefore = false) {
    MenuEntry entry;
    entry.command.label = std::move(label);
    entry.isSubmenu = true;
    entry.submenu = std::move(entries);
    entry.separatorBefore = separatorBefore;
    return entry;
}

std::vector<MenuEntry> notBuiltList(std::initializer_list<const char*> labels) {
    std::vector<MenuEntry> entries;
    for (const char* label : labels) {
        entries.push_back(notBuilt(label));
    }
    return entries;
}

// D-01.
Menu fileMenu(IEditorActions& actions) {
    MenuEntry exit;
    exit.command.label = "Exit";
    exit.command.shortcut = Shortcut{Key::F4, false, false, true};
    exit.command.macosShortcut = ctrl(Key::Q);
    exit.command.enabled = always;
    exit.command.run = [&actions] { actions.exit(); };
    exit.separatorBefore = true;

    return Menu{"File",
                {
                    notBuilt("New Scene", ctrl(Key::N)),
                    notBuilt("Open Scene...", ctrl(Key::O)),
                    notBuilt("Open Scene Additive..."),
                    submenu("Open Recent", {}),
                    notBuilt("Save", ctrl(Key::S), true),
                    notBuilt("Save As...", ctrlShift(Key::S)),
                    notBuilt("New Project...", std::nullopt, true),
                    notBuilt("Open Project..."),
                    notBuilt("Build Settings...", ctrlShift(Key::B), true),
                    notBuilt("Build and Run", ctrl(Key::B)),
                    std::move(exit),
                }};
}

// D-02.
Menu editMenu(IEditorActions& actions) {
    MenuEntry projectSettings;
    projectSettings.command.label = "Project Settings...";
    projectSettings.command.enabled = [&actions] { return actions.canOpenAiPrivacySettings(); };
    projectSettings.command.run = [&actions] { actions.openAiPrivacySettings(); };
    projectSettings.separatorBefore = true;
    return Menu{"Edit",
                {
                    notBuilt("Undo", ctrl(Key::Z)),
                    notBuilt("Redo", ctrl(Key::Y)),
                    notBuilt("Cut", ctrl(Key::X), true),
                    notBuilt("Copy", ctrl(Key::C)),
                    notBuilt("Paste", ctrl(Key::V)),
                    notBuilt("Duplicate", ctrl(Key::D)),
                    notBuilt("Delete", plain(Key::Delete)),
                    notBuilt("Rename", plain(Key::F2)),
                    notBuilt("Select All", ctrl(Key::A), true),
                    notBuilt("Frame Selected", plain(Key::F)),
                    notBuilt("Find", ctrl(Key::F)),
                    std::move(projectSettings),
                    notBuilt("Preferences...", ctrl(Key::Comma)),
                }};
}

// D-03.
Menu assetsMenu() {
    return Menu{"Assets",
                {
                    submenu("Create", notBuiltList({"Folder", "Scene", "Sprite Sheet", "Input Map",
                                                    "Prefab"})),
                    notBuilt("Import New Asset..."),
                    notBuilt("Show in Explorer", std::nullopt, true),
                    notBuilt("Refresh", ctrl(Key::R)),
                    notBuilt("Reimport"),
                }};
}

// D-04.
Menu entityMenu() {
    return Menu{
        "Entity",
        {
            notBuilt("Create Empty", ctrlShift(Key::N)),
            submenu("2D Object", notBuiltList({"Sprite", "Box", "Circle", "Polygon", "Camera"})),
            submenu("Demo",
                    notBuiltList({"Player", "Obstacle", "Collectible", "Unlockable", "Objective"})),
            notBuilt("Create Prefab from Selection", std::nullopt, true),
            notBuilt("Set as Camera Target"),
            notBuilt("Move to Front", std::nullopt, true),
            notBuilt("Move to Back"),
        }};
}

// D-05.
Menu componentMenu() {
    return Menu{"Component",
                {
                    submenu("Core", notBuiltList({"Position", "Velocity", "Tag"})),
                    submenu("Rendering", notBuiltList({"Sprite", "Camera"})),
                    submenu("Audio", notBuiltList({"Audio Source"})),
                    submenu("Physics", notBuiltList({"RigidBody", "Box Collider", "Circle Collider",
                                                     "Polygon Collider"})),
                    submenu("Input", notBuiltList({"Player Input"})),
                    submenu("Gameplay",
                            notBuiltList({"Inventory", "Collectible", "Unlockable", "Objective"})),
                    notBuilt("Search...", std::nullopt, true),
                }};
}

// D-06.
Menu runMenu() {
    return Menu{"Run",
                {
                    notBuilt("Play", ctrl(Key::P)),
                    notBuilt("Pause", ctrlShift(Key::P)),
                    notBuilt("Step", Shortcut{Key::P, true, false, true}),
                    notBuilt("Run Headless...", ctrlShift(Key::H), true),
                    notBuilt("Stop Headless Run", plain(Key::Escape)),
                    notBuilt("Run Playtest Agent...", std::nullopt, true),
                    notBuilt("Telemetry Recording", std::nullopt, true),
                    notBuilt("Export Telemetry..."),
                    submenu("Debug Flags",
                            notBuiltList({"No Collider", "Unlock Without Item",
                                          "Unreachable Objective", "Out of Bounds"}),
                            true),
                }};
}

// D-07.
Menu aiMenu(IEditorActions& actions) {
    MenuEntry openAssistant;
    openAssistant.command.label = "Open Assistant";
    openAssistant.command.enabled = always;
    openAssistant.command.run = [&actions] { actions.openAssistant(); };

    MenuEntry askAboutSelection;
    askAboutSelection.command.label = "Ask about Selection";
    askAboutSelection.command.enabled = [&actions] { return actions.canAskAboutSelection(); };
    askAboutSelection.command.run = [&actions] { actions.askAboutSelection(); };

    MenuEntry diagnoseLastRun;
    diagnoseLastRun.command.label = "Diagnose Last Run";
    diagnoseLastRun.command.enabled = [&actions] { return actions.canDiagnoseLastRun(); };
    diagnoseLastRun.command.run = [&actions] { actions.diagnoseLastRun(); };

    MenuEntry aiSettings;
    aiSettings.command.label = "AI Settings...";
    aiSettings.command.enabled = [&actions] { return actions.canOpenAiPrivacySettings(); };
    aiSettings.command.run = [&actions] { actions.openAiPrivacySettings(); };
    aiSettings.separatorBefore = true;

    return Menu{"AI",
                {
                    std::move(openAssistant),
                    std::move(askAboutSelection),
                    std::move(diagnoseLastRun),
                    notBuilt("Open Semantic Graph", std::nullopt, true),
                    notBuilt("Rebuild Graph"),
                    notBuilt("Export Graph..."),
                    std::move(aiSettings),
                }};
}

// D-08.
Menu windowMenu(IEditorActions& actions) {
    constexpr std::array<Key, panelCount> panelKeys = {Key::Digit1, Key::Digit2, Key::Digit3,
                                                       Key::Digit4, Key::Digit5, Key::Digit6};
    std::vector<MenuEntry> general;
    for (std::size_t i = 0; i < panelKeys.size(); ++i) {
        const auto panel = static_cast<Panel>(i);
        MenuEntry entry;
        entry.command.label = panelName(panel);
        entry.command.shortcut = ctrl(panelKeys[i]);
        entry.command.enabled = always;
        entry.command.run = [&actions, panel] { actions.showPanel(panel); };
        general.push_back(std::move(entry));
    }

    return Menu{
        "Window",
        {
            submenu("General", std::move(general)),
            submenu("Analysis", notBuiltList({"Systems", "Telemetry", "Headless Runs", "Bug Report",
                                              "Event Bus"})),
            submenu("AI", notBuiltList({"Assistant", "Semantic Graph"})),
            submenu("Layouts", notBuiltList({"Default", "AI", "Debug", "Save Layout...", "Reset"}),
                    true),
        }};
}

// D-09.
Menu helpMenu() {
    return Menu{"Help",
                {
                    notBuilt("Documentation"),
                    notBuilt("Keyboard Shortcuts"),
                    notBuilt("Report a Bug", std::nullopt, true),
                    notBuilt("About Aegis", std::nullopt, true),
                }};
}

Command toolbarCommand(const char* label, std::optional<Shortcut> shortcut = std::nullopt) {
    Command command;
    command.label = label;
    command.shortcut = shortcut;
    return command;
}

} // namespace

const char* panelName(Panel panel) {
    switch (panel) {
    case Panel::Scene:
        return "Scene";
    case Panel::Game:
        return "Game";
    case Panel::Hierarchy:
        return "Hierarchy";
    case Panel::Inspector:
        return "Inspector";
    case Panel::Project:
        return "Project";
    case Panel::Console:
        return "Console";
    }
    return "?";
}

std::vector<Menu> buildMainMenu(IEditorActions& actions) {
    std::vector<Menu> menus;
    menus.push_back(fileMenu(actions));
    menus.push_back(editMenu(actions));
    menus.push_back(assetsMenu());
    menus.push_back(entityMenu());
    menus.push_back(componentMenu());
    menus.push_back(runMenu());
    menus.push_back(aiMenu(actions));
    menus.push_back(windowMenu(actions));
    menus.push_back(helpMenu());
    return menus;
}

std::vector<Command> buildToolbar(IEditorActions& actions) {
    std::vector<Command> commands = {
        toolbarCommand("Hand", plain(Key::Q)),
        toolbarCommand("Move", plain(Key::W)),
        toolbarCommand("Rotate", plain(Key::E)),
        toolbarCommand("Scale", plain(Key::R)),
        toolbarCommand("Rect", plain(Key::T)),
        toolbarCommand("Pivot", plain(Key::Z)),
        toolbarCommand("Global", plain(Key::X)),
        toolbarCommand("Snap", ctrl(Key::Backslash)),
        toolbarCommand("Play", ctrl(Key::P)),
        toolbarCommand("Pause", ctrlShift(Key::P)),
        toolbarCommand("Step", Shortcut{Key::P, true, false, true}),
        toolbarCommand("AI Assistant", ctrlShift(Key::A)),
    };
    commands.back().enabled = always;
    commands.back().run = [&actions] { actions.toggleAssistant(); };
    return commands;
}

} // namespace editor
