---
epic: EP-35
group: srs.fr.35
part: editor
---

# EP-35 Common Dialog and Notification Behaviour

**Epic:** As a game developer, I want every dialog, confirmation and message to behave the same way, so that I never have to guess how to confirm or cancel.

This group covers rules that apply to every dialog (F-01 to F-19), the Delete Confirmation dialog (F-02) and toasts (F-13).

## US-35.1 Dialogs

**Story:** As a game developer, I want consistent keys and validation in every dialog, so that dialogs are quick to use and hard to misuse.

*Screens: F-01 to F-19*

- **srs.fr.35.1** In every dialog, pressing Enter shall trigger the primary button, except as stated in srs.fr.35.13, srs.fr.35.15 and srs.fr.35.16.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.35.2** In every dialog, pressing Esc or clicking ✕ shall cancel and close the dialog without changes, except in progress dialogs (srs.fr.35.14).
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.35.3** In every dialog, the primary button shall be disabled until all required fields are valid, and each field error shall be shown in red under its field.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.35.4** Every control shall have hover, pressed and disabled states, and every toggle shall also have an on state.
  - Use cases: UC-02, UC-03; Initial Plan: —

## US-35.2 Delete confirmation

**Story:** As a game developer, I want to see what breaks before I delete something, so that I do not break references by accident.

*Screens: F-02*

- **srs.fr.35.5** The Delete Confirmation dialog shall list every entity, component or asset that references the item to be deleted.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.35.6** When the user clicks Delete, the Editor shall delete the item; deleting entities shall be undoable and deleting assets shall not.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.35.7** The Delete Confirmation dialog shall state whether the deletion can be undone.
  - Use cases: UC-02, UC-03; Initial Plan: —
- **srs.fr.35.8** When the user clicks Cancel, the Editor shall keep the item.
  - Use cases: UC-02, UC-03; Initial Plan: —

## US-35.3 Toasts

**Story:** As a game developer, I want short messages that confirm actions or report errors, so that I always know whether an action worked.

*Screens: F-13*

- **srs.fr.35.9** The Editor shall show confirmations and results (for example Saved, build done, run finished, AI service unavailable) as toasts.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.35.10** An information toast shall hide itself after 5 seconds; an error toast shall stay until the user closes it.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.35.11** When the user clicks a toast's ✕, the Editor shall close the toast.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.35.12** When the user clicks a toast's action link (such as Undo, Open Report or Open Folder), the Editor shall carry out that action and close the toast.
  - Use cases: UC-02; Initial Plan: —

## US-35.4 Keyboard exceptions in dialogs

**Story:** As a game developer, I want the Enter and Esc keys to be safe in dangerous or long-running dialogs, so that a key press never deletes data or sends it by accident.

- **srs.fr.35.13** In dialogs whose primary action deletes, overwrites or cannot be undone, such as Delete Confirmation (F-02), Replace or Variant (F-19), Clear Recent List and Delete run, Enter shall trigger Cancel, and the destructive button shall need a click.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.35.14** In progress dialogs, such as the build progress dialog (F-12), Esc and ✕ shall request cancellation of the operation and the dialog shall close only when the operation has stopped; when the operation cannot be cancelled, Esc shall do nothing.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.35.15** In the AI data consent dialog (F-11), Enter shall not trigger any button, so that data is never sent by accident, and Esc shall act as stated in srs.fr.33.42.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.35.16** In a multi-line text field inside a dialog, Enter shall insert a new line and Ctrl+Enter shall trigger the primary button.
  - Use cases: UC-02; Initial Plan: —

## US-35.5 Rule precedence and coverage

**Story:** As a game developer, I want to to know which keyboard rule wins in each dialog, so that every dialog behaves predictably.

