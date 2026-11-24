---
epic: EP-23
group: srs.fr.23
part: editor
---

# EP-23 Assets and Project Panel

**Epic:** As a game developer, I want to browse, import and manage the project's asset files, so that every sprite, scene and script is easy to find and use.

This group covers the Project panel (C-07), its context menu (E-03), the Assets menu (D-03) and the asset Inspector (C-06).

## US-23.1 Browsing

**Story:** As a game developer, I want to browse and search the Assets folder, so that I can find any asset.

*Screens: C-07*

- **srs.fr.23.1** The Project panel shall show the project's Assets folder and its Scripts folder, which are sibling folders inside the project root, as the two roots of a folder tree beside a grid or list of the current folder's assets; files created by the New Script dialog (srs.fr.36.4) shall appear under Scripts as Script assets.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.2** When the user clicks a folder in the folder tree, the Project panel shall show that folder's assets and update the breadcrumb.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.3** When the user clicks a breadcrumb segment, the Project panel shall show that folder.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.4** When the user types in the Project search box, the Project panel shall list, as one flat list, the assets from all folders whose name contains the typed text, ignoring letter case.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.5** When the search text contains `t:` followed by a type name, such as `t:Sprite` or `t:Scene`, the Project panel shall list only assets of that type.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.6** When the user picks a type from Type filter ▾ (Sprite, Scene, Prefab, Audio, Script or Input Map), the Project panel shall show only assets of that type.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.7** When the user moves the Size slider, the Project panel shall change icon size, switching to list rows at the smallest size.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.8** When the user clicks an asset, the Editor shall select it and show it in the asset Inspector (C-06).
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.9** When the user double-clicks a scene asset, the Editor shall open that scene, applying the unsaved-changes check of srs.fr.15.22 first.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.10** When the user double-clicks a script asset, the Editor shall open it in the external code editor set in Preferences.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.11** When the user double-clicks a sprite asset, the Editor shall open the Sprite Slicer for it.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.12** The Editor shall allow an asset to be dragged from the Project panel to the Hierarchy, the Scene view or an Inspector asset field.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.13** When the user clicks the Project panel's + Create button, the Editor shall show the same items as Assets ▸ Create.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.14** When the user right-clicks an asset or folder, the Editor shall show the asset context menu (E-03).
  - Use cases: UC-03; Initial Plan: —

## US-23.2 Creating and importing

**Story:** As a game developer, I want to create new assets and import files, so that I can bring my art and data into the project.

*Screens: D-03*

- **srs.fr.23.15** When the user chooses Assets ▸ Create ▸ Folder, the Editor shall create a folder in the current Project folder and open its name for editing.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.16** When the user chooses Assets ▸ Create ▸ Scene, the Editor shall create an empty scene asset in the current Project folder and open its name for editing.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.17** When the user chooses Assets ▸ Create ▸ Sprite Sheet with an image asset selected, the Editor shall create in the current Project folder a sprite sheet asset that refers to that image and holds its slices, and open its name for editing; with no image selected, the command shall be disabled with the tooltip "Select an image first".
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.18** When the user chooses Assets ▸ Create ▸ Input Map, the Editor shall create an input map asset that contains the default actions and bindings of srs.fr.5.28 and open its name for editing.
  - Use cases: UC-03; Initial Plan: ip.fr.5.1
- **srs.fr.23.19** When the user chooses Assets ▸ Create ▸ Prefab, the Editor shall create in the current Project folder a prefab asset whose one root entity is an empty entity with a Position component named after the asset (srs.fr.2.29), and open its name for editing.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.20** When the user chooses Assets ▸ Import New Asset…, the Editor shall open the operating system's file picker for png, wav, ttf and json files.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.21** When the user picks files to import, the Editor shall copy them into the current Assets folder and select them.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.22** When an imported file has the same name as an existing file in the folder, the Editor shall add a number to the new file's name instead of overwriting the existing file.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.23** When the user tries to import a file of an unsupported type, the Editor shall show an error toast and not copy the file.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.24** When the user chooses Assets ▸ Show in Explorer with an asset selected, the Editor shall open the operating system's file browser at that asset.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.25** When the user chooses Assets ▸ Refresh or presses Ctrl+R, the Editor shall rescan the Assets and Scripts folders and show files that were added, changed or removed outside the Editor.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.26** When the user chooses Assets ▸ Reimport with an asset selected, the Editor shall re-read the file with its import settings and show the progress in the status bar.
  - Use cases: UC-03; Initial Plan: —

