---
epic: EP-09
group: srs.fr.9
part: engine
---

# EP-09 Automated Bug Detection

**Epic:** As a game developer, I want an automated playtest agent that finds known bug classes, so that I spend less time on manual QA.

These requirements refine ip.fr.9.1 to ip.fr.9.3. The Editor side is in srs.fr.31.

## US-9.1 Playtest Agent

**Story:** As a game developer, I want an agent that plays the game in headless mode, so that the game is tested without me.

- **srs.fr.9.1** The Engine shall provide an Automated Playtest Agent that controls the player in headless mode by sending input actions.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.2** The Playtest Agent shall support at least two strategies: random exploration (srs.fr.9.12) and objective-seeking exploration (srs.fr.9.13 to srs.fr.9.17).
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.3** The Playtest Agent shall run the number of runs requested, numbered k = 0 to n − 1 and shown as "run k + 1 of n", giving run k the seed value (base seed + k) modulo 2³² (srs.fr.7.32).
  - Use cases: UC-09; Initial Plan: ip.fr.9.1, ip.fr.7.3

## US-9.2 Bug classes

**Story:** As a game developer, I want each known bug class to be detected and reported with evidence, so that I can find and fix the cause quickly.

- **srs.fr.9.4** During a headless run, the Engine shall detect a "missing collider" bug for each enabled entity that needs a collider (srs.fr.9.18) but has no enabled collider component, checked at tick 0 and whenever such an entity is created, enabled, given the tag Obstacle, or has a component added, removed, enabled or disabled.
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.9.5** During a headless run, the Engine shall detect an "unlock without item" bug when an UnlockedEvent is created whose payload (srs.fr.2.82) shows an Inventory that does not hold the payload's required item; the check shall use only the payload, never the Inventory or the Unlockable as they are when the event is dispatched or checked.
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.9.6** At the end of a Playtest Agent session whose outcome is Completed (srs.fr.9.39), the Engine shall report an "objective not reached within the test budget" finding when none of the session's runs created an ObjectiveReachedEvent and at least one run had an active Objective (srs.fr.9.45); the finding shall state the strategy, the number of runs, the maximum ticks, the ticks each run executed and the closest approach of srs.fr.9.44, and shall not state that the objective is unreachable.
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.9.7** During a headless run, the Engine shall detect an "out of bounds" bug when the player's position lies outside the map bounds (srs.fr.9.19, srs.fr.9.20).
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.9.8** For each detected bug, the Engine shall record an occurrence with the run number, the tick number and the IDs of the entities involved, as stated in srs.fr.9.43.
  - Use cases: UC-09; Initial Plan: ip.fr.9.3
- **srs.fr.9.9** In Playtest Agent sessions, the Engine shall check only the bug classes that are ticked in the Playtest Agent setup; other headless runs, including single agent runs, shall check the bug classes of srs.fr.7.19.
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.9.10** When the same bug class with the same entity identities (srs.fr.9.46) is found in several runs, the Engine shall report one finding that lists every occurrence and the number of runs in which it occurred; occurrences whose entities cannot be identified across runs shall stay separate findings.
  - Use cases: UC-09; Initial Plan: ip.fr.9.3
- **srs.fr.9.11** The Engine shall not report a finding that the user has marked as expected for the same expectation key: the scene asset ID, the agent strategy (or "no agent" for other headless runs), the bug class and the sorted entity identities of the finding (srs.fr.9.46).
  - Use cases: UC-09; Initial Plan: ip.fr.9.3

## US-9.3 Agent strategies

**Story:** As a game developer, I want the agent's strategies to be simple and exactly defined, so that its results are repeatable and I know what it can and cannot find.