- **srs.fr.35.17** When dialog rules conflict, the more specific rule shall apply, in this order: the consent dialog rule (srs.fr.35.15), the destructive-action rule (srs.fr.35.13), the multi-line text field rule (srs.fr.35.16), the progress dialog rule (srs.fr.35.14), and then the general rules srs.fr.35.1 and srs.fr.35.2.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.35.18** The Project Settings window shall follow srs.fr.27.4 instead of srs.fr.35.2, and the Preferences window, which applies every change at once (srs.fr.27.29), shall close with Esc or ✕ without a prompt and without undoing applied changes.
  - Use cases: UC-02; Initial Plan: —
- **srs.fr.35.19** The general dialog rules shall apply to F-01, F-02, F-03, F-06 to F-11 and F-16 to F-19; F-04 and F-05 shall follow srs.fr.35.18; F-12 shall follow the progress dialog rule; F-14 and F-15 shall close with Enter, Esc or ✕; and toasts (F-13) shall follow srs.fr.35.9 to srs.fr.35.12.
  - Use cases: UC-02; Initial Plan: —

## US-35.6 Entity navigation

**Story:** As a game developer, I want every link to an entity to reach the right one or say why not, so that an old ID never selects an unrelated current entity.

- **srs.fr.35.20** When an action navigates to an entity named by a recorded item (a log entry from a run, event marker, finding, graph node or citation), the Editor shall select the current entity that matches it by srs.fr.8.21; when none matches and the item's run has a recording, it shall open that run's replay at the item's tick, or at the entity's first recorded tick, and select its placeholder, or, when the entity has no placeholder at that tick, make the historical selection of srs.fr.35.21; when that recording has no positions (srs.fr.30.25) or none of its scenes is loaded (srs.fr.30.27), it shall open the Telemetry panel on that run at that tick and show the same note as the Scene view; while replay is unavailable because a simulation operation is going (srs.fr.30.28), it shall act as stated in srs.fr.35.22; otherwise it shall show the toast "<name> (ID n) no longer exists". An item from Edit mode shall select the current entity with that ID.
  - Use cases: UC-06, UC-08, UC-09, UC-10, UC-11; Initial Plan: —

## US-35.7 Historical selection

**Story:** As a game developer, I want to to see a historical entity even when it is not drawn at that tick, so that navigation never promises a placeholder that cannot exist.

- **srs.fr.35.21** When a navigated entity was disabled, already destroyed or not yet created at the end of the item's tick, the Editor shall open replay at that tick without a placeholder, show the entity in the Inspector as a historical selection with its name, ID and recorded lifecycle ("Not present at tick N · created at tick a · destroyed at tick b"), and offer Go to Last Present Tick, which moves the scrubber to the last tick at whose end it existed and was enabled; when there is no such tick, Go to Last Present Tick shall be disabled with the tooltip "Never present at a tick boundary".
  - Use cases: UC-06, UC-08, UC-09, UC-10, UC-11; Initial Plan: —

## US-35.8 Navigation during a run

**Story:** As a game developer, I want to history links to keep working while the game runs, so that clicking a finding never stops a run.

- **srs.fr.35.22** While Play mode or a headless run or session is going, navigating to an entity that needs replay shall not stop or pause the operation: the Editor shall open the Telemetry panel on the item's run at the item's tick, show the entity's recorded lifecycle and values (srs.fr.8.22) in the panel's details, and show the note "Scene view replay is available in Edit mode"; Edit mode shall not be entered automatically.
  - Use cases: UC-06, UC-08, UC-09, UC-10, UC-11; Initial Plan: —

## US-35.9 Navigating to the initial state

**Story:** As a game developer, I want to navigation and citations to reach the initial state of a run, so that an entity that was only present before tick 0 can still be inspected.

- **srs.fr.35.23** When an item or citation names Start, or the navigated entity has records only in the run's initial state (srs.fr.8.26), the navigation of srs.fr.35.20 and srs.fr.35.21 shall use Start instead of a tick number, never tick 0, and keep their conditions on recordings, loaded scenes and modes and those of srs.fr.35.22: when Scene view replay is available it shall show the initial state, and while a simulation operation is going it shall show the initial-state records in the Telemetry panel without entering replay or stopping the operation.
  - Use cases: UC-08, UC-11; Initial Plan: —
