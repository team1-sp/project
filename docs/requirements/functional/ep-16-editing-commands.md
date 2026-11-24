---
epic: EP-16
group: srs.fr.16
part: editor
---

# EP-16 Editing Commands

**Epic:** As a game developer, I want standard edit commands with full undo, so that I can change things freely and take mistakes back.

This group covers the Edit menu (D-02). Every change to the scene goes on the undo stack unless a requirement says otherwise.

## US-16.1 Undo and redo

**Story:** As a game developer, I want to undo and redo any scene change, so that I can experiment safely.

*Screens: D-02*

- **srs.fr.16.1** The Editor shall keep one undo history for the editing session and put on it, as one step per user action: every change to entities, components, field values, hierarchy and scene membership in any loaded scene; prefab override changes and the Apply, Revert, Unpack and Replace Prefab actions; edits in Prefab Mode; field edits of data assets; and the addition, removal, renaming and rebinding of actions and bindings in Input Map assets.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.16.2** When the user chooses Edit ▸ Undo or presses Ctrl+Z while the undo stack is not empty, the Editor shall revert the last change.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.16.3** When the user chooses Edit ▸ Redo or presses Ctrl+Y while the redo stack is not empty, the Editor shall reapply the last undone change.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.16.4** The Undo and Redo menu items shall name the change they will revert or reapply, for example "Undo Move Player".
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.16.5** The Editor shall clear the undo and redo history when New Scene, Open Scene or Open Recent replaces the loaded scenes, and when a scene is unloaded or removed; Save As… shall instead remap the history as stated in srs.fr.16.22.
  - Use cases: UC-02; Initial Plan: —

## US-16.2 Clipboard and duplication

**Story:** As a game developer, I want to cut, copy, paste and duplicate entities, so that I can build scenes quickly.

*Screens: D-02*

- **srs.fr.16.6** When the user chooses Edit ▸ Cut or presses Ctrl+X with entities selected, the Editor shall copy the entities and their components to the clipboard and delete them from the scene.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.16.7** When the user chooses Edit ▸ Copy or presses Ctrl+C with entities selected, the Editor shall copy the entities and their components to the clipboard.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.16.8** When the user chooses Edit ▸ Paste or presses Ctrl+V while the clipboard holds entities, the Editor shall add copies of them to the active scene with new IDs and remapped references (srs.fr.2.66) and with " (1)" added to each name; each copy shall keep the world position of its original and be placed under the original's parent when that parent is in the active scene, and otherwise at the active scene's root, and the copies shall become the selection.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.16.9** When the user chooses Edit ▸ Duplicate or presses Ctrl+D with an entity selected, the Editor shall create a copy beside the original and select the copy.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-16.3 Delete, rename and selection

**Story:** As a game developer, I want to delete, rename and select items with standard keys, with confirmation before any nonundoable asset deletion, so that editing feels familiar and protects my work.

*Screens: D-02*

- **srs.fr.16.10** When the user chooses Edit ▸ Delete or presses Del with entities selected, the Editor shall delete unreferenced entities at once as an undoable operation; referenced entities shall first require the confirmation of srs.fr.16.11. Every asset deletion shall first require the Delete Confirmation dialog (F-02), whether or not the asset is referenced, because asset deletion is not undoable (srs.fr.23.31).
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.16.11** When an item to be deleted is referenced by another entity, component or asset, the Editor shall show the Delete Confirmation dialog (F-02) before deleting.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.16.12** When the user chooses Edit ▸ Rename or presses F2 with one item selected, the Editor shall make the item's name editable in place in the Hierarchy or Project panel.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.16.13** During an in-place rename, Enter shall save the new name and Esc shall cancel; an empty name shall be rejected and the old name kept.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.16.14** When the user chooses Edit ▸ Select All or presses Ctrl+A while the Hierarchy or Project panel has focus, the Editor shall select all items in that panel.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.16.15** When the user chooses Edit ▸ Frame Selected or presses F with a selection, the Editor shall centre the Scene view camera on the selection.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.16.16** When the user chooses Edit ▸ Find or presses Ctrl+F while the Hierarchy, Project, Console, Event Bus or Semantic Graph panel is active, the Editor shall move keyboard focus to that panel's search box; while another panel is active, Edit ▸ Find shall be disabled and Ctrl+F shall do nothing.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.16.17** When the user chooses Edit ▸ Project Settings… with a project open, the Editor shall open the Project Settings window (F-04).
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.16.18** When the user chooses Edit ▸ Preferences… or presses Ctrl+,, the Editor shall open the Preferences window (F-05).
  - Use cases: UC-02, UC-03; Initial Plan: —

## US-16.4 Undo scope

**Story:** As a game developer, I want to know exactly which actions can be undone, so that I am never surprised by what Undo does.

- **srs.fr.16.19** The undo history shall not include Project Settings changes, which use Apply and Revert (srs.fr.27.11), or Preferences changes, which apply at once (srs.fr.27.29); asset file operations in the Project panel (create, rename, move, duplicate and delete, srs.fr.23.31); builds; or runs.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.16.20** Changes made during Play mode shall not be put on the undo history, and the history from before Play mode shall be available again after Stop.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.16.21** Undoing an Apply to Prefab, Apply All or Replace Prefab step shall restore the prefab asset file to its earlier content and update its instances in the loaded scenes.
  - Use cases: UC-02; Initial Plan: —

## US-16.5 Identity and undo

**Story:** As a game developer, I want to undo to stay valid after Save As and prefab changes, so that undoing restores identities, not only file contents.

- **srs.fr.16.22** When Save As… gives the entities of a scene new IDs (srs.fr.2.84), the Editor shall remap every undo and redo step that refers to that scene's entities through the same old-to-new ID map, so that undo keeps working on the new file.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.16.23** Undoing Apply to Prefab, Apply All, Revert or Replace Prefab shall restore the prefab asset's content, the prefab-local IDs of the entities it brings back and the maps and override records of every instance in the loaded scenes that the action changed; it shall not lower the asset's allocation mark (srs.fr.2.119), so that IDs given out by the undone action are never given to a different entity.
  - Use cases: UC-02; Initial Plan: —