## US-23.3 Asset context menu

**Story:** As a game developer, I want to act on an asset from its context menu and confirm every permanent deletion, so that common asset actions are easy to reach without silently losing files.

*Screens: E-03*

- **srs.fr.23.27** When the user chooses an item from Create ▸ in the asset context menu, the Editor shall create that asset inside the right-clicked folder.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.28** When the user chooses Open from the asset context menu, the Editor shall open the asset as a double-click would.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.29** When the user chooses Rename from the asset context menu or presses F2, the Editor shall make the asset's name editable in place.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.30** When the user chooses Duplicate from the asset context menu or presses Ctrl+D, the Editor shall create a copy of the asset in the same folder with a new asset ID; a duplicated scene shall get new entity IDs as stated in srs.fr.2.84, and a duplicated prefab or variant shall be remapped as stated in srs.fr.2.111.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.31** When the user chooses Delete from the asset context menu or presses Del, the Editor shall first show F-02, whether or not the asset is referenced, and shall delete the asset only when the user confirms, retiring its recovery entry as stated in srs.fr.15.20; asset deletion shall not be undoable, and F-02 shall state this and list any references as stated in srs.fr.35.5 and srs.fr.35.7.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.32** When the user chooses Show in Explorer from the asset context menu, the Editor shall open the operating system's file browser at the file.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.33** When the user chooses Copy Path from the asset context menu, the Editor shall copy the asset's path relative to the project root, starting with `Assets/` or `Scripts/`, to the clipboard.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.34** When the user chooses Reimport from the asset context menu, the Editor shall re-read the file with its import settings.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.35** When the user chooses Find References in Scene from the asset context menu, the Editor shall select every entity in the open scene that uses the asset.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.36** When the user chooses ✦ Ask AI about This from the asset context menu, the Editor shall open the AI Assistant (C-14) with the asset as a context chip.
  - Use cases: UC-03; Initial Plan: ip.fr.11.2

## US-23.4 Asset Inspector and Sprite Slicer

**Story:** As a game developer, I want to change an asset's import settings and slice sprite sheets, so that assets look right in the game.

*Screens: C-06*

- **srs.fr.23.37** When an image asset is selected, the Inspector shall show its import settings: texture type (Sprite or Texture, default Sprite), pixels per unit, which shall be a finite number above 0 with the default 32, filter (Project default, Point or Bilinear, default Project default, where the asset's own value wins over the project's Texture filter), wrap mode (Clamp or Repeat, default Clamp), sprite mode (Single or Multiple, default Single, where Multiple makes it a sprite sheet for the Sprite Slicer) and pivot (default the centre). A sprite W by H image pixels in size with P pixels per unit shall be W / P by H / P metres in the world, so that with the default 32 one image pixel is one screen pixel at zoom 1 (srs.fr.3.38); the pivot shall be kept as a fraction of the sprite's width and height, so that changing P scales the sprite about its pivot, and the sprite's selection bounds shall follow its world size.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.23.38** When the user changes an import setting, the Editor shall mark the asset as having unapplied changes.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.23.39** When the user clicks Apply, the Editor shall reimport the asset with the new settings and refresh every sprite that uses it.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.23.40** When the user clicks Revert, the Editor shall ask "Discard these changes?" with Discard and Cancel and, on Discard, discard the unapplied changes; Cancel shall keep them.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.23.41** When the user selects another item while an asset has unapplied changes, the Editor shall ask whether to Apply or Revert them.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.23.42** When the user clicks Open Sprite Slicer… for a sprite sheet, the Editor shall open the Sprite Slicer.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.23.43** In the Sprite Slicer, the Editor shall slice the sheet into a grid by cell size or by row and column count, each a whole number of 1 or more, leaving out cells that would extend past the image's edge, and Apply shall save the slices as named sub-sprites of the asset with their rectangles and pivots.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.23.44** When the user clicks Open, the Editor shall open the asset file in the operating system's default application.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.23.45** The asset Inspector shall show a Used by list of the components that reference the asset; clicking an entry shall select its entity.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1

