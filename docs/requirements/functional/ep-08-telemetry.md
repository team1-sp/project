---
epic: EP-08
group: srs.fr.8
part: engine
---

# EP-08 Telemetry

**Epic:** As a game developer, I want to record what happens in every tick, so that I and the AI can see exactly what the game did.

These requirements refine ip.fr.8.1 to ip.fr.8.5. The Editor side of telemetry is in srs.fr.30.

## US-8.1 Recording

**Story:** As a game developer, I want to choose sampled state and timing fields while events are recorded every tick, so that I can control recording cost and analyse only the data actually recorded.

- **srs.fr.8.1** When telemetry recording is on, the Engine shall record, for every sampled tick, the tick number and the sample fields that are ticked in the project's Telemetry settings (srs.fr.8.3).
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.2** When telemetry recording is on, the Engine shall record every published event as stated in srs.fr.2.79, whatever the sample settings.
  - Use cases: UC-08; Initial Plan: ip.fr.8.2
- **srs.fr.8.3** The sample fields shall be: the world position and world rotation of every entity that has a Position component (srs.fr.3.37), the velocity of every entity that has a Velocity component, tick time (the real time taken to run the tick's phases), physics step time, and the execution time of each system, with all times in milliseconds.
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
- **srs.fr.8.4** Position, velocity, tick time and physics step time shall be ticked by default, and system execution time shall be unticked by default.
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
- **srs.fr.8.5** The Telemetry settings shall select only sample fields; the recording header (srs.fr.8.10), the event records and the bug findings shall always be recorded when recording is on.
  - Use cases: UC-08; Initial Plan: ip.fr.8.5
- **srs.fr.8.6** The Engine shall record the sample fields on every Nth tick counting from tick 0, where N is the "Sample every N ticks" value in the project's Telemetry settings, and on the last tick of the run; events shall be recorded on every tick regardless of N.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.7** The Engine shall measure, in every tick whatever the sample fields, the tick time from the start of input application to the end of telemetry recording and the time of the telemetry recording phase, as stated in srs.fr.8.25, and keep their totals for the run so that the recording overhead can be shown as a percentage of tick time.
  - Use cases: UC-08; Initial Plan: ip.fr.8.3

## US-8.2 Recording control and storage

**Story:** As a game developer, I want to switch recording on or off and keep each recording as a file, so that I control the cost and can reopen old recordings.

- **srs.fr.8.8** The Engine shall allow telemetry recording to be switched on and off by configuration before a run starts; a change made during a run shall apply from the next run.
  - Use cases: UC-08; Initial Plan: ip.fr.8.5
- **srs.fr.8.9** The Engine shall write each recording to the telemetry output folder from the project's Telemetry settings, named with its run number.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.8.10** Each recording shall store its run number, start date and time, run mode, the loaded scenes with the time each was last saved, the sample fields and N it was made with, and its number of ticks.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.8.11** The Engine shall allow a recording to be read while it is still being written.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.8.12** The Engine shall export a recording, or a tick range of it, to a JSON or CSV file whose format is documented.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4

## US-8.3 Recording by run type

**Story:** As a game developer, I want to know exactly when telemetry is recorded and what the file holds, so that I can trust a recording when I debug a run.

- **srs.fr.8.13** A Play mode session shall be recorded when Telemetry Recording (srs.fr.30.1) is on when Play mode starts.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.14** A headless run shall be recorded when its Record telemetry checkbox (srs.fr.29.6) is ticked.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.15** Every run whose input source is the Playtest Agent, in a Playtest Agent session or as a single agent run (srs.fr.7.23), shall be recorded with the project's Telemetry settings, whatever the Record telemetry setting; the Record telemetry checkbox shall show as ticked and disabled while Agent is chosen.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.16** Each recording file shall be a JSON Lines file in a documented format: the recording header on the first line, the schema of srs.fr.8.29 on the second, and then one line for each sampled tick record, with only the ticked sample fields, and one line for each event record.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.17** Bug findings and test reports shall be written whether or not telemetry is recorded; when no recording exists for a finding's run, Jump to Tick on that finding shall be disabled.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1

## US-8.4 Lifecycle and cause records

**Story:** As a game developer, I want to telemetry that tells me which entities existed and what each event changed, so that replay and diagnosis rest on recorded evidence.

- **srs.fr.8.18** When telemetry recording is on, the Engine shall record, whatever N, every entity creation with the entity's initial world position and rotation, its enabled state at creation and its provenance (srs.fr.8.20), every destruction and every enabled-state change with the resulting enabled state, each with its tick, its sequence number (srs.fr.8.27) and the system that requested it, or the Engine part for a change no system made; entities that exist when the run starts shall get creation records in the initial state (srs.fr.8.26), which shall be kept when the run ends before completing a tick and shall not mean that tick 0 completed (srs.fr.1.19).
  - Use cases: UC-08, UC-10; Initial Plan: ip.fr.8.2
- **srs.fr.8.19** When telemetry recording is on, the Engine shall record every component field write made while an event handler runs, with the event's sequence number and type, the handling system's name, the callback's subscription index within that system, the write's position in the order of writes of that dispatch, the entity, the component type, the field and its old and new values; several writes to the same field shall each get their own record in that order.
  - Use cases: UC-08, UC-10; Initial Plan: ip.fr.8.2

## US-8.5 Run provenance and replay limits

**Story:** As a game developer, I want to replay and citations to say exactly which historical entity and data they show, so that old runs are never confused with current entities or edits.

- **srs.fr.8.20** Each creation record shall hold the entity's ID, name, source scene and whether it was created at runtime; each recording header of a headless run shall hold the hashes of srs.fr.7.18, and each recording header of a Play mode session shall hold the start-state record of srs.fr.8.24.
  - Use cases: UC-06, UC-07, UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.21** A recorded entity shall match a current entity only when it was not created at runtime and its ID, source scene and name all equal those of the current entity; an entity that does not match shall be treated as historical.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.22** In replay, the Inspector shall show read-only the recorded name, ID, enabled state, world position and rotation and, when sampled, velocity; every other field shall be shown greyed out as "Not recorded".
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.23** An input recording shall reproduce only input actions, not a run's starting state; Inspector edits made during Play mode shall not be recorded, and when a Play mode session had such edits or started from unsaved content (srs.fr.8.24), its recording header shall say so and playback shall warn "This session had changes that the recording does not contain".
  - Use cases: UC-06, UC-08; Initial Plan: ip.fr.8.1

## US-8.6 Play mode start state

**Story:** As a game developer, I want a Play mode recording to say what it started from, including unsaved edits, so that I can explain why it differs from a headless run.

- **srs.fr.8.24** When Play mode starts, the recording header shall store a start-state record: the loaded scenes in order with the active scene; for each loaded scene, each prefab asset of the project, and each data asset and Input Map asset in use, its saved-file hash, whether it had unsaved changes and a hash of its in-memory content, where an item that has never been saved shall store its unsaved identity and no saved-file hash; a hash of the applied project settings (srs.fr.7.27); the enabled state of every system and group; and the identifiers of the Aegis build and the project scripts build; when any item had unsaved changes, the Engine shall also store a copy of that in-memory content beside the recording, so that the starting state can be compared with saved files.
  - Use cases: UC-06, UC-08; Initial Plan: ip.fr.8.1

## US-8.7 Tick timing measurement

**Story:** As a game developer, I want to recording cost to be measured honestly, so that no part of a tick's work is left out of its timing.

- **srs.fr.8.25** Whatever the sample fields, the Engine shall measure each completed tick's tick time and telemetry phase time up to the end of the tick's finalization (srs.fr.8.28), including the writing of that tick's buffered records to disk, and add them to the run's totals (srs.fr.8.7). The sample record of a sampled tick shall store the tick time only when the tick time sample field is ticked (srs.fr.8.3) and shall always store the telemetry phase time, which is recording metadata and not a selectable sample field; the recording's closing summary shall store both totals and the time of the final write after the last tick.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.8.1

## US-8.8 Tick boundaries

**Story:** As a game developer, I want to one clear meaning for the state at a tick, so that replay, samples and records never disagree about when something happened.

- **srs.fr.8.26** Recorded and replayed state shall be state at a tick boundary: the sample of tick N and replay at tick N shall show the state after tick N completes, and the state after the scenes load and debug injection, before tick 0, shall be the run's initial state, shown as "Start" and not as a tick; events of tick N shall be those published while tick N ran.
  - Use cases: UC-06, UC-07, UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.8.27** Within a tick, every lifecycle, event and field-write record shall carry a sequence number that increases in the order the Engine made it, giving the records of one tick a single order, and the tick's sample shall follow all of them; the initial state's records shall be numbered the same way.
  - Use cases: UC-06, UC-07, UC-08; Initial Plan: ip.fr.8.1

## US-8.9 Tick finalization

**Story:** As a game developer, I want the sample of a tick to include everything that tick did, so that no recorded change of a tick can come after its sample.

- **srs.fr.8.28** The telemetry recording phase of tick N shall end in this order: (1) its systems run; (2) the end-of-phase dispatch (srs.fr.2.46), whose handlers' field writes, creations, destruction requests and enabled-state changes are recorded with tick N; (3) the destructions and disables that take effect at the end of the phase (srs.fr.2.116), recorded with tick N; (4) when tick N is sampled (srs.fr.8.6), its sample is taken from the resulting state and given the last sequence number of tick N (srs.fr.8.27); (5) buffered records are written to disk; (6) the tick time and telemetry phase time are measured, added to the run's totals (srs.fr.8.7) and stored in tick N's sample record as stated in srs.fr.8.25. No system or handler shall run between steps 3 and 6, so that no recorded change of tick N can follow its sample. An event published during step 2 shall be recorded at publication with tick N and a sequence number before the sample, and shall be dispatched at the first dispatch point of tick N + 1 (srs.fr.2.76), where its final status is added, or be dropped as stated in srs.fr.2.99.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.8.1

## US-8.10 Schema record

**Story:** As a game developer, I want every recording to explain its own data, so that the Semantic Graph never needs the source code to read a recording.

- **srs.fr.8.29** Every telemetry recording shall store, right after its header (srs.fr.8.10), a schema of the run: the tick phases; each component, event and enum type with its description and each field's name, type, unit, default value and layout (srs.fr.2.12); each system with its purpose, phase, group, run order, enabled state, declarations (srs.fr.2.16) and, for each subscription, the tick phases in which its handler runs; and, derived from the declarations, the systems that read and write each component type.
  - Use cases: UC-07; Initial Plan: ip.fr.8.1

## US-8.11 Colliders in creation records

**Story:** As a game developer, I want each entity's colliders to be recorded when it is created, so that the assistant can see from the recording which colliders an entity started with.

- **srs.fr.8.30** When telemetry recording is on, each creation record (srs.fr.8.18) shall also list the entity's colliders as they are at creation, after any debug injection: for each, its kind, its size or vertices and offset in metres, its sensor flag and its enabled state.
  - Use cases: UC-07, UC-08, UC-09; Initial Plan: ip.fr.8.1

## US-8.12 Reading and failing recordings

**Story:** As a game developer, I want a recording that is being written, or failed, to read predictably, so that a live view or a disk error never shows half a record.

- **srs.fr.8.31** While a recording is being written, a reader shall see only records that were completely written, and an event record whose final status (srs.fr.2.79) has not yet been added shall read as pending. When writing a recording fails, the Engine shall stop writing it, log an error naming the file and the reason, keep the run going and, for a headless run, state "Telemetry recording incomplete: <reason>" in the report; a recording that failed, or was left by a process that ended, shall reopen as incomplete with the records completely written before the failure.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.8.1
