# RL-Based Automated Game Testing Literature Review

Status: **research findings** for PB-062. Statements about Aegis's future playtest
agent are `[PROPOSED]`; they do not override accepted decisions in
[`decision-log.md`](../history/decision-log.md). Section 6 (AI-bridge context) was added on
2026-11-22. Product information was checked
against public vendor documentation on 2026-11-04 and is not an independent
validation of vendor claims.

## 1. Scope

PB-062 asks how reinforcement-learning agents have been used to find game bugs,
which approaches have evidence behind them, what current tools expose to game
teams, and what Aegis should preserve in its design. The three required papers
cover complementary approaches:

1. goal-directed deep RL that augments scripted testing [1];
2. curiosity-driven RL that maximizes spatial coverage [2]; and
3. evolutionary multi-objective search combined with DRL and explicit test
   oracles [3].

The papers do not show that RL itself recognizes every bug. The recurring pattern
is **agent exploration plus separate evidence or oracles**: the agent reaches
interesting states, while engine events, assertions, boundaries, process health,
or state-change checks determine whether a bug occurred.

## 2. Paper summaries

### 2.1 Bergdahl et al. (2020): RL augments scripted tests

**Question.** Can a learned navigation controller complement scripted and
navigation-mesh agents by reaching states that expose exploits and gameplay
problems? [1]

**Setup.** EA SEED used four isolated 3D FPS-style sandboxes:

- `Exploit`: a missing collision mesh creates an unintended shortcut;
- `Stuck Player`: five regions immobilize the player;
- `Navigation`: goals require jumping and climbing; and
- `Dynamic Navigation`: the navigation task also contains moving platforms.

The controller has continuous forward/backward, turn and strafe inputs plus a
jump action. It does not use a navigation mesh. Its normalized observation vector
contains relative goal position, velocity, rotation, goal distance, climbing and
ground-contact flags, jump cooldown, reset timer, and twelve ray casts. Reward is
positive when the agent moves toward a goal and equally negative when it moves
away. PPO was selected after comparison with SAC, TD3 and DDPG. [1]

**Findings.** The scripted navigation-mesh agent followed only expected paths;
the RL agent learned that walking through the wall shortened the route. Timeout
positions localized stuck areas that the navigation mesh did not cover. Early
training produced broad state visitation, while a trained policy converged on
near-optimal goal routes. The reported distributed setup evaluated about 10,000
environment interactions per second, but training still required roughly 50 to
1,000 million frames depending on task complexity. Time-to-learn also increased
with navigation difficulty. [1]

**Limits.** The environments are deliberately small, observations expose
privileged engine state, and the reward is goal-specific. Training time as a
proxy for human difficulty is only a hypothesis; no human-difficulty validation
is reported. A fully converged goal policy can also cover less space than its
earlier exploratory versions.

**Takeaway.** Use RL for isolated tasks where free movement, exploration or
unintended shortcuts are hard to script. Keep deterministic scripted tests for
known paths and assertions. This is the paper's production recommendation, not
just an Aegis inference. [1]

### 2.2 Gordillo et al. (2021): curiosity drives coverage

**Question.** Can intrinsic novelty reward make agents cover a complex map more
effectively than random exploration, and can the resulting data reveal design
problems? [2]

**Setup.** EA SEED trained 320 distributed PPO agents in a 500 m x 500 m x 50 m
3D map containing jumps, climbing and elevators. Actions match the earlier work.
The base observation has 37 values: position, velocity, rotation, movement-state
flags and twelve semantic ray casts; a second variant adds an 84 x 84 RGB
first-person image. No navigation mesh is used. [2]

The sole motivation is count-based curiosity. Position is discretized by a
distance threshold (5 m in the experiment). A new point receives the largest
reward; repeated visits reduce that reward toward zero. Agents can respawn from
previously visited states to expand the frontier. The random-policy baseline uses
the same respawn logic, so the comparison isolates the learned navigation policy.
[2]

**Coverage and evidence.** Coverage is the number of unique discretized spatial
states relative to an estimated 25,000 reachable points. Results are averaged
over five runs. The curious agents reached complex areas that random agents could
not and covered about 90% of the map in approximately 24 hours. Adding camera
input improved exploration, but increased representation and training cost. [2]

