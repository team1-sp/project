---
epic: EP-03
group: srs.fr.3
part: engine
---

# EP-03 Physics (through the Physics Adapter)

**Epic:** As a game developer, I want to simulate 2D bodies and collisions, so that my game objects move and collide realistically.

These requirements refine ip.fr.3.1 to ip.fr.3.8. The physics library is Box2D v3.1.1 and is reached only through the Physics Adapter.

## US-3.1 Bodies and shapes

**Story:** As a game developer, I want to give entities static, dynamic or kinematic bodies with box, circle or polygon colliders, so that each object collides in the way I intend.

- **srs.fr.3.1** The Physics Adapter shall expose these engine-facing operations, which it implements with the Box2D v3.1.1 C API: CreateBody, DestroyBody, SetBodyPosition, SetLinearVelocity, ApplyForce, ApplyImpulse, Raycast, Step and GetCollisionEvents; no engine code outside the Physics Adapter shall call Box2D.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.2** When an entity has a collider component, the Physics Adapter shall create one physics body for it, of the body type set in its RigidBody component (static, kinematic or dynamic), or a static body when the entity has no RigidBody component.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.2
- **srs.fr.3.3** The Engine shall support box collision shapes with a size and an offset.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.2
- **srs.fr.3.4** The Engine shall support circle collision shapes with a radius and an offset.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.2
- **srs.fr.3.5** The Engine shall support convex polygon collision shapes with up to 8 vertices, the maximum that Box2D v3.1.1 polygons allow.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.8
- **srs.fr.3.6** The Physics Adapter shall apply the gravity scale, linear damping and fixed-rotation values of the RigidBody component to the body, and for a dynamic body shall set the mass data as stated in srs.fr.3.46; the mass value shall have no effect on static and kinematic bodies, which do not respond to forces; runtime changes shall be applied as stated in srs.fr.3.57.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.2
- **srs.fr.3.7** The Physics Adapter shall apply the friction, restitution and is-sensor values of the collider component to the shape; runtime changes shall be applied as stated in srs.fr.3.57 and srs.fr.3.25.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.2
- **srs.fr.3.8** When an entity's last collider component is removed or the entity is destroyed, the Physics Adapter shall destroy the matching physics body; when only its RigidBody component is removed, the body shall become static.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.4

## US-3.2 Simulation and synchronization

**Story:** As a game developer, I want to keep the ECS and the physics world in step, so that positions in the editor always match the simulation.

- **srs.fr.3.9** The Engine shall apply the gravity vector from the project's Physics settings to every dynamic body, multiplied by the body's gravity scale.
  - Use cases: UC-06; Initial Plan: ip.fr.3.3
- **srs.fr.3.10** The Physics Adapter shall keep a one-to-one mapping between each ECS entity that has a body and its physics body ID.
  - Use cases: UC-06; Initial Plan: ip.fr.3.4
- **srs.fr.3.11** In the physics synchronization-in phase, the Physics Adapter shall apply every pending change in the single order of srs.fr.3.58, and shall then clear exactly the marks and commands it applied; it shall not copy a Position or Velocity component that has no pending write to Box2D.
  - Use cases: UC-06; Initial Plan: ip.fr.3.4
- **srs.fr.3.12** In the physics synchronization-out phase, the Physics Adapter shall write the world position and rotation of each kinematic and dynamic body to the entity's Position component, converted to the entity's local transform (srs.fr.3.37), without marking the component as written; it shall skip a Position component that has a pending write (srs.fr.3.48).
  - Use cases: UC-06; Initial Plan: ip.fr.3.5
- **srs.fr.3.13** In the physics synchronization-out phase, the Physics Adapter shall write each dynamic body's linear velocity to the entity's Velocity component without marking it as written, and shall skip a Velocity component that has a pending write (srs.fr.3.48); physics shall not change the Velocity component of a kinematic body.
  - Use cases: UC-06; Initial Plan: ip.fr.3.5
- **srs.fr.3.14** The Physics Adapter shall pass positions, sizes and velocities to Box2D in metres and rotations in radians without conversion, because world units are metres (srs.fr.3.38).
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.3 Collisions and raycasts

