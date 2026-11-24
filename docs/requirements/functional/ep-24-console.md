---
epic: EP-24
group: srs.fr.24
part: editor
---

# EP-24 Console

**Epic:** As a game developer, I want to see every log message with its details and source, so that I can find errors and warnings quickly.

This group covers the Console panel (C-08) and its log entry context menu (E-06).

## US-24.1 Log list

**Story:** As a game developer, I want to filter, search and inspect log messages, so that I can focus on the messages that matter.

*Screens: C-08*

- **srs.fr.24.1** The Console shall list every log entry with its level icon (Info, Warning or Error), its message and, for entries logged during a run, the tick number at which it was logged.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.2** The Console shall show the number of Info, Warning and Error entries on its toolbar and on its tab.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.3** When the user clicks Clear, the Console shall remove all entries and set all counters to 0.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.4** When Clear on Play is on, the Console shall clear itself every time Play mode starts.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.5** When Collapse is on, the Console shall show identical messages as one row with a count badge.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.6** When Error Pause is on, the Editor shall pause Play mode after the tick in which the first error is logged (B-03), at that tick boundary as stated in srs.fr.1.16, even when the render frame had more ticks to run.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.7** When the user turns the Info, Warning or Error toggle off, the Console shall hide entries of that level; turning it on shall show them again.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.8** When the user types in the Console search box, the Console shall show only the entries whose message contains the typed text.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.9** When the user clicks an entry, the Console shall show its full message, tick number and stack trace in the detail pane.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.10** When the user double-clicks an entry that names a source file and line, the Editor shall open that file at that line in the external code editor.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.11** When the user double-clicks an entry that names an entity, the Editor shall navigate to that entity as stated in srs.fr.35.20.
  - Use cases: UC-06; Initial Plan: —

## US-24.2 Log entry context menu

**Story:** As a game developer, I want to act on a log entry from its context menu, so that I can copy, trace or explain a message.

*Screens: E-06*

- **srs.fr.24.12** When the user chooses Copy from the log entry menu, the Editor shall copy the message to the clipboard.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.13** When the user chooses Copy with Stack Trace, the Editor shall copy the message and its stack trace to the clipboard.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.14** When the user chooses Select Entity, the Editor shall navigate to the entity named in the message as stated in srs.fr.35.20; the item shall be disabled when no entity is named.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.15** When the user chooses Jump to Tick, the Editor shall open the Telemetry panel with the scrubber at the entry's tick; Jump to Tick shall be disabled, with the tooltip "No telemetry recorded for this run", when the entry has no tick or its run has no recording.
  - Use cases: UC-06; Initial Plan: ip.fr.8.1
- **srs.fr.24.16** When the user chooses Open Source, the Editor shall open the file at the line in the external code editor; the item shall be disabled when no file is named.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.17** When the user chooses Collapse Duplicates, the Console shall turn on Collapse.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.24.18** When the user chooses ✦ Ask AI to Explain, the Editor shall open the AI Assistant (C-14) with this log entry as a context chip.
  - Use cases: UC-06; Initial Plan: ip.fr.11.3
