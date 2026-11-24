---
epic: EP-36
group: srs.fr.36
part: editor
---

# EP-36 Data Assets, Prefabs and Scene Organisation

**Epic:** As a game developer, I want to editor tools for data assets, linked prefabs, custom types, references, system groups, layers and multiple scenes, so that large projects stay manageable.

This group adds the editor side of the scalability features in srs.fr.2 (data assets, prefabs, custom types, entity references, system groups, tags and layers, multiple scenes). Its screen codes (B-07, B-08, C-05c, C-05d, C-06b, C-09b, C-17, D-10, E-08 to E-10, F-04g, F-04h, F-16 to F-19) are the new frames added to the Aegis Editor UI design for these features.

## US-36.1 Data assets

**Story:** As a game developer, I want to create and edit data assets in the editor, so that shared values are easy to manage.

*Screens: C-06b, C-07, D-10, F-17*

- **srs.fr.36.1** When the user chooses Assets ▸ Create ▸ Data Asset ▸ and a data asset type (D-10), the Editor shall create a data asset of that type with default values in the current Project folder and open its name for editing.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.2** When the user chooses Assets ▸ Create ▸ Data Asset Script…, Component Script… or System Script… (D-10), the Editor shall open the New Script dialog (F-17) for that kind of type.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.3** The New Script dialog shall have a Name field, a required Semantic description field and a fields list, and shall show its destination folder (srs.fr.36.78); Create shall stay disabled until the name is a valid C++ identifier that no registered type uses, no file `<Name>.h` or `<Name>.cpp` exists in the destination folder, and the description is not empty, and the dialog shall name which of these checks fails, with the full path for an existing file.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.4** When the user clicks Create in the New Script dialog, the Editor shall create `<Name>.h` in the destination folder (srs.fr.36.78) from the matching template with the description and fields filled in, never overwriting an existing file, open it in the external code editor and show a note that the type appears after the next build.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.5** When a data asset is selected, the Inspector shall show its type, its fields with editable values and the semantic description of each field (C-06b).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.6** When the user edits a field of a data asset, the Editor shall mark the asset as unsaved; File ▸ Save and Ctrl+S shall save it.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.7** The data asset Inspector shall show "Used by N entities" with a list of the entities and components that refer to the asset; clicking an entry shall select that entity.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.8** When the user drags a data asset onto a component field of the matching type, the Editor shall make the field refer to that asset.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.9** The Inspector shall show a field that refers to a data asset with the asset's name and an Open button that selects the asset.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.10** The Project panel's Type filter ▾ and `t:` search shall include Data Asset and each data asset type.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2

## US-36.2 Prefab instances and overrides

**Story:** As a game developer, I want to see and manage how an instance differs from its prefab, with replacement limited to regular prefab destinations, so that changes remain representable and I control which changes stay local.

*Screens: C-05c, E-08, E-10, F-18, F-19*

