---
epic: EP-02
group: srs.fr.2
part: engine
---

# EP-02 Entity Component System (ECS)

**Epic:** As a game developer, I want to store my game objects as entities with components that systems process, so that game data and game logic stay separate and inspectable.

These requirements refine ip.fr.2.1 to ip.fr.2.4. The ECS is custom-built so that every component carries a semantic description that the Editor and the AI layers can read.

## US-2.1 Entities

**Story:** As a game developer, I want to create and destroy entities while the game runs, so that the game can spawn and remove objects.

- **srs.fr.2.1** The Engine shall allow an entity to be created at runtime and shall give it an entity ID that no loaded entity uses, allocated as described in srs.fr.2.63.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.2** The Engine shall not give the ID of a destroyed entity to a different entity during the same run or editing session; restoring the same entity by Undo, Revert or the Play mode snapshot (srs.fr.2.105) shall not count as reuse.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.3** The Engine shall allow a system or event handler to request that an entity be destroyed at runtime; the request shall take effect as stated in srs.fr.2.116, and the entity's components and physics body shall then be removed.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.4** When an entity is destroyed, the Engine shall also destroy all of its child entities.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.5** Each entity shall have a name, an enabled flag, a tag, a layer and an optional parent entity.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-2.2 Components

**Story:** As a game developer, I want to add, remove, read and change components on an entity, so that I can change what an entity is and how it behaves.

- **srs.fr.2.6** The Engine shall allow a component to be added to an entity at runtime.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.2.7** The Engine shall allow a component to be removed from an entity at runtime.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.2.8** The Engine shall allow the fields of a component to be read at runtime.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.2.9** The Engine shall allow the fields of a component to be modified at runtime.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.2.10** The Engine shall allow at most one component of each component type on an entity.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.2.11** Each component, except Position and Velocity, which have no enabled flag of their own, shall have an enabled flag; systems shall skip a component whose enabled flag is off.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.2.12** Each component type shall carry a semantic description that states its meaning, the name, type, unit and default value of each field, and the systems that read and write it.
  - Use cases: UC-02; Initial Plan: ip.fr.10.1

## US-2.3 Systems and queries

**Story:** As a game developer, I want to register systems and query entities by component, so that game logic runs in a known order on the right entities.

- **srs.fr.2.13** The Engine shall allow systems to be registered in the World.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.2.14** The Engine shall execute every enabled registered system exactly once per simulation tick, in the tick phase of its group and in the order computed by srs.fr.2.50.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.2.15** The Engine shall skip a disabled system and shall skip every entity whose enabled flag is off, including its Position and Velocity components; the Physics Adapter's own bookkeeping for disabling and enabling bodies (srs.fr.3.29) shall not count as system processing.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.2.16** Each system shall declare the component types it reads, the component types it writes, the event types it publishes and subscribes to, and whether it creates entities, destroys entities, changes enabled flags and uses the gameplay random number generator (srs.fr.7.5); every one of these declarations shall be checked as stated in srs.fr.2.126 and exported as stated in srs.fr.8.29.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3, ip.fr.10.2
- **srs.fr.2.17** The Engine shall measure the execution time of each system in every simulation tick, in milliseconds.
  - Use cases: UC-04; Initial Plan: ip.fr.8.3
- **srs.fr.2.18** The Engine shall provide a query that returns every enabled entity that has all of a specified set of component types.
  - Use cases: UC-04; Initial Plan: ip.fr.2.4

## US-2.4 Scene persistence

**Story:** As a game developer, I want to save a scene to a file and load it back, so that my level design is kept between sessions.

- **srs.fr.2.19** The Engine shall save a scene, with all of its entities, their components, field values and parent-child relations, to a `.scene` file in a documented, text-based format.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.20** When a `.scene` file is loaded, the Engine shall recreate every saved entity with its saved ID, components, field values and parent-child relations, except where srs.fr.2.64 or srs.fr.2.69 applies.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.21** When a `.scene` file names a component type that is not registered, the Engine shall load the rest of the scene and keep that component's serialized data as the placeholder of srs.fr.2.101, which systems never execute, and log a warning that names the entity and the type; saving the scene shall write the placeholder's data back unchanged.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1, ip.fr.2.2

## US-2.5 Data assets

**Story:** As a game developer, I want to shared data files that many entities refer to, so that I change a value once instead of on every entity.

- **srs.fr.2.22** The Engine shall support data asset types: project-defined types whose instances are stored as asset files and hold field values that do not belong to any entity (for example enemy stats or item definitions).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.2.23** Each data asset type shall carry a semantic description in the same form as a component type: its meaning and the name, type, unit and default value of each field.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.2.24** A component field shall be able to refer to a data asset by its asset ID instead of holding a copy of the values.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.2.25** When a system reads a component field that refers to a data asset, the Engine shall return the data asset's current values.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.2.26** When a data asset's in-memory values change, by an Inspector edit in Edit mode or Play mode, every entity that refers to it shall use the new values from the next simulation tick, without the entities themselves being changed; saving shall write the values to disk and shall not change them.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.2.27** When a referenced data asset file is missing, the Engine shall use the data asset type's default values and log a warning that names the entity and the missing asset.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2
- **srs.fr.2.28** Changes made to a data asset during Play mode shall reach the running game as stated in srs.fr.2.26 and shall be discarded when Play mode stops, restoring the asset's in-memory values and unsaved-changes state from the snapshot (srs.fr.17.24).
  - Use cases: UC-03; Initial Plan: ip.fr.2.2

