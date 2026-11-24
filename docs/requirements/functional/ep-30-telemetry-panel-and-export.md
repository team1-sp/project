---
epic: EP-30
group: srs.fr.30
part: editor
---

# EP-30 Telemetry Panel and Export

**Epic:** As a game developer, I want to view, replay and export recorded telemetry, so that I can see when and why something happened.

This group covers the Telemetry panel (C-10 and C-10b), the Export Telemetry dialog (F-09) and the telemetry items of the Run menu (D-06).

## US-30.1 Recording control

**Story:** As a game developer, I want to switch telemetry recording on and off, so that I record only when I need to.

*Screens: C-10, D-06*

- **srs.fr.30.1** When the user toggles Run ▸ Telemetry Recording, the Editor shall turn telemetry recording on or off for the next run; the toggle shall be Editor state, set from the project default of srs.fr.27.24 when the project opens and when that default is applied, and never written to the project settings; it shall stay usable during Play mode and runs and shall not change the recording of the operation that is going.
  - Use cases: UC-08; Initial Plan: ip.fr.8.5
- **srs.fr.30.2** The Telemetry panel's Record toggle shall have the same effect as Run ▸ Telemetry Recording and shall show the same state.
  - Use cases: UC-08; Initial Plan: ip.fr.8.5
- **srs.fr.30.3** When the user picks a recording from Recording ▾, the Telemetry panel shall load it; each entry shall show its run number, whether it is live, and its length in ticks executed (srs.fr.1.19).
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.30.4** When the user picks data series in Fields ▾, the Telemetry panel shall show only those series.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1

## US-30.2 Charts and timeline

**Story:** As a game developer, I want to read tick timings and events on a timeline, so that I can spot spikes and see what caused them.

*Screens: C-10*

- **srs.fr.30.5** The Telemetry panel shall draw the tick time and the physics step time of each sampled tick as lines, with a dashed line at the 16.7 ms real-time length of one tick; a series whose sample field was not recorded shall be shown in the legend as "Not recorded" and not drawn, and when neither was recorded the chart shall show "No timing samples in this recording".
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
- **srs.fr.30.6** When the user hovers over the chart, the panel shall show the tick number and the recorded timing values at the cursor, leaving out series that were not recorded.
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
- **srs.fr.30.7** When the user clicks the chart, the panel shall move the scrubber to that tick.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.30.8** When the user Shift+drags across the chart, the panel shall zoom the chart to the dragged tick range.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.30.9** The event timeline shall show a marker for every collision, item collected, unlocked, unlock failed and objective event at its tick.
  - Use cases: UC-08; Initial Plan: ip.fr.8.2
- **srs.fr.30.10** When the user clicks an event marker, the Editor shall navigate to the entities involved in that event as stated in srs.fr.35.20, using the marker's run and tick.
  - Use cases: UC-08; Initial Plan: ip.fr.8.2
- **srs.fr.30.11** When the user clicks Prev event or Next event, the panel shall move the scrubber to the previous or next event marker.
  - Use cases: UC-08; Initial Plan: ip.fr.8.2
- **srs.fr.30.12** The Telemetry panel shall show a "Telemetry share" badge with the run's total telemetry phase time divided by its total tick time (srs.fr.8.7), which every recording stores whatever the sample fields, so far for a live recording; when the total tick time is zero, as for a run that completed no tick, the badge shall show "—" with the tooltip "No completed tick".
  - Use cases: UC-08; Initial Plan: ip.nfr.1.3

## US-30.3 Replay

**Story:** As a game developer, I want to replay recorded positions in the Scene view, so that I can see exactly where things were at any tick.

*Screens: C-10*

