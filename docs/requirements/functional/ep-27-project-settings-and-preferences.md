---
epic: EP-27
group: srs.fr.27
part: editor
---

# EP-27 Project Settings and Preferences

**Epic:** As a game developer, I want to configure the project and my own editor preferences, so that the engine and the editor work the way I need.

This group covers the Project Settings window (F-04 to F-04f, except the AI and Privacy tab, which is in srs.fr.33) and the Preferences window (F-05).

## US-27.1 Settings window

**Story:** As a game developer, I want to move between settings tabs and reset them, so that I can find and undo any setting.

*Screens: F-04*

- **srs.fr.27.1** The Project Settings window shall show the tabs Physics, Input, Rendering, Telemetry, Debug Flags and AI and Privacy in a list on the left.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.2** When the user clicks a tab, the window shall show that tab's fields; unapplied edits on other tabs shall be kept until they are applied or reverted or the window closes.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.3** When the user clicks Reset Tab to Defaults, the Editor shall set every field of the current tab to its factory default (srs.fr.27.45): as unapplied edits on the tabs that have Apply and Revert (srs.fr.27.11), and at once on the Debug Flags and AI and Privacy tabs, which apply each change at once (srs.fr.27.39); when this would replace a value that differs from its default and cannot be got back with Revert, the Editor shall first ask "Discard these changes?" with Discard and Cancel.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.4** When the user clicks Close, presses Esc or clicks ✕ while there are unapplied edits, the Editor shall ask "Apply changes?" with Apply, Discard and Cancel: Apply shall apply the edits and close, Discard shall drop them and close, and Cancel shall keep the window open; during Play mode the prompt shall offer only Discard and Cancel (srs.fr.27.44); without unapplied edits the window shall close at once, and while a binding cell waits for a key Esc shall act only as stated in srs.fr.27.14.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.5** The Editor shall save the project settings in the project folder so that they are shared by everyone who opens the project, except the settings of the AI and Privacy tab, which are stored for the current user in that project (srs.fr.33.57).
  - Use cases: UC-04; Initial Plan: —

## US-27.2 Physics tab

**Story:** As a game developer, I want to set gravity and physics units, so that the simulation matches my game's scale.

*Screens: F-04b*

- **srs.fr.27.6** The Physics tab shall have Gravity X and Gravity Y fields in m/s², defaulting to (0, −9.81) because the Y axis points up (srs.fr.3.38), which the Physics Adapter passes to Box2D as the world gravity.
  - Use cases: UC-04; Initial Plan: ip.fr.3.3
- **srs.fr.27.7** The Physics tab shall show the simulation tick length as a read-only value of 1/60 s, which the Physics Adapter passes to every Box2D world step as its time step.
  - Use cases: UC-04; Initial Plan: ip.fr.1.2
- **srs.fr.27.8** The Physics tab shall have a Sub-steps field that accepts whole numbers from 1 to 8, defaults to 4 and is passed to every Box2D world step as its sub-step count; the tab shall offer no velocity or position iteration settings.
  - Use cases: UC-04; Initial Plan: ip.fr.3.1
- **srs.fr.27.9** The Physics tab shall show the fixed world scale "1 world unit = 1 m = 32 px at zoom 1" read-only; no project setting shall change how world units map to metres or pixels.
  - Use cases: UC-04; Initial Plan: ip.fr.3.1
- **srs.fr.27.10** The Physics tab shall show the direction of the world Y axis as the read-only value Up (srs.fr.3.38).
  - Use cases: UC-04; Initial Plan: ip.fr.3.1
- **srs.fr.27.11** The Project Settings window shall have Apply and Revert buttons for the Physics, Input, Rendering, Telemetry and Tags and Layers tabs: Apply shall store the unapplied edits of all of these tabs, and Revert shall ask "Discard these changes?" with Discard and Cancel and, on Discard, discard them.
  - Use cases: UC-04; Initial Plan: ip.fr.1.2, ip.fr.3.3

## US-27.3 Input tab

**Story:** As a game developer, I want to bind keys to input actions, so that players can use the controls I choose.

*Screens: F-04c*

- **srs.fr.27.12** The Input tab shall list every input action with its bindings, each binding in its own cell with ✕ to remove it and + to add a binding, so that an action can have several bindings (srs.fr.5.25).
  - Use cases: UC-04; Initial Plan: ip.fr.5.1
- **srs.fr.27.13** When the user clicks a binding cell, the cell shall show "Press a key…" and the next key pressed shall become the binding.
  - Use cases: UC-04; Initial Plan: ip.fr.5.1
- **srs.fr.27.14** When the user presses Esc while a binding cell waits for a key, the Editor shall cancel the change.
  - Use cases: UC-04; Initial Plan: ip.fr.5.1