## US-2.6 Prefabs

**Story:** As a game developer, I want to prefabs whose instances stay linked to the original, so that I can update many copies of an object in one place.

- **srs.fr.2.29** The Engine shall store a prefab as an asset that holds one root entity, its child entities, their components and field values.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.30** Every entity created from a prefab shall keep a link to the prefab asset and to the matching entity inside the prefab.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.31** When a prefab asset is changed and saved, the Engine shall apply the change at once to every linked instance in the loaded scenes, except for fields that the instance overrides; instances in scenes that are not loaded shall receive the change when their scene is next loaded (srs.fr.2.70).
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.32** The Engine shall store, for each prefab instance, the fields whose values differ from the prefab (overrides), the components added to or removed from the instance's entities, and the child entities added to the instance, each with its parent and sibling position.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.33** The Engine shall allow a prefab to contain instances of other prefabs (nested prefabs), and changes to an inner prefab shall reach every outer prefab and instance that uses it.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.34** The Engine shall refuse to save a prefab or variant when the save would create a prefab cycle (srs.fr.2.120), and shall report the cycle.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.35** When an instance's prefab asset is missing, the Engine shall load the instance from its last-saved snapshot (srs.fr.2.91) when it has one, keep its prefab link so that it reconnects when the asset returns, log a warning that names the snapshot's save time, and the Editor shall show the badge "Restored from snapshot saved <date>" on the instance in the Hierarchy and Inspector; an instance with no usable snapshot shall be built as stated in srs.fr.2.125.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2

## US-2.7 Custom component and system types

**Story:** As a game developer, I want to register my own component and system types from project scripts, so that the engine grows with my game without changing engine code.

- **srs.fr.2.36** The Engine shall allow project scripts to register new component types through a registration interface, with no change to engine code.
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.2, ip.fr.2.3
- **srs.fr.2.37** The Engine shall allow project scripts to register new system types, each declaring the component types it reads and writes.
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.2, ip.fr.2.3
- **srs.fr.2.38** The Engine shall refuse to register a component, system or data asset type that has no semantic description, and shall report which type is missing it.
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.2, ip.fr.2.3
- **srs.fr.2.39** The Engine shall refuse to register two types with the same name and shall report the conflict.
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.2, ip.fr.2.3
- **srs.fr.2.40** After the project scripts are rebuilt, the Editor shall reload the new component and system types without restarting, while keeping the open scene and its field values; a reload shall never happen during Play mode, a headless run or a Playtest Agent session (srs.fr.2.100).
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.2, ip.fr.2.3
- **srs.fr.2.41** When the fields of a component type or a data asset type change after a rebuild, the Engine shall, for every component and every data asset of that type, keep the value of a field only when a field with the same name and the same type still exists, give new fields and fields whose type changed their default values, and log a warning for each removed or retyped field that held a value, naming the component or asset and the old value; a loaded data asset whose values changed this way shall be marked as unsaved, so that its file keeps the old values until it is saved.
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.2, ip.fr.2.3

## US-2.8 Entity references and events

**Story:** As a game developer, I want entities to refer to each other safely and systems to communicate through events, so that game logic stays decoupled and does not break when entities are deleted.

- **srs.fr.2.42** A component field shall be able to refer to another entity by its entity ID.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.2.1, ip.fr.8.2
- **srs.fr.2.43** Entity references saved in a scene shall keep pointing at the same entities after the scene is saved and loaded; when entities are copied, pasted or duplicated together, references between them shall point to the copies (srs.fr.2.66).
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.2.1, ip.fr.8.2
- **srs.fr.2.44** When a referenced entity is destroyed or its scene is unloaded at runtime, the Engine shall set every runtime reference to it to empty; in Edit mode, an entity reference field shall be in one of the states of srs.fr.36.75, and a field whose target no longer exists shall be shown as "Missing (was ID n)".
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.2.1, ip.fr.8.2
- **srs.fr.2.45** The Engine shall provide an event bus on which systems publish typed events and subscribe to the event types they handle.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.2.1, ip.fr.8.2
- **srs.fr.2.46** The Engine shall give each published event a sequence number in publication order and the tick number of its publication, queue it, and dispatch the queue at the end of the tick phase in which the event was published, delivering events in sequence-number order and each event to its subscribers in system run order, and to several callbacks of one system in the order they subscribed (srs.fr.2.113).
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.2.1, ip.fr.8.2
- **srs.fr.2.47** Each event type shall carry a semantic description, and every published event shall hold the tick number in which it was published, so that telemetry and the Semantic Graph can record it.
  - Use cases: UC-02, UC-06; Initial Plan: ip.fr.2.1, ip.fr.8.2

## US-2.9 System groups and ordering