The collected trajectories and state buffer support several developer views:

- heat maps and paths for visited/unvisited areas;
- an exploration boundary that exposes out-of-bounds trajectories;
- regions of interest for reachability checks;
- a directed connectivity graph for unexpected shortcuts and one-way paths;
- semantic region maps; and
- terminal-state outliers that identify repeated stuck locations.

These views found intentionally missing colliders and stuck zones, plus
unintentional corner-slope and physics-launch exploits. [2]

**Limits.** Spatial position is only one dimension of game state. The 5 m
threshold is empirical and changes the meaning and cost of coverage. Ninety
percent coverage is not proof that the remaining area is unreachable, and the
study does not measure false-positive rates or compare against skilled human
testers.

**Takeaway.** A coverage policy needs an explicit state abstraction, a baseline,
multiple runs, stored trajectories, and developer-facing visualizations. For
Aegis, semantic state coverage must complement 2D position coverage because
inventory, collision, goal and rule states can differ at the same position.

### 2.3 Zheng et al. (2019): Wuji combines progress and exploration

**Question.** How can automated agents both progress through difficult combat
scenarios and keep exploring diverse states where bugs may be hidden? [3]

**Bug study and oracles.** Wuji begins with 1,349 bugs from four NetEase games:
145 crashes, 65 stuck bugs, 488 logical bugs, 12 balance bugs and 639 experience
bugs. It defines four corresponding high-priority oracles:

1. **Crash:** monitor process termination.
2. **Stuck:** compare the current state with a moving average of recent states.
3. **Logical:** execute game-specific assertions.
4. **Balance:** compare performance with designer-defined thresholds and require
   designer confirmation.

The evaluation covers the first three; balance detection is discussed but not
evaluated. [3]

**Method.** Each policy receives two fitness objectives: mission/winning reward
and number of explored states. Evolutionary selection, crossover and mutation
maintain a population of diverse neural policies. DRL improves each selected
policy through gradient-based learning, while Pareto-style multi-objective
selection avoids collapsing the whole population onto the best winning policy.
Tests and oracle checks run while policies are being trained. [3]

**Evaluation.** Wuji was tested on a block maze and two commercial online combat
games. The commercial scenarios contained 28 injected historical crash, stuck
and logical bugs. Compared with random testing and single-technique variants,
Wuji found more bugs and covered substantially more discretized game states in
the large games. Across ten rounds it also found three previously unknown bugs
confirmed by developers. Random testing remained competitive in the small maze
but performed poorly once progress through intermediate missions was required.
[3]

All approaches achieved the same line coverage within each game despite large
differences in bug detection. The authors therefore found state coverage more
informative than line coverage for this task. Each failure records actions and
relevant state, and a replay suite lets testers inspect candidates. [3]

**Limits.** Logical oracles still require domain assertions; stuck and balance
thresholds can cause false alarms. Results rely on proprietary games and
instrumented server-side scenarios without their GUIs. The full experiment cost
1,260 machine-hours, and only selected single-agent combat scenarios were tested.

**Takeaway.** Exploration alone is insufficient when bugs lie behind intermediate
goals. Progress and novelty should be separate objectives, and reproducible test
oracles should remain independent from the learning algorithm.

## 3. Cross-paper synthesis

| Topic | Evidence | Consequence for Aegis |
| --- | --- | --- |
| RL's role | RL reaches states that scripts or random actions miss [1]-[3]. | Add learned policies alongside scripted/random baselines, not as their replacement. |
| Goal vs coverage | Goal reward enables progress [1], curiosity broadens visitation [2], and Wuji optimizes both [3]. | Keep goal and coverage objectives separately measurable; do not hide both in one unexplained score. |
| Coverage | Spatial buffers expose navigation problems [2]; state coverage tracks bugs better than line coverage [3]. | Define 2D spatial, semantic-state, action and event coverage. Line coverage remains a code-test metric, not the playtest objective. |
| Oracles | Crashes, stuck states, logical assertions and boundaries are checked outside the policy [2], [3]. | Keep bug detectors deterministic and policy-independent. An agent supplies trajectories, not verdicts. |
| Reproduction | Wuji records actions/state and replays failures [3]. | A bug report needs seed, policy/config version, action trace, frames, entities and telemetry evidence. |
| Scale | The studies depend on parallel simulation and still consume substantial training time [1]-[3]. | Headless speed and parallel environments are prerequisites; training cost must be reported separately from inference cost. |
| Human review | Difficulty and balance claims require interpretation; oracle thresholds can be biased [1], [3]. | Report candidates with evidence and confidence; do not silently turn an anomaly into a confirmed defect. |