**Story:** As a game developer, I want to receive collision events and cast rays, so that gameplay rules can react to contacts and line of sight.

- **srs.fr.3.15** The Physics Adapter shall turn on contact events for every shape it creates and a contact CollisionEvent for an entity pair shall be published as stated in srs.fr.3.53, holding the IDs of both entities, lower first, its phase (Begin or End), the tick number and the collider pairs added or removed in that step.
  - Use cases: UC-06; Initial Plan: ip.fr.3.6
- **srs.fr.3.16** The Physics Adapter shall turn on sensor events for every shape it creates and a sensor CollisionEvent for an entity pair shall be published as stated in srs.fr.3.53, marked as a sensor contact and holding the IDs of both entities, lower first, its phase (Begin or End), the tick number and the collider pairs added or removed in that step; a sensor shape shall not push other bodies.
  - Use cases: UC-06; Initial Plan: ip.fr.3.6
- **srs.fr.3.17** The Engine shall provide a raycast operation that takes a start point, a direction, a maximum distance, an optional set of layers (all layers when omitted) and whether sensors are included (not included when omitted), and returns the hit with the smallest distance from the start among the enabled colliders on those layers, with its entity, hit point and distance, or no result when nothing is hit; equal distances shall go to the lower entity ID, a start point inside a collider shall be a hit at distance 0 at the start point, and a zero direction or a maximum distance that is not a finite number above 0 shall return no result and log an error.
  - Use cases: UC-06; Initial Plan: ip.fr.3.7

## US-3.4 Physics ownership and Box2D settings

**Story:** As a game developer, I want a single owner for every body's position and velocity, so that gameplay and physics never overwrite each other.

- **srs.fr.3.18** For a static body, the ECS Position component shall be authoritative; physics shall never move a static body, and the entity's Velocity component shall be ignored.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1, ip.fr.3.2
- **srs.fr.3.19** For a kinematic body, the Velocity component shall be authoritative for velocity, and physics shall be authoritative for the position and rotation that result from it.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1, ip.fr.3.2
- **srs.fr.3.20** For a dynamic body, the position, rotation and linear velocity after each physics step shall be authoritative and shall replace the ECS values in the synchronization-out phase, except a Position or Velocity component with a pending write, which keeps the written value until the next synchronization-in applies it (srs.fr.3.48, srs.fr.3.52).
  - Use cases: UC-06; Initial Plan: ip.fr.3.1, ip.fr.3.2
- **srs.fr.3.21** A system or event handler shall change a body's motion only through the ECS write operations for Position and Velocity, which mark the component as having a pending write, or through ApplyForce and ApplyImpulse commands, which the Physics Adapter queues as pending; when the same component is written several times before it is applied, the last write shall be used, and forces and impulses shall add up in the order they were issued.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1, ip.fr.3.2
- **srs.fr.3.22** When the Position of an entity with a body has a pending write, or its world transform changed because an ancestor moved (srs.fr.3.50), the Physics Adapter shall move a static or kinematic body to the new world transform in the next synchronization-in phase, and a dynamic body only for its own pending write; when its Velocity has a pending write, it shall set that velocity; otherwise the body shall keep the position and velocity Box2D computed, including collision responses.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1, ip.fr.3.2
- **srs.fr.3.23** The Physics Adapter shall advance the Box2D world in each physics step by exactly 1/60 s, with the sub-step count from the project's Physics settings (srs.fr.27.8).
  - Use cases: UC-06; Initial Plan: ip.fr.3.1, ip.fr.3.2
- **srs.fr.3.24** The Physics Adapter shall let two colliders touch, collide or overlap only when the layer collision matrix (srs.fr.2.55) allows the pair of their layers.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1, ip.fr.3.2
- **srs.fr.3.25** When a collider's shape kind, size, offset or is-sensor value changes at runtime, the Physics Adapter shall apply the change before the next physics step: it shall update the existing shape for a size or offset change of the same shape kind, and destroy and recreate the shape when the shape kind or the is-sensor value changes.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1, ip.fr.3.2
- **srs.fr.3.26** The Physics Adapter shall create bodies and shapes in ascending entity ID order, so that runs with the same settings build the same physics world (srs.fr.7.6).
  - Use cases: UC-06; Initial Plan: ip.fr.3.1, ip.fr.3.2

