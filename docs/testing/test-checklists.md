# Test Checklists

Source: Senior Project Guide "Testing Checklist", extended with requirement IDs. Tick items by linking the test name (`[x] test name — file`). Items stay in this file; progress is also visible in tests.

## Game loop (ip.fr.1.*, srs.fr.1.1 to 1.18)

Tests are in `tests/unit/core/`.

- [x] Render frame order: collect input, due ticks, render, present (srs.fr.1.1) — `ApplicationTest.NormalModeFrame_CollectsInputThenRunsDueTicksThenRendersThenPresents`
- [x] 60 ticks are 1.0 s at 30, 60, 120, 144 and 240 frames per second (ip.fr.1.2, srs.fr.1.2) — `FixedTimestep.OneSecondAtSteadyFrameRates_RunsExactly60Ticks`, `ApplicationTest.OneSecondAtSteadyFrameRates_RunsSixtyTicks`
- [x] At most 5 ticks per render frame, further whole ticks dropped, part of a tick kept (srs.fr.1.3, 1.4) — `FixedTimestep.FiveSecondSpike_RunsFiveTicksAndDropsTheRest`, `FixedTimestep.MoreThanFiveTicksDue_DropsWholeTicksAndKeepsThePart`
- [x] Slow-simulation warning at most once per second (srs.fr.1.4) — `FixedTimestep.TicksDroppedEveryFrame_WarnsOncePerSecond`
- [x] Tick counter starts at 0 and goes up after each completed tick; simulated time is the counter times 1/60 s (srs.fr.1.5, 1.6) — `ApplicationTest.DuringATick_TickCounterIsThatTicksNumber`, `ApplicationTest.HeadlessSixtyTicks_SimulatedTimeIsOneSecond`
- [x] Headless: no window, no rendering, stops at the maximum number of ticks (ip.fr.1.4, srs.fr.1.7, 7.1, 7.7) — `ApplicationTest.HeadlessMode_RunsMaxTicksWithoutWindowOrRendering`, `Sandbox.HeadlessRun_ExitsCleanly`
- [x] Closing the window ends the run with exit code 0 and releases the window (ip.fr.1.3, srs.fr.1.10) — `ApplicationTest.WindowClosedByUser_RunReturnsZeroAndShutdownClosesWindow`
- [x] A render frame with no tick due still renders and presents the last completed tick (srs.fr.1.15) — `ApplicationTest.FrameWithNoTickDue_StillRendersAndPresents`, `ApplicationTest.RenderFrame_SeesOnlyCompletedTicks`
- [x] Every tick runs the seven tick phases in their fixed order, in headless and normal mode (srs.fr.1.12) — `TickPhases.*`, `ApplicationTest.HeadlessTick_RunsTheSevenPhasesInOrder`, `ApplicationTest.NormalModeTick_RunsTheSevenPhasesInOrder`, `Sandbox.HeadlessRun_RunsSevenPhasesPerTick`
- [x] With the same input source, headless and normal mode give the simulation identical phases, ticks and input states (srs.fr.1.12, 1.13, R-6) — `ApplicationTest.SameInputSource_HeadlessAndNormalModeRunIdenticalPhases`
- [x] A phase that throws a run error ends its tick; later phases do not run (srs.fr.1.11, 1.12) — `ApplicationTest.PhaseThrowsRunError_LaterPhasesOfThatTickDoNotRun`
- [x] Each tick gets the input action state of its source, built by srs.fr.5.6 before phase 1 and unchanged during the tick; keyboard transitions of frames without a tick go to the next tick; headless uses None; another source makes the keyboard ignored (srs.fr.1.14, 5.5, 5.7, 5.11, 7.3) — `InputActionState.*`, `KeyboardQueue.*`, `NoInputSource.*`, `ApplicationTest.InputState_IsTheSameInEveryPhaseOfATick`, `ApplicationTest.HeldAction_StaysHeldInTicksWithoutTransitions`, `ApplicationTest.KeyboardTransitionsOfFramesWithoutTick_GoToTheNextTick`, `ApplicationTest.FrameWithSeveralTicks_OnlyTheFirstTickGetsTheKeyboardTransitions`, `ApplicationTest.NormalModeWithAnotherSource_IgnoresTheKeyboard`, `ApplicationTest.HeadlessWithoutSource_TicksGetNoInput`
- [ ] Real key presses reach the keyboard queue through the Input Map (srs.fr.1.14, 5.1) — needs US-5.1
- [x] Unrecoverable run error: message and tick logged, run stops at that tick, non-zero exit code, outcome Error (srs.fr.1.11) — `ApplicationTest.HeadlessTickThrowsRunError_RunStopsAtThatTickWithOutcomeError`, `ApplicationTest.RunError_LogsMessageAndTickNumber`, `ApplicationTest.NormalModeTickThrowsRunError_StopsWithoutRenderingThatFrame`, `Sandbox.RunError_ExitsNonZero`, `Sandbox.RunError_LogsMessageAndTick`
- [ ] Editor Play mode: a run error Error-Pauses at that tick, Pause and Step disabled until Stop (srs.fr.1.11, 24.6) — needs Play mode (US-17.2)
- [x] Command-line run options `--headless`, `--max-ticks` (default 10 000), `--seed`, `--input-recording`; an invalid or unknown option is refused with an error naming it and no tick runs (srs.fr.1.8, 7.32, 7.33) — `RunOptions.*`, `Sandbox.HeadlessWithoutMaxTicks_RunsTenThousandTicks`, `Sandbox.InvalidSeed_ExitsNonZeroWithoutTicks`
- [ ] Standalone window size and fullscreen from the project's Rendering settings (srs.fr.1.9, 27.23) — `WindowConfig::fullscreen` exists and reaches the window factory (`ApplicationTest.WindowConfigGiven_FactoryReceivesIt`); the settings need projects
- [x] A pause takes effect at the next tick boundary, also from inside a tick; whole ticks due are dropped and the part of a tick is kept (srs.fr.1.16, 24.6) — `ApplicationTest.PauseRequestedDuringATick_NoFurtherTickStarts`, `ApplicationTest.PauseAndResume_DiscardWholeTicksKeepTheFractionAndSimulateNoPausedTime`
- [x] While paused no real time is added and no slow-simulation warning is logged (srs.fr.1.17) — `ApplicationTest.WhilePaused_LongFramesRunNoTickAndDoNotWarn`
- [x] Resume measures real time from the moment of resuming and continues from the kept fraction (srs.fr.1.18) — `ApplicationTest.PauseAndResume_DiscardWholeTicksKeepTheFractionAndSimulateNoPausedTime`, `ApplicationTest.ResumeDuringAFrame_PausedPartOfTheNextFrameTimeIsNotSimulated`, `FrameClock.SinceFrameStart_IsTheTimeSinceTheLastMeasurement`
- [x] Step while paused runs exactly one tick per step without changing the accumulator (srs.fr.17.14, 1.17) — `ApplicationTest.StepWhilePaused_RunsExactlyOneTickAndLeavesTheAccumulator`, `ApplicationTest.TwoStepsInOneFrame_RunTwoTicks`, `ApplicationTest.StepReachesMaxTicks_RunEndsWithMaxTicks`
- [x] Step and resume do nothing while running; a headless run cannot pause; a new run starts unpaused — `ApplicationTest.StepOrResumeWhileRunning_ChangeNothing`, `ApplicationTest.HeadlessRunOrNoRun_CannotBePaused`, `ApplicationTest.SecondRun_StartsUnpaused`
- [ ] Editor Pause, Step and Ctrl+Shift+P / Ctrl+Alt+P in Play mode (srs.fr.17.12 to 17.14) — needs Play mode (US-17.2)