## 4. Industry and engine-tool review

The table separates learned-agent platforms from conventional automation. Vendor
claims are summarized as documented, not treated as measured results.

| Tool | Automation model and integration | Claimed detections / uses | Developer output | Relevance to Aegis |
| --- | --- | --- | --- | --- |
| modl.ai | Current product is a hosted, black-box visual/OCR agent: upload a build, define natural-language tasks, and send simulated inputs without an SDK. It supports dashboard and CI-triggered runs. [4] | Crashes, softlocks, broken menus, visual/missing-asset problems, performance drops and gameplay-logic issues. It currently emphasizes mobile and structured/UI-heavy games; fast timing-critical play is a stated limitation. [4] | Pass/fail results and reports with descriptions, severity, screenshots, video, logs and performance metrics. | Shows the value of low-integration external tests and rich evidence, but a black-box visual approach cannot replace Aegis's deterministic ECS/event oracles. |
| Unity ML-Agents | Open-source Unity SDK plus Python training APIs; supports PPO, SAC, imitation learning, environment randomization and concurrent environments. [5] | Intelligent control, automated build testing and pre-release design evaluation. It is an agent-training toolkit rather than a complete bug oracle/reporting product. | Training statistics, policies and user-built environment instrumentation. | Useful reference for the observation/action/reward contract and parallel environments, but it is Unity-specific and not a dependency candidate for the C++ engine. |
| Unreal Learning Agents | Experimental Unreal plugin for reinforcement and imitation learning; exposes PPO and behavior cloning to game developers. [6] | Game-playing agents, learned NPC behavior and automated QA bots. | Engine-integrated training, replay and policy tooling; bug reporting must be built by the user. | Confirms that learned control can be an engine facility while QA semantics remain game-specific. It is not portable to Aegis. |
| GameDriver | Commercial Unity/Unreal agent embedded in the build; object hierarchy queries, recorded/authored inputs, direct replay, logs and CI/test-framework integration. [7] | Functional, regression, UI, multiplayer/device and exception checks defined by testers. Its core is deterministic automation, not exploratory RL. | Object explorer, input recorder, log viewer and export to .NET test frameworks. | Strong baseline for inspectable object-level automation and CI integration. Demonstrates why known regressions should stay scripted. |
| AltTester | Unity/Unreal SDK instruments the application and connects through WebSocket to desktop/server tooling; C#, Python, Java and Robot Framework clients author tests. [8] | Tester-defined object, UI and interaction assertions; not an autonomous RL bug finder. | Inspector, recorder and generated/maintained automation scripts. | Reinforces the need for stable identifiers, runtime introspection and language-neutral control, without prescribing Aegis's future process boundary. |

## 5. Implications for Aegis

The recommendations below are `[PROPOSED]`. They refine future Phase 11 work but
do not decide the demo game (Q-016). When this review was first written the
Python/C++ process boundary was still open (DEC-011); it has since been decided as
gRPC/protobuf behind `AgentBridge` (DEC-038, ADR-002). Section 6 builds on that
decision.

### 5.1 Proposed playtest kernel

```text
Scenario + seed
      |
      v
Action source -------------------------------+
  - scripted regression                       |
  - random baseline                           v
  - goal-seeking PPO                     Engine.Run(headless)
  - curiosity/coverage PPO                    |
      |                                       v
      +---------------- observations <--- ECS/events/telemetry
                                              |
                                              v
                                  deterministic bug oracles
                                              |
                                              v
                                  report + replay artifacts
```

1. **Preserve one action contract.** Keyboard, scripts and learned policies must
   provide the same abstract action stream already required by
   [`headless-testing.md`](../testing/headless-testing.md). Policies must not call
   Box2D, Raylib or gameplay systems directly.
2. **Start with two learned roles.** A goal-seeking PPO policy should unlock
   intermediate progress; a curiosity/coverage PPO policy should seek novel
   states. Run both, plus scripted and random baselines, before considering the
   evolutionary population used by Wuji.
