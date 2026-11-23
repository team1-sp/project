---
epic: EP-05
group: srs.fr.5
part: engine
---

# EP-05 Input

**Epic:** As a game developer, I want to map keys to game actions and let the playtest agent use the same actions, so that keyboard play and automated play drive the game identically.

These requirements refine ip.fr.5.1 and ip.fr.5.2. Input comes from the keyboard, an input recording or the Playtest Agent; gamepad input, which ip.fr.5.3 lists as optional, is out of scope and is not supported.

## US-5.1 Input actions

**Story:** As a game developer, I want to turn key presses into named game actions, so that game logic does not depend on specific keys.

- **srs.fr.5.1** The Engine shall collect keyboard input once per render frame and translate it into input actions using the Input Map chosen by srs.fr.5.22, applied to ticks as stated in srs.fr.1.14.
  - Use cases: UC-06, UC-09; Initial Plan: ip.fr.5.1
- **srs.fr.5.2** A new project's Input Map shall start with the actions Move Up, Move Down, Move Left, Move Right and Interact and the default bindings of srs.fr.5.28; these are initial defaults, and actions may later be removed as stated in srs.fr.5.26.
  - Use cases: UC-06, UC-09; Initial Plan: ip.fr.5.1
- **srs.fr.5.3** The Engine shall accept input actions from the Automated Playtest Agent through the same action interface that keyboard input uses.
  - Use cases: UC-06, UC-09; Initial Plan: ip.fr.5.2
- **srs.fr.5.4** The Engine shall write an input recording of every Play mode session and of every headless run whose input source is the Playtest Agent, named with that run's number (srs.fr.7.12), whether or not telemetry is recorded.
  - Use cases: UC-06, UC-09; Initial Plan: ip.fr.7.2

## US-5.2 Keyboard input between ticks

**Story:** As a game developer, I want every key press to reach exactly one tick, so that short presses are never lost or repeated.

- **srs.fr.5.5** In normal mode, the Engine shall queue the keyboard transitions collected in each render frame as an ordered list of (action, down or up) in the order they occurred, and the first simulation tick that runs afterwards shall consume the whole list; when a render frame runs no tick, the list shall stay queued for the next tick.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1
- **srs.fr.5.6** When a tick consumes an ordered transition list, an action shall count as pressed in that tick if any of its transitions is down and as released if any is up, and shall be held at the end of the tick if its last transition in the list is down, or if it has no transition and was held before; for example, down, up, down leaves the action pressed, released and held, and down, up leaves it pressed and released but not held.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1
- **srs.fr.5.7** Pressed and released states shall last only for the tick that consumed them, and held states shall last until a tick consumes an up transition or a focus-lost marker (srs.fr.5.8), in normal mode, in recording playback and for the Playtest Agent alike.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1

## US-5.3 Keyboard focus

**Story:** As a game developer, I want to keys to be ignored while the game has no focus, so that nothing happens in the game while I work elsewhere.

- **srs.fr.5.8** When the Game view or game window loses keyboard focus or keyboard capture (srs.fr.20.8), the Engine shall stop interpreting keys: the next tick shall consume a focus-lost marker that clears every held keyboard action, and the Engine shall create no up transitions and no released states for those actions.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1
- **srs.fr.5.9** While focus or capture is lost, the Engine shall ignore keyboard input; when focus or capture returns, a key that is already down shall be ignored until it goes up, so that only presses made after the return are interpreted.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1
- **srs.fr.5.10** The Engine shall write each focus-lost marker into the input recording at the tick that consumed it, and playback shall clear the held actions at that marker in the same way, without released states.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1

## US-5.4 Input sources and recordings

**Story:** As a game developer, I want a single recording format that normal and headless runs read the same way, so that a recorded session replays exactly.