**Story:** As a game developer, I want to group systems and declare ordering rules, so that large projects keep a correct and understandable run order.

- **srs.fr.2.48** The Engine shall allow systems to be placed in named system groups, and each group shall run inside one of the tick phases of srs.fr.1.12 except the physics step, which shall hold no systems (srs.fr.3.47).
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.2.49** The Engine shall allow a system to declare that it runs before or after another system or group.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.2.50** The Engine shall compute the run order from the tick phases, the groups and the before/after rules, and keep the user's manual order where the rules allow it.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.2.51** When the before/after rules form a cycle, or a rule would make a system run before a system of an earlier tick phase or after a system of a later tick phase (srs.fr.1.12), the Engine shall refuse the order, keep the last valid order and report the systems in the cycle, or the rule and the two tick phases it conflicts with.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.2.52** The Engine shall allow a whole system group to be enabled or disabled.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3

## US-2.10 Tags and layers

**Story:** As a game developer, I want to manage project-wide tags and layers and say which layers collide, so that large scenes stay organised and physics only checks the pairs I need.

- **srs.fr.2.53** The Engine shall keep the project's tags in one list; an entity's tag shall be one value from that list.
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.1, ip.fr.3.6
- **srs.fr.2.54** The Engine shall support up to 32 named layers per project.
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.1, ip.fr.3.6
- **srs.fr.2.55** The Engine shall keep a layer collision matrix that states, for each pair of layers, whether their entities collide.
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.1, ip.fr.3.6
- **srs.fr.2.56** The Engine's query shall be able to filter entities by tag and by layer.
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.1, ip.fr.3.6
- **srs.fr.2.57** When a tag or layer that entities still use is deleted, the Engine shall set those entities to the default tag (Untagged) or the default layer (Default, srs.fr.2.80).
  - Use cases: UC-02, UC-04; Initial Plan: ip.fr.2.1, ip.fr.3.6

## US-2.11 Multiple scenes

**Story:** As a game developer, I want to load scenes on top of each other and keep chosen entities across scene changes, so that large games can be split into smaller scenes.

- **srs.fr.2.58** The Engine shall allow a scene to be loaded additively, so that its entities are added to the scenes already loaded; ID collisions shall be resolved by srs.fr.2.64.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.59** The Engine shall allow an additively loaded scene to be unloaded, which destroys only the entities that came from it, except persistent roots and their descendants (srs.fr.2.68).
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.60** The Engine shall allow a root entity to be marked as persistent, so that it and all of its descendants are kept when the active scene is replaced (srs.fr.2.68, srs.fr.2.69).
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.61** The Engine shall allow a system to request a scene change, which takes effect at the start of the next simulation tick, before its input application phase; when several scene changes are requested in one tick, the last request in request order shall replace the earlier ones, and the Engine shall log a warning naming the replaced requests.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-2.12 Entity identity

**Story:** As a game developer, I want every entity to keep one stable ID across saving, loading and additive scenes, so that references, telemetry and bug reports always name the right entity.

- **srs.fr.2.62** The Editor shall give each entity it creates an ID one higher than the project's allocation watermark, which shall be stored in the project's metadata and durably increased before the ID is issued, including for unsaved content that is later discarded; when the project opens, the watermark shall be at least the highest entity ID recorded in any scene header or retained run's authored-entity records, and shall never decrease. Each scene file shall also store the highest entity ID it has ever used. Editor IDs shall stay below the runtime range of srs.fr.2.104; when no Editor ID remains or the updated watermark cannot be saved, creation shall fail without issuing an ID and the Editor shall show the reason.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.63** At runtime, the Engine shall allocate the IDs of entities created by systems, prefab instantiation or ID collisions in increasing order within the runtime range (srs.fr.2.104), starting at its first value in every run, so that runs with the same content snapshot and settings give the same IDs whatever the other scene files of the project hold.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.64** When a scene is loaded and one of its entities has an ID that a loaded entity already uses, the Engine shall treat them as different entities, unless srs.fr.2.69 identifies them as the same persistent entity: it shall give the incoming entity a new ID, by srs.fr.2.62 in Edit mode and by srs.fr.2.63 at runtime, update every reference in the incoming scene that pointed to the old ID, and log a warning that names the scene and the old and new IDs; in Edit mode the Editor shall also mark that scene as changed so that the new ID is saved.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.65** Entity reference fields in a saved scene shall refer only to entities of the same scene; at runtime a system may set a reference to any loaded entity, and runtime references shall never be saved.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.66** When entities are copied, pasted, duplicated or instantiated from a prefab, the Engine shall give each new entity a new ID and point references between the copied entities to the new copies; a reference to an entity outside the copied set shall be kept when that entity is in the same scene as the copy, and cleared otherwise; references stored inside prefab assets shall follow srs.fr.2.123.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.67** Each entity inside a prefab asset shall have a prefab-local ID made of the asset ID of the prefab or variant that first added the entity and a number that this asset allocates from its allocation mark (srs.fr.2.119); the ID shall never be changed or given to a different entity, also when the entity is moved inside the prefab, and each instance in a scene shall store a map from the prefab-local ID path of each prefab entity (the IDs from the instance root through any nested instances) to the scene entity ID it was given.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-2.13 Persistent entities

