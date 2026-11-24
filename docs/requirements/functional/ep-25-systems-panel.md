---
epic: EP-25
group: srs.fr.25
part: editor
---

# EP-25 Systems Panel

**Epic:** As a game developer, I want to see and control the registered systems and their run order, so that I understand and tune what runs each tick.

This group covers the Systems panel (C-09).

## US-25.1 Systems list

**Story:** As a game developer, I want to enable, reorder and inspect systems, so that I can isolate problems and fix ordering bugs.

*Screens: C-09*

- **srs.fr.25.1** The Systems panel shall list every registered system in run order with an enable checkbox and its execution time in the last tick.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3, ip.fr.8.3
- **srs.fr.25.2** When the user clears a system's checkbox, the Engine shall skip that system from the next tick on in Play mode, or from the next run when a headless run or session is going (srs.fr.7.31), and the Console shall log a warning that names the system.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.25.3** When the user drags a system by its handle to a new position, the Editor shall change the run order and save it in the project; the engine systems of srs.fr.25.8 cannot be moved.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
- **srs.fr.25.4** The Systems panel shall show a system's execution time in red when it is more than 25 % of 16.7 ms, the real-time length of one tick.
  - Use cases: UC-04; Initial Plan: ip.fr.8.3, ip.nfr.1.1
- **srs.fr.25.5** When the user clicks a system row, the Systems panel shall show the components the system reads and writes in a side list, each linked to that component's description.
  - Use cases: UC-04; Initial Plan: ip.fr.10.2
- **srs.fr.25.6** When the user clicks Reset Order, the Editor shall ask for confirmation and, if confirmed, discard the manual order and restore the default order of systems within the tick phases: input application, gameplay input processing, physics synchronization-in, physics step, physics synchronization-out, gameplay rules, telemetry recording; within a phase and group, the built-in systems shall come first and then the project's systems, each in ascending ordinal order of their type name, group membership shall be kept, and the before/after rules shall then apply (srs.fr.2.50).
  - Use cases: UC-04; Initial Plan: ip.fr.2.3

## US-25.2 Per render frame stages

**Story:** As a game developer, I want the Systems panel to show what runs per tick and what runs per render frame, so that I never try to reorder rendering into the simulation.

- **srs.fr.25.7** The Systems panel shall show rendering and presentation as a read-only Per render frame row below the tick phases; they shall not be reorderable or disableable and shall not be part of Reset Order.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3

## US-25.3 Engine systems

**Story:** As a game developer, I want to to see the engine's own systems without breaking them, so that physics never stops because a checkbox was cleared.

- **srs.fr.25.8** The Systems panel shall list PhysicsSyncIn and PhysicsSyncOut (srs.fr.3.63) as engine systems in their tick phases, with their execution times; their enable checkboxes and drag handles shall be disabled with the tooltip "Engine system: always runs", and Reset Order shall keep them in their phases.
  - Use cases: UC-04; Initial Plan: ip.fr.2.3