## ECS (ip.fr.2.*)

- [ ] Entity creation returns unique IDs (2.1)
- [ ] Entity deletion removes it from queries; stale handle detected (2.1)
- [ ] Component add / remove (2.2)
- [ ] Component read / modify (2.2)
- [ ] System selects the correct entities for its query (2.4) — e.g. MovementSystem selects Player/Enemy, not Wall/Door
- [ ] Systems run once per frame in defined, repeatable order (2.3)
- [ ] Query iteration order deterministic
- [ ] Structural change during iteration is safe/deferred

## Physics via adapter (ip.fr.3.*)

- [ ] Dynamic body is affected by gravity (3.3)
- [ ] Static body does not move (3.2)
- [ ] Kinematic body moves only by code (3.2)
- [ ] Collider produces a collision (3.2)
- [ ] Collision event generated with correct entity IDs (3.6)
- [ ] Raycast finds the correct entity / none (3.7)
- [ ] Fixed timestep consistent: two runs with the same inputs give identical trajectories (1.2)
- [ ] Entity↔body mapping correct both ways (3.4)
- [ ] Write-back: PositionComponent == body position after Step (3.5)
- [ ] DestroyBody on entity destruction leaves no orphan body

## Rendering data (ip.fr.4.*)

- [ ] Sprite drawn at the entity's PositionComponent (4.1)
- [ ] Sprite position equals physics body position (3.5 + 4.1)
- [ ] Door sprite matches DoorState (open/closed) (4.2)
- [ ] RenderSystem does not modify simulation state (4.3) — compare state hash before/after
- [ ] Camera follows selected entity (4.4, should)
- [ ] Simulation continues when rendering is disabled (7.1)

