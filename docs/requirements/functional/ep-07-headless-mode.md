---
epic: EP-07
group: srs.fr.7
part: engine
---

# EP-07 Headless Mode

**Epic:** As a game developer, I want to run the full simulation without a window, as fast as possible, so that automated tests can run thousands of ticks quickly.

These requirements refine ip.fr.7.1 to ip.fr.7.5. The Editor side of headless runs is in srs.fr.29.

## US-7.1 Headless simulation

**Story:** As a game developer, I want the headless run to behave exactly like a normal run, so that results from automated runs are trustworthy.

- **srs.fr.7.1** In headless mode, the Engine shall run the complete simulation (input, ECS, physics, gameplay and telemetry) without opening a window and without calling any rendering function.
  - Use cases: UC-07; Initial Plan: ip.fr.7.1
- **srs.fr.7.2** In headless mode, the Engine shall not initialise the graphics library and shall refer to assets by asset ID instead of loading textures.
  - Use cases: UC-07; Initial Plan: ip.fr.7.1
- **srs.fr.7.3** In headless mode, the Engine shall take its input actions from one of the sources of srs.fr.5.11 chosen at start: None, an input recording file (srs.fr.5.12) or the Automated Playtest Agent; the keyboard source shall not be available in headless mode.
  - Use cases: UC-07; Initial Plan: ip.fr.7.2
- **srs.fr.7.4** In headless mode, the Engine shall run simulation ticks back to back, without render frames and without waiting for real time.
  - Use cases: UC-07; Initial Plan: ip.fr.7.1
- **srs.fr.7.5** The Engine shall provide one gameplay random number generator, using the PCG32 algorithm implemented in engine code and seeded with the run's seed, and systems shall draw random numbers only from it, through the system context and only when they declare that they use it, so that the same seed gives the same sequence on the same build; the Playtest Agent's own generator (srs.fr.7.22) shall be the only other generator in a run.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3
- **srs.fr.7.6** On the same computer, two runs shall produce the same entity positions, velocities and events at every tick number, whether each run is headless or in normal mode, when they have all the prerequisites of srs.fr.7.28; determinism across computers, operating systems or builds is not required.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3

## US-7.2 Run limits and report

**Story:** As a game developer, I want headless runs to stop on a limit or an event and produce a report, so that I get a clear outcome for every run.

- **srs.fr.7.7** The Engine shall stop a headless run when the tick counter reaches the run's maximum number of ticks.
  - Use cases: UC-07; Initial Plan: ip.fr.7.4
- **srs.fr.7.8** When "Stop on objective" is set, the Engine shall stop a headless run at the tick in which an ObjectiveReachedEvent is created.
  - Use cases: UC-07; Initial Plan: ip.fr.7.4
- **srs.fr.7.9** When "Stop on failure" is set, the Engine shall stop a headless run at the tick in which a bug is detected that is not marked as expected (srs.fr.9.43) or the Engine logs an error.
  - Use cases: UC-07; Initial Plan: ip.fr.7.4
- **srs.fr.7.10** When a stop request is received, the Engine shall end the headless run after the current tick with the outcome "Stopped".
  - Use cases: UC-07; Initial Plan: ip.fr.7.4
- **srs.fr.7.11** At the end of a headless run, the Engine shall write a test report that holds the primary outcome chosen by srs.fr.7.16, every other end condition met in the final tick as secondary reasons, the ticks executed and the last tick number (srs.fr.1.19), the wall-clock duration, the average ticks per second, the detected bugs and the run settings: scene, input source, the input recording's identity when one was used, the agent strategy when Agent was used, seed, maximum ticks, Stop on objective, Stop on failure and Record telemetry.
  - Use cases: UC-07; Initial Plan: ip.fr.7.5
- **srs.fr.7.12** The Engine shall allocate a run number when a Play mode session or headless run starts, by reading and incrementing a run counter stored in the project's Runs folder and saving the counter before the run's first tick; a run number shall never be reused, even after its run is deleted, and each headless run's test report shall be saved in the Runs folder as a JSON file named with its number.
  - Use cases: UC-07; Initial Plan: ip.fr.7.5
- **srs.fr.7.13** While a headless run is going, the Engine shall send its ticks executed so far, maximum ticks and ticks per second to the Editor at least 4 times per second.
  - Use cases: UC-07; Initial Plan: ip.fr.7.5

## US-7.3 Determinism rules

**Story:** As a game developer, I want the conditions for repeatable runs to be stated exactly, so that a difference between two runs always points to a real change.

