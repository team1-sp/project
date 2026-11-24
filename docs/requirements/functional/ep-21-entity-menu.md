---
epic: EP-21
group: srs.fr.21
part: editor
---

# EP-21 Entity Menu

**Epic:** As a game developer, I want to create common kinds of entities from a menu, so that I do not have to add the same components by hand every time.

This group covers the Entity menu (D-04). Every new entity is selected and its name opens for editing.

## US-21.1 Creating entities

**Story:** As a game developer, I want to create empty, shaped and demo entities, so that I can block out a level quickly.

*Screens: D-04*

- **srs.fr.21.1** When the user chooses Entity ▸ Create Empty or presses Ctrl+Shift+N, the Editor shall create an entity with only a Position component, placed as stated in srs.fr.21.13, select it and open its name for editing.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.21.2** When the user chooses Entity ▸ 2D Object ▸ Sprite, the Editor shall create an entity with Position and Sprite components, placed as stated in srs.fr.21.13.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.21.3** When the user chooses Entity ▸ 2D Object ▸ Box, the Editor shall create an entity with Position, Sprite, RigidBody and Box Collider components, placed as stated in srs.fr.21.13.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.21.4** When the user chooses Entity ▸ 2D Object ▸ Circle, the Editor shall create an entity with Position, Sprite, RigidBody and Circle Collider components, placed as stated in srs.fr.21.13.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.21.5** When the user chooses Entity ▸ 2D Object ▸ Polygon, the Editor shall create an entity with Position, Sprite, RigidBody and Polygon Collider components, placed as stated in srs.fr.21.13.
  - Use cases: UC-02; Initial Plan: ip.fr.3.8
- **srs.fr.21.6** When the user chooses Entity ▸ 2D Object ▸ Camera, the Editor shall create an entity with Position and Camera components, placed as stated in srs.fr.21.13.
  - Use cases: UC-02; Initial Plan: ip.fr.4.4
- **srs.fr.21.7** When the user chooses Player, Obstacle, Collectible, Unlockable or Objective from Entity ▸ Demo, the Editor shall create an entity for that gameplay role of the reference scenario of Section 6, placed as stated in srs.fr.21.13.
  - Use cases: UC-02; Initial Plan: ip.fr.12.1

## US-21.2 Arranging entities

**Story:** As a game developer, I want to turn selections into prefabs and change draw order, so that I can reuse objects and fix overlaps.

*Screens: D-04*

- **srs.fr.21.8** When the user chooses Entity ▸ Create Prefab from Selection, the Editor shall save the selected entities as one prefab asset and show it in the Project panel: a single selected root shall become the prefab's root, and several selected roots shall become children of a new empty root entity, named after the prefab and placed at the centre of the selection, so that the prefab still has one root (srs.fr.2.29).
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.21.9** When the user chooses Entity ▸ Set as Camera Target, the Editor shall do for the selected entity what srs.fr.18.31 states.
  - Use cases: UC-02, UC-03; Initial Plan: ip.fr.4.4
- **srs.fr.21.10** When the user chooses Entity ▸ Move to Front, the Editor shall set the selected sprite's order in layer to one more than the highest order in the scene.
  - Use cases: UC-02, UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.21.11** When the user chooses Entity ▸ Move to Back, the Editor shall set the selected sprite's order in layer to one less than the lowest order in the scene.
  - Use cases: UC-02, UC-03; Initial Plan: ip.fr.4.1
- **srs.fr.21.12** Entity menu items that act on a selection shall be disabled when nothing is selected.
  - Use cases: UC-02, UC-03; Initial Plan: —

## US-21.3 Placement of new entities

**Story:** As a game developer, I want every way of creating an entity to put it in a predictable place, so that a new entity never appears under an unexpected parent.

- **srs.fr.21.13** A new entity shall be placed in the active scene by one rule: when created from the canvas menu (srs.fr.19.18) or by dropping an asset on the Scene view (srs.fr.19.16), at the scene root at the click or drop point, whatever is selected; when created from the Entity menu, its shortcuts or the Hierarchy's + button (srs.fr.18.17), as a child of the selected entity at local position zero when exactly one entity is selected and it belongs to the active scene, and otherwise at the active scene's root at the centre of the Scene view.
  - Use cases: UC-02, UC-03; Initial Plan: —
