---
epic: EP-22
group: srs.fr.22
part: editor
---

# EP-22 Inspector and Components

**Epic:** As a game developer, I want to see and edit every field of the selected entity and add or remove components, so that I can shape each entity's data precisely.

This group covers the entity Inspector (C-05), the component ⋮ menu (E-04), the Add Component picker (F-03) and the Component menu (D-05).

## US-22.1 Entity header

**Story:** As a game developer, I want to edit an entity's name, state, tag and layer, so that I can identify and organise entities.

*Screens: C-05*

- **srs.fr.22.1** When one entity is selected, the Inspector shall show its enabled checkbox, name, ID, tag, layer and a lock button, followed by one foldout per component.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.22.2** When the user clears the entity's enabled checkbox, the Editor shall disable the entity, show it in muted text in the Hierarchy and have every system skip it.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.22.3** When the user edits the Name field and presses Enter, the Editor shall rename the entity and update its row in the Hierarchy.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.22.4** When the user clicks the ID label, the Editor shall copy the entity ID to the clipboard and show a toast such as "Copied 42".
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.22.5** When the user picks a tag from Tag ▾, the Editor shall set the entity's tag.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.22.6** When the user picks Add Tag… from Tag ▾, the Editor shall ask for a tag name, add it to the project's tag list and set it on the entity; during Play mode Add Tag… shall be disabled (srs.fr.17.26), while picking an existing tag stays an Inspector edit.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.22.7** When the user picks a layer from Layer ▾, the Editor shall set the entity's layer; during a run the change shall reach the physics as stated in srs.fr.3.61.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.22.8** When the user clicks the padlock, the Inspector shall keep showing this entity when the selection changes, until the padlock is clicked again.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.22.9** When several entities are selected, the Inspector shall show only the components they all have; editing a field shall change it on every selected entity, and fields whose values differ shall show "—".
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2

## US-22.2 Component foldouts

**Story:** As a game developer, I want to manage each component from its header, so that I can enable, inspect, reorder and remove components.

*Screens: C-05, E-04*

- **srs.fr.22.10** When the user clicks a component's foldout arrow, the Inspector shall expand or collapse it, and the Editor shall remember the choice for that component type.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.11** When the user clears a component's checkbox, the Editor shall disable that component so that systems skip it and, for colliders and RigidBody, the physics effects of srs.fr.3.30 and srs.fr.3.31 apply; Position and Velocity shall have no checkbox.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.12** When the user clicks a component's info (i) button, the Inspector shall show a popover with the component's semantic description: its meaning, the unit of each field and the systems that read and write it.
  - Use cases: UC-02; Initial Plan: ip.fr.10.1
- **srs.fr.22.13** When the user clicks a component's ⋮ button, the Inspector shall show the component menu (E-04).
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.14** When the user chooses Reset from the component menu, the Editor shall set every field of the component to its default value as one undo step.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.15** When the user chooses Remove Component, the Editor shall remove the component; when another component on the entity requires it (srs.fr.3.34), the Editor shall first show F-02 naming them, and shall also remove the dependent components only if the user confirms; systems that read or write the removed component shall simply stop matching the entity (srs.fr.2.16).
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.16** When the user chooses Move Up or Move Down, the Inspector shall move the component's foldout one place up or down.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.17** When the user chooses Copy Component, the Editor shall copy the component's field values to the clipboard.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.18** When the user chooses Paste Component Values, the Editor shall overwrite this component's fields with the copied values; the item shall be disabled unless a component of the same type is on the clipboard.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.19** When the user chooses Paste as New Component, the Editor shall add a copy of the copied component to this entity; the item shall be disabled when the entity already has that component type.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.20** When the user chooses ✦ Ask AI about Component, the Editor shall open the AI Assistant (C-14) and ask it to explain what the component does in this entity.
  - Use cases: UC-02; Initial Plan: ip.fr.11.2