3. **Expose structured observations first.** Position, velocity, contacts,
   gameplay flags and nearby semantic entities are cheaper and more explainable
   than pixels. Pixel observations can be an optional later black-box experiment,
   not the only interface.
4. **Keep oracles outside the policy.** Policies optimize exploration; bug
   detectors consume state/events and emit evidence. This makes the same oracle
   usable with every policy and prevents reward changes from redefining a bug.
5. **Store every candidate trajectory.** The minimum replay identity is build,
   scenario, seed, policy hash, observation schema, reward configuration and
   ordered actions. The report links this identity to frames, entities and
   telemetry.

### 5.2 Mapping to current bug classes

| Current class | Useful agent | Required oracle and evidence | Important limitation |
| --- | --- | --- | --- |
| `B-NO-COLLIDER` | Curiosity/coverage plus scripted level validation | Static `solid`-without-collider check; runtime boundary/collision violation; trajectory and involved entity | RL may reveal a path through missing geometry, but the static oracle should find the malformed entity faster. |
| `B-DOOR-NO-KEY` | Goal-seeking PPO to reach the door under varied inventory states | `DoorOpenedEvent` joined with the player's inventory and the door requirement at the same frame | Coverage cannot infer the intended rule; a domain assertion is mandatory, as Wuji found for logical bugs. |
| `B-GOAL-UNREACHABLE` | Goal-seeking and curiosity policies over multiple seeds | No goal event within budget, uncovered goal region, connectivity/reachability result, and repeated-run evidence | Failure to reach the goal is evidence, not proof of unreachability; distinguish agent weakness from a level defect. |
| `B-OUT-OF-BOUNDS` | Curiosity/coverage policy | Position outside declared bounds, boundary crossing frame, prior contacts and replay trajectory | Bounds must be explicit per scenario; unusual but permitted routes must not be false positives. |

Repeated terminal-state outliers also justify considering a future `B-STUCK`
class, but adding a product bug class requires a human decision. Until then,
stuck-state analysis should be reported as a research candidate.

### 5.3 Coverage and reporting metrics

Each policy should be compared under the same scenario, seed set and frame budget.
Report distributions across runs, not only the best run.

| Group | Minimum metrics |
| --- | --- |
| Exploration | unique 2D spatial bins; semantic-state bins; action and event coverage; regions of interest reached; connectivity edges discovered |
| Goal progress | goal success rate; intermediate objectives completed; frames to first goal; episode return, reported separately from coverage |
| Bug detection | candidates by class; confirmed bugs; time/frames to first candidate; reproduction rate; false-positive rate after review |
| Cost | training environment frames; training wall time and hardware; inference steps/s; parallel environment count; telemetry overhead |
| Reproducibility | build/commit; scenario/config hash; seed; policy and reward version; action trace; observation-schema version; telemetry path |

Spatial-bin size and semantic-state fields are part of the metric definition and
must be versioned. Changing them changes the coverage number. The proposed
telemetry schema in [`telemetry-and-events.md`](../design/telemetry-and-events.md)
already contains many of the required run and evidence fields, but it needs human
review before implementation.

### 5.4 Suggested evaluation sequence

1. Choose the demo game and define explicit scenario bounds, goals and semantic
   state fields.
2. Run scripted and random baselines; verify every oracle with injected bugs and
   a clean no-false-positive case.
3. Train a goal-seeking PPO policy and compare goal progress against random.
4. Train a curiosity PPO policy and compare state/region coverage against both.
5. Run each policy against every injected bug over multiple seeds; retain all
   replay artifacts.
6. Add multi-objective or evolutionary populations only if the two-policy setup
   demonstrably misses bugs behind conflicting progress/exploration objectives.

This sequence keeps the smallest useful vertical slice and makes the incremental
value of RL measurable.

## 6. AI-bridge context for autonomous testing

Status: **research findings and `[PROPOSED]` implications**, added 2026-11-22 in
response to the PR #25 review. The bridge transport is decided (gRPC/protobuf behind
`AgentBridge`, DEC-038). Everything below about what the bridge *carries* is a
proposal; changing the wire contract needs its own decision entry.

### 6.1 Why the bridge matters for testing