## US-3.5 Motion write timing

**Story:** As a game developer, I want gameplay motion changes to apply at a defined point without wiping out physics results, so that collisions and gameplay both behave correctly.

- **srs.fr.3.27** Writes and commands made in the input application, gameplay input processing or physics synchronization-in phase before the Physics Adapter synchronizes, including those of handlers dispatched at the end of the first two phases, shall take effect in the same tick's physics step; those made later in the tick (in the physics synchronization-out, gameplay rules or telemetry recording phase, or by handlers dispatched at the end of those phases) shall stay pending and take effect in the next tick's physics step.
  - Use cases: UC-06; Initial Plan: ip.fr.3.2, ip.fr.6.6
- **srs.fr.3.28** For a kinematic body, a written Velocity shall stay in effect in later ticks until it is written again.
  - Use cases: UC-06; Initial Plan: ip.fr.3.2, ip.fr.6.6

## US-3.6 Enabled state and component dependencies

**Story:** As a game developer, I want to disabled entities and components to leave physics consistently, so that a disabled or collected entity never keeps colliding.

- **srs.fr.3.29** When an entity is disabled, systems shall skip it from the next query on, and PhysicsSyncIn shall disable its body in the next synchronization-in phase (srs.fr.2.116), so that from the following physics step it does not move, collide or report events; when the entity is enabled again, PhysicsSyncIn shall enable the body in the next synchronization-in phase, keeping its position and velocity.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.30** When a collider component is disabled, the Physics Adapter shall remove its shape from the body until the component is enabled again, and when every collider of a body is disabled, the body shall be disabled.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.31** When a RigidBody component is disabled, the body shall be made static in the next synchronization-in and stay static until the component is enabled again, when the next synchronization-in shall restore its body type and set its mass data again (srs.fr.3.46).
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.32** When the Physics Adapter disables, enables or recreates a body or shape, it shall create no CollisionEvent for that change itself; the raw Box2D begin and end notifications that follow shall only update the touching sets of srs.fr.3.53, so that a public CollisionEvent is published only when an entity pair's set changes between empty and not empty in a step (srs.fr.3.56).
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.33** A recreated shape (srs.fr.3.25) shall be mapped to the same entity and collider component, and after any shape of a dynamic body is added, removed, resized or recreated the Physics Adapter shall set its mass data again as stated in srs.fr.3.46, so that the body keeps the RigidBody mass.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.34** Each component type's semantic description shall list the component types it requires: colliders and RigidBody require Position, RigidBody requires Velocity and at least one collider, Player Input requires Position, Velocity and Inventory, and Collectible, Unlockable and Objective require Position.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.35** When a component is added, the Editor or the Engine shall also add every required component the entity lacks, with default values (a Box Collider when RigidBody needs a collider), and log an Info message that names them.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.36** When a system removes a component that another component on the entity requires, the Engine shall refuse the removal and log an error that names both components.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1

## US-3.7 Transforms and coordinates

**Story:** As a game developer, I want to one clear rule for local and world coordinates, so that parented entities, physics and rendering always agree on where things are.

- **srs.fr.3.37** The Position component shall hold the entity's position, rotation and scale relative to its parent, or relative to the world for a root entity, and the world transform shall be the parent's world transform combined with the local one.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1, ip.fr.4.1
- **srs.fr.3.38** World units shall be metres, with the Y axis pointing up, and rotations shall be stored in radians counter-clockwise; the Inspector shall show and accept rotations in degrees and convert them; only the renderer shall convert to screen space, at a fixed 32 pixels per metre multiplied by the camera zoom, flipping Y and negating angles, and no other part of the Engine shall convert units.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1, ip.fr.4.1
- **srs.fr.3.39** The Physics Adapter shall give Box2D world transforms; a dynamic body shall not follow its parent (srs.fr.3.51), while the static and kinematic bodies of child entities shall be moved to follow their parent's world transform in synchronization-in as stated in srs.fr.3.22.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1, ip.fr.4.1
- **srs.fr.3.40** Collider sizes and a circle's radius shall be multiplied by the absolute value of the entity's world scale (a circle using the larger of the absolute X and Y scale), while collider offsets and polygon vertices shall be multiplied by the signed world scale, so that negative scale mirrors collider geometry together with the sprite; a box shall stay a rotated rectangle without shear, and mirrored polygon vertices shall be passed through the hull rule of srs.fr.22.57, which restores counter-clockwise order.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1, ip.fr.4.1
- **srs.fr.3.41** When an entity is reparented in the Editor or at runtime, its world transform shall be kept as far as srs.fr.3.45 allows and its local Position recomputed.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1, ip.fr.4.1