**Story:** As a game developer, I want the Engine to handle persistent entities predictably when scenes change, so that a persistent player is never lost or duplicated.

- **srs.fr.2.68** Only a root entity shall be markable as persistent, and every descendant of a persistent root shall be kept and destroyed together with it; when a persistent entity becomes the child of another entity, the Engine shall clear its persistent flag and log a warning that names it.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.69** When the active scene is replaced, the Engine shall destroy every entity of the replaced scenes except persistent roots and their descendants, set every reference from a kept entity to a destroyed entity to empty, and load the new scene; an incoming entity shall be the same entity as a kept persistent entity only when both its ID and its source scene (srs.fr.2.83) match, in which case its saved copy and descendants shall not be loaded and references to them in the new scene shall point to the kept entity; every other ID match shall be handled by srs.fr.2.64.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-2.14 Prefab propagation

**Story:** As a game developer, I want every prefab change to reach every instance without rewriting scene files, so that a prefab edit is safe and fast even in a large project.

- **srs.fr.2.70** When a scene is loaded, the Engine shall build each prefab instance from the current prefab asset, give each prefab entity the scene ID stored in the instance's map, and then apply the instance's stored overrides, so that no scene file is rewritten to pass on a prefab change.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.71** When an override refers to a field, component or entity that the prefab no longer has, the Engine shall ignore the override, log a warning that names the instance and the field, component or entity, and drop the override the next time the scene or variant is saved; an added component or added entity is not such an override (srs.fr.2.32, srs.fr.2.112).
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.72** When a prefab change removes an entity from the prefab, the Engine shall remove the matching entity and its map entry from every instance, move any entity the user added under it in an instance to the nearest remaining ancestor in that instance, and make every Edit-mode reference to the removed entity Missing with its old ID as a tombstone (srs.fr.36.75), marking the scene as changed (srs.fr.2.90); runtime references, which are never saved, shall become empty (srs.fr.2.44); undoing the prefab change shall make those references Set again (srs.fr.16.23).
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.73** When a prefab change adds an entity, each instance whose map has no entry for it shall give it an ID: in Edit mode a new ID by srs.fr.2.62, with the scene marked as changed so that the map entry is saved; at runtime an ID by srs.fr.2.63, allocated in prefab-local ID order so that repeated runs of the same files give the same IDs.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.74** Prefab assets shall not change during Play mode (srs.fr.36.69); a headless run shall use the prefabs of its content snapshot (srs.fr.7.25), and instances created at runtime shall use the prefab as it was when the run started.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1

## US-2.15 Event delivery

**Story:** As a game developer, I want an exact rule for when events are delivered, so that systems that react to events behave the same in every run.

- **srs.fr.2.75** Events dispatched at the end of a tick phase shall be handled before the next tick phase starts, so that systems in later phases of the same tick see the changes the handlers made.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.2.3
- **srs.fr.2.76** An event published by a handler during a dispatch shall be queued for the next dispatch point and keep its publication tick; when that dispatch is the one at the end of the last tick phase, the next dispatch point shall be the end of the first tick phase of the next tick.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.2.3
- **srs.fr.2.77** A subscriber shall receive only events published after it subscribed.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.2.3
- **srs.fr.2.78** The Engine shall dispatch at most 10 000 events per tick, counting events deferred from the previous tick first; publication itself shall not be limited, and once the limit is reached every event that reaches a dispatch point later in that tick, including events published after the limit was reached, shall be marked dropped with the reason "limit", not delivered and not carried to the next tick; an event deferred into the next tick by srs.fr.2.76 shall count towards the next tick's limit, and the Engine shall log one error per tick that names the event type with the most dropped events.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.2.3
- **srs.fr.2.79** Telemetry shall record each published event once, at publication, with its sequence number, type, entities, payload (srs.fr.2.82) and publication tick, and shall add its final status: dispatched, with the tick of dispatch, or dropped, with the reason "limit" (srs.fr.2.78) or "run ended" (srs.fr.2.99).
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.2.3

## US-2.16 Default tags

**Story:** As a game developer, I want a fixed set of built-in tags, so that bug checks that rely on tags work in every project.

- **srs.fr.2.80** A new project's tag list shall contain Untagged, Player and Obstacle, and its layer list shall contain the layer Default as layer 0; Untagged shall be the default tag and Default the default layer; Untagged, Obstacle and Default shall not be deletable, and Default shall not be renamable, because srs.fr.2.57 and srs.fr.9.18 rely on them.
  - Use cases: UC-04; Initial Plan: ip.fr.2.1

## US-2.17 Event lifecycle and payloads

**Story:** As a game developer, I want each event to go through one defined lifecycle and carry the data that explains it, so that telemetry and the assistant can tell what happened and why.

- **srs.fr.2.81** Each published event shall go through these states only: published, queued, and then either dispatched or dropped (by the limit of srs.fr.2.78 or at the end of the run); the Engine shall remove an event from the queue before delivering it, so that no event is delivered twice to the same subscriber.
  - Use cases: UC-06, UC-10; Initial Plan: ip.fr.2.3, ip.fr.8.2