The papers in section 2 all gave their agents **privileged engine state**: relative
goal position, contact flags and semantic ray casts [1], [2], or instrumented
server-side game state [3]. Each team hand-built that state for one game, and in
every case the oracles were separate, game-specific code. The commercial tools in
section 4 sit at the other end. modl.ai sees only pixels and OCR [4], and
GameDriver and AltTester see an object hierarchy without meaning [7], [8].

Aegis can do something none of them do: the engine already describes itself. The
custom ECS registers every component with a description, units and field layout,
every system declares what it reads and writes, every event is typed and numbered by
tick, and runs are deterministic (`ecs-api-design.md` §11, §12, §16;
`telemetry-and-events.md`). The AI-bridge layer (`AgentBridge`, plus the Semantic
Graph it can send) can therefore hand a testing agent **general, structured context
for any game built in the editor**, not a per-game feature vector. The research
question is which parts of that context make the agent find more bugs, faster and
more reproducibly.

### 6.2 What context the bridge can provide

| Tier | Context | Source in Aegis | Status (2026-11-22) |
| --- | --- | --- | --- |
| C0 | Per-tick observation bytes and one action id | `AgentBridge::Exchange(Observation) -> Action` in `proto/agent_bridge.proto` | **Implemented** (the only tier in code); payload is opaque bytes |
| C1 | Structured per-tick state: the components `AgentObservationSystem` declares it reads | `ecs-api-design.md` §16, item 2 (D-25) | **Already proposed** in the ECS design; built later than Phase 1 |
| C2 | Session schema: component names, fields, units, descriptions, systems with `reads`/`writes`, event types | `Schema` record (§12), sent once per session (§16) | **Already proposed**: the schema record is decided Phase 1 telemetry (DEC-042) and §16 already sends it once per session over the bridge. Only the action mask is new here |
| C3 | Event stream: typed events with sequence number, tick, phase, publisher, entities and payload | Event bus and `event` telemetry records | **Events already decided** (Phase 1 telemetry, DEC-042); sending them over the bridge to the agent is **new** here |
| C4 | Semantic Graph: static structure (`System —writes→ Component`) plus tick-stamped runtime edges (`collidedWith`, `triggered`) | `semantic-layer.md` §2 | **Graph already planned** (roadmap phase 10; only `structureForAssistant()` exists, DEC-067); using it for test goals and a playtest query call is **new** here |
| C5 | Episode control: seed, `World::snapshot()`/`restore()`, input-only actions, deterministic replay | `ecs-api-design.md` §16 (D-24); DEC-005 | **Already proposed** (snapshot/restore for episode reset); the reset/restore RPCs are **new** here |

Only C0 is in code today. C1, C2 and C5 were already proposed in the ECS design
(`ecs-api-design.md` D-24, D-25, §16; the D-choices still wait for team acceptance in
PB-010.3), and the schema and event records are decided telemetry (DEC-042). What this
section adds is using them **for testing**, plus a few new contract pieces: the action
mask, events over the bridge, episode-control RPCs and a graph query (section 6.6).

### 6.3 How each tier can improve autonomous testing