- **srs.fr.7.14** Systems shall not read wall-clock time, and every query shall return entities in ascending entity ID order, so that a run's results do not depend on timing or memory layout.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3
- **srs.fr.7.15** Determinism comparisons, such as run comparison (srs.fr.29.18), shall compare sampled entity positions and velocities at tick numbers sampled in both runs and events over every tick number both runs executed; events shall be compared tick by tick in sequence-number order by type, entities, payload, publication tick and final status, with the dispatch tick or drop reason, so that a different dispatch delay is a difference. Only timing measurements shall be ignored: tick time, physics step time, system execution times, telemetry phase time and wall-clock time; tick numbers are simulation behaviour, not timing.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3

## US-7.4 Run end and start state

**Story:** As a game developer, I want to know how a run starts and which outcome wins when several things happen at once, so that a report always tells me the real reason a run ended.

- **srs.fr.7.16** When several end conditions are met in the same tick, the primary outcome shall be the first that applies in this order: Error, Stopped, Failure, Objective reached, Max ticks.
  - Use cases: UC-07; Initial Plan: ip.fr.7.4, ip.fr.7.5
- **srs.fr.7.17** A headless run shall start from the content snapshot of srs.fr.7.25, built from the saved files of the project's scenes, prefabs and data assets and the applied project settings, not from unsaved edits in the Editor.
  - Use cases: UC-07; Initial Plan: ip.fr.7.4, ip.fr.7.5
- **srs.fr.7.18** The test report shall store a content hash of each frozen input of srs.fr.7.27 that the run loaded and the identifiers of the build inputs listed there; together these shall be the run's recorded inputs.
  - Use cases: UC-07; Initial Plan: ip.fr.7.4, ip.fr.7.5
- **srs.fr.7.19** Headless runs that are not Playtest Agent sessions, including single agent runs (srs.fr.7.23), shall check the missing collider, unlock without item and out of bounds bug classes, and Stop on failure shall stop on any of them; the objective not reached finding shall apply only to Playtest Agent sessions (srs.fr.9.6).
  - Use cases: UC-07; Initial Plan: ip.fr.7.4, ip.fr.7.5

## US-7.5 Deterministic ordering

**Story:** As a game developer, I want every source of ordering in a run to be fixed, so that two identical runs on my computer never drift apart.

- **srs.fr.7.20** Entities created during a tick shall get their IDs in the order the creation requests are made (srs.fr.2.63) and shall be visible to systems that run after the request; destruction requests shall take effect at the end of the current tick phase, after its event dispatch, in the order they were made (srs.fr.2.116).
  - Use cases: UC-07; Initial Plan: ip.fr.7.3
- **srs.fr.7.21** The PhysicsSyncOut system (srs.fr.3.63) shall publish the CollisionEvents of a physics step sorted by the lower and then the higher entity ID of each pair, with contact events before sensor events; because there is at most one event of each kind per entity pair per step (srs.fr.3.53), this order shall be complete, and the collider pairs inside an event shall be sorted by their collider identifiers.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3
- **srs.fr.7.22** The gameplay random number generator shall be used only by systems, in their run order; the Playtest Agent shall use a separate generator, seeded with a documented function of the run's seed, for every random decision it makes: action choice, hold duration, random exploration used as a fallback (srs.fr.9.16) and any random tie break, so that the agent's choices do not change the gameplay's random numbers.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3

## US-7.6 Single agent runs

**Story:** As a game developer, I want to to start one agent-controlled headless run from the Headless Run dialog, so that I can try the agent without setting up a session.

- **srs.fr.7.23** When Agent is chosen in the Headless Run dialog, Run shall start one single agent run, not a Playtest Agent session: the agent shall use the dialog's Strategy, Seed and Max ticks, Stop on objective and Stop on failure shall apply, the bug classes shall be those of srs.fr.7.19, telemetry shall be recorded (srs.fr.8.15), an input recording shall be written (srs.fr.5.4), and the report shall state the strategy.
  - Use cases: UC-07, UC-09; Initial Plan: ip.fr.7.1
- **srs.fr.7.24** A single agent run shall not produce the objective not reached finding and shall not appear as a session in the Bug Report panel; its findings shall be listed like those of other headless runs.
  - Use cases: UC-07, UC-09; Initial Plan: ip.fr.7.1

## US-7.7 Headless content isolation

**Story:** As a game developer, I want a headless run to test exactly the content it reports, so that editing during a run never changes it halfway.

- **srs.fr.7.25** When a headless run or a Playtest Agent session starts, the Engine shall read into memory, as its content snapshot, the frozen inputs of srs.fr.7.27, taking every scene, prefab, data asset and Input Map asset file of the project so that content loaded later in the run is also captured (srs.fr.7.30), and shall not read those files again until the run or session ends; every run of a session shall use the snapshot taken when the session started.
  - Use cases: UC-07, UC-08, UC-09; Initial Plan: ip.fr.7.1
