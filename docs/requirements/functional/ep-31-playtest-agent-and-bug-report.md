---
epic: EP-31
group: srs.fr.31
part: editor
---

# EP-31 Playtest Agent and Bug Report

**Epic:** As a game developer, I want to run the playtest agent, inject bugs and review its findings, so that automated QA fits into my normal work.

This group covers the Playtest Agent setup dialog (F-08), the Bug Report panel (C-12 and C-12b) and the Debug Flags items of the Run menu (D-06).

## US-31.1 Debug flags

**Story:** As a game developer, I want to switch injectable bugs on from the Run menu, so that I can create known faults quickly.

*Screens: D-06*

- **srs.fr.31.1** When the user toggles a flag in Run ▸ Debug Flags ▸ (no collider, unlock without item, unreachable objective, out of bounds) while the game is not playing, the Editor shall turn that debug flag on or off.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2
- **srs.fr.31.2** The Run ▸ Debug Flags ▸ submenu shall be disabled while the game is playing or a headless run is going.
  - Use cases: UC-09; Initial Plan: ip.fr.12.2

## US-31.2 Agent setup and run

**Story:** As a game developer, I want to configure and start the playtest agent, so that it tests the parts of the game I care about.

*Screens: F-08, D-06*

- **srs.fr.31.3** When the user chooses Run ▸ Run Playtest Agent… while no simulation operation is going (srs.fr.17.28), the Editor shall open the Playtest Agent setup dialog (F-08).
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.31.4** The Playtest Agent setup dialog shall have a Strategy ▾ field that lists the agent strategies.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.31.5** The Playtest Agent setup dialog shall have a Runs field that accepts whole numbers from 1 to 100.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.31.6** The Playtest Agent setup dialog shall have a Seed field, which accepts a seed and has the default of srs.fr.7.32, and a Max ticks field with the default and range of srs.fr.29.4, which shall refuse any other value with the reason under the field and disable Start until it is corrected.
  - Use cases: UC-09; Initial Plan: ip.fr.7.3, ip.fr.7.4
- **srs.fr.31.7** The Playtest Agent setup dialog shall have one checkbox per bug class (missing collider, unlock without item, objective not reached within the test budget, out of bounds), and Start shall be disabled until at least one is ticked.
  - Use cases: UC-09; Initial Plan: ip.fr.9.2
- **srs.fr.31.8** When the user clicks Start, the Editor shall run the agent and show its progress as "run k + 1 of n" with the ticks executed so far; Stop Run (srs.fr.29.10) during a session shall stop the current run, start no further run and end the session with the outcome Stopped.
  - Use cases: UC-09; Initial Plan: ip.fr.9.1
- **srs.fr.31.9** When the agent finishes, the Editor shall show its findings in the Bug Report panel (C-12), update the AI button badge as stated in srs.fr.17.23, and show the session outcome; for a Stopped or Error session it shall show "Session incomplete: k of n runs completed".
  - Use cases: UC-09; Initial Plan: ip.fr.9.3

## US-31.3 Bug Report

**Story:** As a game developer, I want to review each finding and jump to its evidence, so that I can confirm and fix bugs quickly.

*Screens: C-12, C-12b*

- **srs.fr.31.10** The Bug Report panel shall list each finding with its bug class, entity IDs, the number of runs in which it occurred and the tick of its first occurrence, with an expandable list of all occurrences; a session-level finding (srs.fr.9.25) shall show "—" as its tick and its session occurrence in the list.
  - Use cases: UC-09; Initial Plan: ip.fr.9.3
- **srs.fr.31.11** When there are no findings, the Bug Report panel shall show its empty state (C-12b).
  - Use cases: UC-09; Initial Plan: ip.fr.9.3
- **srs.fr.31.12** When the user clicks Jump to Tick on a finding, the Editor shall open the Telemetry panel on the run of the finding's first occurrence, or of the occurrence the user chose in its list, with the scrubber at that occurrence's tick; Jump to Tick shall be disabled when that run has no recording or the finding has no tick.
  - Use cases: UC-09; Initial Plan: ip.fr.8.1
- **srs.fr.31.13** When the user clicks Select Entities on a finding, the Editor shall navigate to the finding's entities as stated in srs.fr.35.20, using the run and tick of its first occurrence or of the occurrence chosen in its list.
  - Use cases: UC-09; Initial Plan: ip.fr.9.3
- **srs.fr.31.14** When the user clicks Ask AI on a finding, the Editor shall open the AI Assistant (C-14) with the finding as a context chip.
  - Use cases: UC-09; Initial Plan: ip.fr.11.3
- **srs.fr.31.15** When the user clicks Mark as Expected on a finding, the Editor shall store its expectation key (srs.fr.9.11) at once in the project's Editor data, also during Play mode (srs.fr.17.25), and hide that finding in runs that start later with the same key; a run or session that is going shall keep the keys it read at its start (srs.fr.7.31).
  - Use cases: UC-09; Initial Plan: ip.fr.9.3
- **srs.fr.31.16** When the user chooses Undo Mark as Expected from a finding's row menu, or from its row in the Expected list (srs.fr.31.18), the Editor shall remove its expectation key at once from the project's Editor data, also during Play mode, and report that finding again in runs that start later; a run or session that is going shall keep the keys it read at its start (srs.fr.7.31).
  - Use cases: UC-09; Initial Plan: ip.fr.9.3

## US-31.4 Deleted runs

**Story:** As a game developer, I want to references to a deleted run to say so, so that I am never sent to data that no longer exists.

- **srs.fr.31.17** A chat citation, comparison, finding occurrence or Re-run that refers to a deleted run shall show "Run deleted" and be disabled.
  - Use cases: UC-09; Initial Plan: —

## US-31.5 Expected findings list

**Story:** As a game developer, I want to to see and undo the findings I marked as expected, so that I can reverse a mark after it is hidden.

- **srs.fr.31.18** The Bug Report panel shall have a Show expected toggle that lists the findings hidden by Mark as Expected, each with its expectation key and an Undo Mark as Expected button.
  - Use cases: UC-09; Initial Plan: —

## US-31.6 Session scene

**Story:** As a game developer, I want to to choose which scene an agent session tests, so that sessions and single runs use scenes the same way.

- **srs.fr.31.19** The Playtest Agent setup dialog shall have a Scene ▾ field that lists the project's scenes and defaults to the active scene; the session shall load only the chosen scene as stated in srs.fr.31.20.
  - Use cases: UC-09; Initial Plan: —
- **srs.fr.31.20** A headless run and every run of a Playtest Agent session shall load only the chosen scene from their content snapshot, as their active scene, with its persistent entities; other scenes loaded in the Editor shall not take part, and scenes, prefabs and data assets that systems load at runtime (srs.fr.2.58, srs.fr.2.61) shall come from the same snapshot (srs.fr.7.25).
  - Use cases: UC-09; Initial Plan: —
- **srs.fr.31.21** When Start is clicked while there is an item with unsaved changes (srs.fr.15.24), the Playtest Agent setup shall first offer the choice of srs.fr.29.21.
  - Use cases: UC-09; Initial Plan: —