- **srs.fr.30.13** When the user drags the scrubber to a tick in Edit mode, the Scene view shall enter replay and show the state at the end of that tick (srs.fr.8.26): the entities that existed and were enabled then by the lifecycle records (srs.fr.8.18), each at the world position and rotation of the latest of its creation record and its samples at or before that tick, drawn with its current sprite only when it matches a current entity by srs.fr.8.21 and as an outlined placeholder labelled with its name and ID when it does not.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1
- **srs.fr.30.14** While the Scene view shows a replay, it shall show the banner "Replay · tick N", or "Replay · Start" for the initial state, followed by "· sparse samples" when the position of any entity shown comes from a sample or creation record before tick N, with an Exit Replay button, and the Inspector shall show for the selected entity "Position from tick M", or "Position from Start" when the position comes from the initial state; the Scene view shall disable every editing command and gizmo and show in the Inspector only the recorded values of srs.fr.8.22, and selection shall still work.
  - Use cases: UC-08; Initial Plan: ip.fr.4.3
- **srs.fr.30.15** When the user presses Esc, clicks Exit Replay, closes the Telemetry panel, starts Play mode or a headless run, or opens another scene, the Scene view shall leave the replay and show the scene as edited, with the selection and Scene view camera it had before the replay.
  - Use cases: UC-08; Initial Plan: ip.fr.8.1

## US-30.4 Export

**Story:** As a game developer, I want to export telemetry to files, so that I can analyse it in other tools.

*Screens: F-09, D-06*

- **srs.fr.30.16** When the user clicks Export… in the Telemetry panel, or chooses Run ▸ Export Telemetry… while a recording exists, the Editor shall open the Export Telemetry dialog (F-09).
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.30.17** The Export Telemetry dialog shall offer the formats JSON and CSV.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.30.18** The Export Telemetry dialog shall have a tick range, from and to, that defaults to the whole recording.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.30.19** When the tick range is empty, reversed or outside the recording, the dialog shall show the error under the field and disable Export.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.30.20** The Export Telemetry dialog shall have a checklist of the data fields to export.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.30.21** The Export Telemetry dialog shall have a file path field with a Browse… button.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4
- **srs.fr.30.22** When the user clicks Export, the Editor shall write the file and show a toast with an Open Folder link.
  - Use cases: UC-08; Initial Plan: ip.fr.8.4

## US-30.5 Replay rules

**Story:** As a game developer, I want the replay to be view-only and to say what it can show, so that replaying a run can never damage my scene.

- **srs.fr.30.23** Replay shall draw from a separate replay state and shall never write to entities, components or the undo history, so that leaving replay needs no restore of scene data.
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
- **srs.fr.30.24** Replay shall reconstruct only recorded data: entity existence and enabled state from the lifecycle records, positions and rotations from the samples, and event markers; sprites shall be drawn from the current assets and field values of the edited scene, and the banner shall say "Positions from telemetry · visuals are current, not recorded".
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
- **srs.fr.30.25** When a recording has no position sample field, the scrubber shall still move the chart and event timeline, but the Scene view shall not enter replay and shall show the note "This recording has no positions".
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
- **srs.fr.30.26** When a loaded scene has been saved since the recording was made, the replay banner shall add "Scene changed since this recording", and entities shall still be matched by srs.fr.8.21.
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
- **srs.fr.30.27** When none of the recording's scenes is loaded, the Scene view shall not enter replay and shall show "Open <scene> to replay this recording".
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
- **srs.fr.30.28** Replay in the Scene view shall be available only in Edit mode; during Play mode and headless runs, the scrubber shall move only the chart and the event timeline.
  - Use cases: UC-08; Initial Plan: ip.fr.8.3

## US-30.6 Telemetry export failures

**Story:** As a game developer, I want telemetry export to protect existing files and report failures, so that an export never overwrites work by surprise or leaves half a file.

- **srs.fr.30.29** When the export file already exists, the Editor shall ask for confirmation before replacing it; the Editor shall write the export to a temporary file and replace the target only when writing has completed, show the success toast (srs.fr.30.22) only then, and on failure delete the temporary file, leave any existing file unchanged and show an error toast naming the path and the reason.
  - Use cases: UC-08; Initial Plan: ip.fr.8.3