- **srs.fr.22.21** When the user chooses Open Documentation, the Editor shall open the component's documentation page in the default web browser.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2

## US-22.3 Field editing

**Story:** As a game developer, I want to edit fields with the right kind of control, so that I can enter values quickly and correctly.

*Screens: C-05*

- **srs.fr.22.22** When the user types a value in a number field and presses Enter or moves focus away, the Editor shall store the value.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.23** When the user drags a number field's label left or right, the Editor shall decrease or increase the value continuously.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.24** When the user enters text that is not a valid number, or a value outside the field's allowed range (srs.fr.22.56), the field shall flash red and revert to its previous value.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.25** Vector fields shall show one number field per axis (X and Y), each edited as a number field.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.26** When the user clicks an asset field, the Editor shall open a picker that lists the assets of the field's type.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.27** When the user drags an asset of the right type from the Project panel onto an asset field, the Editor shall assign it; an asset of the wrong type shall be refused.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.28** When the user clicks ✕ on an asset field, the Editor shall clear the field.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.29** When the user picks Static, Kinematic or Dynamic in the RigidBody Body Type field, the Inspector shall show only the fields that apply to that body type, except the mass field, which stays shown and greyed out for static and kinematic bodies (srs.fr.22.37).
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.30** When the user clicks a colour field, the Editor shall open a colour picker with a hex value field.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.31** List fields, such as Inventory items or state sprites, shall have + and − buttons that add an item to the end or remove the selected item.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.32** When the user clicks Edit Collider, the Scene view shall show handles for the collider's size and offset; clicking Edit Collider again shall end collider editing.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.33** The Editor shall record each field change as one undo step.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2

## US-22.4 Component fields

**Story:** As a game developer, I want each built-in component to expose a fixed set of fields, so that designers and testers know exactly what can be edited.

*Screens: C-05*

- **srs.fr.22.34** The Position component shall have the fields x and y (metres), rotation (radians, shown in degrees in the Inspector, srs.fr.3.38) and scale (x, y).
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.35** The Velocity component shall have the fields x and y (metres per second).
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.36** The Sprite component shall have the fields sprite asset, tint colour, order in layer, flip (x, y) and a list of state sprites, each pairing the state Locked or Unlocked with a sprite and used as stated in srs.fr.4.4; the list shall hold at most one entry per state, and the Inspector shall refuse to add or change an entry to a state already in the list, keeping the list unchanged.
  - Use cases: UC-02; Initial Plan: ip.fr.4.1, ip.fr.4.2
- **srs.fr.22.37** The RigidBody component shall have the fields body type, mass (kg), gravity scale, linear damping and fixed rotation; the mass field shall apply to dynamic bodies only (srs.fr.3.46) and shall be shown greyed out, with the tooltip "Mass applies to dynamic bodies only", for static and kinematic bodies.
  - Use cases: UC-02; Initial Plan: ip.fr.3.2
- **srs.fr.22.38** The Box Collider component shall have the fields size (width, height), offset (x, y), friction, restitution and is sensor.
  - Use cases: UC-02; Initial Plan: ip.fr.3.2
- **srs.fr.22.39** The Circle Collider component shall have the fields radius, offset (x, y), friction, restitution and is sensor.
  - Use cases: UC-02; Initial Plan: ip.fr.3.2
- **srs.fr.22.40** The Polygon Collider component shall have the fields vertex list, offset (x, y), friction, restitution and is sensor.
  - Use cases: UC-02; Initial Plan: ip.fr.3.8
- **srs.fr.22.41** The Player Input component shall have the fields input map asset, which is empty to use the project's Input Map (srs.fr.5.22), and speed (metres per second).
  - Use cases: UC-02; Initial Plan: ip.fr.5.1
- **srs.fr.22.42** The Inventory component shall have the field items, a list of item IDs.
  - Use cases: UC-02; Initial Plan: ip.fr.6.2
- **srs.fr.22.43** The Collectible component shall have the field item ID.
  - Use cases: UC-02; Initial Plan: ip.fr.6.2