- **srs.fr.9.12** Random exploration shall choose, uniformly at random from the agent's generator (srs.fr.7.22), one of the movement actions that the Input Map in use contains or no action, hold it for a number of ticks from 10 to 60 drawn from the same generator, and press Interact as stated in srs.fr.9.33 whenever the gameplay target of srs.fr.12.6 exists and the Input Map contains Interact; the agent shall never send an action that the Input Map in use does not contain, so that its input recording names only known actions (srs.fr.5.15).
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.13** The objective-seeking agent shall read only the observations of srs.fr.9.41; it shall not read source code, other runs or future ticks.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.14** At the start of each run, the objective-seeking agent shall build a grid over its planning area (srs.fr.9.26) as stated in srs.fr.9.28, mark a cell as blocked as stated in srs.fr.9.29, treat as blocking every enabled collider that is not a sensor in effect (srs.fr.12.8) and belongs to a static or kinematic body, and every collider of a locked Unlockable entity that is not a sensor in effect, whatever its body type; ignore disabled entities, which includes collected Collectibles (srs.fr.12.2); and find a path with the fewest steps that moves only up, down, left or right between unblocked cells, choosing the same path for the same inputs (srs.fr.9.49).
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.15** The objective-seeking agent shall choose its target in this order: (1) an Objective it has a path to; (2) a locked Unlockable entity whose required item the player holds and one of whose approach cells (srs.fr.9.36) it has a path to; (3) a Collectible it has a path to whose item is required by a locked Unlockable entity and is not yet held; ties shall go to the shorter path and then to the lower entity ID; Interact shall be pressed only as stated in srs.fr.9.33.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.16** When no target has a path, the agent shall use random exploration for 120 ticks, drawing from its own generator, and then choose again; when the path length from the player's cell to its target has not become at least one cell shorter in 120 ticks, or Interact on the target creates an UnlockFailedEvent, the agent shall exclude that target for 600 ticks.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.17** The objective-seeking agent shall move only with the four movement actions and Interact and shall not predict moving entities; it shall support games whose player moves freely in the plane with the Player Input component (srs.fr.5.23), which is the player that srs.fr.9.22 finds; when no such player exists, Start shall be refused as stated in srs.fr.9.30.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.4 Bug class rules

**Story:** As a game developer, I want the bug checks to report only real problems, so that I am not flooded with false findings.

- **srs.fr.9.18** An entity shall need a collider when it is enabled and has an enabled RigidBody, Collectible, Unlockable or Objective component or the tag Obstacle; a disabled role component shall not make it need one, other entities, such as decorative or logic-only ones, shall never be reported as missing a collider, and an intended exception shall be recorded with Mark as Expected (srs.fr.9.11).
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.9.19** The map bounds shall be the union of the Map Bounds rectangles (srs.fr.2.86) of the loaded scenes, read when the run starts, as the region covered by at least one rectangle, which may be disjoint; when no loaded scene has Map Bounds, the Engine shall infer them as the smallest rectangle that contains the extents of every enabled collider with a non-zero size on entities tagged Obstacle as saved, before debug flags are applied, and state in the report that the bounds were inferred.
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.9.20** The player shall be out of bounds only when its position, the entity's origin, lies strictly outside every rectangle of the map bounds; a position on the boundary of a rectangle shall count as inside; the bug shall be detected for the entity that is the player (srs.fr.9.22) at each tick and recorded once per run for each such entity, at the first tick in which that entity is out of bounds, as stated in srs.fr.9.43, so that two different player entities in one run give two occurrences.
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.9.21** When no loaded scene has Map Bounds and no entity tagged Obstacle has an enabled collider with a non-zero size, the Engine shall skip the out-of-bounds check and state in the report "No map bounds" or, when Obstacle entities exist, "No map bounds: Obstacle entities have no usable colliders".
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.9.22** The player for bug checks and for the Playtest Agent shall be the enabled entity that has an enabled Player Input component; when there is more than one, it shall be the one with the lowest entity ID.
  - Use cases: UC-09; Initial Plan: ip.fr.9.2

## US-9.5 Agent replanning

**Story:** As a game developer, I want the agent to notice progress and prerequisites, so that it does not keep walking into a locked path.

- **srs.fr.9.23** An Objective shall count as accessible only when the grid built from the current state has a path to it; a locked Unlockable entity shall block paths through its colliders as stated in srs.fr.9.14 until the agent observes it as unlocked (srs.fr.9.41), whether an UnlockedEvent was published or the field was set in another way.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.24** The objective-seeking agent shall rebuild the grid and choose its target again after an ItemCollectedEvent or UnlockedEvent, after reaching a target, and at least every 60 ticks.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.25** The objective not reached finding shall be session-level: it shall have no tick and one session occurrence that holds the number of runs, the maximum ticks and the closest approach of srs.fr.9.44; instead of Jump to Tick it shall offer Open Closest Run, which opens that run with the scrubber at that tick, and shall be disabled with the tooltip "No distance was measured" when there is no closest run.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.6 Planning area and grid

