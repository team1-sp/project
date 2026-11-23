---
epic: EP-18
group: srs.fr.18
part: editor
---

# EP-18 Hierarchy Panel

**Epic:** As a game developer, I want a tree of the scene's entities that I can search, select and rearrange, so that I can find and organise objects in large scenes.

This group covers the Hierarchy panel (C-02), its entity context menu (E-01) and its empty-area context menu (E-02).

## US-18.1 Tree and search

**Story:** As a game developer, I want to see and search the scene's entity tree, so that I can find any entity quickly.

*Screens: C-02*

- **srs.fr.18.1** The Hierarchy shall show the open scene as a tree whose root is the scene name and whose rows are its entities, nested under their parents.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.2** The Hierarchy shall show a disabled entity's row in muted text.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.3** When the user types in the Hierarchy search box, the Hierarchy shall hide the rows whose entity name does not contain the typed text, ignoring letter case, keep the parents of matching rows visible, and highlight the matching text.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.4** When the user types `t:` followed by a component name, the Hierarchy shall show only the entities that have that component.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.5** When the user types `id:` followed by a number, the Hierarchy shall show only the entity with that ID.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.6** When the user clicks ✕ in the search box, the Hierarchy shall clear the search and show all rows.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.7** When the user clicks a row's fold arrow, the Hierarchy shall expand or collapse that entity's children; Alt+click shall expand or collapse all of its descendants.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-18.2 Selection

**Story:** As a game developer, I want to select one or several entities in the tree, so that I can edit them together.

*Screens: C-02*

- **srs.fr.18.8** When the user clicks a row, the Editor shall select that entity, show it in the Inspector and outline it in the Scene view.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.9** When the user Ctrl+clicks a row, the Editor shall add the entity to the selection or remove it if it is already selected.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.10** When the user Shift+clicks a row, the Editor shall select every row between the last clicked row and this one.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.11** When the user double-clicks a row, the Editor shall centre the Scene view camera on that entity.
  - Use cases: UC-02; Initial Plan: —

## US-18.3 Row icons and badges

**Story:** As a game developer, I want to hide, lock and see problems on entities from the tree, so that I can control what I click in the scene and notice errors.

*Screens: C-02*

- **srs.fr.18.12** When the user clicks a row's eye icon, the Editor shall hide or show that entity in the Scene view only; this state shall be editor-only and not saved into the game.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.13** When the user clicks a row's lock icon, the Editor shall prevent the entity from being selected in the Scene view, while it stays selectable in the Hierarchy.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.14** An entity's Hierarchy row shall show an error badge when one of its components is the placeholder of a missing type (srs.fr.2.21), it lacks a component that one of its components requires (srs.fr.3.34), one of its entity reference fields is Missing (srs.fr.36.75), or it has an Unlockable component with no required item; the badge's tooltip shall name each component and problem.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.15** When the user clicks an error badge, the Editor shall select the entity and show the warning at the top of the Inspector.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.16** The row of the camera's follow target shall show a camera icon.
  - Use cases: UC-02; Initial Plan: ip.fr.4.4

## US-18.4 Creating and rearranging

**Story:** As a game developer, I want to create entities and drag them into place in the tree, so that the scene's structure matches my design.

*Screens: C-02*

- **srs.fr.18.17** When the user clicks the Hierarchy's + button, the Editor shall show the Entity menu items (D-04), and the chosen entity shall be created and placed as stated in srs.fr.21.13.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.18** When the user drops a dragged row between two rows, the Editor shall move the entity to that position among its siblings, shown by a blue line while dragging.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.19** When the user drops a dragged row onto another row, the Editor shall make the dragged entity a child of that row's entity, shown by a highlight while dragging.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.20** When the user drops a row onto itself or one of its own descendants, the Editor shall refuse the drop and leave the hierarchy unchanged.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.21** When the user drags a prefab asset from the Project panel into the Hierarchy, the Editor shall create an instance of the prefab at the scene origin.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.22** When the user drags a sprite asset from the Project panel into the Hierarchy, the Editor shall create an entity with Position and Sprite components that uses that sprite, at the scene origin.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.23** When the user right-clicks an entity row, the Editor shall show the entity context menu (E-01); when the user right-clicks an empty area, it shall show the empty-area menu (E-02).
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-18.5 Entity context menu

**Story:** As a game developer, I want to act on an entity from its context menu, so that common actions are one right-click away.

*Screens: E-01*

- **srs.fr.18.24** When the user chooses Rename from the entity context menu, the Editor shall make the name editable in place.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.25** When the user chooses Duplicate from the entity context menu, the Editor shall create a copy beside the original and select it.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.26** When the user chooses Delete from the entity context menu, the Editor shall delete the entity, showing F-02 first if it is referenced.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.27** When the user chooses Copy from the entity context menu, the Editor shall copy the entity and its components to the clipboard.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.28** When the user chooses Paste as Child from the entity context menu, the Editor shall paste the clipboard entities as children of this entity.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.29** When the user chooses Create Empty Child from the entity context menu, the Editor shall create an entity with only a Position component as a child of this entity.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.30** When the user chooses Create Prefab from the entity context menu, the Editor shall save the entity and its children as a prefab asset in the current Project folder.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.31** When the user chooses Set as Camera Target from the entity context menu, the Editor shall set the follow target of the active camera (srs.fr.4.12) to this entity and show a camera icon beside it in the Hierarchy; when no enabled camera exists, the command shall be disabled with the tooltip "No enabled camera".
  - Use cases: UC-02; Initial Plan: ip.fr.4.4
- **srs.fr.18.32** When the user chooses Select Children from the entity context menu, the Editor shall add all of the entity's children to the selection.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.18.33** When the user chooses ✦ Ask AI about This from the entity context menu, the Editor shall open the AI Assistant (C-14) with this entity as a context chip.
  - Use cases: UC-02; Initial Plan: ip.fr.11.2
- **srs.fr.18.34** When the user chooses Show in Semantic Graph from the entity context menu, the Editor shall open the Semantic Graph (C-13) centred on this entity's node.
  - Use cases: UC-02; Initial Plan: ip.fr.10.4

## US-18.6 Empty-area context menu

**Story:** As a game developer, I want to create, paste or fold from the empty area of the tree, so that I can act on the scene root quickly.

*Screens: E-02*

- **srs.fr.18.35** When the user chooses an item from Create ▸ in the empty-area menu, the Editor shall create that entity at the scene root.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.36** When the user chooses Paste in the empty-area menu, the Editor shall paste the clipboard entities at the scene root; the item shall be disabled when the clipboard is empty.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.37** When the user chooses Expand All in the empty-area menu, the Hierarchy shall expand every row.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.18.38** When the user chooses Collapse All in the empty-area menu, the Hierarchy shall collapse every row.
  - Use cases: UC-02; Initial Plan: —
