---
epic: EP-15
group: srs.fr.15
part: editor
---

# EP-15 Scene and Project Files

**Epic:** As a game developer, I want to create, open and save scenes and switch projects safely, so that I never lose work by accident.

This group covers the File menu (D-01) and the Unsaved Changes dialog (F-01).

## US-15.1 Scenes

**Story:** As a game developer, I want to create, open and save scenes, so that I can work on several levels.

*Screens: D-01*

- **srs.fr.15.1** When the user chooses File ▸ New Scene or presses Ctrl+N with a project open, the Editor shall create an empty scene that contains one Camera entity.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.15.2** When the user chooses File ▸ Open Scene… or presses Ctrl+O, the Editor shall open the operating system's file picker filtered to `.scene` files.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.15.3** When the user picks a `.scene` file, the Editor shall load it and refresh the Hierarchy, Scene view and Inspector.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.15.4** File ▸ Open Recent ▸ shall list the last 10 opened scenes, most recent first, and shall be disabled when the list is empty.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.15.5** When the user picks a scene from Open Recent ▸ whose file no longer exists, the Editor shall show the toast "Scene not found" and remove the entry from the list.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.15.6** Before New Scene, Open Scene or Open Recent replaces the loaded scenes, the Editor shall apply the unsaved-changes check of srs.fr.15.22.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.15.7** When the user chooses File ▸ Save or presses Ctrl+S, the Editor shall write every item with unsaved changes (srs.fr.15.24) and apply srs.fr.15.23 to the result.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.15.8** When a scene to be saved has never been saved, the Editor shall ask for its file name as Save As… does before writing it.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.15.9** When the user chooses File ▸ Save As… or presses Ctrl+Shift+S, the Editor shall save the active scene under the file name chosen in the operating system's save dialog, with new entity IDs as stated in srs.fr.2.84, and load that file in place of the old one; the other loaded scenes shall not change.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.15.10** When writing a scene file fails, the Editor shall show an error toast with the reason and keep that scene marked as unsaved, shall still save the other items, and shall report the overall result as stated in srs.fr.15.23.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-15.2 Projects and exit

**Story:** As a game developer, I want to switch projects or leave the editor, so that I always get a chance to save first.

*Screens: D-01*

- **srs.fr.15.11** When the user chooses File ▸ New Project…, the Editor shall open the New Project dialog (A-02).
  - Use cases: UC-01, UC-02; Initial Plan: —
- **srs.fr.15.12** When the user chooses File ▸ Open Project…, the Editor shall apply the unsaved-changes check of srs.fr.15.22 and then return to the Project Hub (A-01).
  - Use cases: UC-01, UC-02; Initial Plan: —
- **srs.fr.15.13** When the user chooses File ▸ Build Settings… or presses Ctrl+Shift+B with a project open, the Editor shall open the Build Settings dialog (F-06).
  - Use cases: UC-01, UC-02, UC-05; Initial Plan: —
- **srs.fr.15.14** When the user chooses File ▸ Build and Run or presses Ctrl+B while at least one scene is in the build, the Editor shall build with the last build settings and then start the built game (F-12).
  - Use cases: UC-01, UC-02, UC-05; Initial Plan: —
- **srs.fr.15.15** When the user chooses File ▸ Exit or presses Alt+F4 (Cmd+Q on macOS), the Editor shall apply the unsaved-changes check of srs.fr.15.22 and then close.
  - Use cases: UC-01, UC-02; Initial Plan: —

## US-15.3 Unsaved changes and autosave

**Story:** As a game developer, I want recovery copies of every unsaved authoring item, including during headless testing, and deliberate discards to retire their recovery entries, so that crash recovery protects work without reviving changes I rejected.

*Screens: F-01, F-05*