**Story:** As a game developer, I want the agent's grid to be exactly defined for any player shape, so that its plans are repeatable and honest about what they prove.

- **srs.fr.9.26** The planning area shall be the bounding rectangle of the map bounds of srs.fr.9.19 or, when there are none, the box around all enabled colliders other than the player's, enlarged by one player footprint on each side and stated in the report as inferred; Start for the objective-seeking strategy shall be refused with the message "Objective-seeking needs map bounds or colliders other than the player's" only when the scene has neither, and the player checks of srs.fr.9.30 shall be made separately.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.27** The player footprint shall be the axis-aligned box around all of the player's enabled colliders when the run starts, stored as its minimum and maximum offsets from the player's world position; it shall stay fixed for the whole run, also when the player later rotates, changes scale or changes its colliders.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.28** The grid shall have square cells whose side is the larger side of the player footprint, aligned so that one cell corner lies on the planning area's minimum corner.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.29** A cell shall be blocked when the player footprint, translated so that the player's world position is at the cell's centre, overlaps a blocking collider by the model of srs.fr.9.48; the player's own colliders shall never block, and a move between two 4-neighbouring cells shall be allowed only when both cells are free.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.7 Agent prerequisites and input transitions

**Story:** As a game developer, I want the agent to check what it needs and to press keys like a player, so that it fails clearly and its inputs match gameplay rules.

- **srs.fr.9.30** At Start, after the choice of srs.fr.29.21, the Playtest Agent setup and the Headless Run dialog with Agent chosen shall check the scene in the content snapshot and refuse Start with a message when: no player exists by srs.fr.9.22 ("No player: add an enabled Player Input to an enabled entity"); the Input Map lacks every movement action ("The Input Map has no movement actions"); or, for objective-seeking, the Input Map lacks any of the four movement actions or Interact ("Objective-seeking needs Move Up, Move Down, Move Left, Move Right and Interact"), the player has no enabled collider ("The player has no collider"), the player footprint has zero width or height ("The player's colliders have zero size"), or the planning area check of srs.fr.9.26 fails.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.31** At tick 0 of each agent run, after debug injection (srs.fr.12.14), the Engine shall repeat every check of srs.fr.9.30, including the planning area check of srs.fr.9.26, on the run's state as the authoritative check, and when one fails it shall end the run with the outcome Error and the same message, even when the saved scene passed; random exploration shall run without Interact when the Input Map has no Interact action.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.32** The agent shall send its decisions as down and up transitions in the input application phase; it shall hold at most one movement action, and when it changes the movement action it shall send the up transition of the old action before the down transition of the new one in the same tick.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.33** To interact, the agent shall send a down transition of Interact in one tick and its up transition in the next tick, and shall not press Interact again for the same target within 10 ticks; it shall press Interact only when the player is within the interaction range of the intended locked Unlockable entity and that entity is the gameplay target of srs.fr.12.6 in the current state.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.34** When the agent changes strategy, changes target or reaches the end of a hold, it shall send the up transition of every action it still holds, at most once per action, never for an action already released, and never earlier than the Interact pulse of srs.fr.9.33 allows: an Interact pressed in the same tick shall still be released in the next tick, and a new movement decision shall not change that pulse; at the end of a run it shall send no further transitions.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.8 Grid positions and reaching targets

**Story:** As a game developer, I want the agent to know which cell it and its target are in and when it has arrived, so that plans and outcomes are measured the same way.