- **srs.fr.2.82** Each event type's semantic description shall declare its payload fields; CollisionEvent shall hold the fields of srs.fr.3.15 and srs.fr.3.16, ItemCollectedEvent shall hold the collector, the Collectible entity and the item ID, and UnlockedEvent and UnlockFailedEvent shall hold the player, the Unlockable entity, the required item and the contents of the player's Inventory, both captured by the gameplay system from the same state it uses to decide the attempt.
  - Use cases: UC-06, UC-10; Initial Plan: ip.fr.2.3, ip.fr.8.2

## US-2.18 Scene identity and copies

**Story:** As a game developer, I want to copies of scenes and entities never to be mistaken for the originals, so that loading two scenes never loses or merges content.

- **srs.fr.2.83** Each entity shall record its source scene: the scene asset it was loaded from or created in; an entity created at runtime shall record the active scene.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.84** When a scene is saved with Save As… or duplicated in the Project panel, the Editor shall give every entity in the new file a new ID by srs.fr.2.62 and remap the references inside it, so that the copy and the original can be loaded together without collisions.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.85** When the Editor finds two asset files with the same asset ID, such as a file copied outside the Editor, it shall keep the ID for the file with the earlier modification time, or, when the times are equal, for the file whose project-relative path comes first in ordinal order, give the other file a new asset ID and log a warning naming both, so that the source-scene check of srs.fr.2.69 cannot confuse them; when the renamed file is a prefab or variant, the Editor shall also remap the prefab-local IDs it owns, its internal references and its override records to the new ID as stated in srs.fr.2.111, so that it becomes an independent copy, and instances that use the old ID shall keep pointing to the file that kept it.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1
- **srs.fr.2.86** Each scene shall have an optional Map Bounds rectangle, given as minimum and maximum x and y in world units and stored in the scene file.
  - Use cases: UC-02; Initial Plan: ip.fr.2.1

## US-2.19 Prefab identity and structure

**Story:** As a game developer, I want to prefab entities to keep their identity through every change, so that overrides and references in my scenes survive prefab edits.

- **srs.fr.2.87** When a scene whose instances received new map entries (srs.fr.2.73) is closed without saving, those entries shall be discarded and the next load shall allocate new IDs.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.88** Moving an entity inside a prefab shall keep its prefab-local ID and its prefab-local ID path (srs.fr.2.110), so that instance overrides of it and scene references to it are kept; an entity that comes from a nested instance shall not be moved out of that instance, as in srs.fr.2.92.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.89** Prefab values shall resolve recursively: for an asset, start from the values of the root of its base chain and apply each variant's overrides from that root down to the asset; for each nested instance inside it, resolve the nested asset the same way and then apply the containing asset's overrides of that nested instance; finally apply the scene instance's overrides, a later step winning over an earlier one for the same field; a change to any asset in this structure shall reach every affected instance when it is next built, without rewriting other asset files.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.90** Saving a prefab shall update instances in the loaded scenes without marking those scenes as changed when only derived prefab values change; a scene shall be marked as changed whenever the update changes data the scene file stores: a map entry added or removed, an override dropped, an added entity moved to another parent (srs.fr.2.72) or a reference made Missing.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.91** When a scene is saved, each prefab instance shall also store a snapshot of its resolved entities, components and field values with the time of the save; the snapshot is the last-saved state and may be older than the values the Editor last showed.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.92** In a prefab instance, the Editor shall refuse to delete, reparent or reorder an entity that is part of the instance's inherited structure (every entity that comes from the prefab except the instance root, including the roots of nested instances inside it) and show the toast "Unpack the prefab to change its structure"; the instance root may be deleted, duplicated, reparented and reordered as a whole, together with the whole instance, and entities added to the instance may be deleted, moved and reordered; in Prefab Mode, a nested instance's root is the root of a whole instance inside the open prefab and may be handled as a whole.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.93** Apply to Prefab on an added child entity shall add it, with its children, to the chosen asset with new prefab-local IDs owned by that asset (srs.fr.2.67) as stated in srs.fr.2.109, and Revert on it shall delete it from the instance.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1

## US-2.20 Prefab variants

**Story:** As a game developer, I want to variants of a prefab that inherit from a base, so that I can make many similar prefabs and change their shared parts in one place.

- **srs.fr.2.94** A prefab variant shall be a prefab asset that links to a base prefab and stores its differences from the base (overridden fields, added or removed components and added entities, in the same form as instance overrides, srs.fr.2.32) and, as the only exception to storing differences only, the fallback snapshot of srs.fr.2.107.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.95** Entities a variant inherits from its base shall keep their prefab-local IDs, and entities added in the variant shall get IDs owned by the variant asset (srs.fr.2.67), so that later additions to a base can never take an ID used in a variant.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.96** When a base prefab changes, every variant of it and every instance of those variants shall receive the change, except for fields that the variant or the instance overrides and for the structural overrides of srs.fr.2.112.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.97** A variant may be the base of another variant; a base chain that contains the variant itself is a prefab cycle and shall be refused and reported as stated in srs.fr.2.120.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.98** When a variant's base is missing, the Engine shall build the variant from its fallback snapshot (srs.fr.2.107), keep the base link, log a warning that names the snapshot's save time, and the Editor shall show the badge "Restored from snapshot saved <date>" on the variant and its instances; when no usable snapshot exists, it shall fall back as stated in srs.fr.2.108.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1