## US-3.8 Transform limits

**Story:** As a game developer, I want the limits of the transform model to be stated, so that I know when a world shape is only approximated.

- **srs.fr.3.42** The transform model shall be position, rotation and per-axis scale without shear, composed as stated in srs.fr.3.49; world position and rotation shall be exact, and the world shape shall be approximate only in the case of srs.fr.3.43.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.43** An entity shall be in the approximate case when the world scale of its parent has X and Y magnitudes that differ by more than 1 part in 10⁶ and its local rotation is not a multiple of 180°; the Editor shall then show a warning badge on it in the Hierarchy and the Inspector stating that its world shape is approximated.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.44** Scale fields shall reject values whose absolute value is below 0.001; a negative scale shall mirror the entity and, through the product of srs.fr.3.42, all of its descendants along that axis, its sprite shall be drawn flipped, and its collider offsets and polygon vertices shall be mirrored as stated in srs.fr.3.40, while sizes and radii stay positive.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.45** Reparenting shall keep the entity's world position, world rotation and world scale, as defined by srs.fr.3.42, exactly, by computing its local position with the inverse of that composition, its local rotation as its world rotation minus the new parent's world rotation (with the sign rule of srs.fr.3.42), and its local scale as its world scale divided per axis by the new parent's world scale; when that local scale would have an absolute value below 0.001 on an axis (srs.fr.3.44), the Editor shall refuse the reparent with the toast "Can't reparent: local scale would be below 0.001" and a runtime reparent shall be refused with a logged error; when the entity enters or leaves the approximate case, the Editor or Engine shall log a warning that names the entity.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.3.1

## US-3.9 Mass data

**Story:** As a game developer, I want a dynamic body to have exactly the mass I set, so that physics matches the Inspector.

- **srs.fr.3.46** For a dynamic body, the Physics Adapter shall let Box2D compute the mass data from the body's shapes and then override it with the RigidBody mass, the computed centre of mass and the computed rotational inertia multiplied by the ratio of the RigidBody mass to the computed mass; when the shapes give zero mass, the centre shall be the body origin and the rotational inertia that of a solid box around its colliders with the RigidBody mass.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.10 Pending motion writes

**Story:** As a game developer, I want every motion write from any phase to have a defined effect, so that physics never silently loses a write.

- **srs.fr.3.47** In the physics synchronization-in phase, the Physics Adapter shall synchronize after the systems of that phase and after the dispatch of the events they published, and in the physics synchronization-out phase it shall write the results and collect collision events before the systems of that phase run; the Systems panel shall refuse to move a system or group into the physics step phase and show "The physics step runs no systems".
  - Use cases: UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.48** A Position or Velocity write or a force or impulse command made after the synchronization-in of a tick shall stay pending until the next synchronization-in applies it; synchronization-out shall leave a component with a pending write as written, and reading that component shall return the written value.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.11 Transform composition

**Story:** As a game developer, I want the transform rules to be precise enough to test, so that world transforms are the same in the Editor, renderer and physics.

- **srs.fr.3.49** For an entity with local position p, rotation r and scale s under a parent with world position P, rotation Rw and scale Sw: world scale shall be Sw × s per axis; world rotation shall be Rw + r, or Rw − r when exactly one axis of Sw is negative; and world position shall be P + rotate(Rw, Sw × p), where × is per-axis multiplication; a root entity shall use P = (0, 0), Rw = 0 and Sw = (1, 1).
  - Use cases: UC-02; Initial Plan: ip.fr.3.1

