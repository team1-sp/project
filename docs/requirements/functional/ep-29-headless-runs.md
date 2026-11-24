---
epic: EP-29
group: srs.fr.29
part: editor
---

# EP-29 Headless Runs

**Epic:** As a game developer, I want to set up, watch and compare headless runs from the editor, so that automated testing is easy to start and review.

This group covers the Headless Run setup dialog (F-07), the headless-run state of the main window (B-04), the Headless Runs panel (C-11 and C-11b) and the headless items of the Run menu (D-06).

## US-29.1 Run setup

**Story:** As a game developer, I want to choose a scene, input source, limits and seed for a headless run, so that every run is reproducible.

*Screens: F-07, D-06*

- **srs.fr.29.1** When the user chooses Run ▸ Run Headless… or presses Ctrl+Shift+H while no simulation operation is going (srs.fr.17.28), the Editor shall open the Headless Run setup dialog (F-07).
  - Use cases: UC-07; Initial Plan: ip.fr.1.4
- **srs.fr.29.2** The Scene ▾ field shall list the project's scenes and default to the active scene; the run shall load only the chosen scene, as its active scene (srs.fr.31.20).
  - Use cases: UC-07; Initial Plan: ip.fr.7.2, ip.fr.7.4
- **srs.fr.29.3** The Input source field shall offer None, Recording and Agent; the recording list beside it shall be enabled only when Recording is chosen, and a Strategy ▾ field with the strategies of srs.fr.9.2 shall be shown only when Agent is chosen (srs.fr.7.23).
  - Use cases: UC-07; Initial Plan: ip.fr.7.2
- **srs.fr.29.4** The Max ticks field shall default to 10 000 and accept whole numbers from 1 to 1 000 000.
  - Use cases: UC-07; Initial Plan: ip.fr.7.4
- **srs.fr.29.5** The dialog shall have Stop on objective and Stop on failure checkboxes.
  - Use cases: UC-07; Initial Plan: ip.fr.7.4
- **srs.fr.29.6** The dialog shall have a Record telemetry checkbox whose default comes from the Telemetry settings.
  - Use cases: UC-07; Initial Plan: ip.fr.8.5
- **srs.fr.29.7** The dialog shall have a Seed field that accepts a seed as stated in srs.fr.7.32 and defaults to the last seed used in this dialog, or to 0 the first time in a project.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3
- **srs.fr.29.8** When the user clicks Run, the Editor shall start the headless run from the saved files (srs.fr.7.17) and switch the main window to the headless-run state (B-04); when there is an item with unsaved changes (srs.fr.15.24), the dialog shall first offer the choice of srs.fr.29.21.
  - Use cases: UC-07; Initial Plan: ip.fr.7.1

## US-29.2 Run in progress

**Story:** As a game developer, I want to watch a headless run and stop it, so that I know how far it got.

*Screens: B-04*

- **srs.fr.29.9** During a headless run, the Editor shall replace the Scene view with a progress view that shows ticks per second, "N of max ticks" with N the ticks executed so far (srs.fr.1.19), and a progress bar.
  - Use cases: UC-07; Initial Plan: ip.fr.7.5
- **srs.fr.29.10** When the user clicks Stop Run, presses Esc or chooses Run ▸ Stop Headless Run, the Editor shall end the run after the current tick and save the report with the outcome "Stopped".
  - Use cases: UC-07; Initial Plan: ip.fr.7.4
- **srs.fr.29.11** When the user clicks Open Telemetry during a run, the Editor shall open the Telemetry panel on the live recording; Open Telemetry shall be disabled, with the tooltip "No telemetry recorded for this run", when the run does not record telemetry.
  - Use cases: UC-07; Initial Plan: ip.fr.8.1
- **srs.fr.29.12** When a run ends, the Editor shall show a toast such as "Run finished: Objective reached at tick 1 820" with an Open Report link.
  - Use cases: UC-07; Initial Plan: ip.fr.7.5
- **srs.fr.29.13** When the user clicks Open Report, the Editor shall open the Headless Runs panel (C-11) with that run's report.
  - Use cases: UC-07; Initial Plan: ip.fr.7.5

## US-29.3 Run history