## US-2.21 Event finalization

**Story:** As a game developer, I want every event to end with a final status, even when the run stops, so that telemetry never has events in an unknown state.

- **srs.fr.2.99** When a run ends for any reason (Max ticks, Objective reached, Failure, Stopped, Error or the game window closing), the Engine shall not dispatch the events still queued, shall give each of them the final status dropped with the reason "run ended", and shall write these statuses to telemetry before the recording is closed.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.2.3

## US-2.22 Script reload during runs

**Story:** As a game developer, I want to script rebuilds never to change a running game, so that runs stay deterministic and snapshots stay valid.

- **srs.fr.2.100** When the project scripts build successfully during Play mode, a headless run or a Playtest Agent session, the Editor shall keep the loaded types until the run ends and then reload them, after the Play mode snapshot is restored; when the build fails during a run, the Editor shall keep the loaded types, show the errors in the Console and disable Play from the end of the run until they are fixed.
  - Use cases: UC-02, UC-06, UC-07; Initial Plan: —
- **srs.fr.2.101** When a component type is not registered, because a reload removed it or a loaded scene or prefab names it, every entity that has it shall keep the component's serialized data as a placeholder shown in the Inspector as "Missing component type <Name>", which systems ignore, which survives saving, closing and reopening the project, and which is restored to a real component when the type is registered again, until the user removes the placeholder.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.2.102** When a reload removes a system type, the Systems panel shall drop it, and ordering rules that name it shall be ignored with a Console warning that names each rule.
  - Use cases: UC-02, UC-03, UC-04; Initial Plan: —
- **srs.fr.2.103** A renamed type or field shall be handled as a removed one and a new one; the Engine shall not match types or fields by anything other than their name and, for fields, their type.
  - Use cases: UC-02; Initial Plan: —

## US-2.23 Runtime ID range and restoration

**Story:** As a game developer, I want runtime IDs to be independent of unrelated scenes and Editor IDs never to be reused across sessions, so that runs are reproducible and historical recordings never identify a different entity.

- **srs.fr.2.104** Entity IDs shall be 64-bit; IDs from 1 to 2³² − 1 shall be reserved for the Editor (srs.fr.2.62) and IDs from 2³² up for runtime allocation (srs.fr.2.63), and the Editor shall refuse to save a scene with an entity ID in the runtime range.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.2.1
- **srs.fr.2.105** Undo of a deletion, Revert and restoring the Play mode snapshot shall bring back the same entity with its own ID, references and source scene; this restores an identity and is not an allocation, so they shall not lower the Editor's highest-ID value or any prefab's allocation mark, and IDs allocated in the runtime range during Play mode shall be discarded with the snapshot restore.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.2.1
- **srs.fr.2.106** An Editor ID allocated for content that is later discarded shall remain reserved in the project's persisted allocation watermark and shall never be given to a different entity, including after the project is reopened; restoring the same entity by Undo, Revert, recovery or the Play mode snapshot shall preserve its ID as stated in srs.fr.2.105.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.2.1

## US-2.24 Variant snapshots and prefab identity rules

**Story:** As a game developer, I want to prefab structure changes to keep every entity's identity, so that applying, duplicating and moving never duplicate or lose entities.

- **srs.fr.2.107** Every save of a variant shall store in it a fallback snapshot of its fully resolved entities, components and field values, base chain applied, with the time of the save; the snapshot shall be refreshed only by saving the variant, and it shall not count as overrides.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.2.108** When a variant's base is missing and the variant has no usable snapshot, the Engine shall build its instances from their scene snapshots (srs.fr.2.91) when they have one, and otherwise as stated in srs.fr.2.125.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.2.109** When an added entity is applied to an asset, its scene entities shall become prefab entities of that asset: each shall get a new prefab-local ID owned by the asset, the originating instance's map shall get an entry from that ID's path to the entity's existing scene ID, so that its scene ID and references are kept, its added-entity record and the field overrides now held by the asset shall be removed, and other instances shall receive it by srs.fr.2.73; the originating instance shall not receive a second copy.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.110** An entity's prefab-local ID path shall be determined by the asset that owns it and the chain of nested instances through which that asset is reached, not by the entity's parent, so that a move that changes only the parent never changes the path.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.111** Duplicating a prefab or variant shall replace the owning asset ID in the prefab-local IDs of the entities owned by the original with the new asset's ID, keep their numbers and the asset's counter, remap the references and override records inside the copy in the same way, keep the IDs of entities owned by other assets (a base or a nested prefab) unchanged, and leave the original asset and its instances unchanged.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1
- **srs.fr.2.112** Structural overrides shall resolve in the order of srs.fr.2.89, like field overrides: a component removed by a variant or instance shall stay removed when the base changes that component's fields; a component added by a variant or instance shall be kept, and when the base later adds a component of the same type, the added component's values shall become field overrides of the base's component, with a warning that names it; an added entity whose parent the base removes shall move to the nearest remaining ancestor (srs.fr.2.72); and a removal that names a component or entity the base no longer has shall be obsolete and dropped as stated in srs.fr.2.71.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2