## US-3.12 Ancestor moves and body state

**Story:** As a game developer, I want to children to follow their parents in physics without dynamic bodies jumping, so that physics and the ECS agree on where everything is.

- **srs.fr.3.50** When an entity's Position changes, by a write or by synchronization-out, the Engine shall mark the world transforms of all its descendants as changed; this mark shall not be a pending write of the descendants' own Position.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.51** When the world transform of an ancestor of an entity with a dynamic body changes and that entity has no pending Position write, the Engine shall at once recompute its local Position, without marking it written, so that its world position and world rotation stay unchanged while its world scale follows the ancestor: the local scale shall be kept, and the local position and rotation shall be computed from the unchanged world position and rotation with the inverse composition and sign rule of srs.fr.3.45; a resulting change of world scale or mirroring shall resize its shapes as stated in srs.fr.3.60; when it has a pending Position write, the written local value shall be kept and composed with the ancestor's new world transform in the next synchronization-in (srs.fr.3.55).
  - Use cases: UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.52** The body transform and the ECS world transform shall be separate states with the precedence of srs.fr.3.55; rendering and telemetry shall always use the ECS world transform, and the body shall be brought to it only by synchronization-in.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.13 Entity-pair collision events

**Story:** As a game developer, I want to one collision event per entity pair even when entities have several colliders, so that items are never collected twice from one touch.

- **srs.fr.3.53** CollisionEvents shall represent entity pairs: the Physics Adapter shall keep, separately for contact and sensor, the set of touching collider pairs of each entity pair (srs.fr.3.62), and the PhysicsSyncOut system (srs.fr.3.63) shall publish, for each kind, a Begin event when that set is empty before a step's events are processed and not empty after, and an End event when it is not empty before and empty after; pairs that the Adapter itself removes before the step, by destroying, disabling, recreating or refiltering a shape (srs.fr.3.56), shall publish no End event.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.54** Each collider shall be identified in events and telemetry by its entity ID and component type; because an entity has at most one component of each type (srs.fr.2.10), this identifier shall be stable and unique.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.14 Transform states

**Story:** As a game developer, I want the body and ECS transforms to have one clear precedence, so that rendering, telemetry and physics never disagree silently.

- **srs.fr.3.55** The Engine shall keep three states per entity with a body: the ECS Position, with the world transform derived from it and its ancestors; a pending-write mark (srs.fr.3.21); and the body transform. They may differ while a pending write exists or while an ancestor change mark (srs.fr.3.50) is set, including after an ancestor moves later in a tick than synchronization-in; in the next synchronization-in, a pending write shall take precedence over an ancestor change, static and kinematic bodies shall be moved to the ECS world transform and both marks cleared.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.15 Raw physics notifications

**Story:** As a game developer, I want to raw physics notifications and public collision events to be clearly separate, so that recreating a collider never repeats a collision event.

- **srs.fr.3.56** The Physics Adapter shall treat Box2D begin and end notifications as raw input only, applied to the touching sets as stated in srs.fr.3.62; disabling, destroying, recreating or refiltering a shape shall remove its collider pairs when the Adapter makes that change, without an end event of its own, and a recreated shape's new begins shall add pairs again, so that an entity pair still touching through another collider publishes no new CollisionEvent.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.16 Runtime physics properties

**Story:** As a game developer, I want to runtime edits to physics properties to reach the body at a defined moment, so that mass stays what I set after any change.

- **srs.fr.3.57** Runtime changes to body type, mass, gravity scale, linear damping, fixed rotation, friction, restitution and layer (srs.fr.3.61) shall be applied in the next synchronization-in at their steps of srs.fr.3.58: to a disabled body (srs.fr.3.29, srs.fr.3.30), which keeps its body and enabled shapes, they shall be applied as for an enabled one; for a disabled collider, which has no shape (srs.fr.3.30), the latest values shall stay only in its ECS component, no shape update shall be kept pending for it, and its shape shall be created from the values current when it is enabled again; the mass data of srs.fr.3.46 shall be set again after every body type change, shape change (srs.fr.3.33), world scale change (srs.fr.3.60) and re-enable.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.17 Synchronization-in order