- **srs.fr.5.11** Each simulation tick shall take its input action state from exactly one input source: None (no actions in any tick), the keyboard (normal mode only), an input recording, or the Playtest Agent; the Engine shall ignore the keyboard while another source is active.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.5.1, ip.fr.7.2
- **srs.fr.5.12** An input recording shall be a text file in a documented format whose header holds the format version, the Aegis build, the scene, the seed, the source (keyboard, none, recording with the played file's name and content hash, agent, or hand-written), the length in ticks and a completion flag, followed by one entry for each tick that consumed at least one transition or focus-lost marker, holding the tick number and the ordered list of transitions (action, down or up) and markers as that tick consumed them; a session with the source None shall have no entries.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.5.1, ip.fr.7.2
- **srs.fr.5.13** When an input recording is played back, the Engine shall start with no action held, apply at each tick the ordered list of that tick's entry by the rules of srs.fr.5.6 and srs.fr.5.10, keep held actions held between entries, and give no pressed or released actions in ticks without an entry, so that normal-mode and headless runs read the same recording identically.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.5.1, ip.fr.7.2
- **srs.fr.5.14** When a recording's header names a different scene, build or seed from the run, the Engine shall still play it back and add a warning to the run's report that the results may differ.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.5.1, ip.fr.7.2
- **srs.fr.5.15** The Engine shall accept hand-written recordings in the same format, including an action that goes down and up several times in one tick, and shall reject a recording that names an unknown action, has tick numbers that do not increase, has an entry at or after its length, or has an up transition for an action that is not held at that point of the ordered list, naming the line; for an Incomplete recording, these checks shall apply only to its committed prefix (srs.fr.5.27).
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.5.1, ip.fr.7.2

## US-5.5 Recording length and lifecycle

**Story:** As a game developer, I want to recordings to have a clear end and to survive interruptions, so that a replayed session never runs on past its input or loses its last part.

- **srs.fr.5.16** When playback reaches the tick equal to the recording's length, the Engine shall release every held action in that tick as up transitions, which the new run's own recording shall store as an entry of that tick, give no input for the rest of the run, and state in the run's report the tick at which the recording ended.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.7.2
- **srs.fr.5.17** A recording with no entries shall give no input for its length and then end as stated in srs.fr.5.16.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.7.2
- **srs.fr.5.18** The Engine shall write the recording header with the completion flag off when the run starts, append entries as ticks run, flush them to disk at least every 60 ticks together with a marker that holds the number of ticks executed so far (srs.fr.1.19), and, on a normal end (the run finishes or is stopped, Play mode stops, or the game window closes), write the length and turn the completion flag on.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.7.2
- **srs.fr.5.19** When a run ends without a normal end, its recording shall keep its completion flag off; when the project next opens, the Editor shall label the recording Incomplete and recover it as stated in srs.fr.5.27.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.7.2
- **srs.fr.5.20** An Incomplete recording shall be listed with the label Incomplete and may be played back up to its recovered length, with a warning in the run's report that states the number of entries ignored after the last marker.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.7.2
- **srs.fr.5.21** When writing a recording fails, the Engine shall stop writing it, log an error that names the file and the tick, show the toast "Input recording failed at tick N" and label the recording Incomplete; the failure shall not end the run by itself, but the error shall be handled like any other logged error, so that Stop on failure (srs.fr.7.9) and Error Pause (srs.fr.24.6) apply to it when they are on.
  - Use cases: UC-07, UC-08; Initial Plan: ip.fr.7.2

## US-5.6 Input Map and action semantics

**Story:** As a game developer, I want to keys, bindings and movement to behave predictably, so that holding or combining keys never gives surprising movement.

- **srs.fr.5.22** The Input tab of Project Settings shall edit the project's Input Map; keyboard translation shall use the Input Map asset of the player's Player Input (srs.fr.9.22) when that field is set, and the project's Input Map otherwise, and the Input Map used shall be stated in the run's report.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1
- **srs.fr.5.23** The movement vector shall be (Right − Left, Up − Down), where each term is 1 when its action is held and 0 otherwise, so that opposite actions cancel, normalised to length 1 when both components are non-zero, so that diagonal speed equals the speed field.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1
- **srs.fr.5.24** The Engine shall ignore operating-system key repeat: a key shall create a down transition only when it goes from up to down and an up transition only when it goes from down to up.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1
- **srs.fr.5.25** An action shall be held while any of its bound keys or buttons is held; it shall get a down transition when the first of them goes down and an up transition when the last of them goes up, and a key bound to several actions shall create transitions for each of them.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1

## US-5.7 Missing gameplay actions

**Story:** As a game developer, I want gameplay to keep working when I remove a default action, so that removing an action has a predictable effect.

- **srs.fr.5.26** When the Input Map in use lacks a movement action, that action shall never be held, and when it lacks Interact, Interact shall never be pressed; the Input tab shall show the warning "Gameplay uses Move Up, Move Down, Move Left, Move Right and Interact" while any of them is missing.
  - Use cases: UC-06; Initial Plan: ip.fr.5.1

## US-5.8 Incomplete recording recovery

**Story:** As a game developer, I want an interrupted recording to be usable up to the point it was safely written, so that recovery never fails its own validation.

- **srs.fr.5.27** Recovery of an Incomplete recording shall take the committed prefix: the entries before the last complete, parseable marker whose tick numbers are below that marker's ticks executed; it shall ignore later entries and a partly written last line, set the length to that marker's ticks executed, or to 0 when there is no valid marker, and only then apply the checks of srs.fr.5.15 to the prefix; the file shall not be rewritten.
  - Use cases: UC-08; Initial Plan: ip.fr.5.1

## US-5.9 Default bindings

**Story:** As a game developer, I want to new projects to be playable with the keyboard at once, so that Reset to Defaults always gives the same keys.

- **srs.fr.5.28** The default bindings shall be: Move Up to W and Up Arrow, Move Down to S and Down Arrow, Move Left to A and Left Arrow, Move Right to D and Right Arrow, and Interact to E; they shall be used for a new project's Input Map (srs.fr.5.2), a new Input Map asset (srs.fr.23.18) and Reset to Defaults on the Input tab and on an asset (srs.fr.27.18, srs.fr.23.51).
  - Use cases: UC-03, UC-04, UC-06; Initial Plan: ip.fr.5.1