## US-2.25 Dispatch rules

**Story:** As a game developer, I want to event delivery to be exact when systems or subscriptions change, so that every event reaches the same handlers in every run.

- **srs.fr.2.113** A disabled system, or a system in a disabled group, shall not receive events; an event whose subscribers are all disabled shall still be marked dispatched.
  - Use cases: UC-06; Initial Plan: ip.fr.2.3
- **srs.fr.2.114** A subscription or unsubscription made during a dispatch shall take effect after that dispatch point, so that the set of subscribers is fixed for each dispatch point.
  - Use cases: UC-06; Initial Plan: ip.fr.2.3
- **srs.fr.2.115** An event shall be delivered even when an entity it names is no longer available; an entity whose destruction was only requested in the current tick phase shall still be found by every handler of that phase's dispatch (srs.fr.2.116), while an entity whose destruction was completed at an earlier phase boundary shall be found by no query; the Physics Adapter shall publish no CollisionEvent for a body whose entity was destroyed before collision collection (srs.fr.3.58).
  - Use cases: UC-06; Initial Plan: ip.fr.2.3

## US-2.26 Destruction and disable timing

**Story:** As a game developer, I want to to know exactly when a destroyed or disabled entity stops taking part, so that no stale entity is processed or collides.

- **srs.fr.2.116** A destruction request shall leave the entity available to queries and handlers until the end of the tick phase in which it was made, after that phase's event dispatch; at that moment the entity shall be destroyed and removed from every query and lookup. A disable shall set the entity's enabled flag at once and take effect for queries from the next query. In both cases the body shall be handled in the next synchronization-in (srs.fr.3.58): a destroyed entity's body and shapes are destroyed and a disabled entity's body is disabled, so that it takes part in no later physics step; until then no other code shall call the Physics Adapter for it (srs.fr.3.63).
  - Use cases: UC-06; Initial Plan: ip.fr.2.3
- **srs.fr.2.117** Destruction and disable requests made by handlers in the physics synchronization-out phase shall take effect at the end of that phase, after its dispatch, so that every CollisionEvent of that step, including several events that name the same entity, is delivered while the entity is still found (srs.fr.2.115); a disable shall nevertheless set the entity's enabled flag at once, so that later handlers that check it see the entity as disabled.
  - Use cases: UC-06; Initial Plan: ip.fr.2.3

## US-2.27 Type cache

**Story:** As a game developer, I want to assets of missing types to stay viewable, so that their values are not lost while the type is gone.

- **srs.fr.2.118** The Editor shall keep, in the project's Library folder, a type cache that holds the last registered semantic description of every component and data asset type, with its fields and defaults, so that placeholders and assets of types that are not registered can be shown read-only and restored when the type returns.
  - Use cases: UC-02, UC-03; Initial Plan: —

## US-2.28 Prefab ID allocation mark

**Story:** As a game developer, I want prefab-local IDs never to be given to a different entity, even after undo, discarded edits, crashes or reopening, so that references and overrides never point at the wrong entity.

- **srs.fr.2.119** Each prefab or variant asset shall have an allocation mark, the highest prefab-local number it has ever given out; before issuing a new number, the Editor shall durably reserve the previous mark plus 1 in project metadata keyed by that asset's ID, independently of its editable content. The mark shall only increase and shall survive Undo, Redo, Revert, discarded edits, Replace Prefab, crashes and reopening the project; restoring the same entity shall preserve its existing number. Each saved asset shall also store the mark, and loading or duplicating it shall use at least the higher of its stored mark and the persisted reservation for its asset ID; a duplicate with a new asset ID shall initialize its own reservation from the copied mark before issuing any number. If a reservation cannot be saved, allocation shall fail without issuing the number and show the reason; saving, recovery or discarding asset content shall never lower or remove the reservation.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1

## US-2.29 Prefab cycles

**Story:** As a game developer, I want to prefab links that can never loop, so that a broken or hand-edited prefab file cannot hang the Editor.

- **srs.fr.2.120** The Engine shall keep one dependency graph of all prefab assets whose edges are base links (variant to base) and nesting links (asset to each prefab it directly contains an instance of); a cycle in this graph, made of any mix of the two kinds, shall be a prefab cycle. The Editor shall check the graph before Create Variant, placing an instance in Prefab Mode, Replace Prefab, Apply of an added entity that contains an instance, and saving a prefab or variant, and refuse an operation that would create a cycle with the toast "A prefab cannot contain itself" naming the assets of the cycle.
  - Use cases: UC-03; Initial Plan: —