- **srs.fr.9.35** An entity's cell shall be the grid cell that contains its world position, where a position on a shared cell edge belongs to the cell with the higher column or row index; an entity whose position lies outside the planning area shall have no cell.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.36** The approach cells of an Unlockable entity shall be the free cells whose centre lies within its interaction range of the entity's world position and at which that entity would be the gameplay target of srs.fr.12.6, given the locked Unlockable entities in the current state; the agent shall plan to the nearest approach cell and never to the Unlockable's own blocked cells.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.37** When the agent plans a path to a Collectible or an Objective, the cells covered by that target's own colliders shall not count as blocked; this exception shall never apply to an Unlockable entity, whose cells stay blocked while it is locked (srs.fr.9.36).
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.38** A Collectible target shall count as reached when its ItemCollectedEvent is published, an Objective when its ObjectiveReachedEvent is published, and an Unlockable when the player is within its interaction range and it is the gameplay target of srs.fr.12.6 in the current state, so that the agent may press Interact (srs.fr.9.33); reaching the target cell alone shall not count as reaching the target.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.9 Session outcome

**Story:** As a game developer, I want an interrupted session never to look like a full test, so that the report says exactly what was tested.

- **srs.fr.9.39** Each Playtest Agent session shall end with one outcome: Completed, when every requested run ran to an end condition of srs.fr.9.42; Stopped, when the user stopped it; or Error, when a run ended with Error, which shall end the session at once without starting further runs; the session report shall state the outcome, the number of runs requested and completed, and the ticks executed by each run, including an interrupted one.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.40** A Stopped or Error session shall not produce the objective not reached finding, and its report shall state "Session incomplete: k of n runs completed", where completed runs are those that ended by Max ticks or Objective reached; the bug findings of completed runs, and those already detected in the interrupted run, shall still be reported, the latter marked "from incomplete run".
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.10 Agent observations

**Story:** As a game developer, I want the agent's allowed inputs to be listed completely, so that its decisions use only state a player could see.

- **srs.fr.9.41** The agent's observations shall be, from the state after the last completed tick or, before tick 0, from the state after the scenes load and debug flags are applied: for each entity, its ID, enabled state, tag and world position; for each collider, its kind, world axis-aligned bounding box (srs.fr.9.48), sensor flag in effect (srs.fr.12.8) and enabled state and its body's type; for each Player Input, Inventory, Collectible, Unlockable and Objective component, its enabled state; for Collectible entities their item ID, for Unlockable entities their required item, interaction range and unlocked state, and which entities are Objectives; the player's Inventory and Player Input speed; the Input Map in use; the map bounds; and the gameplay events published in the last completed tick; the agent shall treat an entity as having a gameplay role only as stated in srs.fr.9.45.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.11 Session run policy

**Story:** As a game developer, I want every session run to follow one stated stop policy, so that session results mean the same thing every time.

- **srs.fr.9.42** Each run of a Playtest Agent session shall run until Max ticks or an ObjectiveReachedEvent (Stop on objective on, Stop on failure off) or until it ends with Stopped or Error, and the next run shall start from the session's content snapshot only after the previous one has ended with Max ticks or Objective reached.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.12 Findings pipeline

**Story:** As a game developer, I want to detection, occurrences and expected findings to have one clear effect, so that Mark as Expected behaves the same everywhere.

- **srs.fr.9.43** Bug handling shall have these stages: detection (srs.fr.9.4 to srs.fr.9.7); recording one occurrence per bug class, entity set and run, at the first tick of detection, however often the condition repeats; grouping occurrences into findings (srs.fr.9.10); and visibility, where a finding whose key is marked as expected (srs.fr.9.11) shall still have its occurrences recorded but shall be hidden from the Bug Report panel and the AI button badge, listed under Show expected (srs.fr.31.18), and shall not trigger Stop on failure.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.44** The closest approach shall be the smallest distance between the player and an active Objective (srs.fr.9.45) measured at the end of any tick of any run in which both existed, with its run and tick; when no distance was measured, the finding shall show "—" for distance and closest run; when no run had an active Objective, the session report shall state "No objective in the scene" instead of the finding.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.13 Active gameplay roles

**Story:** As a game developer, I want to disabled role components to count as no role, so that the agent and the checks agree with what gameplay does.

- **srs.fr.9.45** An entity shall have an active gameplay role (player, Collectible, Unlockable or Objective) only when the entity and that role's component are both enabled; the agent's targets, grid rules, start checks and bug checks shall ignore inactive roles, and a disabled Inventory shall count as an empty Inventory.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.14 Finding identity across runs

