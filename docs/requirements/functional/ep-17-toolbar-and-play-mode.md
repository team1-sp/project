---
epic: EP-17
group: srs.fr.17
part: editor
---

# EP-17 Toolbar and Play Mode

**Epic:** As a game developer, I want tools for transforming entities and controls for playing, pausing and stepping the game, so that I can edit and test without leaving the editor.

This group covers the toolbar (C-01), Play mode (B-02), Paused (B-03) and the play items of the Run menu (D-06).

## US-17.1 Transform tools

**Story:** As a game developer, I want to pick a transform tool and its options, so that I can move, rotate and scale entities precisely.

*Screens: C-01*

- **srs.fr.17.1** When the user clicks the Hand tool or presses Q, the Editor shall select the pan tool, so that dragging in the Scene view pans the camera.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.17.2** When the user clicks the Move tool or presses W, the Editor shall show X and Y arrows on the selection, and dragging them shall change the Position.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.17.3** When the user clicks the Rotate tool or presses E, the Editor shall show a ring on the selection, and dragging it shall change the rotation.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.17.4** When the user clicks the Scale tool or presses R, the Editor shall show square handles on the selection, and dragging them shall change the scale.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.17.5** When the user clicks the Rect tool or presses T, the Editor shall show corner handles around the selected entity; dragging a handle shall change the entity's Position and scale so that the dragged corner follows the pointer while the opposite corner stays fixed, so that the sprite and the colliders, which follow the entity's scale (srs.fr.3.60), resize together without changes to their own fields.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.17.6** When the user clicks the Pivot/Center toggle or presses Z, the Editor shall switch the gizmo origin between Pivot, the position of the last selected entity, and Center, the centre of the bounds of all selected entities' positions, and flip the label.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.17.7** When the user clicks the Local/Global toggle or presses X, the Editor shall switch the gizmo axes between the entity's own axes and the world axes and flip the label.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.17.8** When the user clicks the Grid snap toggle or presses Ctrl+\, the Editor shall turn snapping of moves to the grid step on or off.
  - Use cases: UC-02; Initial Plan: —

## US-17.2 Play, pause and step

**Story:** As a game developer, I want to play, pause and step my game inside the editor, so that I can test changes immediately and inspect single ticks.

*Screens: C-01, B-02, B-03, D-06*

- **srs.fr.17.9** When the user clicks Play or presses Ctrl+P with a project open, no compile errors, the mode switch on Normal and no simulation operation going (srs.fr.17.28), the Editor shall save a Play mode snapshot (srs.fr.17.24), start the game loop in normal mode with every loaded scene, turn the Play button blue, tint the window as stated in srs.fr.17.17 and focus the Game tab.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.17.10** When the user clicks Play while the Console shows compile errors, the Editor shall not start the game and shall show the toast "Fix errors in Console first".
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.17.11** When the user clicks Play or presses Ctrl+P during Play mode, the Editor shall stop the game loop, restore the Play mode snapshot (srs.fr.17.25) and return to Edit mode, discarding every change made during play.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.17.12** When the user clicks Pause or presses Ctrl+Shift+P during Play mode, the Engine shall pause at the next tick boundary as stated in srs.fr.1.16, keep showing the last completed tick, and the Editor shall stop the tick counter and enable Step.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.17.13** When the user clicks Pause or presses Ctrl+Shift+P while paused, the Editor shall resume the game loop as stated in srs.fr.1.18.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.17.14** When the user clicks Step or presses Ctrl+Alt+P while paused, the Engine shall run exactly one simulation tick of 1/60 s without reading or changing the accumulator (srs.fr.1.17), which advances its tick counter by 1, and the Editor shall show the Engine's new counter value and refresh the Scene view, Game view, Inspector and Telemetry panels.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.17.15** The toolbar shall show the last completed tick number, the ticks executed and the simulated time, for example "Tick 1 239 · 1 240 executed · 20.67 s", and "Tick — · 0 executed · 0.00 s" before the first tick completes (srs.fr.1.19).
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.17.16** The Run ▸ Play, Run ▸ Pause and Run ▸ Step menu items shall behave exactly like the matching toolbar buttons.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.17.17** During Play mode, the Editor shall tint the whole main window.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.17.18** During Play mode, the Inspector shall show live values; edits shall apply at the next tick boundary and shall be lost on Stop, and a yellow bar shall say "Changes in Play mode are not saved".
  - Use cases: UC-06; Initial Plan: ip.fr.1.1, ip.fr.1.2

## US-17.3 Mode switch and AI button

**Story:** As a game developer, I want to choose what Play does and open the assistant from the toolbar, so that running headless or asking the AI is one click away.

*Screens: C-01*

- **srs.fr.17.19** The mode switch shall offer Normal and Headless and shall be enabled only while the game is not playing.
  - Use cases: UC-06, UC-07, UC-11; Initial Plan: ip.fr.1.4
