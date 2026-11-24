---
epic: EP-12
group: srs.fr.12
part: engine
---

# EP-12 Demo Game and Bug Injection

**Epic:** As a game developer, I want a demo project whose bugs I can switch on deliberately, so that automated testing and the assistant have known ground truth.

These requirements refine ip.fr.12.1 and ip.fr.12.2, and the reference mechanics of ip.fr.6.2 to ip.fr.6.5. The demo game is still undecided, so the requirements below name only the abstract roles of Section 6 that any chosen demo must fill, not specific objects. The demo mechanics are implemented as the demo game's own gameplay systems under the rules of srs.fr.6; they are not built into the Engine.

## US-12.1 Demo content

**Story:** As a game developer, I want a demo project template, so that I can start testing right away.

- **srs.fr.12.1** The project shall include a demo game, available as a project template, that contains at least one entity in each role of the reference scenario of Section 6: a player, obstacles, a collectible item, an unlockable entity that requires that item, and an objective.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1

## US-12.2 Demo mechanics

**Story:** As a game developer, I want the demo player to collect items, unlock entities and reach an objective, so that the demo has a complete loop that tests can check.

- **srs.fr.12.2** When a Begin CollisionEvent is handled between the player and an entity that has a Collectible component, and that entity's enabled flag is still on, the gameplay system shall add the collectible's item ID to the player's Inventory component, disable the collectible entity, whose flag turns off at once (srs.fr.2.116), and create one ItemCollectedEvent; "collected" shall mean only that the entity is disabled, so later CollisionEvents with it change nothing, and a collectible that is enabled again can be collected again.
  - Use cases: UC-06; Initial Plan: ip.fr.6.2
- **srs.fr.12.3** When the player's Interact action is pressed (srs.fr.5.6), the target Unlockable entity (srs.fr.12.6) is locked and the player's Inventory holds its required item, the gameplay system shall set its IsUnlocked field to true, which makes its colliders sensors as stated in srs.fr.12.8, and create an UnlockedEvent.
  - Use cases: UC-06; Initial Plan: ip.fr.6.3
- **srs.fr.12.4** When the player's Interact action is pressed, the target Unlockable entity is locked and the player's Inventory does not hold its required item, the gameplay system shall keep the entity locked and create an UnlockFailedEvent.
  - Use cases: UC-06; Initial Plan: ip.fr.6.4
- **srs.fr.12.5** When a Begin CollisionEvent is handled between the player and an enabled entity that has an Objective component, the gameplay system shall create an ObjectiveReachedEvent, at most once per Objective entity per run.
  - Use cases: UC-06; Initial Plan: ip.fr.6.5

## US-12.3 Interact target

**Story:** As a game developer, I want to Interact to pick one clear target and ignore unlocked ones, so that interacting gives one predictable result.

- **srs.fr.12.6** When Interact is pressed, the target shall be chosen only among locked Unlockable entities that have the player within their interaction range: the one whose world position is nearest to the player's, and then the one with the lowest entity ID; unlocked Unlockable entities shall never be the target.
  - Use cases: UC-06; Initial Plan: ip.fr.6.3, ip.fr.6.4
- **srs.fr.12.7** When no locked Unlockable entity has the player within its interaction range, Interact shall change nothing and create no event.
  - Use cases: UC-06; Initial Plan: ip.fr.6.3, ip.fr.6.4

## US-12.4 Unlockable state

**Story:** As a game developer, I want an Unlockable to behave the same however it is unlocked, so that a door never stays solid through one of its colliders.

- **srs.fr.12.8** While an Unlockable entity's is unlocked value is true, every collider component of that entity shall be a sensor in effect, whatever its is-sensor field, which keeps its authored value; while it is false, each shall follow its is-sensor field. The value loaded with a scene or prefab shall be applied when the shapes are created; a change by the gameplay system (srs.fr.12.3), by another system or by an Inspector edit during play shall be applied in the next synchronization-in as a sensor change (srs.fr.3.25); only srs.fr.12.3 creates an UnlockedEvent.
  - Use cases: UC-06; Initial Plan: ip.fr.6.3, ip.fr.6.4

## US-12.5 Debug flags

**Story:** As a game developer, I want to switch each known bug on and off, so that I can check that each bug is found.

- **srs.fr.12.9** The demo game shall provide a "no collider" debug flag that, when on, removes the colliders of one obstacle entity by the debug injection of srs.fr.12.14.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2
- **srs.fr.12.10** The demo game shall provide an "unlock without item" debug flag that, when on, makes an Interact on the locked Unlockable entity of the demo whose required item the player does not hold set is unlocked to true and create an UnlockedEvent holding the required item and the player's Inventory as they were before the Interact, instead of the UnlockFailedEvent of srs.fr.12.4, so that the detector of the unlock bug sees the unlock.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2
- **srs.fr.12.11** The demo game shall provide an "unreachable objective" debug flag that, when on, adds after the scene loads and before tick 0, through a debug injection like the one of srs.fr.12.14, a closed ring of static obstacles one grid cell (srs.fr.9.28) thick around the cells covered by the objective's colliders, with no opening; when the player has no enabled collider or its footprint has zero width or height, so that no grid exists, or when the player's start position would be inside or on the ring, the Engine shall not inject the flag and shall report it as not applied (srs.fr.12.15), with the reason "no player footprint" or "player inside the ring", in Play mode and in every headless run alike.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2
- **srs.fr.12.12** The demo game shall provide an "out of bounds" debug flag that, when on, removes the colliders of one boundary obstacle by the debug injection of srs.fr.12.14, so the player can leave the map.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2
- **srs.fr.12.13** The Engine shall store the debug flags, and the entity each collider flag acts on (srs.fr.12.9, srs.fr.12.12), in the project settings and apply them only to runs that start while the flags are on.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2

## US-12.6 Debug injection

**Story:** As a game developer, I want to injected bugs to work on any obstacle without breaking component rules, so that each flag always produces the bug it names.

- **srs.fr.12.14** Each collider debug flag shall act on the obstacle entity named for it in the demo scene's debug flag settings, which shall be an entity tagged Obstacle with at least one collider and no RigidBody component; the Debug Flags tab shall refuse to name another entity, keep the previous value and show the reason under the field. After the scene loads and before tick 0, the Engine shall remove that entity's colliders through a debug injection that is not a component removal by a system, so that srs.fr.3.36 does not refuse it and srs.fr.3.35 adds nothing back.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2
- **srs.fr.12.15** When the named entity is missing or not eligible when a run starts, the Engine shall not inject the flag and shall state in the report "<flag> not applied: no eligible obstacle".
  - Use cases: UC-09; Initial Plan: ip.fr.12.2

## US-12.7 Applied debug flag records

**Story:** As a game developer, I want each applied debug flag and its effect to be recorded, so that a diagnosis can cite what the flag actually changed.

- **srs.fr.12.16** For each debug flag applied to a run, the Engine shall state in the report, and record in the initial state of the run's recording when recording is on, "<flag> applied" with the entity named for it and the change made: for srs.fr.12.9 and srs.fr.12.12 the colliders removed, with their kind, size or vertices and offset; for srs.fr.12.11 the obstacle entities added, with their IDs and colliders; for srs.fr.12.10 the Unlockable entity it acts on.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2