## US-23.5 Asset Inspector by type

**Story:** As a game developer, I want the Inspector to show what fits each kind of asset, so that I only see settings that apply.

- **srs.fr.23.46** When a scene, prefab or data asset is selected, the Inspector shall show that asset type's own view: for a scene its entity count and an Open button, for a prefab an Open button that enters Prefab Mode (srs.fr.36.27), and for a data asset its fields (srs.fr.36.5).
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.47** When a script asset is selected, the Inspector shall show a read-only preview of its text; when an audio clip is selected, it shall show its duration, channels and sample rate with a Play/Stop preview button; any other asset shall show its name, type, size and path.
  - Use cases: UC-03; Initial Plan: —

## US-23.6 Scripts folder actions

**Story:** As a game developer, I want to create and import to behave sensibly when Scripts is selected, so that files never land in the wrong root.

- **srs.fr.23.48** While a folder under Scripts is the current Project folder, Assets ▸ Create items other than Folder and the script items (srs.fr.36.2), and Import New Asset…, shall be disabled with the tooltip "Create this in Assets", and Folder shall create its folder there; script items shall use the destination of srs.fr.36.78.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.49** Script items chosen while a folder under Assets is current shall use the Scripts folder root as their destination (srs.fr.36.78).
  - Use cases: UC-03; Initial Plan: —

## US-23.7 Input Map assets

**Story:** As a game developer, I want to to edit the actions and bindings of an Input Map asset, so that a player can use a custom map that I can maintain.

- **srs.fr.23.50** When an Input Map asset is selected, the Inspector shall show the same action and binding table as the Input tab (srs.fr.27.12 to srs.fr.27.18), with + Add Action, ✕ and Reset to Defaults; edits shall mark the asset as unsaved and be saved with File ▸ Save, like a data asset (srs.fr.36.6); during Play mode the table shall be read-only with the tooltip "Stop Play mode first" (srs.fr.17.26).
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.23.51** An Input Map asset shall be independent of the project's Input Map: changing one shall not change the other, and Reset to Defaults on an asset shall replace its actions and bindings with those of srs.fr.5.28.
  - Use cases: UC-03; Initial Plan: —

## US-23.8 Imported file checks

**Story:** As a game developer, I want damaged or mislabelled files to be stopped at import, so that a renamed or broken file never enters the project.

- **srs.fr.23.52** Before copying an imported file, the Editor shall check its content against its type: a png file shall decode as a PNG image, a wav file as a PCM WAV audio clip, a ttf file as a TrueType font and a json file as UTF-8 JSON; a file that fails shall not be copied and shall be named, with the reason, in an error toast, while the other files of the same import shall still be imported and selected.
  - Use cases: UC-03; Initial Plan: —

## US-23.9 Re-slicing a sprite sheet

**Story:** As a game developer, I want sprite references to survive re-slicing, so that changing a grid never silently swaps the sprites my entities use.

- **srs.fr.23.53** Apply in the Sprite Slicer shall name the slices <asset name>_<n>, where n counts from 0 row by row from the top-left, and a sprite reference shall name the asset and n; after a re-slice, a reference to an n that still exists shall use that slice's new rectangle and pivot, and a reference to an n that no longer exists shall be kept, drawn as a missing sprite (srs.nfr.5.5) and named, with its entity or prefab, in a Console warning.
  - Use cases: UC-03; Initial Plan: ip.fr.4.1