- **srs.fr.27.15** When a key is already bound to another action, the Input tab shall show both bindings in yellow and name the other action.
  - Use cases: UC-04; Initial Plan: ip.fr.5.1
- **srs.fr.27.16** When the user clicks ✕ on an action row, the Editor shall remove that action as an unapplied edit; when the action is one that gameplay uses (srs.fr.5.2), the Input tab shall show the warning of srs.fr.5.26.
  - Use cases: UC-04; Initial Plan: ip.fr.5.1
- **srs.fr.27.17** When the user clicks + Add Action, the Input tab shall add a new row with its name field focused.
  - Use cases: UC-04; Initial Plan: ip.fr.5.1
- **srs.fr.27.18** When the user clicks Reset to Defaults on the Input tab, the Editor shall replace the project's Input Map with the default actions and bindings of srs.fr.5.28.
  - Use cases: UC-04; Initial Plan: ip.fr.5.1

## US-27.4 Rendering tab

**Story:** As a game developer, I want to set how the game is drawn, so that the game looks and paces the way I want.

*Screens: F-04d*

- **srs.fr.27.19** When the user clicks the Clear colour swatch, the Editor shall open a colour picker; the hex value field shall also be editable.
  - Use cases: UC-04; Initial Plan: ip.fr.4.1
- **srs.fr.27.20** When the user turns VSync on or off, the Editor shall store the setting for Play mode and builds (srs.fr.4.10).
  - Use cases: UC-04; Initial Plan: ip.fr.4.1
- **srs.fr.27.21** The Target FPS field shall be enabled only when VSync is off.
  - Use cases: UC-04; Initial Plan: ip.fr.4.1
- **srs.fr.27.22** The Texture filter ▾ field shall offer Point (pixel art) and Bilinear and shall set the project's default filter, which applies to every image asset whose own filter is Project default (srs.fr.4.11).
  - Use cases: UC-04; Initial Plan: ip.fr.4.1
- **srs.fr.27.23** The Rendering tab shall have Window size (width, height) and Fullscreen fields used by the standalone game window (srs.fr.1.9); they shall not change the Game view's resolution in Play mode.
  - Use cases: UC-04; Initial Plan: ip.fr.1.3

## US-27.5 Telemetry tab

**Story:** As a game developer, I want to set what telemetry is recorded and where, so that I record what I need at a cost I accept.

*Screens: F-04e*

- **srs.fr.27.24** The Telemetry tab shall have a Record telemetry checkbox that sets the project's default for Run ▸ Telemetry Recording, which the Editor copies to that toggle when the project opens and when the tab is applied (srs.fr.30.1).
  - Use cases: UC-04, UC-08; Initial Plan: ip.fr.8.5
- **srs.fr.27.25** The Telemetry tab shall have a Sample every N ticks field, where 1 means every tick.
  - Use cases: UC-04, UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.27.26** The Telemetry tab shall have an Output folder field with a Browse… button that sets where recordings are written.
  - Use cases: UC-04, UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.27.27** The Telemetry tab shall have one checkbox per sample field (srs.fr.8.3) and shall update an estimate of the recording size per 1 000 ticks whenever a checkbox changes.
  - Use cases: UC-04, UC-08; Initial Plan: ip.fr.8.1

## US-27.6 Debug Flags tab

**Story:** As a game developer, I want to switch injectable bugs on and off from settings, so that I can prepare test runs.

*Screens: F-04f*

- **srs.fr.27.28** The Debug Flags tab shall have one toggle for each of the four injectable bugs, synchronised with Run ▸ Debug Flags.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2

## US-27.7 Preferences

**Story:** As a game developer, I want to set my personal editor preferences, so that the editor suits how I work.

*Screens: F-05*

- **srs.fr.27.29** The Preferences window shall apply every change at once and save it for the current user on this computer, not in the project.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.30** The Theme field shall offer Dark and shall show Light as not available yet; it shall be the same per-user setting as the Hub's Theme setting (srs.fr.13.58).
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.31** When the user changes UI scale in Preferences to 90, 100, 125 or 150 %, the Editor shall rescale at once and store the value in the per-user setting that the Hub also uses (srs.fr.13.59).
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.32** When the user sets the External code editor path with the field or Browse…, the Editor shall use that program to open scripts and source files.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.33** The Autosave field shall offer Off, Every 5 minutes, Every 30 minutes, Every hour and Custom…, with the default Every 5 minutes; Custom… shall ask for a whole number of minutes from 1 to 1 440, refuse any other value with the reason under the field and keep the previous choice; Off shall turn autosave off.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.34** The Shortcuts list shall show every command with its shortcut; clicking a shortcut and pressing keys shall rebind it.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.35** When a new shortcut is already used by another command in the same context, Preferences shall name that command and ask whether to move the shortcut; if the user agrees, the other command shall lose the shortcut, and otherwise the change shall be cancelled, so that no two commands share a shortcut; when the other command is one that srs.nfr.1.1 requires to have a shortcut, Preferences shall instead refuse the move with "<command> needs a shortcut: give it another one first".
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.36** When the user clicks Reset in the Shortcuts list, the Editor shall restore the default shortcuts.
  - Use cases: UC-04; Initial Plan: —

