---
epic: EP-14
group: srs.fr.14
part: editor
---

# EP-14 Editor Main Window and Layout

**Epic:** As a game developer, I want a Unity-like main window with dockable panels and saved layouts, so that I can arrange the editor for the task at hand.

This group covers the main window (B-01), the empty-project state (B-06), the status bar, panel tabs (E-07), the Window menu (D-08) and the layout menu on the toolbar (C-01).

## US-14.1 Main window

**Story:** As a game developer, I want a familiar main window, so that I can find every tool where I expect it.

*Screens: B-01*

- **srs.fr.14.1** When a project opens, the Editor shall show the main window with the menu bar and toolbar at the top, Hierarchy on the left, Scene and Game in the centre, Inspector on the right, Project and Console at the bottom, and the status bar at the very bottom.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.2** The main window title shall show the project name and the active scene name, followed by "*" while any loaded scene or open asset has unsaved changes.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.3** When the user closes the main window, the Editor shall apply the unsaved-changes check of srs.fr.15.22 before closing.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.4** The menu bar shall hold the menus File, Edit, Assets, Entity, Component, Run, AI, Window and Help (D-01 to D-09).
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.5** The Editor shall show a menu item or control as disabled, in muted text and with no hover effect, whenever the condition under which it is enabled is not met.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.6** The Editor shall use Ctrl in keyboard shortcuts on Windows and Cmd in their place on macOS.
  - Use cases: UC-02; Initial Plan: —

## US-14.2 Status bar

**Story:** As a game developer, I want a status bar with the editor's current state, so that I notice logs, recording and service problems at a glance.

*Screens: B-01*

- **srs.fr.14.7** The status bar shall show the last Console message; clicking it shall open the Console panel.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.8** The status bar shall show the current mode (Edit, Play, Paused or Headless), the current FPS and the number of entities in the scene.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.9** The status bar shall show the AI service state as a green dot when connected and a grey dot when offline; clicking it shall open Project Settings at the AI and Privacy tab.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.10** While telemetry is recording, the status bar shall show a red REC indicator; clicking it shall open the Telemetry panel.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.11** While any debug flag is on, the status bar shall show the names of the active flags in orange.
  - Use cases: UC-02; Initial Plan: ip.fr.12.2

## US-14.3 Panels and docking

**Story:** As a game developer, I want to move, split, float and close panels, so that the layout fits my screen and task.

*Screens: B-01, E-07*

- **srs.fr.14.12** When the user clicks a panel tab, the Editor shall show that panel in its dock.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.13** When the user drags a panel tab onto another dock, the Editor shall move the panel into that dock as a tab.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.14** When the user drags a panel tab onto the edge of a dock, the Editor shall split that dock and place the panel in the new part.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.15** When the user drags a panel tab outside the main window, the Editor shall show the panel in a floating window.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.16** When the user clicks a dock's ⋮ button or right-clicks a panel tab, the Editor shall show the panel tab menu (E-07).
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.17** When the user chooses Maximize or presses Shift+Space, the Editor shall make the panel fill the main window; choosing it again shall restore the layout.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.18** When the user chooses Close Tab or presses Ctrl+W, the Editor shall close that panel.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.19** When the user chooses a panel from Add Tab ▸, the Editor shall add that panel as a tab in the same dock.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.20** When the user chooses Lock for the Inspector or Hierarchy, the panel shall keep showing its current item when the selection changes, until it is unlocked.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.21** When the user chooses Reset Layout, the Editor shall restore the Default layout.
  - Use cases: UC-02; Initial Plan: —

## US-14.4 Window menu and layouts

**Story:** As a game developer, I want to open any panel and switch between saved layouts, so that I can move between editing, debugging and AI work quickly.

*Screens: D-08, C-01, B-05*