**Story:** As a game developer, I want to findings from different runs to be grouped only when they are about the same entity, so that an expected finding never hides a new bug.

- **srs.fr.9.46** The entity identity of a finding occurrence shall be (source scene, entity ID) for an authored entity; (creating prefab asset ID, prefab-local ID path, creating system, creation ordinal) for a runtime entity instantiated from a prefab, where the creation ordinal counts that system's instantiations of that prefab in the run from 0, so that repeated instances under the same parent stay distinct and match across runs only when they were created as the same numbered instantiation; and (run number, entity ID) for any other runtime entity, which therefore never matches across runs.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.9.47** Mark as Expected shall be disabled, with the tooltip "Created at runtime: can't be matched across runs", for a finding whose identity includes a run number.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.15 Grid overlap model

**Story:** As a game developer, I want a stated, conservative collision model for planning, so that an agent never plans through a shape it thinks is free.

- **srs.fr.9.48** The objective-seeking agent shall model every collider other than the player's by its world axis-aligned bounding box (for a box or polygon, the box around its transformed vertices; for a circle, the box around the circle), and the player by the footprint of srs.fr.9.27; a footprint shall overlap a collider only when the two boxes intersect with positive area. No cell that a collider's shape overlaps shall be treated as free, and the report shall state that the agent plans with bounding boxes and a fixed player footprint.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.16 Following the planned path

**Story:** As a game developer, I want the objective-seeking agent to follow the path it planned, so that its movement is predictable and repeatable.

- **srs.fr.9.49** While the objective-seeking agent has a target with a path, it shall each tick hold the one movement action that moves the player toward the next cell of the path and, when the player's cell becomes that cell, continue with the following cell; it shall plan again when the target changes or the path becomes blocked, and for the same grid, player cell, target and seed it shall choose the same path.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.17 Reaching the target at the end of a path

**Story:** As a game developer, I want the agent to finish the approach when its path ends before the target is reached, so that it never stands still next to a target it could still reach.

- **srs.fr.9.50** When the player's cell is the last cell of the path but the target is not yet reached (srs.fr.9.38), the objective-seeking agent shall move the player toward the target's world position, or for an Unlockable toward the centre of the chosen approach cell (srs.fr.9.36), holding in each tick the one movement action (srs.fr.9.32) along the axis with the larger distance to that point, the X axis when the distances are equal, and no movement action when both distances are zero, until the target is reached or the progress rule of srs.fr.9.16 excludes it; when the target is reached it shall release its movement action as stated in srs.fr.9.34.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1

## US-9.18 Agent bridge

**Story:** As a game developer, I want an agent to play through the same path as a player, so that an external agent can never change a run except through input.

- **srs.fr.9.51** The Engine shall give the Playtest Agent its observations (srs.fr.9.41) and take its input actions (srs.fr.5.3) through the AgentBridge interface, whether the agent runs in the Engine process or in a separate Python 3.12 process over gRPC: actions shall enter only as the input applied in tick phase 1; the observation after tick N shall be taken, without changing any state, from the state left by step 3 of srs.fr.8.28, the same state from which tick N's sample is taken, and the component types it reads shall be declared like a system's (srs.fr.2.16) so that the Semantic Graph shows them; and the exchange shall happen only between ticks, so that the transport never changes a run's results and an agent run replays like a keyboard run (srs.fr.7.6).
  - Use cases: UC-08; Initial Plan: ip.fr.9.1

## US-9.19 Agent connection failures

**Story:** As a game developer, I want a crashed or silent external agent to end the run cleanly, so that an agent run never hangs or plays input nobody chose.

- **srs.fr.9.52** When the Playtest Agent runs in a separate process and the process ends, the connection is lost, no action arrives within the deadline of the AgentBridge contract (srs.nfr.4.11), or the action names no input action of the Input Map in use, the Engine shall not run the next tick and shall end the run with the outcome Error and a message naming the failure and the tick, finalizing its report and recording (srs.nfr.5.11); Stop while an exchange is pending shall cancel the exchange and end the run as a Stop between ticks does.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
