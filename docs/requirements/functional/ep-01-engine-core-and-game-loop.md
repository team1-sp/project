---
epic: EP-01
group: srs.fr.1
part: engine
---

# EP-01 Engine Core and Game Loop

**Epic:** As a game developer, I want to run my game on a predictable loop of render frames and simulation ticks in either a window or headless, so that the simulation behaves the same way every time I run it.

These requirements refine ip.fr.1.1 to ip.fr.1.4. They use the two time units defined in the Conventions: the *simulation tick*, one fixed update of 1/60 s of simulated time, and the *render frame*, one drawing and presentation cycle that runs zero or more ticks.

## US-1.1 Render frames and simulation ticks

**Story:** As a game developer, I want the engine to run every simulation tick in a fixed phase order, so that systems always see data produced by the earlier phases.

- **srs.fr.1.1** In normal mode, the Engine shall execute each render frame in this order: (1) collect keyboard input from the operating system, (2) run the number of simulation ticks given by srs.fr.1.3, each as defined in srs.fr.1.12, (3) render the state left by the last completed tick, (4) present the image.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.1
- **srs.fr.1.2** The Engine shall advance the simulation only in simulation ticks of exactly 1/60 s of simulated time each, independent of the render frame rate.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.2
- **srs.fr.1.3** In normal mode, while the game loop is not paused, the Engine shall in each render frame run one simulation tick for each whole 1/60 s of real time that has elapsed and not yet been simulated, up to a maximum of 5 ticks per render frame, and carry any part of a tick that remains over to the next render frame.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.2
- **srs.fr.1.4** When more than 5 ticks are due in one render frame, the Engine shall run 5 ticks, drop the real time of the further whole ticks, carry the remaining part of a tick over to the next render frame, and log the warning "Simulation is running slower than real time" to the Console at most once per second.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.2
- **srs.fr.1.5** The Engine shall keep a tick counter that is 0 when a run starts and increases by 1 after every completed simulation tick; the tick number of a tick is the counter value at the start of that tick.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.2
- **srs.fr.1.6** The Engine shall report the simulated time of a run as the tick counter multiplied by 1/60 s.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.2

## US-1.2 Run modes and lifecycle

**Story:** As a game developer, I want to choose between a windowed run and a headless run, so that I can watch the game or test it at full speed.

- **srs.fr.1.7** The Engine shall provide two run modes: normal mode, which opens a window and runs render frames, and headless mode, which runs simulation ticks without a window and without render frames.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.3, ip.fr.1.4
- **srs.fr.1.8** The Engine shall start in the run mode selected in the Editor's mode switch (srs.fr.17.19) or, for a built game, in the mode given by the command-line option `--headless`, whose run settings are set by the options of srs.fr.7.33.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.4
- **srs.fr.1.9** When the Engine runs as a standalone game in normal mode (a built game started without `--headless`, Build and Run, or a game started outside the Editor), it shall open an application window whose size and fullscreen state are taken from the project's Rendering settings (srs.fr.27.23); a built game started with `--headless` shall open no window and run as stated in srs.fr.7.1 and srs.fr.7.2; in Play mode inside the Editor, the game shall render in the Game view at its Resolution ▾ size (srs.fr.20.2, srs.fr.20.3).
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.3
- **srs.fr.1.10** When the user closes the game window in normal mode, the Engine shall finish the current render frame, release every physics, rendering and file resource it holds, and exit with exit code 0.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.3
- **srs.fr.1.11** When a run stops because of an unrecoverable error, such as a failed precondition, an access check (srs.fr.2.126) or an invariant violated by a system, for example an invalid entity handle or a missing component in its own world access, which every build shall check, the Engine shall write the error message and the current tick number to the log; in Play mode the Editor shall then pause at that tick boundary as Error Pause does (srs.fr.24.6), whatever the Error Pause setting, with Pause and Step disabled until Stop; a headless run shall end with the outcome Error, and a headless or built game process shall exit with a non-zero exit code. Invalid data read from files, recordings or the agent are recoverable and follow srs.nfr.5.10.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.3, ip.fr.1.4

## US-1.3 Simulation tick phases

**Story:** As a game developer, I want every simulation tick to run the same phases in the same order, so that the game behaves the same in normal and headless mode.

- **srs.fr.1.12** The Engine shall execute each simulation tick in this fixed order of tick phases: (1) input application, (2) gameplay input processing, (3) physics synchronization-in, run by the PhysicsSyncIn system, (4) physics step, which holds no systems, (5) physics synchronization-out and collision event collection, run by the PhysicsSyncOut system (srs.fr.3.63), (6) gameplay rules, (7) telemetry recording.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.1.13** Every ECS system, the Physics Adapter, event dispatch and telemetry recording shall run per simulation tick; keyboard input collection, camera follow and bounds (srs.fr.4.7, srs.fr.4.8), rendering, presentation and the refresh of Editor panels shall run per render frame.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.1.14** In the input application phase, the Engine shall give the tick the input action state of its input source (srs.fr.5.11), built from action transitions by the rules of srs.fr.5.6; in normal mode the transitions come from the keyboard queue of srs.fr.5.5.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.1, ip.fr.1.2
- **srs.fr.1.15** When a render frame runs no simulation tick, the Engine shall still render and present the state of the last completed tick, without interpolating between ticks.
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.1, ip.fr.1.2

## US-1.4 Pause, resume and step

**Story:** As a game developer, I want to pausing never to make the game catch up or warn about slowdowns, so that pause and step are exact.

- **srs.fr.1.16** A pause request, from Pause (srs.fr.17.12) or Error Pause (srs.fr.24.6), shall take effect at the next tick boundary: no further tick shall start, the whole ticks of time still in the accumulator shall be discarded, and the remaining fraction of a tick shall be kept.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1
- **srs.fr.1.17** While paused, the Engine shall add no elapsed real time to the accumulator.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1
- **srs.fr.1.18** On resume, the Engine shall measure elapsed real time from the moment of resuming and continue from the kept fraction, so that no paused time is simulated and no "Simulation is running slower than real time" warning is caused by the pause.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1

## US-1.5 Tick numbers and counts

**Story:** As a game developer, I want to tick numbers and tick counts never to be confused, so that reports, progress and citations agree.

- **srs.fr.1.19** A tick number shall identify one executing tick and start at 0 (srs.fr.1.5); the ticks executed and the length of a run or recording shall be the number of completed ticks, so that a run with N ticks executed completed ticks 0 to N − 1 and its last tick number is N − 1; reports, progress displays, recording markers, replay and citations shall use tick numbers for ticks and ticks executed for counts.
  - Use cases: UC-06; Initial Plan: ip.fr.1.1
- **srs.fr.1.20** A run that ends before tick 0 completes, for example when a start check fails (srs.fr.9.31), shall report 0 ticks executed and no last tick number, shown as "—".
  - Use cases: UC-06; Initial Plan: ip.fr.1.1