- **srs.fr.36.11** The Hierarchy shall show prefab instance rows with a prefab icon and in blue text, and the root row of an instance shall have an arrow that opens the prefab.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.12** When a prefab instance is selected, the Inspector shall show a prefab bar with the prefab's name and the buttons Open, Select Asset and Overrides ▾ (C-05c).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.13** The Inspector shall show each overridden field with a bold label and a blue line in the margin.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.14** When the user right-clicks an overridden field, the Editor shall show the overridden-field menu (E-08) with the items Apply to Prefab and Revert.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.15** When the user chooses Apply to Prefab on a field, the Editor shall write that value to the prefab asset, or to the asset chosen as stated in srs.fr.36.71 when more than one asset can hold it, and remove the override only when the instance's resolved value then equals the applied value (srs.fr.36.74); a reference value whose target is outside the chosen asset shall not be applied (srs.fr.2.124).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.16** When the user chooses Revert on a field, the Editor shall restore the prefab's value and remove the override.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.17** When the user opens Overrides ▾, the Editor shall show the overrides popover (F-18), which lists, per entity, every overridden field, every added or removed component and every added child entity, with Apply All and Revert All buttons.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.18** When the user clicks Apply All, the Editor shall apply the instance's overrides to one destination asset chosen as in srs.fr.36.71, override by override as stated in srs.fr.36.79, update all other instances in the loaded scenes (srs.fr.2.31), and show the toast "N overrides kept" with their reasons when any override stays on the instance.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.19** When the user clicks Revert All, the Editor shall discard every override of the instance and delete every entity added to it.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.20** When the user right-clicks a prefab instance row in the Hierarchy, the Editor shall show the prefab instance menu (E-10) with Open Prefab, Select Prefab Asset, Unpack Prefab and Unpack Completely.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.21** When the user chooses Open Prefab from the prefab instance menu, the Editor shall open the prefab in Prefab Mode (B-07).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.22** When the user chooses Select Prefab Asset from the prefab instance menu, the Editor shall select the prefab asset in the Project panel.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.23** When the user chooses Unpack Prefab, the Editor shall turn the outer instance into normal entities with no link to its prefab, keeping nested prefab instances linked.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.24** When the user chooses Unpack Completely, the Editor shall turn the instance and every nested prefab instance inside it into normal entities with no prefab links.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.25** When the user drags an entity onto an existing prefab asset, the Editor shall show the Replace or Variant dialog (F-19) with Cancel, Replace Prefab and Create Variant. Replace Prefab shall be disabled when the destination is a prefab variant, with the reason "Can't replace a variant: edit its overrides in Prefab Mode"; Create Variant shall have the eligibility rules of srs.fr.2.122.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.26** When the user clicks Replace Prefab in F-19 for a regular prefab destination, the Editor shall overwrite the prefab with the dragged entity, matching identities as stated in srs.fr.36.77, and update every instance in the loaded scenes; a variant destination shall never be replaced by this operation (srs.fr.36.25).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2

## US-36.3 Prefab Mode

**Story:** As a game developer, I want to edit a prefab on its own, so that I can change the original without touching a scene.

*Screens: B-07*

- **srs.fr.36.27** When the user double-clicks a prefab asset or clicks Open on the prefab bar, the Editor shall open Prefab Mode, which shows only the prefab's entities in the Hierarchy and Scene view, with a breadcrumb of the scene and prefab names, a back arrow, an Auto Save toggle and a Save button (B-07).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.28** In Prefab Mode, the Editor shall show nested prefabs as instances that can be opened in turn, adding a breadcrumb segment for each level; before opening another prefab from a nested instance or a breadcrumb segment while the open prefab has unsaved changes, the Editor shall show F-01 for it: Save shall continue only when the save succeeds, Don't Save shall discard its changes and continue, and Cancel shall keep the current prefab open and the breadcrumb unchanged.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.29** When the user clicks Save in Prefab Mode, or Auto Save is on and a change is made, the Editor shall save the prefab asset and update every instance in the loaded scenes; instances in other scenes shall get the change when their scene is next loaded (srs.fr.2.70).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.30** When the user clicks the back arrow or the breadcrumb's scene segment, or presses Esc, the Editor shall leave Prefab Mode, showing F-01 first if the prefab has unsaved changes.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.31** When the user tries to place an instance of a prefab inside itself, or inside a prefab that it contains or derives from through any chain of base and nesting links, the Editor shall refuse the drop and show the toast "A prefab cannot contain itself" (srs.fr.2.120).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2

## US-36.4 Custom types and hot reload

**Story:** As a game developer, I want to see my own types appear and reload after a build, so that I can extend the engine without restarting the editor.

*Screens: C-15, F-17*