- **srs.fr.15.16** When the user clicks Save in the Unsaved Changes dialog, the Editor shall save every item listed in the dialog and continue the action that opened the dialog only if every save succeeded (srs.fr.15.21).
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.15.17** When the user clicks Don't Save in the Unsaved Changes dialog, the Editor shall discard the listed changes, retire the corresponding recovery entries as stated in srs.fr.15.20, and continue the action that opened the dialog; a recovery entry for an item whose changes were not discarded shall remain available.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.15.18** When the user clicks Cancel in the Unsaved Changes dialog, the Editor shall stop the action that opened the dialog and keep the scene as it is.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.15.19** When autosave is on, the Editor shall write a recovery copy of every item with unsaved authoring changes in the set of srs.fr.15.24 to the project's Temp folder at intervals no longer than N minutes, where N is set in Preferences, without changing the original files or their unsaved state; recovery shall retain each item's type, asset identity and original path, or its unsaved scene identity when it has no path. Recovery autosave shall continue during headless runs and Playtest Agent sessions, using the editable authoring state rather than the run's frozen content. Before entering Play mode, the Editor shall write recovery copies of the unsaved authoring items in its pre-Play snapshot and retain them throughout Play mode; temporary Play mode changes shall never overwrite those copies, and periodic authoring autosave shall resume after the snapshot is restored. A recovery write failure shall show an error naming the affected item and shall keep its last successful recovery copy.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.15.20** When a project opens and an active recovery copy is newer than its original file, the original file is missing, or the copy belongs to a scene that has never been saved, the Editor shall offer to restore or discard it, identifying the item and its original path or unsaved scene identity; restore shall open the recovered item with its original identity and mark it as unsaved, without overwriting the original file. A confirmed discard of unsaved authoring changes, Revert that deliberately discards them, an explicitly confirmed asset deletion, or confirmed discard in the recovery prompt shall retire the corresponding recovery entry, so that it is never offered again; retirement shall be persisted, even if deletion of the copy's bytes fails, and its failure shall cancel the discard or deletion and show the reason. This retirement shall not apply to a crash, a failed save, or restoration of the pre-Play snapshot, which shall preserve the applicable authoring recovery copies. This shall cover scenes, data assets, Input Map assets and prefabs as stated in srs.fr.15.19.
  - Use cases: UC-02; Initial Plan: —

## US-15.4 Save failures

**Story:** As a game developer, I want a failed save never to be treated as a success, so that I never lose work because a write failed.

- **srs.fr.15.21** When any save started from the Unsaved Changes dialog fails, or the user cancels a Save As… it needs, the Editor shall cancel the pending action and apply srs.fr.15.23: items saved successfully become clean, failed or cancelled items stay open and marked as unsaved, and the Editor shall show which items were not saved.
  - Use cases: UC-02; Initial Plan: —

## US-15.5 Unsaved-changes policy

**Story:** As a game developer, I want every way of leaving my work to check for unsaved changes the same way, so that I never lose an edited data asset or prefab.

- **srs.fr.15.22** Before any action that replaces or discards the current editing state (closing the main window, File ▸ Exit, New Scene, Open Scene, Open Recent, double-clicking a scene asset, File ▸ Open Project…, creating a project from the New Project dialog, and leaving Prefab Mode), the Editor shall first handle a simulation operation that is going as stated in srs.fr.15.25, and then show the Unsaved Changes dialog (F-01) when there is an item with unsaved changes (srs.fr.15.24), listing each of them.
  - Use cases: UC-02; Initial Plan: —

## US-15.6 Save results

**Story:** As a game developer, I want a partial save to say exactly what was and wasn't saved, so that the window never claims everything is saved when it isn't.

- **srs.fr.15.23** After a save of several items, each item saved successfully shall lose its "*" and become clean, each failed or cancelled item shall stay marked as unsaved, the title's "*" shall be removed only when no unsaved item remains, and the Editor shall show the toast "Saved" only when every item was saved, and otherwise an error toast "Saved N of M · K not saved" that lists the failed items.
  - Use cases: UC-02; Initial Plan: —

## US-15.7 Items with unsaved changes

**Story:** As a game developer, I want every editable item to be saved and protected the same way, so that no edited Input Map or data asset is ever lost or skipped.

- **srs.fr.15.24** The items with unsaved changes shall be every loaded scene, data asset and Input Map asset with unsaved changes and the prefab open in Prefab Mode; File ▸ Save, the Unsaved Changes dialog (srs.fr.15.22), save-result reporting (srs.fr.15.23), recovery autosave (srs.fr.15.19) and Save and Run (srs.fr.29.21) shall all use this set.
  - Use cases: UC-02; Initial Plan: —

## US-15.8 Active runs and scene changes

**Story:** As a game developer, I want to opening, switching or exiting during a run to be safe, so that a run's recording and report are never lost by closing the Editor.

- **srs.fr.15.25** When an action of srs.fr.15.22 other than leaving Prefab Mode starts while a simulation operation is going (srs.fr.17.28): during Play mode, the Editor shall first stop Play mode and restore its snapshot (srs.fr.17.11), and then apply the unsaved-changes check to the restored state; during a headless run or Playtest Agent session, New Scene, Open Scene, Open Recent and double-clicking a scene asset shall go on without affecting it (srs.fr.7.26), while File ▸ Exit, closing the main window, File ▸ Open Project… and creating a project shall ask "Stop the run?" with Stop and Cancel. Stop shall end the operation after the current tick as srs.fr.29.10 does, finalize and save its recording and report, and then continue with the unsaved-changes check and the action; when finalizing fails, the Editor shall show the error and cancel the action; Cancel shall leave the run going and change nothing.
  - Use cases: UC-02; Initial Plan: —