- **srs.fr.22.44** The Unlockable component shall have the fields required item, interaction range, is unlocked, locked sprite and unlocked sprite; the is unlocked value shall act on the entity's colliders as stated in srs.fr.12.8, whatever sets it.
  - Use cases: UC-02; Initial Plan: ip.fr.6.3
- **srs.fr.22.45** The Objective component shall have no fields.
  - Use cases: UC-02; Initial Plan: ip.fr.6.5
- **srs.fr.22.46** The Camera component shall have the fields zoom, follow target and bounds, where bounds is a rectangle of minimum and maximum x and y in world units, or empty for no bounds; the camera entity's Position shall be the authored start point of the view.
  - Use cases: UC-02; Initial Plan: ip.fr.4.4

## US-22.5 Adding components

**Story:** As a game developer, I want to find and add the right component, so that I can extend entities without remembering every type.

*Screens: C-05, F-03, D-05*

- **srs.fr.22.47** When the user clicks Add Component, the Editor shall open the Add Component picker (F-03) with its search box focused.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.48** When the user types in the picker's search box, the picker shall list only the components whose names contain the typed text.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.49** When the user clicks a category in the picker, the picker shall list that category's components; the Back arrow shall return to the category list.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.50** When the user hovers over or highlights a component in the picker, the picker's right pane shall show that component's semantic description.
  - Use cases: UC-02; Initial Plan: ip.fr.10.1
- **srs.fr.22.51** When the user clicks a component or presses Enter on the highlighted one, the Editor shall add it to the entity, close the picker and show the new foldout expanded at the bottom of the Inspector.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.52** The picker shall show components that the entity already has as disabled.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.53** The Component menu shall be enabled only when one or more entities are selected and shall list the categories Core (Position, Velocity), Rendering (Sprite, Camera), Audio (Audio Source), Physics (RigidBody, Box Collider, Circle Collider, Polygon Collider), Input (Player Input) and Gameplay (Inventory, Collectible, Unlockable, Objective); tag and layer are entity properties (srs.fr.2.5), not components.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.54** When the user chooses a component from the Component menu, the Editor shall add it to every selected entity that does not already have it, and log a Console warning for each entity that was skipped.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.55** When the user chooses Component ▸ Search…, the Editor shall open the Add Component picker (F-03).
  - Use cases: UC-02; Initial Plan: ip.fr.2.2

## US-22.6 Field validity rules

**Story:** As a game developer, I want the Inspector to reject values the Engine cannot use, so that invalid physics and camera values never reach a run.

- **srs.fr.22.56** The Inspector shall refuse any numeric value that is not a finite number and shall accept: RigidBody mass from 0.001 to 1 000 000 kg for dynamic bodies, linear damping of 0 or more and any finite gravity scale; Box Collider width and height and Circle Collider radius of 0.001 or more; friction of 0 or more and restitution from 0 to 1; Unlockable interaction range and Player Input speed of 0 or more; Camera zoom from 0.01 to 100; and Camera bounds and Map Bounds only with minimum less than maximum on both axes.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.57** A Polygon Collider shall have 3 to 8 vertices (the Box2D v3.1.1 limit); after converting the vertices to metres (srs.fr.3.14), the Editor and the Physics Adapter shall accept the polygon only when its vertices form a valid convex polygon for Box2D v3.1.1 in which every vertex is kept, so that no vertex is welded to another, removed as collinear or left out as concave, and shall store accepted vertices in counter-clockwise order; the Editor shall refuse a vertex edit that breaks this rule and show the reason under the field, naming the vertex, and the Physics Adapter shall skip such a shape at runtime with a logged error.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.22.58** Project settings shall accept: Sample every N ticks as a whole number from 1 to 10 000, and window width and height as whole numbers from 320 to 7 680 and from 240 to 4 320.
  - Use cases: UC-04; Initial Plan: ip.fr.2.2