| Tier | Testing use | Evidence | Expected gain for Aegis | Caveat |
| --- | --- | --- | --- | --- |
| C1 + C2 structured state with schema | A generic observation encoder built from the schema, so the same agent code works on any scene without hand-picked features | Privileged structured state made PPO training feasible in [1], [2]; pixels improved exploration but at higher cost [2]; LLM playtesters work better when the board is converted into structured input [12] | No per-game feature engineering; smaller payloads than pixels; observations are explainable because each value has a name and unit | The agent sees things a human cannot. That is fine for bug finding but invalidates "human difficulty" claims [1] |
| C2 valid-action mask | Send which abstract actions are currently meaningful (e.g. "interact" only near an `Unlockable`) | Invalid action masking keeps a valid policy gradient and scales as the invalid action space grows, while penalising invalid actions does not [9] | Fewer wasted steps, faster training on interaction-heavy scenes | A wrong mask hides bugs that need "invalid" input; keep an unmasked baseline |
| C3 event stream | (a) Rewards: goal reward from `Objective`/`Unlocked` events instead of hand-coded distance. (b) Exploration: count novelty over (event type, semantic state) cells, not only 5 m spatial bins. (c) Oracles: feed the same events to deterministic bug checks | Count-based spatial curiosity reached ~90% map coverage [2]; Go-Explore showed that domain-knowledge cell representations sharply improve hard exploration [10]; curiosity conditioned on known trajectories focused testing near intended routes in 3D maps [17]; Wuji found state coverage more informative than line coverage [3] | Event and semantic-state coverage can reach logic bugs that sit at an already-visited position (door with and without key) | Cell definitions change the metric; version them (section 5.3) |
| C4 Semantic Graph | (a) Test targeting: find every `Unlockable` and its required item, every `Objective`, every obstacle without a collider, and turn them into goals or scenarios. (b) Change impact: after an edit, follow `writes`/`reads` edges to re-test only affected rules. (c) Triage: attach the causal chain (event → system → component change) to each bug candidate | Goals generated from game scenarios let synthetic agents match human testers on 45 injected bugs [11]; a knowledge graph of game elements and causal relations let an LLM tester locate update-affected functions and finish tests in fewer steps [14]; state abstraction is a core component of a deployed LLM MMORPG tester [13] | Goal-seeking PPO gets goals for free from the graph; regression runs focus on what changed; reports explain *why* | Graph quality depends on declared reads/writes being complete; undeclared access stops the run by design, which helps |
| C5 snapshot, seed, determinism | "Return then explore": restore an archived state instead of replaying from the start; exact replay of every candidate | Go-Explore's key step is returning to archived states before exploring [10]; respawning from visited states extended the frontier in [2]; Wuji relied on replay to confirm bugs [3] | Cheaper exploration of deep states; every bug is reproducible from (build, scene, seed, snapshot, actions) | Determinism must hold across agent runs; already a design rule (R-6, DEC-005) |
| C3 + C4 for the assistant | The read-only assistant diagnoses a candidate from cited ticks and entities (ip.fr.11.3) | Feeding raw game logs straight to an LLM did not identify logic and balance bugs well, while a structured two-stage method did [15]; zero-shot LLMs picked the buggy event in textual sequences about 70% of the time [16] | Bug reports explained in plain language, with evidence a developer can check | ~70% is not oracle quality: the LLM explains, deterministic oracles decide |

### 6.4 Where an LLM can use the context (optional layer)

A language model is not needed to play or to judge. Where it can help is
**planning and explaining** from C2 to C4 context:

