---
epic: EP-06
group: srs.fr.6
part: engine
---

# EP-06 Gameplay Rules

**Epic:** As a game developer, I want clear rules for where my gameplay logic runs and how it uses input, physics and events, so that any game I build stays deterministic and inspectable.

These requirements refine ip.fr.6.1 and ip.fr.6.6. They state the general rules by which Aegis lets a game define its gameplay: where gameplay logic runs, how it reads input and physics results and how it keeps state and reports what happened. They do not describe any particular game; the mechanics of the demo game are in srs.fr.12.

## US-6.1 Gameplay systems

**Story:** As a game developer, I want gameplay logic to run only in my own gameplay systems with all of its state in components, so that every gameplay rule can be saved, replayed and inspected.

- **srs.fr.6.1** The Engine shall run gameplay logic only in gameplay systems: systems registered by the game's project scripts (srs.fr.2.36) that belong to a system group in the gameplay input processing phase or the gameplay rules phase (srs.fr.1.12).
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.6.6
- **srs.fr.6.2** Gameplay decisions, such as whether an entity unlocks or an objective is reached, shall be made only by gameplay systems and never by the physics subsystem, the renderer or the Editor.
  - Use cases: UC-06; Initial Plan: ip.fr.6.6
- **srs.fr.6.3** A gameplay system shall keep all gameplay state in components or data assets and none in static or global variables, so that the state is saved in scenes, restored by Stop and replays, recorded in telemetry and shown in the Inspector and the Semantic Graph.
  - Use cases: UC-06; Initial Plan: ip.fr.6.6
- **srs.fr.6.4** A gameplay system that needs randomness shall draw it only from the gameplay random number generator (srs.fr.7.5).
  - Use cases: UC-06, UC-07; Initial Plan: -

## US-6.2 Input and movement

**Story:** As a game developer, I want gameplay to read input only as input actions and to move players through a Player Input component, so that keyboard, recordings and the agent all drive the game the same way.

- **srs.fr.6.5** In the gameplay input processing phase, a gameplay system shall read player input only as input action states of the current tick (srs.fr.1.14), never from the keyboard or another device directly, so that keyboard, recording and agent sources drive gameplay the same way.
  - Use cases: UC-06; Initial Plan: ip.fr.6.1
- **srs.fr.6.6** The Engine shall provide a Player Input component with a speed field; in the gameplay input processing phase of every tick, the Engine shall write the Velocity component of each enabled entity that has it from the movement vector of srs.fr.5.23 multiplied by its speed, so that for a dynamic body the input-driven velocity replaces the one left by the previous physics step (srs.fr.3.22).
  - Use cases: UC-06; Initial Plan: ip.fr.6.1

## US-6.3 Reacting to physics

**Story:** As a game developer, I want gameplay systems to react to physics only through collision events and component writes, so that physics never makes gameplay decisions.

- **srs.fr.6.7** A gameplay system shall learn about contacts only from CollisionEvents published by PhysicsSyncOut (srs.fr.3.63) and from raycasts (srs.fr.3.17), and shall never call the Physics Adapter directly.
  - Use cases: UC-06; Initial Plan: ip.fr.6.6
- **srs.fr.6.8** A gameplay system shall change physical behaviour only by writing components, such as Velocity, a collider's is-sensor field or an entity's enabled flag; the changes shall take effect in the next physics synchronization-in (srs.fr.3.58).
  - Use cases: UC-06; Initial Plan: ip.fr.6.6

## US-6.4 Gameplay events

**Story:** As a game developer, I want to define my own gameplay event types, so that the events of my game appear in the Event Bus, telemetry and the Semantic Graph.

- **srs.fr.6.9** The Engine shall allow a game to define its own gameplay event types with typed payload fields in its project scripts, registered with a semantic description like component types (srs.fr.2.36), so that they appear in the Event Bus panel, telemetry and the Semantic Graph.
  - Use cases: UC-02, UC-10; Initial Plan: -