- **srs.fr.14.22** When the user chooses Scene, Game, Hierarchy, Inspector, Project or Console from Window ▸ General, the Editor shall open that panel, or bring it to the front if it is already open.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.23** The shortcuts Ctrl+1 to Ctrl+6 shall open or bring to the front the Scene, Game, Hierarchy, Inspector, Project and Console panels, in that order.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.24** When the user chooses Systems, Telemetry, Headless Runs or Bug Report from Window ▸ Analysis, the Editor shall open that panel or bring it to the front.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.25** When the user chooses Assistant or Semantic Graph from Window ▸ AI, the Editor shall open that panel or bring it to the front.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.26** When the user chooses Default, AI or Debug from Window ▸ Layouts or from the toolbar's Layout dropdown, the Editor shall apply that layout; the Debug layout shall show the Scene view, the Inspector, the Console, the Systems panel and the Telemetry panel docked together.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.27** The AI layout shall show the Scene view, the Semantic Graph and the AI Assistant docked together (B-05).
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.28** When the user chooses Save Layout…, the Editor shall ask for a name of 1 to 64 characters after leading and trailing spaces are removed and save the current panel arrangement under that name in the Layout dropdown; the names Default, AI and Debug shall be refused, compared without regard to case, with the reason shown in the dialog; when a saved layout already has the name, the Editor shall ask "Replace layout <name>?", replace it if confirmed and keep it on Cancel.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.29** When the user chooses Delete Layout…, the Editor shall ask which saved layout to delete and then ask "Delete layout <name>?"; if confirmed it shall remove the layout, and Cancel shall keep it; the built-in layouts Default, AI and Debug shall not be deletable.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.30** When the user chooses Reset in the Layout dropdown or Window ▸ Layouts, the Editor shall restore the Default layout.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.31** When a project is reopened, the Editor shall restore the layout that was in use when it was last closed.
  - Use cases: UC-02; Initial Plan: —

## US-14.5 Empty project state

**Story:** As a game developer, I want helpful first steps in an empty project, so that I know how to start.

*Screens: B-06*

- **srs.fr.14.32** When a panel has no content, the Editor shall show the panel's empty state with a short hint about the first step.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.14.33** When the user clicks ＋ Create Entity in the empty Hierarchy, the Editor shall create an empty entity at the scene root and open its name for editing.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.14.34** When the user clicks ＋ Add Demo Player in the empty Hierarchy, the Editor shall add the demo Player preset at the centre of the Scene view.
  - Use cases: UC-02, UC-03; Initial Plan: ip.fr.12.1
- **srs.fr.14.35** When the user clicks Import Asset… in the empty Project panel, the Editor shall open the operating system's file picker for png, wav, ttf and json files and copy the chosen files into the open folder.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.14.36** While nothing is selected, the Inspector shall show the hint "Select an entity or asset to see its properties".
  - Use cases: UC-02, UC-03; Initial Plan: —

## US-14.6 Keyboard focus and shortcuts

**Story:** As a game developer, I want the shortcuts to stay inactive while I type or work in a dialog, so that I do not change the scene by accident.

- **srs.fr.14.37** Shortcuts without Ctrl, Alt or Cmd, such as Q, W, E, R, T, Z, X, F, F2 and Del, shall act only when keyboard focus is not in a text field.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.38** While a text field has focus, Ctrl+Z, Ctrl+Y, Ctrl+X, Ctrl+C, Ctrl+V and Ctrl+A shall act on the field's text and not on the scene.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.39** Edit menu shortcuts shall act on the panel that has focus; for example, Ctrl+D shall duplicate the selected entities when the Hierarchy or Scene view has focus and the selected assets when the Project panel has focus (srs.fr.16.9, srs.fr.23.30).
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.40** While a modal dialog is open, the Editor shall ignore main-window shortcuts, and only the dialog's own keys (srs.fr.35.1, srs.fr.35.2) shall act.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.14.41** While the Game view captures keyboard input in Play mode (srs.fr.20.8), keys without Ctrl, Alt or Cmd shall go to the game, and the Play, Pause and Step shortcuts shall still work.
  - Use cases: UC-02; Initial Plan: —