- **srs.fr.36.32** After the project scripts build successfully, the Editor shall reload the component, system and data asset types and show the toast "Scripts reloaded"; during Play mode, a headless run or a Playtest Agent session it shall instead show "Scripts built · reload after the run" and reload when the run ends (srs.fr.2.100).
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.2.3
- **srs.fr.36.33** When the project scripts fail to build, the Editor shall keep the previously loaded types and show the build errors in the Console; a simulation operation that is going shall continue and its Stop controls shall stay enabled (srs.fr.17.28), and starting Play mode shall be disabled until the errors are fixed.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.2.3
- **srs.fr.36.34** When a reload removes fields that held values, the Editor shall list the lost values in a Console warning per entity.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.2.3
- **srs.fr.36.35** When a component, system or data asset type is rejected because it has no semantic description (srs.fr.2.38), the Components panel shall list it in a Not registered section with a warning badge that explains what is missing, and the type shall not be usable until it is registered.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.2.3

## US-36.5 Entity references

**Story:** As a game developer, I want to set and follow entity reference fields, so that links between entities are easy to make and to check.

*Screens: C-05d, F-16*

- **srs.fr.36.36** The Inspector shall show an entity reference field with the referenced entity's name and ID.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.37** When the user drags an entity from the Hierarchy onto an entity reference field of a scene entity, the Editor shall set the reference to that entity; when that entity is in a different scene from the field's entity, the Editor shall refuse the drop and show the toast "References must stay in the same scene" (srs.fr.2.65); in Prefab Mode, targets shall follow srs.fr.36.80.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.38** When the user clicks the target button of an entity reference field, the Editor shall open the Pick Entity dialog (F-16), which lists the entities of the field's scene, or in Prefab Mode those of srs.fr.36.80, with a search box; choosing an entity shall set the reference and Esc shall cancel.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.39** While the Pick Entity dialog is open, clicking an entity in the Scene view or Hierarchy shall also choose it.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.40** When the user double-clicks a set entity reference field, the Editor shall select the referenced entity.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.41** When a referenced entity has been deleted in Edit mode, the field shall become Missing (srs.fr.36.75) and show "Missing (was ID n)" in yellow, and the entity's Hierarchy row shall show the error badge.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-36.6 System groups and ordering

**Story:** As a game developer, I want to see and manage system groups and ordering rules, so that I understand why systems run in the order they do.

*Screens: C-09b*

- **srs.fr.36.42** The Systems panel shall show systems nested under their groups, and groups nested under their tick phase (C-09b).
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.36.43** When the user clears a group's checkbox, the Engine shall skip every system in the group from the next tick on in Play mode, or from the next run when a headless run or session is going (srs.fr.7.31).
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.36.44** When the user selects a system, the Systems panel shall list its Run before and Run after rules, and the user shall be able to add or remove a rule with + and ✕.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.36.45** When the user drags a system to a position that breaks a before/after rule, the Systems panel shall refuse the move and name the rule.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.36.46** When the ordering rules form a cycle, the Systems panel shall show the systems in the cycle in red and a message that names them.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.36.47** When the user clicks ＋ New Group, the Systems panel shall ask for a name and a tick phase and add the empty group.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3

## US-36.7 Tags and layers

**Story:** As a game developer, I want to manage tags, layers and layer collisions in Project Settings, so that the project's organisation and physics filtering are in one place.

*Screens: F-04g, F-04h*

- **srs.fr.36.48** Project Settings shall have a Tags and Layers tab (F-04g) that lists the project's tags and its 32 layer slots.
  - Use cases: UC-04; Initial Plan: ip.fr.3.6
- **srs.fr.36.49** When the user clicks + under Tags, the Editor shall add a tag with its name field focused; ✕ shall remove a tag after showing how many entities use it.
  - Use cases: UC-04; Initial Plan: ip.fr.3.6
- **srs.fr.36.50** When the user types a name into an empty layer slot, the Editor shall create that layer; clearing the name shall remove it after showing how many entities use it.
  - Use cases: UC-04; Initial Plan: ip.fr.3.6
- **srs.fr.36.51** The Physics tab shall show the layer collision matrix as a triangle of checkboxes, one per pair of named layers (F-04h).
  - Use cases: UC-04; Initial Plan: ip.fr.3.6