- **srs.fr.2.121** When the Engine finds a prefab cycle while loading prefab files, including files changed outside the Editor, it shall resolve each asset at most once on a resolution path (srs.fr.2.89): at a base or nesting link that leads to an asset already on the path, it shall stop and build that base or nested instance as it builds a missing one (srs.fr.2.98, srs.fr.2.108), log one error per load naming the cycle, and show the badge "Prefab cycle" on the assets and instances affected; the assets shall stay open for editing so that the user can remove the link that closes the cycle, and saving one of them shall stay refused until it is removed (srs.fr.2.34).
  - Use cases: UC-03; Initial Plan: —

## US-2.30 Variant eligibility

**Story:** As a game developer, I want to Create Variant only when the result is a real variant, so that a variant never misreads an unrelated hierarchy as differences.

- **srs.fr.2.122** Create Variant in F-19 shall be enabled only when the dragged entity is the root of an instance of the target prefab and every difference of that instance is one a variant can store (srs.fr.2.94): field overrides, added or removed components and added entities, with identities taken from the instance's map (srs.fr.2.67); otherwise it shall be disabled and F-19 shall show the reason under it: "Create Variant needs an instance of this prefab" for any other entity, or "Can't create a variant: <entity> was removed, moved or reordered" when inherited structure differs, which srs.fr.2.92 prevents in the Editor but a changed file can cause.
  - Use cases: UC-03; Initial Plan: —

## US-2.31 References in prefabs

**Story:** As a game developer, I want to references inside prefabs to survive instancing, nesting and applying, so that a prefab never stores a reference into one scene.

- **srs.fr.2.123** Inside a prefab or variant asset, an entity reference field shall store its target's prefab-local ID path (srs.fr.2.110) from the asset's root, through any nested instances, never a scene entity ID; when an instance is built, the path shall be resolved to a scene ID through the instance's map (srs.fr.2.67), and a path the map cannot resolve shall make the field Missing (srs.fr.36.75). Create Prefab, Create Variant, Replace Prefab, Apply, duplicating an asset (srs.fr.2.111) and the variant snapshot (srs.fr.2.107) shall convert references between scene IDs and paths by this rule.
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2
- **srs.fr.2.124** A reference whose target is outside the hierarchy being stored in an asset shall not be stored in it: Create Prefab, Create Variant and Replace Prefab shall store None in the asset and keep the reference as a field override of the instance, with the toast "n references to scene entities were kept on the instance"; Apply to Prefab of such a value shall be disabled with the reason "References a scene entity". When an added entity is applied (srs.fr.2.109), its references to entities of the asset shall be stored as paths, references to it from the asset's entities shall stay field overrides of the instance unless the user applies them, and they shall still resolve because its map entry keeps its scene ID.
  - Use cases: UC-03; Initial Plan: ip.fr.2.2

## US-2.32 Missing prefab content

**Story:** As a game developer, I want an instance whose prefab is gone to keep everything needed to reconnect, so that a missing file never silently erases an instance's data.

- **srs.fr.2.125** When an instance's prefab, a nested prefab inside it or a variant's base is missing and no usable snapshot exists (srs.fr.2.91, srs.fr.2.107), the Engine shall build, at the root of the part that cannot be resolved, one entity that keeps the scene ID, name and Position of that root, place under it the entities added to the instance whose parents cannot be resolved, and keep unchanged the prefab link, the instance's map and its field and structural overrides, which are written back unchanged when the scene or asset is saved and applied again when the asset returns; it shall log an error naming the missing asset ID, and the Editor shall show the badge "Prefab missing: no snapshot" on that entity and the Inspector note "Content of <asset ID> is unavailable".
  - Use cases: UC-03; Initial Plan: ip.fr.2.1, ip.fr.2.2

## US-2.33 Access checks

**Story:** As a game developer, I want the declared reads and writes to be the truth, so that the Semantic Graph never shows an access a system does not really make.

- **srs.fr.2.126** In every build, the Engine shall check each system's queries, component reads and writes, structural changes, enabled-flag changes, random draws, publications and subscriptions against its declarations (srs.fr.2.16), including those made by its event handlers; an undeclared access shall be an unrecoverable run error (srs.fr.1.11) naming the system, the type, the operation and the tick, and a system that declares an unregistered type shall be refused at registration with an error naming both.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.2.3

## US-2.34 Engine systems and groups

**Story:** As a game developer, I want the physics synchronization systems to run whatever I disable, so that turning off a group never breaks physics.

- **srs.fr.2.127** PhysicsSyncIn and PhysicsSyncOut (srs.fr.3.63) shall belong to no system group and shall not be added to one, and shall run in every tick whatever the enabled state of any system or group; disabling a group (srs.fr.2.52, srs.fr.36.43) shall never stop them.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.2.3

## US-2.35 Missing data asset types

**Story:** As a game developer, I want data assets whose type is gone to stay safe, so that a missing type never loses asset data.

- **srs.fr.2.128** When a data asset type is not registered, its asset files shall be kept unchanged and shown as "Missing type <Name>" with their values read-only from the type cache of srs.fr.2.118, and a component field that refers to such an asset shall read as referring to no asset, with a warning that names the entity and the asset.
  - Use cases: UC-03; Initial Plan: —