## US-27.8 Applying settings

**Story:** As a game developer, I want a clear rule for when settings take effect, so that I never lose or apply a change by accident.

- **srs.fr.27.37** Edits on the Physics, Input, Rendering, Telemetry and Tags and Layers tabs shall stay unapplied until Apply, and each tab with unapplied edits shall show "*" after its name.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.38** When the Project Settings window is reopened, it shall show the applied values; unapplied edits shall not survive closing the window.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.39** The Debug Flags and AI and Privacy tabs shall apply each change at once, with no Apply step; the Debug Flags tab shall be disabled while the game is playing or a headless run is going (srs.fr.31.2).
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.40** Applied settings shall take effect from the next Play mode session or run, and a running session shall keep the settings it started with; the Scene view and Game view in Edit mode shall show applied Rendering settings at once.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.41** Preferences shall not allow the operating system's reserved shortcuts, Alt+F4 on Windows and Cmd+Q on macOS, to be assigned to another command.
  - Use cases: UC-04; Initial Plan: —
- **srs.fr.27.42** The Rendering tab's Target FPS field shall accept 0 (no limit) or whole numbers from 30 to 240 and default to 60.
  - Use cases: UC-04; Initial Plan: —

## US-27.9 Settings during Play mode

**Story:** As a game developer, I want to settings edits during Play mode to wait until Stop, so that a play session never changes project settings.

- **srs.fr.27.43** Project Settings fields on the Physics, Input, Rendering, Telemetry and Tags and Layers tabs may be edited during Play mode, but the edits shall stay unapplied while the window stays open and may be applied after Play mode stops (srs.fr.17.26); the AI and Privacy tab shall apply at once in every mode, as the one explicit exception to srs.fr.17.25 (srs.fr.33.57); the Debug Flags tab shall stay disabled (srs.fr.27.39), and Preferences, which are Editor data, shall apply at once in every mode.
  - Use cases: UC-04; Initial Plan: —

## US-27.10 Project Settings close during Play mode

**Story:** As a game developer, I want to closing Project Settings during Play mode never to apply changes, so that no path bypasses the Play mode rule.

- **srs.fr.27.44** During Play mode, the close prompt of srs.fr.27.4 shall ask "Discard unapplied changes?" with Discard and Cancel and offer no Apply; unapplied edits kept in an open window shall survive Stop and may then be applied, and no other control shall apply Project Settings during Play mode.
  - Use cases: UC-04; Initial Plan: —

## US-27.11 Factory defaults of project settings and Preferences

**Story:** As a game developer, I want to to know the value every project setting and preference starts with, so that a new project and Reset Tab to Defaults have a fixed, testable result.

- **srs.fr.27.45** The factory defaults shall be: on the Physics tab, the defaults of srs.fr.27.6, srs.fr.27.8 and srs.fr.27.9 and a layer collision matrix in which every pair of named layers, including each layer with itself, collides; on the Input tab, the Input Map of srs.fr.5.28; on the Rendering tab, Clear colour #000000, VSync on, Target FPS 60, Texture filter Point, Window size 1280 × 720 and Fullscreen off; on the Telemetry tab, Record telemetry on, Sample every 1 tick, Output folder Telemetry inside the project folder and the sample fields ticked as stated in srs.fr.8.4; on the Debug Flags tab, every flag off; on the Tags and Layers tab, the tags and layers of srs.fr.2.80; and in Preferences, Theme Dark, UI scale 100 %, no external code editor (scripts open in the operating system's default application) and Autosave Every 5 minutes.
  - Use cases: UC-04; Initial Plan: —

## US-27.12 Input action names

**Story:** As a game developer, I want input action names to be unique, so that recordings and gameplay always find the right action.

- **srs.fr.27.46** On the Input tab, a name shall be 1 to 64 characters after leading and trailing spaces are removed, and shall differ from every other name in the same list when compared without regard to case; when the user leaves a name field with a name that breaks this rule, the Editor shall keep the previous name, or remove a new row that never had a valid name, and show the reason under the field.
  - Use cases: UC-04; Initial Plan: —