- **srs.fr.36.52** When the user clears a checkbox in the layer collision matrix, entities on those two layers shall no longer collide from the next run on.
  - Use cases: UC-04; Initial Plan: ip.fr.3.6
- **srs.fr.36.53** The Inspector's Tag ▾ field shall end with the item Add Tag…, which behaves as stated in srs.fr.22.6, and the Layer ▾ field shall end with the item Edit Layers…, which opens the Tags and Layers tab.
  - Use cases: UC-04; Initial Plan: ip.fr.3.6

## US-36.8 Event Bus panel

**Story:** As a game developer, I want to see which events exist and who sends and receives them, so that I can follow how systems talk to each other.

*Screens: C-17*

- **srs.fr.36.54** The Event Bus panel (C-17) shall list every registered event type with its semantic description, its publishing systems and its subscribing systems.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.8.2
- **srs.fr.36.55** During Play mode and headless runs, the Event Bus panel shall show, for each event type, the number of events published in the last tick and in total.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.8.2
- **srs.fr.36.56** When the user clicks an event type, the Event Bus panel shall show its fields and highlight its publishers and subscribers in the Systems panel.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.8.2
- **srs.fr.36.57** When the user types in the Event Bus panel's search box, it shall show only the event types whose name or description contains the text.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.8.2
- **srs.fr.36.58** When the user clicks Show in Graph for an event type, the Editor shall open the Semantic Graph filtered to that event type.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.10.4
- **srs.fr.36.59** When the user chooses Window ▸ Analysis ▸ Event Bus, the Editor shall open the Event Bus panel or bring it to the front.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.8.2

## US-36.9 Multiple scenes

**Story:** As a game developer, I want to work with several scenes loaded at once, so that I can split a large game into parts.

*Screens: B-08, E-09, D-01*

- **srs.fr.36.60** When the user chooses File ▸ Open Scene Additive… or drags a scene asset into the Hierarchy, the Editor shall load the scene beside the open ones, resolving entity ID collisions by srs.fr.2.64, and show it as its own scene header row in the Hierarchy (B-08).
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.61** Each scene header row shall show the scene name and "*" while it has unsaved changes; right-clicking it or its ⋮ button shall show the scene menu (E-09) with Set Active Scene, Save Scene, Unload Scene, Remove Scene and Load Additively.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.62** When the user chooses Save Scene from the scene menu, the Editor shall save only that scene.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.63** When the user chooses Remove Scene, the Editor shall close the scene and remove its header row, showing F-01 first if it has unsaved changes; when it was the active scene, the first remaining loaded scene in load order shall become active, or, when none remains, srs.fr.36.81 shall apply.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.64** When the user chooses Load Additively from the scene menu, the Editor shall open the file picker and load the chosen scene beside the open ones.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.65** When the user chooses Set Active Scene, new entities shall be created in that scene and its root row shall be shown in bold.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.66** When the user chooses Unload Scene, the Editor shall remove the scene's entities from the editor but keep its row (greyed) so it can be loaded again; F-01 shall be shown first if it has unsaved changes; when it was the active scene, the first remaining loaded scene in load order shall become active, or, when none remains, srs.fr.36.81 shall apply.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.67** When the user drags an entity from one scene header to another, the Editor shall move the entity and its children to that scene as one undo step, turn every reference between the moved entities and the entities left behind into a Missing reference (srs.fr.36.75), and list those references in a Console warning.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.36.68** The Inspector shall have a Persistent checkbox, shown only for root entities (srs.fr.2.68), that keeps the entity and its descendants loaded when the active scene changes at runtime; persistent entities shall be listed in a Persistent section of the Hierarchy (B-08).
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-36.10 Assets during Play mode

**Story:** As a game developer, I want the prefabs and data assets to stay unchanged while the game runs, so that a play session never changes my assets.