- **srs.fr.17.20** When the mode switch is on Headless and the user clicks Play, the Editor shall open the Headless Run setup dialog (F-07) instead of starting Play mode.
  - Use cases: UC-06, UC-07, UC-11; Initial Plan: ip.fr.1.4
- **srs.fr.17.21** While a headless run or Playtest Agent session is going, the Editor shall disable Play, Run ▸ Run Headless… and Run ▸ Run Playtest Agent… as stated in srs.fr.17.28 and show Headless on the mode switch.
  - Use cases: UC-06, UC-07, UC-11; Initial Plan: ip.fr.7.1
- **srs.fr.17.22** When the user clicks the AI button or presses Ctrl+Shift+A, the Editor shall open or close the AI Assistant panel in the right dock and, when opening, move focus to its message box.
  - Use cases: UC-06, UC-07, UC-11; Initial Plan: ip.fr.11.1
- **srs.fr.17.23** The AI button shall show a badge with the number of findings not viewed yet: a new finding shall be not viewed until the user selects its row in the Bug Report panel (C-12), and a new occurrence of an existing finding (srs.fr.9.46) shall not add to the badge.
  - Use cases: UC-06, UC-07, UC-11; Initial Plan: ip.fr.9.3

## US-17.4 Play mode snapshot

**Story:** As a game developer, I want each Play mode session to start from and return to exactly what I had loaded, so that nothing I do while playing changes my saved work.

- **srs.fr.17.24** The Play mode snapshot shall hold the list of loaded scenes with their order and which one is active, every entity of the loaded scenes with its components, field values and persistent flag, the in-memory values and unsaved-changes state of data assets, each scene's unsaved-changes state, the enabled state of every system and group, the selection and the undo history.
  - Use cases: UC-06; Initial Plan: ip.fr.1.3
- **srs.fr.17.25** When Play mode stops, the Editor shall restore everything in the snapshot, discarding entities created or destroyed, scenes loaded, unloaded or changed by systems, the persistent set and every value changed during play; during Play mode the Editor shall write no game content (scenes, prefabs, data assets, project settings or other assets) and may write only Editor data: input recordings, telemetry, logs, chats, answer feedback, the window layout, Preferences and expectation keys (srs.fr.31.15, srs.fr.31.16), which Stop does not restore.
  - Use cases: UC-06; Initial Plan: ip.fr.1.3
- **srs.fr.17.26** During Play mode, the Editor shall disable every action that writes game content or project configuration: Save, Save As…, the scene menu actions (E-09), the prefab actions of srs.fr.36.69, Project Settings Apply and Reset Tab to Defaults, Add Tag… (srs.fr.22.6), moving systems in the Systems panel (srs.fr.25.3), Reset Order, adding or removing ordering rules (srs.fr.36.44), ＋ New Group and other group management actions, import settings Apply, and creating, importing, renaming, moving, duplicating, deleting and reimporting assets; the disabled controls shall show the tooltip "Stop Play mode first", and Ctrl+S shall show the toast "Stop Play mode to save". System and group enable checkboxes shall stay usable as temporary changes that Stop restores (srs.fr.17.24); Run ▸ Telemetry Recording and the Record toggle (srs.fr.30.1), Mark as Expected and Undo Mark as Expected (srs.fr.31.15, srs.fr.31.16) shall stay usable.
  - Use cases: UC-06; Initial Plan: ip.fr.1.3
- **srs.fr.17.27** Play mode shall run every loaded scene, with the active scene as the active scene; a scene change requested at runtime shall replace all loaded scenes except the persistent entities (srs.fr.2.69).
  - Use cases: UC-06; Initial Plan: ip.fr.1.3

## US-17.5 One simulation operation

**Story:** As a game developer, I want to only one run to go at a time, so that Stop always targets the one thing that is running.

- **srs.fr.17.28** Play mode, a headless run and a Playtest Agent session shall each be a simulation operation, and at most one shall be going at a time: while one is going, every command that would start another operation (Play while a headless run or session is going, Run ▸ Run Headless…, Run ▸ Run Playtest Agent…, their shortcuts and the Start and Run buttons of open setup dialogs) shall be disabled with the tooltip "A run is already going", and Stop commands shall target the operation that is going; during Play mode, running or paused, Play and Ctrl+P shall stay enabled as the Stop toggle of srs.fr.17.11, together with Pause; Step and Ctrl+Alt+P shall be enabled only while Play mode is paused (srs.fr.17.14).
  - Use cases: UC-06; Initial Plan: —

## US-17.6 Gizmo with several entities

**Story:** As a game developer, I want the gizmo to treat a multi-selection predictably, so that moving or rotating several entities gives the same result each time.

- **srs.fr.17.29** While several entities are selected, the gizmo shall be drawn at the origin of srs.fr.17.6 and its axes shall follow the last selected entity in Local mode (srs.fr.17.7); a selected entity whose parent is also selected shall be changed only through its parent.
  - Use cases: UC-02; Initial Plan: —
