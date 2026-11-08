# RL-Based Automated Game Testing Literature Review

Status: **research findings** for PB-062. Statements about Aegis's future playtest
agent are `[PROPOSED]`; they do not override accepted decisions in
[`decision-log.md`](../history/decision-log.md). Product information was checked
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
do not decide the Python/C++ process boundary (DEC-011 is still deferred) or the
demo game (Q-016).

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

## 6. References

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

## Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-11-04 | Created for PB-062: three-paper review, industry comparison and proposed Aegis implications | codex/gpt-5/pb062 |