- **srs.fr.36.69** During Play mode, the Editor shall disable Prefab Mode, Apply to Prefab, Apply All, Replace Prefab and Create Variant (srs.fr.2.74); during a headless run they shall stay enabled, because the run uses its content snapshot (srs.fr.7.25).
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.36.70** While the user edits a data asset during Play mode, the Inspector shall show the yellow bar "Changes in Play mode are not saved", the running game shall use the new values from the next tick (srs.fr.2.26), and the changes shall be discarded on Stop (srs.fr.2.28).
  - Use cases: UC-03; Initial Plan: —

## US-36.11 Variants and scene bounds in the Editor

**Story:** As a game developer, I want to to edit variants and map bounds where I already work, so that I can set them up without leaving the Editor.

- **srs.fr.36.71** When the user applies an overridden field, or chooses Apply All, on an instance whose values come from more than one asset, the Editor shall list by name every asset that can hold the change (the instance's prefab or variant, each base up its chain, and for an entity of a nested instance each containing prefab and the nested prefab with its chain), show for a single field the value the instance would then resolve to and for Apply All how many of the overrides that asset can take (srs.fr.36.79), disable an asset that is masked for a single field (srs.fr.36.74) or can take none of the overrides, with a tooltip that names the reason, and write to the one the user picks.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.36.72** In Prefab Mode for a variant, the breadcrumb shall show the base prefab's name, and the Inspector shall mark the fields the variant overrides as in srs.fr.36.13.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.36.73** When the user selects a scene header row, the Inspector shall show the scene's Map Bounds with an Enable checkbox, minimum and maximum X and Y fields and an Edit Bounds button that shows drag handles in the Scene view; the Scene view shall draw enabled map bounds as a dashed rectangle.
  - Use cases: UC-02, UC-03; Initial Plan: —

## US-36.12 Apply destinations

**Story:** As a game developer, I want to applying an override to keep the value I see, so that Apply never makes my change disappear.

- **srs.fr.36.74** An asset shall be masked for a field when a later step of srs.fr.2.89 between that asset and the instance (a variant down the chain or a containing prefab's override of the nested instance) also overrides the field; writing to a masked asset would not change the instance's resolved value, so the Editor shall not offer it for Apply and shall never remove the instance's override after writing to it.
  - Use cases: UC-03; Initial Plan: —

## US-36.13 Reference states

**Story:** As a game developer, I want a broken reference to stay visible until I fix it, so that it is never silently resolved to another entity.

- **srs.fr.36.75** In Edit mode, an entity reference field shall be None (intentionally empty), Set (pointing to an entity) or Missing (its target was deleted, moved to another scene or not found when the scene loaded), and a Missing field shall keep the old ID only as an editor-side tombstone.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.36.76** A Missing reference shall be saved in the scene file as missing with its old ID, reloaded as Missing, read by systems as empty, and never resolved again to any entity, even one with the same ID; Undo of the deletion or move that made it Missing shall make it Set again, and ✕ or picking a new target shall make it None or Set. This shall not apply to a reference inside a prefab instance that is Missing only because a prefab, nested prefab or base asset is unavailable (srs.fr.2.125): it shall keep its prefab-local ID path and be resolved again by srs.fr.2.123 when the asset returns.
  - Use cases: UC-02; Initial Plan: —

## US-36.14 Replace identity

**Story:** As a game developer, I want to Replace Prefab to keep the identity of entities that still match, so that scene references and overrides survive a replace.

- **srs.fr.36.77** Replace Prefab shall apply only to regular prefab destinations (srs.fr.36.25) and shall keep prefab-local IDs only through the map of the dragged entity's own instance, when the dragged entity is the root of an instance of this prefab: each dragged entity in that map keeps the prefab-local ID it is mapped from, which the map holds once, so that no ID is kept by two entities; any other instance of this prefab inside the dragged hierarchy would make the prefab contain itself and shall be refused as stated in srs.fr.2.120. Every other dragged entity shall get a new ID owned by the prefab, above its allocation mark (srs.fr.2.119), and the IDs that no dragged entity keeps shall be retired; retired IDs stay below the mark and are never given out again; instances shall remove the entities of retired IDs as stated in srs.fr.2.72 and drop overrides of them as stated in srs.fr.2.71; references of the dragged entities shall be stored as stated in srs.fr.2.123 and srs.fr.2.124.
  - Use cases: UC-03; Initial Plan: —

## US-36.15 Script destination

**Story:** As a game developer, I want to new scripts to go exactly where the dialog says, so that a script is never created over an existing file.

- **srs.fr.36.78** The destination folder of the New Script dialog shall be the current Project folder when it is Scripts or a folder under it, and the Scripts folder root otherwise; validation, creation and error messages shall all use that folder.
  - Use cases: UC-02, UC-03, UC-04; Initial Plan: —

## US-36.16 Apply All destinations

**Story:** As a game developer, I want to Apply All to say which overrides each destination can take, so that nothing is silently lost or duplicated.

- **srs.fr.36.79** Apply All shall use one destination for the whole operation and handle each override separately: a field override shall be applied unless the destination is masked for it (srs.fr.36.74) or its value refers to an entity outside the destination (srs.fr.2.124); an added or removed component and an added entity shall be applied only when the destination contains the entity they belong to and no later step of srs.fr.2.89 overrides the same component or entity; every other override shall stay on the instance, listed with its reason; applied added entities shall follow srs.fr.2.109, so that no duplicate is created and identities are kept.
  - Use cases: UC-03; Initial Plan: —

## US-36.17 References in Prefab Mode

**Story:** As a game developer, I want to to set references while editing a prefab, so that a prefab reference never points outside the prefab.

- **srs.fr.36.80** In Prefab Mode, an entity reference field of an entity of the open prefab shall accept only entities of that prefab, including those inside nested instances: the Pick Entity dialog (F-16) shall list them in hierarchy order, with nested instances' entities under their instance root, dragging any other entity onto the field shall be refused with the toast "References must stay inside the prefab", and the chosen target shall be stored as its prefab-local ID path (srs.fr.2.123) when the prefab is saved.
  - Use cases: UC-03; Initial Plan: —

## US-36.18 No loaded scene

**Story:** As a game developer, I want to to know what the Editor does when no scene is loaded, so that removing the last scene never leaves commands acting on nothing.

- **srs.fr.36.81** When no loaded scene remains, the Editor shall have no active scene, show in the Hierarchy, where the greyed rows of unloaded scenes (srs.fr.36.66) stay visible, the hint "No scene loaded", and disable every command that needs an active scene, such as creating entities, Play and Add Open Scene, until a scene is created or loaded, which then becomes the active scene.
  - Use cases: UC-02; Initial Plan: —

## US-36.19 Tag and layer names

**Story:** As a game developer, I want tag and layer names to be unique, so that an entity's tag or layer always means one entry.

- **srs.fr.36.82** For tags and for layers, a name shall be 1 to 64 characters after leading and trailing spaces are removed, and shall differ from every other name in the same list when compared without regard to case; when the user leaves a tag or layer name field with a name that breaks this rule, the Editor shall keep the previous name, or remove a new tag or leave the layer slot empty when it never had a valid name, and show the reason under the field; the protected defaults of srs.fr.2.80 shall not be renamed.
  - Use cases: UC-04; Initial Plan: —

## US-36.20 Create Variant and Cancel in F-19

**Story:** As a game developer, I want to save a dragged instance as a variant or cancel the drop, so that I can keep the existing prefab unchanged.

- **srs.fr.36.83** When the user clicks Create Variant in F-19, available only as stated in srs.fr.2.122, the Editor shall save a new prefab variant (srs.fr.2.94) whose base is the existing prefab and whose differences are the dragged instance's overrides, and link the dragged entity to the new variant.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.36.84** When the user clicks Cancel in F-19, the Editor shall change nothing.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