**Story:** As a game developer, I want to review, repeat and compare past runs, so that I can confirm fixes and check determinism.

*Screens: C-11, C-11b*

- **srs.fr.29.14** The Headless Runs panel shall list past runs with run number, date, scene, outcome, ticks, duration and number of bugs found, newest first.
  - Use cases: UC-07; Initial Plan: ip.fr.7.5
- **srs.fr.29.15** When the user clicks a run row, the panel shall show the run's report: outcome, ticks, ticks per second, bugs found and run settings.
  - Use cases: UC-07; Initial Plan: ip.fr.7.5, ip.nfr.1.2
- **srs.fr.29.16** When the user clicks Re-run, the Editor shall start a new headless run with the run settings stored in the selected run's report (srs.fr.7.11) from the current saved files, and shall warn that the results may differ when a content hash differs from the selected run's report (srs.fr.7.18); when a stored setting can no longer be used, such as a deleted scene or input recording, it shall open the Headless Run dialog with the stored settings, name the missing item and start no run until the dialog is valid.
  - Use cases: UC-07; Initial Plan: ip.fr.7.4
- **srs.fr.29.17** The Compare button shall be enabled only when exactly two runs are selected and both have telemetry recordings.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3
- **srs.fr.29.18** When the user clicks Compare, the panel shall compare the sample fields recorded in both runs at the tick numbers sampled in both, and the event records over the shared tick range (tick 0 to the last tick number of the shorter run) whatever the sampling, as stated in srs.fr.7.15; it shall show the differences at equal tick numbers, name the first tick where they differ, state the two runs' lengths when they differ, and list the fields and ticks it compared and the data it left out (ticks beyond the shorter run, fields recorded in only one run, and the timing measurements of srs.fr.7.15).
  - Use cases: UC-07; Initial Plan: ip.fr.7.3
- **srs.fr.29.19** When two compared runs have no differences, the panel shall show "No differences in the compared data" with the compared fields, the sampled ticks and the event tick range, and any unequal lengths and left-out data of srs.fr.29.18.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3
- **srs.fr.29.20** When the user clicks Delete run, the Editor shall ask for confirmation (F-02) and, if confirmed, delete the run's report, telemetry recording, input recording and the finding occurrences that belong to it, removing findings left with no occurrence; a session-level finding (srs.fr.9.25) shall keep its session occurrence until every run of its session is deleted, and shall show Open Closest Run as "Run deleted" (srs.fr.31.17) when its closest run is deleted; the run number shall not be reused (srs.fr.7.12).
  - Use cases: UC-07; Initial Plan: ip.fr.7.5

## US-29.4 Save and Run

**Story:** As a game developer, I want to Save and Run to save everything the run needs or not run at all, so that a run never uses an older version of something I edited.

- **srs.fr.29.21** Before a headless run or Playtest Agent session with unsaved changes, the dialog shall offer Save and Run, Run Saved Version and Cancel. Start shall then proceed in this order: (1) for Save and Run, save every item with unsaved changes (srs.fr.15.24), asking for a file name as Save As… does for a scene never saved, and report the result as stated in srs.fr.15.23; when any save fails or a Save As… is cancelled, keep the dialog open, name the items not saved and go no further; (2) take the content snapshot (srs.fr.7.25) from the saved files; (3) make the start checks of the dialog, such as srs.fr.9.30, on the snapshot, and when one fails show its message and keep the dialog open, with any saves already made kept; (4) start the run, whose tick-0 checks (srs.fr.9.31) stay authoritative. Run Saved Version shall skip step 1 and state in the report "Unsaved changes were not included"; Cancel shall change nothing.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3, ip.fr.7.5

## US-29.5 Never-saved scenes in runs

**Story:** As a game developer, I want to to be told when a scene must be saved before it can run, so that a run never starts from a scene that has no saved version.

- **srs.fr.29.22** When the scene chosen in the Headless Run dialog or the Playtest Agent setup has never been saved, Run Saved Version (srs.fr.29.21) shall be disabled with the tooltip "Save this scene before running it", and Save and Run shall save it as stated in step 1 of srs.fr.29.21, starting no run when the save is cancelled or fails.
  - Use cases: UC-07; Initial Plan: ip.fr.7.3, ip.fr.7.5