**Story:** As a game developer, I want every physics change in a tick to apply in one defined order, so that the result never depends on implementation order.

- **srs.fr.3.58** Synchronization-in shall apply, in this order: (1) destroying the bodies of destroyed entities and disabling the bodies of entities disabled since the previous synchronization-in (srs.fr.2.116); (2) creating the bodies and shapes of new entities in ascending entity ID order; (3) adding, removing, resizing and recreating shapes (srs.fr.3.25), including resizing for a world scale change (srs.fr.3.60); (4) body type changes, including a RigidBody disabled or enabled (srs.fr.3.31); (5) shape friction, restitution and collision filter (srs.fr.3.61); (6) gravity scale, linear damping and fixed rotation; (7) mass data of dynamic bodies (srs.fr.3.46); (8) enabling the bodies of entities and colliders enabled again; (9) pending Position writes and ancestor-following transforms (srs.fr.3.55); (10) pending Velocity writes; (11) force and impulse commands in issue order.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.59** A pending write or command that cannot apply to the body as it is after steps 1 to 8 shall be dropped and cleared with one Info message per entity per run: a Velocity write to a static body, a force or impulse on a static or kinematic body, and a Velocity write, force or impulse on a body that is still disabled; a Position write to a disabled body shall still move it.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.18 Scale and layer at runtime

**Story:** As a game developer, I want to scale and layer edits during a run to reach the physics, so that colliders never keep an old size or layer.

- **srs.fr.3.60** When an entity's world scale changes, by a write to its own scale or to an ancestor's (srs.fr.3.50), the Physics Adapter shall resize every shape of that entity and of its descendants with bodies from the new world scale (srs.fr.3.40) at step 3 of srs.fr.3.58, recreating a shape whose kind must change and passing mirrored polygons through srs.fr.22.57, and set the mass data of their dynamic bodies again at step 7; a dynamic descendant shall keep its world position and rotation as stated in srs.fr.3.51, and only its scale shall follow.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1
- **srs.fr.3.61** When an entity's layer changes during a run, the Physics Adapter shall apply the new layer's collision rules of srs.fr.3.24 to each of its shapes at step 5 of srs.fr.3.58, removing the collider pairs that the new layer does not allow as stated in srs.fr.3.56; pairs it allows shall be added only by Box2D begin events of later steps. The layer collision matrix shall be part of the applied project settings frozen at the start of a run (srs.fr.7.27), so that changing it during a run shall affect only later runs.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.19 Touching sets

**Story:** As a game developer, I want to collision bookkeeping that never drifts, so that a stale Box2D event can never produce a negative count or a duplicate event.

- **srs.fr.3.62** Each touching collider pair shall be keyed by its two collider identifiers (srs.fr.3.54) in sorted order and held at most once: adding a pair already held and removing a pair not held shall change nothing. The Physics Adapter shall associate each Box2D notification with its collider, including a notification for a shape destroyed or recreated during the step being processed, and shall ignore a notification it cannot associate with a collider; it shall process each step's end events before its begin events, each in sorted collider-pair order.
  - Use cases: UC-06; Initial Plan: ip.fr.3.1

## US-3.20 Physics synchronization systems

**Story:** As a game developer, I want every physics read and write to be visible as a system, so that physics data never changes outside a declared system.

- **srs.fr.3.63** Synchronization-in and synchronization-out shall be done by two engine systems, PhysicsSyncIn in tick phase 3 and PhysicsSyncOut in tick phase 5 (srs.fr.1.12), which shall be the only code that calls the Physics Adapter during a tick; they shall declare the component types they read and write and the events they publish like any system (srs.fr.2.16), so that the Semantic Graph shows them; PhysicsSyncIn shall apply srs.fr.3.58, PhysicsSyncOut shall write positions and velocities back as stated in srs.fr.3.48 and publish CollisionEvents; the physics step phase shall only step the physics world by 1/60 s; destroyed and disabled entities shall be handled in the next PhysicsSyncIn (srs.fr.2.116).
  - Use cases: UC-06; Initial Plan: ip.fr.3.1