1. Read the schema and graph and propose test goals in words ("open the door with
   an empty inventory", "reach the goal without the key"), which become goals or
   reward settings for the goal-seeking PPO policy.
2. After a run, summarise the candidates and their causal chains for the
   developer, citing ticks and entities.

This keeps the existing rules: the assistant is read-only (ip.fr.11.6, DEC-067),
uses the user's own provider (DEC-045), and sends no telemetry or source outside the
machine unless the user allows that data category. RL training and the oracles stay
local and do not need an LLM at all.

### 6.5 Bug classes and the bridge context they need

Codes follow `telemetry-and-events.md` §4; section 5.2 used the earlier `B-*` names.

| Bug class | Context that helps find it | How the context is used |
| --- | --- | --- |
| `missing-collider` | C2 (which components imply a collider), C4 (entities with `RigidBody`/`Collectible`/`Unlockable`/`Objective` or tag `Obstacle`) | A static graph query flags the entity before any agent runs; C1 + C3 then confirm a pass-through at runtime with a replay |
| `unlock-without-item` | C3 (`Unlocked` payload), C4 (required-item relation), C2 action mask | The graph yields the goal "unlock with the item missing"; goal-seeking PPO tries it under varied inventories; the oracle checks only the payload (srs.fr.9.5) |
| `objective-not-reached` | C3 (`Objective` events), C4 (path and connectivity edges), C5 (multi-seed restarts) | Graph-derived goals plus restore-and-explore separate "agent too weak" from "level defect" across seeds |
| `out-of-bounds` | C1 (position), C2 (map bounds), C3 (contacts before the crossing) | Curiosity over spatial and contact cells pushes the edges; the oracle reports the crossing tick and prior contacts |

### 6.6 `[PROPOSED]` additions to the bridge contract

The current proto is deliberately narrow (`Observation.payload` bytes, one
`action_id`). To deliver C1 to C5 without per-component code generation
(`ecs-api-design.md` §16), the bridge task could add:

1. **A session handshake** that sends the `Schema` once (C2), so Python decodes
   observation bytes by name and layout.
2. **An action mask** next to each observation (C2), as a packed bit field.
3. **Events since the last exchange** (C3), either inside the observation or on a
   separate server-streaming telemetry RPC, so a slow consumer does not block the
   tick loop.
4. **Episode control RPCs** (C5): reset with a seed, restore a snapshot id, report
   the replay identity.
5. **A graph query or export call** (C4) for the playtest planner, read-only like
   `structureForAssistant()`.

Payload size must stay in the range PB-065 measured: a 256 B observation and a
64 KiB one both stayed under 1.7 ms p99 with gRPC at 8 clients on Windows
([`cpp-python-bridge.md`](cpp-python-bridge.md)). A compact structured observation
for the small test scene (player, wall, door, key) should fit well under 1 KiB per
tick; events are sparse. Pixels would not.

### 6.7 How to show the bridge's value: a context ablation

The strongest evidence for Aegis's unique feature is an **ablation** on the demo
game's injected bugs. Keep the policy, scenario, seed set and frame budget fixed and
add one context tier at a time:

| Run | Context | What it tests |
| --- | --- | --- |
| A | Position and ray casts only (the [1], [2] baseline) | Baseline |
| B | A + C1/C2 structured state, schema and action mask | Does generic structured state match or beat hand-built features? |
| C | B + C3 events for reward, novelty cells and oracles | Does event coverage find logic bugs that spatial coverage misses? |
| D | C + C4 graph-derived goals | Do graph goals find `unlock-without-item` and `objective-not-reached` sooner? |
| E | D + C5 restore-and-explore | Does returning to archived states cut frames to first bug? |

Report the section 5.3 metrics for each run, plus bytes per exchange and bridge
time per tick. If C to E show no gain over B, the extra contract is not worth
building yet.

### 6.8 Risks and limits

- **Privileged information.** Agents that know the goal and the rules find bugs
  efficiently but do not measure human difficulty [1]. Keep "bug finding" and
  "playtest feel" results apart.
- **Garbage in.** The graph is only as good as the declared reads/writes and
  component descriptions. The access check that stops undeclared access is what
  keeps it honest.
- **Over-fitting to the schema.** A policy trained on one schema version may break
  when components change; version the observation schema with the policy.
- **Overhead.** Streaming events every tick costs telemetry time (limit: 10% of
  tick time, ip.nfr.1.3). Measure before adding a second stream.
- **Evidence quality.** Several sources here ([12]-[14]) are recent preprints or
  single-team results; treat their numbers as indicative only.

## 7. References

[1] J. Bergdahl, C. Gordillo, K. Tollmar, and L. Gisslén, “Augmenting
automated game testing with deep reinforcement learning,” in *2020 IEEE
Conference on Games (CoG)*, 2020, pp. 600-603, doi:
[10.1109/CoG47356.2020.9231552](https://doi.org/10.1109/CoG47356.2020.9231552).

[2] C. Gordillo, J. Bergdahl, K. Tollmar, and L. Gisslén, “Improving
playtesting coverage via curiosity driven reinforcement learning agents,” in
*2021 IEEE Conference on Games (CoG)*, 2021, pp. 1-8, doi:
[10.1109/CoG52621.2021.9619048](https://doi.org/10.1109/CoG52621.2021.9619048).

[3] Y. Zheng *et al.*, “Wuji: Automatic online combat game testing using
evolutionary deep reinforcement learning,” in *2019 34th IEEE/ACM International
Conference on Automated Software Engineering (ASE)*, 2019, pp. 772-784, doi:
[10.1109/ASE.2019.00077](https://doi.org/10.1109/ASE.2019.00077).

[4] modl.ai, “Let AI find game bugs before your players do.” Accessed:
Nov. 4, 2026. [Online]. Available: [https://modl.ai/](https://modl.ai/)

[5] Unity Technologies, “Unity ML-Agents Toolkit.” Accessed: Nov. 4, 2026.
[Online]. Available:
[https://github.com/Unity-Technologies/ml-agents](https://github.com/Unity-Technologies/ml-agents)

[6] Epic Games, “Learning Agents.” Accessed: Nov. 4, 2026. [Online].
Available:
[https://dev.epicgames.com/documentation/unreal-engine/API/PluginIndex/LearningAgents](https://dev.epicgames.com/documentation/unreal-engine/API/PluginIndex/LearningAgents)

[7] GameDriver, “GameDriver Test Assistant.” Accessed: Nov. 4, 2026.
[Online]. Available:
[https://kb.gamedriver.ai/gamedriver-test-assistant](https://kb.gamedriver.ai/gamedriver-test-assistant)

[8] Altom, “AltTester Unreal SDK overview.” Accessed: Nov. 4, 2026.
[Online]. Available:
[https://alttester.com/docs/unreal-sdk/latest/pages/overview.html](https://alttester.com/docs/unreal-sdk/latest/pages/overview.html)

[9] S. Huang and S. Ontañón, “A closer look at invalid action masking in
policy gradient algorithms,” in *Proc. 35th Int. FLAIRS Conf.*, 2022, doi:
[10.32473/flairs.v35i.130584](https://doi.org/10.32473/flairs.v35i.130584).

[10] A. Ecoffet, J. Huizinga, J. Lehman, K. O. Stanley, and J. Clune, “First
return, then explore,” *Nature*, vol. 590, pp. 580-586, 2021, doi:
[10.1038/s41586-020-03157-9](https://doi.org/10.1038/s41586-020-03157-9).

[11] S. Ariyurek, A. Betin-Can, and E. Surer, “Automated video game testing
using synthetic and humanlike agents,” *IEEE Trans. Games*, vol. 13, no. 1,
pp. 50-67, 2021, doi:
[10.1109/TG.2019.2947597](https://doi.org/10.1109/TG.2019.2947597).

[12] Y. Zhao and C. Tang, “Towards LLM-based automatic playtest,” in *FSE
Companion '25*, 2025, doi:
[10.1145/3696630.3728700](https://doi.org/10.1145/3696630.3728700).
Preprint: [arXiv:2507.09490](https://arxiv.org/abs/2507.09490).

[13] C. Wang, L. Tang, M. Yuan, J. Yu, X. Xie, and J. Bu, “Leveraging LLM
agents for automated video game testing,” arXiv:2509.22170, 2025. [Online].
Available: [https://arxiv.org/abs/2509.22170](https://arxiv.org/abs/2509.22170)

[14] E. Mu, J. Cai, Y. Lu, M. Zhang, K. Tei, and J. Li, “Knowledge
graph-enhanced large language model for incremental game playtesting,”
arXiv:2511.02534, 2025. [Online]. Available:
[https://arxiv.org/abs/2511.02534](https://arxiv.org/abs/2511.02534)

[15] C. Jin *et al.*, “Automatic bug detection in LLM-powered text-based games
using LLMs,” in *Findings of the Association for Computational Linguistics: ACL
2024*, 2024, pp. 15353-15368, doi:
[10.18653/v1/2024.findings-acl.907](https://doi.org/10.18653/v1/2024.findings-acl.907).

[16] M. R. Taesiri, F. Macklon, Y. Wang, H. Shen, and C.-P. Bezemer, “Large
language models are pretty good zero-shot video game bug detectors,”
arXiv:2210.02506, 2022. [Online]. Available:
[https://arxiv.org/abs/2210.02506](https://arxiv.org/abs/2210.02506)

[17] A. Sestini, L. Gisslén, J. Bergdahl, K. Tollmar, and A. D. Bagdanov,
“Automated gameplay testing and validation with curiosity-conditioned
proximal trajectories,” *IEEE Trans. Games*, vol. 16, no. 1, pp. 113-126,
2024, doi: [10.1109/TG.2022.3226910](https://doi.org/10.1109/TG.2022.3226910).

## Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-11-04 | Created for PB-062: three-paper review, industry comparison and proposed Aegis implications | codex/gpt-5/pb062 |
| 2026-11-22 | Section 6 added after the PR #25 review: how AI-bridge (`AgentBridge`) context improves autonomous testing, context tiers, `[PROPOSED]` contract additions and an ablation plan; section 5 note updated for DEC-038; references [9]-[17] | claude (PR #25 thread) |