- **srs.fr.7.26** While a headless run or Playtest Agent session is going, the Editor may edit, save, import, reimport and delete scenes, prefabs, data assets and other assets and apply project settings; none of these shall change the running run, and the report shall describe only its content snapshot.
  - Use cases: UC-07, UC-09; Initial Plan: ip.fr.7.1

## US-7.8 Run inputs

**Story:** As a game developer, I want the report to list exactly which inputs a run used, so that nothing that changes behaviour is left unrecorded or editable mid-run.

- **srs.fr.7.27** A run's inputs shall be: frozen inputs, read once at the start into the content snapshot and hashed when loaded, which are the scene, prefab, data asset and Input Map asset files, the applied project settings (including the project's Input Map, the system order and ordering rules, the system and group enabled states and the debug flags) and the selected input recording file; and build inputs, identified in the report by ID and not hashed, which are the Aegis build and the build of the project scripts that registered the component, system and data asset types.
  - Use cases: UC-07, UC-08, UC-09; Initial Plan: ip.fr.7.1

## US-7.9 Determinism prerequisites

**Story:** As a game developer, I want the determinism guarantee to list everything it depends on, so that I know when two runs should match.

- **srs.fr.7.28** Two runs shall be expected to match only when they have: the same Aegis build and project scripts build; equal starting content, either the same frozen inputs (srs.fr.7.27) or Play mode start-state records (srs.fr.8.24) with equal in-memory hashes, settings hash and system and group enabled states; the same loaded scenes in the same order with the same active scene; the same seed and input actions per tick; the same project settings, system order, ordering rules and system and group enabled states; the same debug flags; and either no Editor edits during the run or the same edits applied at the same tick boundaries.
  - Use cases: UC-06, UC-07, UC-08; Initial Plan: ip.fr.7.1
- **srs.fr.7.29** Inspector edits and system or group enable changes made during Play mode shall take effect at the next tick boundary; the recording header shall flag a session that had them (srs.fr.8.23), and run comparison of a flagged session shall warn that the runs may differ because of Editor edits.
  - Use cases: UC-06, UC-08; Initial Plan: ip.fr.7.1

## US-7.10 Content outside the snapshot

**Story:** As a game developer, I want a run to fail clearly when it needs content it does not have, so that missing content never silently comes from disk.

- **srs.fr.7.30** When a system requests a scene, prefab, data asset or Input Map asset that is not in the content snapshot, the Engine shall not read it from disk: it shall log the error "Not in the run's content snapshot: <path>", treat the request as failed (srs.fr.2.27 for a data asset reference), and record only the files the run actually loaded in the hashes of srs.fr.7.18.
  - Use cases: UC-07, UC-09; Initial Plan: ip.fr.7.1

## US-7.11 Editor state during runs

**Story:** As a game developer, I want to Editor toggles during a headless run to wait for the next run, so that a run's result never depends on when the user clicked.

- **srs.fr.7.31** A headless run or Playtest Agent session shall use the system and group enabled states, the expectation keys of srs.fr.9.11 and the telemetry recording setting that hold when it starts, unchanged until it ends; changing them while it is going shall change only Editor state and later runs, and its Stop on failure decisions and report shall use the values read at its start.
  - Use cases: UC-07, UC-09; Initial Plan: ip.fr.7.1

## US-7.12 Seed range

**Story:** As a game developer, I want to seeds that behave the same in every run dialog, so that a large base seed never breaks an agent session.

- **srs.fr.7.32** A seed shall be a whole number from 0 to 4 294 967 295 (2³² − 1); the Seed fields of the Headless Run dialog (srs.fr.29.7) and the Playtest Agent setup (srs.fr.31.6) shall refuse any other value and show "Seed must be a whole number from 0 to 4294967295" under the field, default to the last seed used in that dialog or to 0 the first time in a project, and seeds derived from a base seed (srs.fr.9.3) shall wrap around modulo 2³².
  - Use cases: UC-07, UC-09; Initial Plan: ip.fr.7.1

## US-7.13 Standalone headless options

**Story:** As a game developer, I want to to run a built game headless from the command line with its own settings, so that a build server can test the game without the Editor.

- **srs.fr.7.33** A built game started with `--headless` shall take its run settings from the command-line options `--max-ticks <N>` (default 10 000, a whole number from 1 to 1 000 000), `--seed <S>` (default 0, a seed as stated in srs.fr.7.32) and `--input-recording <file>` (default: input source None); when an option is unknown or its value is not valid, the game shall write an error naming the option to the standard error output, run no tick and exit with a non-zero exit code, and otherwise it shall stop at the maximum number of ticks as stated in srs.fr.7.7.
  - Use cases: UC-07; Initial Plan: ip.fr.1.4, ip.fr.7.4
