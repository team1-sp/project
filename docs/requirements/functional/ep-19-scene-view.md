---
epic: EP-19
group: srs.fr.19
part: editor
---

# EP-19 Scene View

**Epic:** As a game developer, I want a 2D canvas where I place, select and transform entities, so that I can build levels visually.

This group covers the Scene view (C-03) and its canvas context menu (E-05).

## US-19.1 Navigation

**Story:** As a game developer, I want to pan and zoom the scene, so that I can work on any part of a level.

*Screens: C-03*

- **srs.fr.19.1** The Scene view shall show the scene on a 2D canvas with a grid, sprites, collider outlines and camera bounds.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.2** When the user drags with the middle mouse button, or drags with Alt held, the Scene view shall pan.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.3** When the Hand tool is active and the user drags with the left mouse button, the Scene view shall pan.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.4** When the user scrolls the mouse wheel, the Scene view shall zoom in or out around the cursor and show the zoom percentage in the overlay.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.5** When the user clicks the zoom percentage in the overlay, the Scene view shall reset the zoom to 100 %.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.6** The Scene view shall show the world coordinates of the mouse cursor in its lower-right corner.
  - Use cases: UC-02; Initial Plan: —

## US-19.2 Selection and gizmos

**Story:** As a game developer, I want to select entities on the canvas and transform them with handles, so that I can lay out a level by hand.

*Screens: C-03*

- **srs.fr.19.7** When the user clicks a sprite in the Scene view, the Editor shall select its entity and show the gizmo of the active tool on it; when the click hits the sprites of several entities, it shall first leave out entities hidden with the eye toggle (srs.fr.18.12) or locked with the lock icon (srs.fr.18.13) and select, among the rest, the one whose sprite is drawn last (srs.fr.4.2); when no such entity is hit, the selection shall stay unchanged.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.8** When the user clicks a sprite of a locked entity, the Editor shall not select it.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.9** When the user drags with the left mouse button on an empty area, the Scene view shall draw a selection box and, when the button is released, select the entities whose displayed sprite bounds, or position for an entity without a sprite, lie entirely inside the box, leaving out entities hidden with the eye toggle (srs.fr.18.12) or locked with the lock icon (srs.fr.18.13).
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.10** When the user drags a gizmo handle, the Editor shall change the position, rotation or scale of every selected entity as one undoable change and update the Inspector values live: a move shall add the same world offset to each; in Center mode a rotation or scale shall act on the whole selection about the gizmo origin, changing each entity's position and its own rotation or scale by the same amount; in Pivot mode each entity shall rotate or scale about its own position.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.11** The Editor shall record each gizmo drag, from mouse down to mouse up, as one undo step.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.12** While grid snapping is on, the Editor shall round positions changed by the Move tool to the nearest multiple of the grid step.
  - Use cases: UC-02; Initial Plan: —

## US-19.3 Overlays

**Story:** As a game developer, I want to choose what helper graphics the Scene view shows, so that I see what matters for the task.

*Screens: C-03*

- **srs.fr.19.13** When the user clicks the overlay's Grid toggle, the Scene view shall show or hide the grid; the grid step field shall set the grid and snap step, 1 unit by default, and shall accept only finite numbers above 0, refusing any other value with "Grid step must be a number above 0" and keeping the previous step.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.14** When the user toggles colliders, velocity arrows, camera bounds, entity IDs or raycast debug in the overlay's Gizmos ▾ menu, the Scene view shall show or hide that helper.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.19.15** The Editor shall save the Gizmos ▾ choices with the current layout.
  - Use cases: UC-02; Initial Plan: —

## US-19.4 Assets and context menu

**Story:** As a game developer, I want to drop assets on the canvas and use a canvas context menu, so that I can place objects exactly where I want.

*Screens: C-03, E-05*

- **srs.fr.19.16** When the user drops an asset from the Project panel onto the Scene view, the Editor shall create at the drop point an entity with a Sprite for a sprite asset, a prefab instance for a prefab, or an entity with an Audio Source for an audio clip; for any other asset type it shall refuse the drop and show the toast "Can't place <type> in the scene".
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.19.17** When the user right-clicks the Scene view canvas, the Editor shall show the canvas context menu (E-05).
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.19.18** When the user chooses an item from Create ▸ in the canvas menu, the Editor shall create that entity at the click point, at the root of the active scene, as stated in srs.fr.21.13.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.19.19** When the user chooses Paste Here in the canvas menu, the Editor shall paste the clipboard entities as stated in srs.fr.16.8, but at the active scene's root, moved together so that the centre of their combined positions lies at the click point and their positions relative to one another are kept.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.19.20** When the user chooses Frame Selected or presses F, the Scene view shall centre on the selection.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.19.21** When the user chooses Frame All or presses Shift+F, the Scene view shall zoom and pan so that every entity is visible.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.19.22** When the user chooses Align Selection to Grid, the Editor shall snap the selected entities' positions to the grid step.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.19.23** When the user chooses Show Grid, Show Gizmos or Show Colliders in the canvas menu, the Scene view shall toggle that overlay and save the choice with the layout.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