## Gameplay (ip.fr.6.*)

- [ ] Movement input updates VelocityComponent (6.1)
- [ ] Key pickup adds to Inventory (6.2)
- [ ] Door opens with key + Interact in range → DoorOpenedEvent, IsOpen=true (6.3)
- [ ] Door stays closed without key → DoorOpenFailedEvent (6.4)
- [ ] Goal reached → GoalReachedEvent (6.5)
- [ ] Physics code never decides door state (6.6) — review check

## Integration chain

- [ ] Input → ECS → Physics → ECS → Rendering: each link asserted separately
- [ ] Walk right into wall: velocity set → body moves → collision event → position stops → render data matches

## Headless (ip.fr.7.*)

- [ ] No window and no render function invoked (7.1)
- [ ] Accepts agent input stream (7.2)
- [ ] Same results as normal mode for identical inputs (7.3)
- [ ] Run limited by max frames or terminated by goal/failure event (7.4)
- [ ] Test report produced (outcome, frames, bugs) (7.5)

## Telemetry (ip.fr.8.*)

- [ ] Per-frame state recorded (8.1)
- [ ] Events recorded with frame number (8.2)
- [ ] Frame and physics-step timings recorded (8.3)
- [ ] Export file matches documented schema (8.4)
- [ ] Recording can be switched off (8.5); off ⇒ no overhead/alloc
- [ ] Overhead ≤ 10% (ip.nfr.1.3)

## Bug detection (ip.fr.9.*, 12.2)

- [ ] `B-NO-COLLIDER`, `B-DOOR-NO-KEY`, `B-GOAL-UNREACHABLE`, `B-OUT-OF-BOUNDS` each detected with class, frame, entity IDs
- [ ] Clean build: zero detections (no false positives)

## Semantic graph (ip.fr.10.*)

- [ ] Nodes for entities, components, systems, events (10.1)
- [ ] Edges: has, reads/writes, collidedWith, triggered (10.2)
- [ ] Built from telemetry only (10.3)
- [ ] Query interface answers the acceptance sketch (10.4)
- [ ] 10,000-frame recording processed < 60 s (ip.nfr.1.4)

## AI assistant (ip.fr.11.*)

- [ ] Answers "Why did the door not open?" with cited frames/entities (11.2, 11.3)
- [ ] Says "unavailable" when evidence is missing (11.5)
- [ ] Never modifies engine/source (11.6)
- [ ] No external transmission unless enabled (ip.nfr.2.1)
- [ ] Response time target (ip.nfr.1.5)
- [x] Read-only public boundary and manual text suggestions (srs.fr.11.8–11.9) —
  `ReadOnlyAssistantTest.DesignSuggestion_IsManualTextAndCannotMutateGraphOrProjectFiles`
- [x] Configured answers use AgentBridge (srs.fr.11.10) —
  `ReadOnlyAssistantTest.ConfiguredQuestion_RoutesOnlyThroughAgentBridgeAndPersistsReply`
- [x] Cancellation before, during and after an answer is safe and idempotent (srs.fr.11.11) —
  `ReadOnlyAssistantTest.Cancel*`
- [x] Versioned user-local chats round-trip messages, citations and data categories and reject bad
  data safely (srs.fr.11.12) — `ChatStoreTest.*`

## Performance (ip.nfr.1.*) — reference hardware `[TBD]`

- [ ] ≥ 60 FPS average with up to 200 entities, normal mode (1.1)
- [ ] ≥ 600 simulation steps/s headless, demo game (1.2)
