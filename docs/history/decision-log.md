# Decision Log

**Append-only. This log is the single source of truth.** Every architectural, technological, scope or process decision, with rationale, so nobody has to re-argue it. Where any other document (including the course documents under `senior-project/docs/`) disagrees with an `Accepted` entry here, this log wins and the other document must be updated (project docs) or flagged for humans (course docs). Template: [../agents/templates/decision-entry.md](../agents/templates/decision-entry.md). Procedure: [../agents/playbooks/record-decision.md](../agents/playbooks/record-decision.md).

Statuses: `Proposed` (suggested; binds nobody) · `Accepted` (binding) · `Rejected` · `Superseded by DEC-###`.
**Humans decide; agents consult.** An entry is `Accepted` only because a human decided it (see `Decided by`). Agents may write `Accepted` solely to record an explicit human decision. To reverse a decision, add a new entry that supersedes it.

## Index

| ID | Decision | Status | Date |
| --- | --- | --- | --- |
| DEC-001 | Three-layer architecture with one-way dependencies | Accepted | 2026-10-12 (extracted) |
| DEC-002 | ECS is the central data hub; gameplay decisions live in gameplay systems | Accepted | 2026-10-12 (extracted) |
| DEC-003 | Physics is accessed only through a Physics Adapter | Accepted | 2026-10-12 (extracted) |
| DEC-004 | Fixed physics timestep 1/60 s and fixed frame order | Accepted | 2026-10-12 (extracted) |
| DEC-005 | Headless mode is a core requirement | Accepted | 2026-10-12 (extracted) |
| DEC-006 | MVP is 2D; architecture must not block 3D | Superseded by DEC-016 | 2026-10-12 (extracted) |
| DEC-007 | Graphics framework is accessed through an abstraction | Accepted | 2026-10-12 (extracted) |
| DEC-008 | Telemetry is recorded every frame inside the loop | Accepted | 2026-10-12 (extracted) |
| DEC-009 | Adopt this docs structure and agent protocol | Accepted | 2026-10-12 |
| DEC-010 | MCP is an optional later interoperability layer, not the first integration | Proposed | 2026-10-12 |
| DEC-011 | AI orchestration in Python, engine-facing contract via gRPC/protobuf | Superseded by DEC-038 (protocol) and DEC-039 (Python) | 2026-10-12 |
| DEC-012 | Graphics framework selection is decided by the shader benchmark | Superseded by DEC-015 | 2026-10-12 (extracted) |
| DEC-013 | Programming language is C++ | Accepted | 2026-10-13 |
| DEC-014 | Physics library is Box2D (behind the Physics Adapter) | Accepted | 2026-10-13 |
| DEC-015 | Graphics framework is Raylib (behind the rendering abstraction) | Accepted | 2026-10-13 |
| DEC-016 | Product scope is 2D only; 3D is future work, architecture keeps it possible | Accepted | 2026-10-13 |
| DEC-017 | Decision authority: humans decide, agents consult; decision log is the single truth; course docs are not authoritative | Accepted | 2026-10-13 |
| DEC-018 | ECS is custom-built (so components can be LLM-instructive via the semantic layer) | Accepted | 2026-10-13 |
| DEC-019 | C++20 (its platform part was decided in DEC-029) | Accepted | 2026-10-13 |
| DEC-020 | Box2D version is v3.1.1 (C API) | Accepted | 2026-10-13 |
| DEC-021 | Raylib version is v6.0 | Superseded by DEC-033 | 2026-10-13 |
| DEC-022 | One repository for code and engineering docs; `team1-sp/docs` holds course deliverables only | Accepted | 2026-10-15 |
| DEC-023 | Build system CMake, test framework GoogleTest | Accepted | 2026-10-15 |
| DEC-024 | Işık Dönger reviews every PR; area owners are second code owners | Accepted | 2026-10-15 |
| DEC-025 | AI agents push only after a human approves; humans approve and merge every PR | Accepted | 2026-10-15 |
| DEC-026 | The semantic layer lives in its own top-level folder, separate from engine and ai-services | Accepted | 2026-10-15 |
| DEC-027 | Trunk-based git workflow; branch protection deferred, rules kept as a team agreement | Accepted | 2026-10-15 |
| DEC-028 | GitHub issues are created from ClickUp tasks with ClickUp's GitHub integration | Accepted | 2026-10-15 |
| DEC-029 | Platforms: develop on Windows and macOS (Windows priority); the engine application and the development tool must run on both; games are built for Windows only for now | Accepted | 2026-10-15 |
| DEC-030 | Compilers: whatever each platform needs; none pinned now | Accepted | 2026-10-15 |
| DEC-031 | Coding standards, Definition of Done and clang-format 19 accepted | Accepted | 2026-10-23 |
| DEC-032 | Only Işık Dönger and the advisor approve PRs; Işık's own PRs need no approval; `main` ruleset on | Accepted | 2026-10-23 |
| DEC-033 | Raylib version is 6.2 | Accepted | 2026-10-24 |
| DEC-034 | The product name is Aegis | Accepted | 2026-10-24 |
| DEC-035 | PR titles put the PB id in the Conventional Commit scope: `<type>(PB-<nnn>): <description>` | Accepted | 2026-10-24 |
| DEC-036 | World units are meters with y up; 32 pixels per meter; conversion only in the renderer | Accepted | 2026-11-01 |
| DEC-037 | The PB-008 spike in `sandbox/` may link Box2D and raylib directly (exception to R-3/R-4 for that spike only) | Proposed | 2026-11-01 |
| DEC-038 | ADR-002: the C++ to Python bridge is gRPC/protobuf behind AgentBridge | Accepted | 2026-11-04 |
| DEC-039 | `ai-services` uses Python 3.12 with pinned Python project tooling | Accepted | 2026-11-04 |
| DEC-040 | The physics body owns velocity; the ECS Velocity component mirrors it and carries pending writes | Accepted | 2026-11-05 |
| DEC-041 | The ECS and the Physics Adapter meet only in two declared systems, PhysicsSyncIn and PhysicsSyncOut | Accepted | 2026-11-05 |
| DEC-042 | Pre-implementation ECS gaps settled: push events, tick vocabulary, Phase 1 components, input, time, RNG, scene, ids, errors, telemetry records | Accepted | 2026-11-05 |
| DEC-043 | About 20 % of the whole project is built this semester; the ECS builds its Phase 1 slice | Accepted | 2026-11-05 |
| DEC-044 | Coding standards amended for the ECS design: naming, tags, declarations, errors, RNG | Accepted | 2026-11-05 |
| DEC-045 | No own LLM is built; users supply their own model provider and key; only free-tier or local use is guaranteed | Accepted | 2026-11-06 |
| DEC-046 | PB-011: the window lives in `rendering-raylib` behind `IWindow`; the loop reaches the game through `ISimulation` and `IFrameRenderer`; the leak check is one guarded MSVC call | Proposed | 2026-11-10 |
| DEC-047 | PB-012: the game loop follows srs.fr.1.2 to 1.6 (at most 5 ticks per render frame) with an exact integer accumulator and the tick counter in `engine/core` | Proposed | 2026-11-10 |
| DEC-049 | PB-025: the physics interface is named `IPhysicsAdapter`; bodies are addressed by `BodyId` with `findBody(EntityId)`; `Vec2` and `EntityId` live in `engine/core` | Accepted | 2026-11-11 |

### Decisions still open (no entry yet — do not decide silently)

what "LLM-instructive components" means concretely (Q-013, deferred by the human) · license · GitHub access, visibility and branch protection (see Q-014) · reference hardware for the performance targets · telemetry file format (Proposed in design doc, not yet decided). See `agents/state/open-questions.md`.

---

### DEC-001 — Three-layer architecture with one-way dependencies
- **Date:** 2026-10-12 (extracted from Senior Project Guide; confirm — Q-004)
- **Status:** Accepted
- **Proposed by:** team (guide) — recorded by claude-code/sonnet-5.5/012r
- **Context:** The product's value is that the engine exposes its internals to AI. Needs separation so each part is testable alone.
- **Options considered:** (1) monolith with AI bolted on; (2) three layers Engine / Semantic Architecture / AI Assistant.
- **Decision:** Three layers, lowest to highest: Engine (physics, ECS, rendering, UI) → Semantic Architecture (data, graph, relationships) → AI Assistant (architecture reader, LLM model, user assistant). A layer depends only on the layer(s) below it.
- **Rationale:** AI-native design; engine remains usable without AI; AI layer replaceable.
- **Consequences:** Engine emits data/events/telemetry but never imports semantic/AI code (rule R-1). AI Assistant is read-only toward the engine (ip.fr.11.6).
- **Revisit when:** a requirement forces upward calls from the engine.

### DEC-002 — ECS is the central data hub; gameplay decisions live in gameplay systems
- **Date:** 2026-10-12 (extracted)
- **Status:** Accepted
- **Context:** Physics, rendering and gameplay all need the same game objects.
- **Decision:** Data-oriented ECS: entities are IDs, components are pure data, systems hold logic, a World owns them, queries select entities. Physics and rendering do not communicate directly — only via ECS components and events. Whether a door opens is a gameplay-system decision, not a physics decision (ip.fr.6.6).
- **Rationale:** Enables headless mode, telemetry, and a clean semantic graph (entity *has* component, system *reads/writes* component).
- **Consequences:** Rules R-2, R-7, R-8. Systems must declare reads/writes (feeds ip.fr.10.2).

### DEC-003 — Physics is accessed only through a Physics Adapter
- **Date:** 2026-10-12 (extracted)
- **Status:** Accepted
- **Context:** Box2D (2D, C/C++) and BEPUphysics (3D, C#) are candidates; library not chosen.
- **Decision:** A Physics Adapter exposes `CreateBody, DestroyBody, SetBodyPosition, SetLinearVelocity, ApplyForce, ApplyImpulse, Raycast, Step, GetCollisionEvents` (ip.fr.3.1), maintains the entity↔body mapping (ip.fr.3.4), writes positions back to `PositionComponent` after each step (ip.fr.3.5) and translates collisions to `CollisionEvent` (ip.fr.3.6).
- **Rationale:** Swappable library; protects the 2D→3D path.
- **Consequences:** Rule R-3. No library types outside the adapter.

### DEC-004 — Fixed physics timestep 1/60 s and fixed frame order
- **Date:** 2026-10-12 (extracted; ip.fr.1.1, ip.fr.1.2)
- **Status:** Accepted
- **Decision:** Per frame: input → gameplay input processing → physics synchronisation → physics step (fixed 1/60 s) → collision event processing → gameplay rules → telemetry → rendering → present. Rendering FPS is independent.
- **Rationale:** Determinism, reproducible tests (ip.fr.7.3).
- **Consequences:** Rules R-5, R-9. Changing the order requires a new decision.

### DEC-005 — Headless mode is a core requirement
- **Date:** 2026-10-12 (extracted; ip.fr.1.4, ip.fr.7.*)
- **Status:** Accepted
- **Decision:** The full simulation (input, ECS, physics, gameplay, telemetry) runs without window or rendering calls, accepts an agent's input stream, and yields the same results as normal mode for identical inputs.
- **Rationale:** Automated playtesting, bug detection, telemetry for the semantic graph. Listed as core, not optional, in the guide.
- **Consequences:** Rule R-6; tests run headless.

### DEC-006 — MVP is 2D; architecture must not block 3D
- **Date:** 2026-10-12 (extracted)
- **Status:** Superseded by DEC-016 (clarified: 2D only, 3D future)
- **Decision:** Build a working 2D prototype first. Do not invest in 3D or advanced rendering until it works. Keep abstractions (adapter, render layer) so 3D is possible later.
- **Consequences:** Rule S-6. Technology comparisons use "3D expansion potential" as one criterion among others, not as the driver.

### DEC-007 — Graphics framework is accessed through an abstraction
- **Date:** 2026-10-12 (extracted from guide §6.6)
- **Status:** Accepted
- **Decision:** The engine is not directly coupled to a specific graphics framework (MonoGame / raylib / SFML / OpenGL); only the rendering layer touches it.
- **Consequences:** Rule R-4.

### DEC-008 — Telemetry is recorded every frame inside the loop
- **Date:** 2026-10-12 (extracted; ip.fr.8.*)
- **Status:** Accepted
- **Decision:** Per frame: frame number, position/velocity of entities, collision/door/key/goal events with frame numbers, frame time and physics step time; exportable in a documented machine-readable format; switchable by configuration; ≤ 10% overhead (ip.nfr.1.3).
- **Consequences:** `design/telemetry-and-events.md` defines the schema.

### DEC-009 — Adopt this docs structure and agent protocol
- **Date:** 2026-10-12
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager), who requested the docs system on 2026-10-12
- **Context:** Several humans and several AI models/tools will work on the project and need shared memory.
- **Decision:** Use `project/docs/` as described in its README, with append-only `history/` logs and the `agents/` coordination folder. (Authority rules refined by DEC-017.)
- **Consequences:** All agents follow `agents/AGENT-PROTOCOL.md`.

### DEC-010 — MCP is an optional later interoperability layer, not the first integration
- **Date:** 2026-10-12
- **Status:** Proposed (from earlier research conversation; not yet team-approved)
- **Context:** MCP could expose project tools to external AI clients. The core assistant must be proven first.
- **Decision (proposed):** Build order: local script → retrieval/graph queries → structured checks → engine↔AI integration → optional MCP server.
- **Rationale:** Avoid spending time on protocol plumbing before the core works.
- **Revisit when:** Phase 12 (AI Assistant Layer) starts.
- **Amendments:** 2026-10-15 — Işık Dönger: too early to decide; stays Proposed, agents must not raise it until a human does.

### DEC-011 — AI orchestration in Python; engine-facing contract via gRPC/protobuf
- **Date:** 2026-10-12
- **Status:** Superseded by DEC-038 (engine-facing protocol) and DEC-039 (Python for `ai-services`)
- **Context:** The guide says the assistant "may use an LLM via gRPC to a Python AI service" (ip.fr.11.8).
- **Decision (proposed):** Keep model orchestration/experiments in Python; define a strongly-typed protobuf contract between engine-side exporter and AI service. The engine side is C++ (DEC-013); the AI service language is still open.
- **Consequences:** Privacy rule ip.nfr.2.1 (no source/telemetry leaves the machine unless enabled).
- **Amendments:** 2026-10-15 — Işık Dönger: too early to decide; stays Proposed, agents must not raise it until a human does. 2026-11-04 — DEC-039 accepts Python 3.12 for `ai-services`; only the proposed gRPC/protobuf engine-facing contract remains open and deferred. 2026-11-04 — PB-065 benchmarked the options; DEC-038 (Proposed) and ADR-002 recommend gRPC/protobuf behind `AgentBridge`. This entry stays Proposed until a human decides DEC-038. 2026-11-04 — Işık Dönger accepted DEC-038, so both parts of this proposal are now decided and this entry is superseded.

### DEC-012 — Graphics framework selection is decided by the shader benchmark
- **Date:** 2026-10-12 (extracted from Initial Plan §4.2 and `shader-test/test-guideline.md`)
- **Status:** Superseded by DEC-015 (the framework was chosen: Raylib)
- **Decision:** The same scene is rendered with OpenGL, Raylib and SFML; the choice uses the benchmark criteria (visual fidelity, shader integration, code complexity, FPS, engine fit). Established as a decision criterion with the advisor.
- **Consequences:** TASK-001 blocks TASK-002. See `testing/benchmark-protocol.md`.

---

### DEC-013 — Programming language is C++
- **Date:** 2026-10-13
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager), on behalf of the team `[confirm the whole team agrees]`  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** "Tech stack is decided. It's C++, Box2D, Raylib, 2d only with 3d expandibility in the future."
- **Context:** Options were C++ and C# (.NET) (guide §6.1; Q-001). The shader benchmark and Initial Plan §3 already framed the work as systems-level C++.
- **Options considered:** C++ (performance, control, native fit with Box2D and Raylib; manual memory management) vs C# (easier, GC; needs bindings for Box2D). C++ chosen.
- **Decision:** The engine is written in C++.
- **Rationale:** `[rationale not given beyond the decision — ask if needed]`. Natural fit with Box2D and Raylib (both native), avoids bindings.
- **Consequences:** C++ coding standards apply (C# sections removed); GC-related pitfalls obsolete; ownership/lifetime rules matter (entities are IDs, not pointers). Exact C++ standard, compilers, platforms and build system still open (Q-007, Q-009).
- **Docs updated:** `architecture/tech-stack.md`, `guidelines/coding-standards.md`, `guidelines/repository-structure.md`, `troubleshooting/common-errors.md`, `troubleshooting/environment-setup.md`, `agents/rules.md`, `agents/model-compatibility.md`, `agents/state/*`, `design/*`, `testing/*`, `README.md`.
- **Course documents affected (for humans):** Initial Plan "Required Software" and any text saying "C++ or C#".
- **Supersedes / Related:** answers Q-001; unblocks TASK-003.
- **Amendments:** 2026-10-13 — C++ standard and platforms: proposed only, see DEC-019. 2026-10-13 — Işık Dönger confirmed the team agrees on C++.

### DEC-014 — Physics library is Box2D (behind the Physics Adapter)
- **Date:** 2026-10-13
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager) `[confirm team]`  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** see DEC-013.
- **Context:** Box2D vs BEPUphysics (guide §6.2). With C++ and 2D-only scope, Box2D fits.
- **Decision:** Use Box2D. It is still accessed only through the Physics Adapter (DEC-003, R-3); Box2D types never leave the adapter implementation.
- **Rationale:** `[not given — ask if needed]`. 2D-focused, native C/C++, static/kinematic/dynamic bodies, raycast, joints, independent of rendering.
- **Consequences:** BEPUphysics no longer a candidate. **Open:** which Box2D major version (v2.4.x C++ API vs v3.x C API) — it changes adapter code and lifetime/handle handling (Q-008). The adapter keeps the swap-ability path to a 3D library later.
- **Docs updated:** as DEC-013 (notably `design/physics-adapter-design.md`).
- **Course documents affected (for humans):** Initial Plan text listing Box2D/BEPUphysics alternatives.
- **Related:** DEC-003, DEC-016, TASK-003, Q-008.
- **Amendments:** 2026-10-13 — version pinned: see DEC-020 (v3.1.1).
- **Amendments:** 2026-10-13 — Işık Dönger confirmed the team agrees (C++, Box2D and 2D-only).

### DEC-015 — Graphics framework is Raylib (behind the rendering abstraction)
- **Date:** 2026-10-13
- **Status:** Accepted
- **Decided by:** the whole team, reported by Işık Dönger (Project Manager)  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** see DEC-013; team agreement confirmed 2026-10-13 ("The whole team agreed on raylib").
- **Context:** DEC-012 selected the framework via the OpenGL / Raylib / SFML shader benchmark.
- **Decision:** Use Raylib. Only the rendering layer touches it (DEC-007, R-4).
- **Rationale:** best compromise of ease of use and functionality; the benchmark showed no notable differences, so code complexity and ease of use decided (see Amendments).
- **Consequences:** Benchmark no longer decides anything; keep results as evidence. Raylib needs a window/GL context for texture loading and drawing, so **headless mode must never initialise Raylib** and simulation must use asset *IDs*, not loaded textures (see `testing/headless-testing.md`). PRB-001 (wrap mode) remains relevant.
- **Docs updated:** as DEC-013 (notably `testing/benchmark-protocol.md`, `architecture/engine-layer.md`).
- **Course documents affected (for humans):** Initial Plan text mentioning "Raylib or MonoGame".
- **Supersedes / Related:** DEC-012, DEC-007; TASK-001, TASK-002.
- **Amendments:** 2026-10-13 — rationale given by Işık Dönger: the whole team agreed on Raylib because it offered the best compromise of ease of use and functionality. The shader benchmark finished with no notable difference between frameworks (as expected), so the team decided on code complexity and ease of use instead. Version: see DEC-021.

### DEC-016 — Product scope is 2D only; 3D is future work, architecture keeps it possible
- **Date:** 2026-10-13
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager) `[confirm team]`  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** see DEC-013 ("2d only with 3d expandibility in the future").
- **Decision:** Everything built is 2D. No 3D features are implemented. The architecture (Physics Adapter, rendering abstraction, ECS) must not make a later 3D extension impossible; this is a design constraint, not work.
- **Consequences:** Rule S-6 now means: no 3D work, not even "basic 3D cube" experiments, unless a human decides otherwise. Stretch items "3D" are removed from the plan; 3D is documented as future work.
- **Docs updated:** as DEC-013 (notably `agents/rules.md`, `process/roadmap.md`).
- **Course documents affected (for humans):** Initial Plan text implying 2D/3D engine ("modular, data-driven 2D/3D").
- **Supersedes / Related:** DEC-006.
- **Amendments:** 2026-10-13 — Işık Dönger confirmed the team agrees (C++, Box2D and 2D-only).

### DEC-017 — Decision authority: humans decide, agents consult; decision log is the single truth; course docs are not authoritative
- **Date:** 2026-10-13
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** all uncertainty must be clearly stated to the human; the decision made after that conversation is "the one-truth"; everything else must be updated to the new decision; same when disagreeing with a decision; course documents (initial plan) "do not hold the absolute truths"; decisions made by the human, consulted with the agent, "need to be accepted and logged as the truth".
- **Decision:** (1) Agents never resolve a material uncertainty themselves; they surface it to the human with options. (2) The human's decision is logged `Accepted` and becomes the truth. (3) All project documents are updated to match (propagation). (4) Agents disagreeing with a decision say so; the old decision binds until the human changes it. (5) `senior-project/docs/` course documents are not authoritative and are not edited by agents; conflicts are listed for humans.
- **Consequences:** `AGENT-PROTOCOL.md` rewritten (Truth Rule, Raising uncertainty, Disagreeing); new playbook `record-decision.md`; rules P-3/P-4/S-3 revised; decision template gains fields; requirements `ip.*` are a reference snapshot.
- **Docs updated:** `agents/AGENT-PROTOCOL.md`, `agents/rules.md`, `agents/README.md`, `agents/communication-protocol.md`, `agents/model-compatibility.md`, `agents/playbooks/*`, `agents/templates/decision-entry.md`, `guidelines/documentation-rules.md`, `process/development-process.md`, `README.md`, `history/decision-log.md`.
- **Related:** DEC-009.

---

### DEC-018 — The ECS is custom-built
- **Date:** 2026-10-13
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** "Custom ECS so we can make sure components are more llm instuctive with our semantics layer."
- **Context:** Q-012 / TASK-004. Options were a custom ECS or an existing C++ library (e.g. EnTT).
- **Options considered:** (A) custom — full control over component metadata and introspection; more work. (B) library — faster to start; must be wrapped to expose semantics. A chosen.
- **Decision:** Build our own ECS (entities as IDs, plain-data components, systems, World, queries). Components must be designed so the semantic layer can give an LLM clear, structured meaning about them ("LLM-instructive").
- **Rationale:** the semantic layer and AI assistant are the product's unique value; owning the ECS lets components carry the semantics the LLM needs.
- **Consequences:** `design/ecs-design.md` introspection is a requirement of the ECS itself (declared reads/writes, component and field descriptions). What "LLM-instructive" concretely means (field descriptions? units? ownership? human-readable names?) is **not yet decided** → Q-013. Custom ECS is also work to schedule (phase 3).
- **Docs updated:** `design/ecs-design.md`, `architecture/tech-stack.md`, `architecture/semantic-layer.md`, `agents/state/*`, `agents/rules.md`, `process/roadmap.md`.
- **Course documents affected (for humans):** none known (Initial Plan describes a custom ECS; check any mention of EnTT/Unity ECS).
- **Related:** DEC-002, Q-012, Q-013, TASK-004, TASK-010.

### DEC-019 — C++20; development on Windows and macOS; engine app and games target Windows only (for now)
- **Date:** 2026-10-13
- **Status:** Accepted (2026-10-15, see Amendments)
- **Decided by:** Işık Dönger (Project Manager) on 2026-10-15; first proposed 2026-10-13  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** "C++20, Development will be done on Windows and MacOS but engine app and games are for now only Windows targeted , Raylib v6.0, still open" — clarified later: "still open" covers the whole answer; these are propositions, not decisions.
- **Decision:** Language standard is C++20. (The platform statements in this entry were decided separately in DEC-029.)
- **Consequences:** Code must not require anything newer than C++20, and every compiler used on a supported platform (DEC-030) must support C++20. Platform questions are answered in DEC-029 and DEC-030.
- **Docs updated:** `guidelines/coding-standards.md`, `troubleshooting/environment-setup.md`, `architecture/tech-stack.md`, `agents/state/*`.
- **Course documents affected (for humans):** any platform statement in the Initial Plan's required software/hardware.
- **Related:** DEC-013, Q-007, Q-009.
- **Amendments:** 2026-10-15 — the platform part (develop on Windows and macOS, Windows-only game builds) was decided in DEC-029; only the C++20 standard remains proposed here. 2026-10-13 — recording error by claude-code/sonnet-5.5/012r: I first logged this as Accepted. Işık Dönger clarified that "still open" covered the entire Q-007 answer (C++20, platforms, Raylib v6.0). C++ itself (DEC-013) is unaffected. 2026-10-15 — Işık Dönger: "all decisions are decided" (2026-10-15, said after the open points were listed: C++20 and Raylib v6.0). C++20 is now Accepted.

### DEC-020 — Box2D version is v3.1.1 (C API)
- **Date:** 2026-10-13
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** "v3.1.1" (answer to Q-008: which Box2D version).
- **Decision:** Use Box2D v3.1.1, pinned. Not v2.4.x.
- **Consequences:** Box2D v3 is a C API using id handles (world/body/shape ids) rather than object pointers; many online tutorials target v2.4 and do not apply. The Physics Adapter's entity↔body mapping stores Box2D ids (wrapped in our own types). Check the pinned version's headers for exact names before writing adapter code. `[ASSUMPTION — v3 API details from memory; verify against v3.1.1 headers.]`
- **Docs updated:** `design/physics-adapter-design.md`, `troubleshooting/common-errors.md`, `guidelines/repository-structure.md`, `architecture/tech-stack.md`, `agents/state/*`.
- **Related:** DEC-014, Q-008.

### DEC-021 — Raylib version is v6.0
- **Date:** 2026-10-13
- **Status:** Superseded by DEC-033
- **Decided by:** Işık Dönger (Project Manager) on 2026-10-15; first proposed 2026-10-13 from their answer  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** "all decisions are decided" (2026-10-15, said after the open points were listed: C++20 and Raylib v6.0)
- **Context:** Q-007. `[ASSUMPTION — a Raylib v6.0 release exists and matches the API you expect; verify before pinning.]`
- **Decision:** Pin Raylib v6.0.
- **Consequences:** `[VERIFIED 2026-10-15]` Raylib 6.0 exists (GitHub release tag `6.0`, marked Latest). Still to do at TASK-005: pin the exact version in the build (Q-009) and check the API against the 6.0 headers/CHANGELOG — 6.0 changed fullscreen/High-DPI handling (High-DPI needs `FLAG_WINDOW_HIGHDPI`) and the model/skeletal-animation API, so 5.5-era tutorials may not match.
- **Docs updated:** `architecture/tech-stack.md`, `guidelines/repository-structure.md`, `troubleshooting/*`, `agents/state/*`.
- **Related:** DEC-015, Q-007.
- **Amendments:** 2026-10-15 — Işık Dönger asked to confirm v6.0; claude-code/sonnet-5.5/012r checked the Raylib GitHub releases page and release notes: 6.0 is the latest release (previous: 5.5). Observation only, not a decision: 6.0 adds a software renderer and a headless "Memory" platform backend, which may matter for headless mode (DEC-003 area) — to be evaluated when headless mode is designed. 2026-10-24 — pinned in `cmake/Dependencies.cmake` as git tag `6.0` (commit `dbc56a8`) by PB-002. Observation only: tag `6.2` also exists now; the pin stays at 6.0 unless a human decides otherwise. 2026-10-24 — superseded by DEC-033 (6.2), decided by Işık Dönger the same day.

### DEC-022 — One repository for code and engineering docs
- **Date:** 2026-10-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (session 0171)
- **Human's words:** "One repo, with docs only as course deliverables."
- **Context:** Backlog item PB-001.1 planned separate engine, ai-services and docs repositories. Cross-cutting changes (adapter interface + its users, the engine/Python contract) would then need several PRs merged in order.
- **Options considered:** (A) one repo with modules (CMake targets, Python packages) and per-area CI and owners; (B) one repo per part; (C) git submodules. A chosen.
- **Decision:** All code and the engineering docs live in `team1-sp/project`. `team1-sp/docs` holds course deliverables only.
- **Consequences:** Modularity comes from build targets, CODEOWNERS and path-filtered CI (`guidelines/repository-structure.md`). PB-001.1's acceptance criterion ("repos engine, ai-services and docs exist") needs updating in ClickUp.
- **Docs updated:** `guidelines/repository-structure.md`.
- **Related:** DEC-023, DEC-026.

### DEC-023 — Build system CMake, test framework GoogleTest
- **Date:** 2026-10-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (session 0171)
- **Human's words:** "CMake+GoogleTest is fine."
- **Context:** Q-009 (deferred until now). The ClickUp backlog (PB-002) already assumed CMake with FetchContent and GoogleTest.
- **Decision:** CMake (with presets) builds everything; third-party code comes in through FetchContent at pinned versions; C++ tests use GoogleTest.
- **Consequences:** Unblocks TASK-005 / PB-002 except for the still-open C++ standard and toolchain (Q-007).
- **Docs updated:** `guidelines/coding-standards.md`, `guidelines/repository-structure.md`, `agents/state/open-questions.md`.
- **Related:** Q-009, DEC-019, PB-002.
- **Amendments:** 2026-10-16 — the "still-open C++ standard and toolchain" in Consequences is resolved: C++20 Accepted (DEC-019), compilers per platform need (DEC-030), platforms (DEC-029). TASK-005 / PB-002 is no longer blocked by Q-007. 2026-10-24 — PB-002 pinned GoogleTest at `v1.17.0` (commit `52eb810`) in `cmake/Dependencies.cmake`; the version was chosen by the agent and confirmed by Işık Dönger the same day: "googletest 1.17.0 is fine". Box2D is pinned at `v3.1.1` (DEC-020).

### DEC-024 — Işık Dönger reviews every PR; area owners are second code owners
- **Date:** 2026-10-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (session 0171)
- **Human's words:** "I think I should be the reviewer for every pr because I am the project manager but we can discuss this." Owners: "physics+rendering adapter is Zeynep Ertürk, ecs+engine core Işık Dönger and Arda Karabulut, semantics layer is Elifnur Özçelik and Arda Sarıtaş".
- **Decision:** `.github/CODEOWNERS` makes Işık Dönger the default owner of every path and adds area owners as second owners: physics and rendering adapters Zeynep Ertürk; ECS and engine core Arda Karabulut; semantic layer Elifnur Özçelik and Arda Sarıtaş. One approval from any owner of a path counts.
- **Consequences:** Işık's own PRs and days he cannot review within 1 working day are covered by the area owner. Paths without an area owner (editor, game, ai-services, tooling) currently depend on Işık alone, and his own PRs there need a teammate's review by agreement. Revisit at the Increment 1 review.
- **Docs updated:** `guidelines/git-workflow.md`, `.github/CODEOWNERS`.
- **Related:** DEC-027.

### DEC-025 — AI agents push only after a human approves
- **Date:** 2026-10-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (session 0171)
- **Human's words:** "AI agents should only push from a human aprrovement."
- **Context:** Q-006.
- **Decision:** An AI agent pushes a branch only after a human approves that push. A human approves and merges every PR an agent opens. Agents never push to `main`.
- **Docs updated:** `guidelines/git-workflow.md`, `agents/state/open-questions.md`, `CONTRIBUTING.md`.
- **Related:** S-2, Q-006.

### DEC-026 — The semantic layer has its own top-level folder
- **Date:** 2026-10-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (session 0171)
- **Human's words:** "Semantic-layer should live in seperate from engine and ai-services if possible."
- **Decision:** `semantic/` is a top-level folder with its own package, tests and CI job. It reads only what the engine exports; `ai-services/` may use it; `engine/` never depends on it (DEC-001).
- **Consequences:** Language is still open; Python is assumed for the skeleton. If it becomes C++, `semantic/` becomes its own CMake target under the same rule.
- **Docs updated:** `guidelines/repository-structure.md`.
- **Related:** DEC-001, DEC-022.

### DEC-027 — Trunk-based git workflow; branch protection deferred
- **Date:** 2026-10-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (session 0171)
- **Human's words:** "for now we can defer branch-protection, I will talk with our instructor."
- **Context:** GitHub does not offer branch protection or rulesets for private repositories on the organization's free plan (the API answers "Upgrade to GitHub Pro or make this repository public"). Options: GitHub Team through GitHub Education, a public repo, or a team agreement.
- **Decision:** Trunk-based workflow with squash-merged PRs, PB-numbered branch names and Conventional Commit PR titles (`guidelines/git-workflow.md`). Until protection is available the rules are a team agreement, checked by CI (`ci-ok`) and the `Main guard` workflow.
- **Consequences:** Once the instructor's answer allows it, protection is switched on with the same rules; nothing else changes.
- **Docs updated:** `guidelines/git-workflow.md`, `CONTRIBUTING.md`.
- **Related:** DEC-024, PB-001.2.

### DEC-028 — GitHub issues are created from ClickUp tasks
- **Date:** 2026-10-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (session 0171)
- **Human's words:** "Clickup lets issue creation directly from a task. I think we will use that."; "pr should be created from clickup when a task is about to be started."
- **Decision:** When a task is started, the developer creates its GitHub issue from the ClickUp task with ClickUp's GitHub integration. One issue per top-level backlog item, titled like the task. A GitHub Action is planned to add the branch and a draft PR for each new `PB-` issue.
- **Consequences:** ClickUp stays the planning tool; setting the task to done after a merge is manual (ClickUp's automatic status changes are a paid feature).
- **Docs updated:** `CONTRIBUTING.md`.
- **Related:** DEC-027, PB-001.5.

### DEC-029 — Platforms: develop on Windows and macOS (Windows priority); games are built for Windows only for now
- **Date:** 2026-10-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** "our final decision is that project needs to be able to developed on windows and mac, windows being priority. the games need to be able to built for windows only for now. supports for other platforms and builds will be delayed."
- **Context:** Q-007; DEC-019 (platform part). Human asked whether other compilers/platforms are easy to add later.
- **Options considered:** (1) Windows only; (2) Windows and macOS development, Windows-only game builds; (3) also web or other game targets now.
- **Decision:** The project is developed on Windows and macOS, with Windows the priority (when they conflict, Windows wins). Games are built for Windows only for now. Support for other platforms and other game build targets (e.g. web) is delayed.
- **Rationale:** `[rationale not given beyond the words above]`
- **Consequences:** The engine and tooling must build and run on macOS too, at lower priority than Windows (confirmed by the human 2026-10-15, see Amendments); no work on other game targets now. Engine code must not depend on Windows-only or macOS-only APIs without asking. Compilers: DEC-030. The build system must support both OSes (Q-009, deferred).
- **Docs updated:** `architecture/tech-stack.md`, `guidelines/coding-standards.md`, `troubleshooting/environment-setup.md`, `agents/state/*`.
- **Course documents affected (for humans):** platform/hardware statements in the Initial Plan, if any.
- **Related:** DEC-019, DEC-013, Q-007, Q-009.
- **Amendments:** 2026-10-16 — renumbered by claude-code/sonnet-5.5/012r: recorded locally on 2026-10-15 as DEC-022, but `team1-sp/project` main had already used DEC-022…028; to avoid two meanings for one ID this entry became DEC-029. 2026-10-15 — Işık Dönger: "2 of our team members will be using mac for the project. so both the development tool and the engine application must run on mac." Rationale: two team members work on macOS. This confirms that the engine application and the development tool must run on macOS ("development tool" read by the recording agent as the engine's editor/dev tooling `[ASSUMPTION]`).

### DEC-030 — Compilers: whatever each platform needs; none pinned now
- **Date:** 2026-10-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** "compilers are whatever is needed based on the current platform needs"
- **Context:** Q-007 (compilers); DEC-029 (platforms).
- **Decision:** No compiler is pinned. Each platform uses the compiler that platform needs. Code must therefore compile with the compilers each supported platform needs, and must not rely on compiler-specific extensions.
- **Rationale:** Two team members use macOS and the rest Windows (DEC-029); the platform decides the toolchain.
- **Consequences:** The compiler and version per OS are written into `environment-setup.md` when TASK-005 sets up the build. Which compiler that is on each OS was not named by the human; do not assume (`[ASSUMPTION]` MSVC on Windows and Apple Clang on macOS — to be confirmed when the skeleton is built). The chosen C++ standard (DEC-019, Proposed) must be supported by all of them.
- **Docs updated:** `architecture/tech-stack.md`, `guidelines/coding-standards.md`, `troubleshooting/environment-setup.md`, `agents/state/*`, `process/roadmap.md`.
- **Course documents affected (for humans):** none known.
- **Related:** DEC-029, DEC-019, Q-007, Q-009.
- **Amendments:** 2026-10-16 — renumbered by claude-code/sonnet-5.5/012r: recorded locally on 2026-10-15 as DEC-023 (collision with the build-system decision on main).

---

### DEC-031 — Coding standards, Definition of Done and clang-format 19 accepted
- **Date:** 2026-10-23
- **Status:** Accepted
- **Decided by:** the team, approved by the advisor; reported by Işık Dönger (Project Manager)  **Recorded by:** claude-code (session 0171)
- **Human's words:** "everything in PB-005 is team decided and advisor approved."
- **Context:** PB-005 (coding standards and Definition of Done). The formatter was the last open item in `coding-standards.md`.
- **Decision:** `guidelines/coding-standards.md` and `process/definition-of-done.md` are accepted as written on 2026-10-23. C++ is formatted with clang-format 19 using the repo's `.clang-format` (LLVM base, 4-space indent, 100 columns, braces always inserted); other files follow `.editorconfig`. The DoD is pinned in the PR template.
- **Rationale:** `[rationale not given beyond the words above]` clang-format is the formatter the standards already named; the version is pinned because different clang-format versions format the same code differently.
- **Consequences:** CI fails a PR with unformatted C++ files. Everyone installs the same version (`pip install clang-format==19.1.7`). Changing a rule needs a PR and a new decision entry. The style options in `.clang-format` were picked by the recording agent to match `.editorconfig` `[ASSUMPTION — change them in a PR if the team wants different ones]`.
- **Docs updated:** `guidelines/coding-standards.md`, `process/definition-of-done.md`, `troubleshooting/environment-setup.md`, `architecture/tech-stack.md`, `CONTRIBUTING.md`, `.github/pull_request_template.md`.
- **Course documents affected (for humans):** none known.
- **Related:** DEC-019, DEC-023, PB-005.

### DEC-032 — Only Işık Dönger and the advisor approve PRs; Işık's own PRs need no approval
- **Date:** 2026-10-23
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (session 0171)
- **Human's words:** "make it so that my personal pr's or issues do not require a review? but only for me, and everyone else requires a review specifically from me or our advisor."
- **Context:** DEC-024 made area owners second code owners, so their approval counted. DEC-027 deferred branch protection because the repo was private on the free plan; the repo is now public, so rulesets are available.
- **Decision:** The only code owners are Işık Dönger (`@isikdonger`) and the advisor, Leyla Sezer (`@leylasezer196`). A ruleset on `main` requires a PR, one code-owner approval, squash merge, a green `ci-ok`, and blocks force pushes and deletion. Repository admins and organization owners may bypass it, only through a PR; that is how Işık merges his own PRs without review. Area teammates stay listed in CODEOWNERS as people to ask, without approval rights. Issues have no review step, so nothing changes for issues.
- **Rationale:** `[rationale not given beyond the words above]`
- **Consequences:** Supersedes the "area owners are second owners" part of DEC-024 and the "protection deferred" part of DEC-027. Every PR from another member waits for Işık or the advisor. Anyone else who is a repository admin or organization owner can also bypass `[ASSUMPTION — Işık is the only one; check Settings > Collaborators]`. The advisor needs write access for their approval to count.
- **Docs updated:** `.github/CODEOWNERS`, `CONTRIBUTING.md`, `guidelines/git-workflow.md`.
- **Course documents affected (for humans):** none known.
- **Related:** DEC-024, DEC-027.

### DEC-033 — Raylib version is 6.2
- **Date:** 2026-10-24
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (PB-002 session)
- **Human's words:** "raylib 6.2 came after we decided so it can be updated to v6.2 since we havent started yet"
- **Context:** PB-002 pinned raylib 6.0 (DEC-021) and noted that tag `6.2` now exists. No engine code uses raylib yet, so moving costs nothing.
- **Options considered:** (1) stay on 6.0; (2) move to 6.2 before any rendering code exists. 2 chosen.
- **Decision:** Pin raylib 6.2 (git tag `6.2`, commit `0256645`) in `cmake/Dependencies.cmake`.
- **Rationale:** 6.2 is newer and nothing has been built on 6.0 yet.
- **Consequences:** API examples must match 6.2. The 6.0 notes in DEC-021 (High-DPI flag, fullscreen, software renderer, headless "Memory" backend) still need checking against 6.2 when rendering and headless work start.
- **Docs updated:** `cmake/Dependencies.cmake`, `architecture/tech-stack.md`, `guidelines/coding-standards.md`, `guidelines/repository-structure.md`, `troubleshooting/environment-setup.md`, `troubleshooting/common-errors.md`, `process/roadmap.md`, `agents/state/*`.
- **Course documents affected (for humans):** any place that names the raylib version.
- **Supersedes / Related:** supersedes DEC-021; DEC-015, PB-002.

### DEC-034 — The product name is Aegis
- **Date:** 2026-10-24
- **Status:** Accepted
- **Decided by:** the team, reported by Işık Dönger (Project Manager)  **Recorded by:** claude-code (PB-002 session)
- **Human's words:** "project name is Aegis decided by the team."
- **Context:** Q-005 (open since 2026-10-12): needed for namespaces, binary names and repo naming.
- **Decision:** The product is called Aegis. The CMake project is `aegis`.
- **Rationale:** `[rationale not given beyond the words above]`
- **Consequences:** C++ namespaces (`engine::`), target names (`engine_*`) and the repository name (`team1-sp/project`) are unchanged; renaming any of them is a separate decision `[ASSUMPTION — ask before renaming]`.
- **Docs updated:** `CMakeLists.txt`, `README.md`, `agents/state/open-questions.md`, `agents/state/current-status.md`, `agents/state/task-board.md`.
- **Course documents affected (for humans):** the Initial Plan and other deliverables that name the product.
- **Related:** Q-005.

### DEC-035 — PR titles put the PB id in the Conventional Commit scope
- **Date:** 2026-10-24
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code (PB-002 session)
- **Human's words:** "pr title should be how it is now. with requirement right after the tag." Then chose `ci(PB-002): Create the CMake…` when asked which format that meant.
- **Context:** `guidelines/git-workflow.md` used the area label as the scope and put the PB id at the end (`feat(phys): … (PB-025)`); the Issue start workflow wrote `feat: … (PB-025)`.
- **Options considered:** (1) `ci(PB-002): Create …`; (2) `ci: PB-002 Create …`; (3) keep the id at the end. 1 chosen.
- **Decision:** PR titles, and so the squash commits on `main`, are `<type>(PB-<nnn>): <description>`. The area stays visible through the `area:*` labels. Bugs without a PB item were not discussed; they keep the old form `fix(game): … (#142)` until a human says otherwise `[ASSUMPTION]`.
- **Rationale:** `[rationale not given beyond the words above]`
- **Consequences:** `tools/start_task.py` writes draft PR titles in this form. The CI title check (`amannn/action-semantic-pull-request`) accepts any scope, so no workflow change is needed.
- **Docs updated:** `tools/start_task.py`, `guidelines/git-workflow.md`, `CONTRIBUTING.md`.
- **Supersedes / Related:** replaces the scope rule in DEC-027's workflow; DEC-028.

### DEC-036 — World units are meters with y up; 32 pixels per meter; conversion only in the renderer
- **Date:** 2026-11-01
- **Status:** Accepted
- **Decided by:** Zeynep Ertürk (owner of ClickUp PB-008.3)  **Recorded by:** claude (PB-008 session)
- **Human's words:** chose "32 px per meter" with "the world is in meters with Y pointing up (Box2D's way), and the conversion to pixels with Y pointing down happens only in the renderer" when asked which convention the PB-008 prototype should use.
- **Context:** TASK-008 / PB-008.3. Box2D works in meters and is tuned for objects of about 0.1–10 m, with y up; raylib draws in pixels with y down (E-004, E-005, E-025).
- **Options considered:** 32, 50 or 100 pixels per meter, all with a y-up meter world converted at the render boundary. 32 chosen. 64 was then tried in the spike and compared on screen: in the 800 x 450 window it shows only 12.5 x 7 m, so the box starts and ends off-screen unless a camera follows it. Zeynep kept 32.
- **Decision:** Simulation, ECS components and gameplay use meters, kilograms, seconds and radians, y up, angles counter-clockwise. The renderer converts with 32 pixels per meter, flips y and negates angles. Nothing else converts.
- **Rationale:** 32 is the value the PB-008 task suggested; a 1 m object is 32 px and an 800 x 450 window shows 25 m x 14 m, inside Box2D's preferred size range. `[further rationale not given]`
- **Consequences:** `PositionComponent` is in meters. The camera (where the world origin is on screen, zoom) is still open; the spike's bottom-left origin is not a decision.
- **Docs updated:** `components.md` (new), `research/box2d.md`, `design/physics-adapter-design.md`, `design/ecs-design.md`, `guidelines/coding-standards.md`, `troubleshooting/common-errors.md`, `agents/state/*`.
- **Course documents affected (for humans):** none known.
- **Revisit when:** the rendering abstraction or the camera is designed.
- **Supersedes / Related:** TASK-008, PB-008, DEC-014, DEC-015.

### DEC-037 — The PB-008 spike in `sandbox/` may link Box2D and raylib directly
- **Date:** 2026-11-01
- **Status:** Proposed
- **Proposed by:** Zeynep Ertürk (chose this option for PB-008.2)  **Recorded by:** claude (PB-008 session)
- **Context:** PB-008.2 asks for a Box2D + raylib prototype in `sandbox/` before the Physics Adapter exists. R-3/R-4 allow Box2D only in `engine/physics-box2d/` and raylib only in `engine/rendering-raylib/`.
- **Options considered:** (1) a separate spike target in `sandbox/` that links both libraries directly, marked as research code; (2) split the spike across the two engine targets, which starts building the adapter early. 1 chosen by Zeynep.
- **Decision (proposed):** `sandbox_falling_box` (links Box2D) and `box2d_raylib_spike` (links raylib) may break R-3/R-4. Box2D stays in `falling_box_scene.cpp` and raylib in `box2d_raylib.cpp`; no engine target uses them. The exception covers this spike only.
- **Consequences:** needs confirmation from Işık Dönger (architecture rules); asked in Q-017 and in the PB-008 pull request.
- **Supersedes / Related:** R-3, R-4, DEC-003, DEC-007, PB-008, Q-017.

### DEC-038 — ADR-002: the C++ to Python bridge is gRPC/protobuf behind AgentBridge
- **Date:** 2026-11-04
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager), on PR #23  **Proposed by:** Codex, from PB-065 evidence  **Recorded by:** claude (PR #23 review)
- **Human's words:** "I accept dec-38"
- **Context:** The headless C++ engine needs synchronous observation/action exchange with Python RL services and later assistant-facing services. PB-065 benchmarked gRPC and ZeroMQ with equivalent protobuf payloads and concurrent clients.
- **Options considered:** gRPC/protobuf; ZeroMQ via cppzmq/pyzmq with protobuf, MessagePack or FlatBuffers; shared memory; raw sockets; and Python bindings. See `research/ipc.md`.
- **Decision:** Start with gRPC/protobuf behind `AgentBridge` for fault-isolated service communication. Permit an optional pybind11 local-RL adapter if end-to-end profiling justifies it. Keep communication dependencies out of the default engine build.
- **Rationale:** Both transports meet the measured latency needs. ZeroMQ scaled better at 4–8 clients; gRPC's contracts, deadlines, status semantics and tooling reduce integration and maintenance risk. FlatBuffers did not beat protobuf on ZeroMQ; pybind11 established a much faster in-process ceiling but trades away isolation and remote access.
- **Consequences:** The first production bridge task pins gRPC and protobuf and generates code from a `.proto` contract; engine code depends only on `engine::bridge`. ZeroMQ and FlatBuffers stay benchmark evidence, not production dependencies. The Windows build of gRPC has not been tried yet and is the main risk (see Revisit when).
- **Docs updated:** `history/adr-002-cpp-python-communication.md`, `research/ipc.md`, `research/cpp-python-bridge.md`, `architecture/tech-stack.md`, `architecture/ai-assistant-layer.md`, `agents/state/current-status.md`, `agents/state/plan.md`, `agents/state/handoff-log.md`, `README.md`.
- **Course documents affected (for humans):** any course document that names the engine-to-AI-service protocol or still treats it as open (the guide only says the assistant "may use an LLM via gRPC", ip.fr.11.8).
- **Revisit when:** an end-to-end RL benchmark shows bridge overhead is material; concurrency exceeds 8 local instances; or Windows deployment cost changes the tradeoff.
- **Supersedes / Related:** supersedes the protocol part of DEC-011; related to DEC-005, DEC-013, DEC-019, DEC-029 and PB-065.
- **Amendments:** 2026-11-04 — Windows evidence (recorded by claude, PR #23 review): with MSVC 14.44 and vcpkg the bridge builds and passes after the PRB-004 fixes; the first vcpkg install took about 22 minutes. ZeroMQ had 3.4–5.5x gRPC's throughput at 8 clients there (macOS: about 2.5x). Packaging did not block gRPC, so the "Windows deployment cost" revisit trigger has not fired; the throughput gap is the number to watch in the end-to-end RL benchmark.

### DEC-039 — `ai-services` uses Python 3.12 with pinned Python project tooling
- **Date:** 2026-11-04
- **Status:** Accepted
- **Decided by:** Arda Saritas (PB-006 owner)  **Recorded by:** codex/gpt-5/pb006
- **Human's words:** "proceed with 3.12"
- **Context:** PB-006 / GitHub issue #20 requires a Python `ai-services` skeleton with a pinned Python version, Ruff and pytest. No accepted entry named the Python minor version; HND-013 explicitly treated Python 3.12 as an assumption pending PB-006.
- **Options considered:** Python 3.12 or 3.13. The agent recommended 3.12 for mature AI-library compatibility and alignment with the existing CI job; Arda selected 3.12.
- **Decision:** `ai-services` uses Python 3.12. The package metadata accepts `>=3.12,<3.13`, and `.python-version` records `3.12`. PB-006 also pins its required project tools: setuptools 84.2.1 (MIT, build backend, about 2.1 MB release), Ruff 0.17.3 (MIT, linter, platform wheel about 9–11 MB) and pytest 9.2.0 (MIT, test framework, about 2.2 MB release). These were current stable releases verified on PyPI on 2026-11-04.
- **Rationale:** A single supported minor version makes local and CI environments reproducible. Ruff and pytest are explicit PB-006 requirements; setuptools supplies the standard `src/`-layout build backend. All three projects are actively maintained and support Python 3.12.
- **Consequences:** Python CI and local setup use Python 3.12 and install the package's `dev` extra. This accepts only the Python language/runtime part of DEC-011; the engine-facing protocol remains Proposed and deferred.
- **Docs updated:** `ai-services/README.md`, `architecture/tech-stack.md`, `guidelines/repository-structure.md`, `troubleshooting/environment-setup.md`, `agents/state/*`.
- **Course documents affected (for humans):** any software-environment section that names the AI-service Python version.
- **Supersedes / Related:** accepts the language/runtime part of DEC-011; PB-006, GitHub issue #20, Q-018.
- **Amendments:** 2026-11-04 — renumbered from DEC-038 by Işık Dönger while merging `main` into the PB-065 branch, because both branches had created a DEC-038. Citations in project docs were updated to DEC-039. 2026-11-04 — the engine-facing protocol mentioned in Consequences is now decided by DEC-038.

### DEC-040 — The physics body owns velocity
- **Date:** 2026-11-05
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager, reviewer of PR #28)  **Proposed by:** `physics-adapter-design.md` §4 (TASK-008)  **Recorded by:** claude (PR #28 review)
- **Human's words:** "for physics adapter body (phyics adapter) should own velocity"
- **Context:** `physics-adapter-design.md` §4 left open whether the ECS or the Box2D body is authoritative for velocity. It decides which system declares `writes<Velocity>` and how gameplay changes motion. The SRS already describes the behaviour (srs.fr.3.13, 3.18–3.22, 3.28, 3.51).
- **Options considered:** (a) ECS owns velocity and pushes it into the body every step; (b) the body owns it and the adapter writes it back to the ECS.
- **Decision:** (b). After each physics step the body's position, rotation and linear velocity are authoritative for dynamic bodies and are written back to `PositionComponent` and `VelocityComponent` in PhysicsSyncOut. Gameplay changes motion only by writing Position or Velocity, which marks the component as a **pending write**, or by force and impulse commands; the next PhysicsSyncIn applies pending writes to the body, and write-back skips a component with a pending write. As the SRS states: static bodies follow the ECS Position and ignore Velocity; a kinematic body keeps a written Velocity until it is written again, and physics never changes a kinematic body's Velocity.
- **Rationale:** Box2D's collision response must not be overwritten by stale ECS values every step; pending writes keep gameplay control explicit and match the SRS.
- **Consequences:** The ECS needs write tracking for Position and Velocity (`ecs-api-design.md` §6) and an untracked write-back path used only by PhysicsSyncOut. `VelocityComponent` is a mirror for telemetry and gameplay reads.
- **Docs updated:** `design/physics-adapter-design.md` §4, `design/ecs-design.md`, `design/ecs-api-design.md`.
- **Course documents affected (for humans):** none known.
- **Supersedes / Related:** closes the velocity point of TASK-008; DEC-014, DEC-036, DEC-041.

### DEC-041 — ECS and Physics Adapter meet only in PhysicsSyncIn and PhysicsSyncOut
- **Date:** 2026-11-05
- **Status:** Accepted
- **Decided by:** Işık Dönger  **Proposed by:** claude (PR #28 review)  **Recorded by:** claude (PR #28 review)
- **Human's words:** "PhysicsSync system works."
- **Context:** Nothing defined how bodies are created and destroyed and who reads and writes ECS data for physics. Options were an `IWorldObserver` reacting to component adds, or declared systems.
- **Decision:** Two engine systems, `PhysicsSyncIn` (phase PhysicsSyncIn) and `PhysicsSyncOut` (phase PhysicsSyncOut), are the only code that calls the Physics Adapter from the simulation. PhysicsSyncIn creates and changes bodies and shapes and applies pending writes, in the order of srs.fr.3.61. PhysicsSyncOut writes positions and velocities back and publishes `CollisionEvent`. The game loop calls `adapter.step(1/60)` in PhysicsStep, which holds no systems. The adapter keeps an `IWorldObserver` only to learn about destroyed and disabled entities, and acts on them in the next PhysicsSyncIn (srs.fr.2.119). Both systems declare their reads, writes and publishes like any system and are created by factories that receive that World's adapter.
- **Rationale:** Declared systems make every physics read and write visible in the Semantic Graph and keep the access check complete.
- **Consequences:** The Physics Adapter uses the ECS `EntityId` type. `CollisionEvent` is published by the PhysicsSyncOut system, not by an engine module.
- **Docs updated:** `design/physics-adapter-design.md`, `design/ecs-design.md`, `design/ecs-api-design.md`, `design/ecs-event-bus-design.md`.
- **Course documents affected (for humans):** none known.
- **Supersedes / Related:** DEC-002, DEC-003, DEC-040, R-2, R-3.

### DEC-042 — Pre-implementation ECS gaps settled
- **Date:** 2026-11-05
- **Status:** Accepted
- **Decided by:** Işık Dönger  **Proposed by:** claude (PR #28 review, "Gaps before implementation")  **Recorded by:** claude (PR #28 review)
- **Human's words:** "for everything else your suggestions work."
- **Context:** After the PB-010 API design, `ecs-design.md` and `telemetry-and-events.md` contradicted it, and several things Phase 1 needs were `[TBD]`.
- **Decision:**
  1. Events are **pushed**: handlers run at the end of the publishing tick phase (event bus design). `ecs-design.md` §4's per-frame pull model is replaced.
  2. Simulation steps are **ticks** (srs.fr.1.12) everywhere in engine docs; "frame" means render frame only. PhysicsStep holds no systems; rendering is not a system; telemetry recording is done by a recorder that observes the World and the bus, not by a TelemetrySystem.
  3. Phase 1 components use the SRS names and fields (srs.fr.22.34–22.45), listed in `ecs-design.md` §2.
  4. `InputStateComponent` holds pressed, released and held states for the Phase 1 actions (MoveUp, MoveDown, MoveLeft, MoveRight, Interact) and the movement vector; it is also the agent's action shape.
  5. Systems read the fixed step through `ctx.fixedDeltaSeconds()`; there is one seeded gameplay random number generator (PCG32, srs.fr.7.5), reached through `ctx.random()` by systems that declare `usesRandom()`.
  6. Phase 1 scenes are built by C++ spawn code with explicit Editor-range entity IDs; the `.scene` format waits for the Editor.
  7. `AssetId` is a 64-bit FNV-1a hash of the asset name, with the name kept for export; `TagId` indexes the project's tag list (Phase 1: Untagged, Obstacle).
  8. Unrecoverable run errors (precondition and access violations) throw `engine::RunError`, caught only by the game loop, which logs the message and tick, Error-Pauses in the Editor and exits non-zero headless (srs.fr.1.11). The checks are on in every build. Exceptions never cross the adapter boundary.
  9. Telemetry Phase 1 records: run header, schema, per-tick sample, entity lifecycle, event publication. `telemetry-and-events.md` is rewritten to ticks.
  10. Q-013 is answered for Phase 1: description, unit, and the derived readBy/writtenBy; ranges, examples and notes can be added later.
- **Rationale:** Each item was a blocker or a contradiction for Phase 1 implementation; see the PR #28 review.
- **Consequences:** `ecs-design.md`, `telemetry-and-events.md`, `ecs-api-design.md` and `ecs-event-bus-design.md` are updated; the PB-010 D-decisions still go through the PB-010.3 review and ADR-001.
- **Docs updated:** `design/ecs-design.md`, `design/telemetry-and-events.md`, `design/ecs-api-design.md`, `design/ecs-event-bus-design.md`, `design/physics-adapter-design.md`, `architecture/engine-layer.md`, `architecture/glossary.md`, `agents/state/open-questions.md` (Q-013).
- **Course documents affected (for humans):** any course text that describes events as living for a frame or uses DoorState/RequiredItem component names.
- **Supersedes / Related:** DEC-002, DEC-008, DEC-018, Q-013, PB-010.

### DEC-043 — About 20 % of the whole project is built this semester
- **Date:** 2026-11-05
- **Status:** Accepted
- **Decided by:** Işık Dönger  **Recorded by:** claude (PR #28 review)
- **Human's words:** "for scope we will build only %20 of the whole project this semester."
- **Context:** The PB-010 ECS design covers the full SRS; it is more than the ECS owners can build and test this semester.
- **Decision:** This semester builds about 20 % of the whole project. For the ECS this is Phase 1 of `ecs-api-design.md` §17 (D-26): enough for the test scene (player, wall, door, key, goal), the semantic layer's schema and telemetry, and the AI bridge seams. Later items keep their place in the API.
- **Rationale:** `[not given]`
- **Consequences:** Implementation tasks for the ECS are cut to Phase 1; later items need their own PB tasks.
- **Docs updated:** `design/ecs-api-design.md` §17, `design/ecs-event-bus-design.md` §11, `design/ecs-design.md`.
- **Course documents affected (for humans):** the semester plan or roadmap sections of course documents, if they promise more.
- **Supersedes / Related:** D-26, PB-010.

### DEC-044 — Coding standards amended for the ECS design
- **Date:** 2026-11-05
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Proposed by:** `ecs-api-design.md` §14 (D-17, D-19) and DEC-042  **Recorded by:** claude (PR #28 review)
- **Human's words:** "you can edit coding-standart and pr description"
- **Context:** `coding-standards.md` (DEC-031) says changes go through a PR and a decision entry. The ECS design's naming (D-17, D-19), error policy and randomness rules (DEC-042) did not match it.
- **Decision:** `guidelines/coding-standards.md` is amended: systems declare everything in `describe()`; no static mutable state in systems; fixed step and gameplay RNG through `SystemContext`; invariant checks stay on in every build and throw `engine::RunError`; per-tick instead of per-frame wording; naming rows for types (PascalCase), functions and locals (camelCase), private members (trailing underscore), component and event fields (camelCase with units), namespace `engine`, tags (`<Name>Tag`, empty, filter only; empty SRS roles stay components); components and events are aggregates with `typeName` and no raw C arrays; only `RunError` may be thrown, never across the adapter boundary.
- **Rationale:** One set of rules for the code that implements PB-010.
- **Consequences:** Code review checks the new rules from the first ECS implementation PR.
- **Docs updated:** `guidelines/coding-standards.md`, `design/ecs-api-design.md` §14.
- **Course documents affected (for humans):** any course text that quotes the coding standard's naming table.
- **Supersedes / Related:** amends DEC-031; D-17, D-19, DEC-042.

### DEC-045 — No own LLM is built; users supply their own model provider and key
- **Date:** 2026-11-06
- **Status:** Accepted
- **Decided by:** Işık Dönger   **Recorded by:** claude (use cases thread)
- **Human's words:** "we must have decided that we will not build our own model at this time. [...] I am thinking that we can only gurantee free tier usage from other models and the user must use their own subscripti[on]" and, asked whether to record it, "you should only change related document in the project repo".
- **Context:** The AI service is a separate Python process reached over gRPC (DEC-038, DEC-039), and the provider sits behind an interface (`architecture/ai-assistant-layer.md` principle 6). No decision said which models are used or who pays for them. PB-068 (LLM provider selection and cost estimate) is still open.
- **Options considered:**
  1. Train or host our own model. Rejected: out of scope for a senior project.
  2. The team pays for one cloud provider for all users. Not possible to guarantee after the course.
  3. Users bring their own provider account or subscription and API key; the project guarantees only free-tier or local use. Chosen.
- **Decision:** Aegis does not build or train its own language model. The AI service talks to third-party or local models through the provider interface. Users supply their own provider account, subscription and API key. The project guarantees only that the assistant works with a free tier or a local model; paid tiers are the user's choice.
- **Rationale:** `[rationale not given — ask]`
- **Consequences:** The provider interface must support at least one free-tier cloud provider and one local option. Provider choice, quotas and terms are outside the project's control, so answer quality and limits depend on the user's provider. API keys stay in the OS credential store (SRS srs.nfr.3.7). PB-068 narrows to choosing which free-tier and local providers to support and test.
- **Docs updated:** `architecture/ai-assistant-layer.md`, `architecture/tech-stack.md`.
- **Course documents affected (for humans):** SRS Section 4 (assumptions and dependencies) and Section 5 (the "model provider" row says no provider is imposed). Handed to the SRS-A review thread as point 6.
- **Revisit when:** PB-068 picks the supported providers, or the free tiers they offer change.
- **Supersedes / Related:** DEC-010, DEC-038, DEC-039, PB-068.
- **Amendments:**

### DEC-046 — PB-011: window in `rendering-raylib` behind `IWindow`, loop seams `ISimulation` and `IFrameRenderer`, guarded MSVC leak check
- **Date:** 2026-11-10
- **Status:** Proposed
- **Proposed by:** Arda Karabulut (owner of PB-011; engine core area, DEC-024), who chose these options  **Recorded by:** claude-code (PB-011 session)
- **Context:** The ClickUp text of PB-011 puts the Window wrapper in a new `engine/platform` module, has `Application::Run` "call the scheduler each frame", names `demo/main.cpp` and asks for `_CrtSetDbgFlag`. Against that: only `engine/rendering-raylib` may link raylib (R-4, DEC-007, DEC-022) and `engine/core` must build without it (headless preset); `engine/ecs` does not exist yet and systems run per tick, not per render frame (srs.fr.1.13); no demo game is chosen (Q-016); `_CrtSetDbgFlag` exists only in the Microsoft C runtime, and the coding standard says to ask before adding platform-specific APIs.
- **Options considered:**
  1. Window: (a) `RaylibWindow` in `engine/rendering-raylib` with a raylib-free `IWindow` in `engine/core`; (b) a new `engine/platform` target that also links raylib, which needs an exception to R-4. Arda chose (a).
  2. Leak check: (a) one guarded C runtime call; (b) the same plus sanitizers in CI; (c) sanitizers only. Arda chose (a).
  3. How the loop calls the game: (a) two interfaces; (b) `std::function` callbacks in the config. Arda chose (a).
- **Decision (proposed):**
  1. `engine/core` defines `IWindow`, `WindowConfig` and `WindowFactory`. `engine/rendering-raylib` implements them as `RaylibWindow`, the only code that calls `InitWindow`, `WindowShouldClose` and `CloseWindow`. `Application` is given the factory and never calls it in headless mode. No `engine/platform` module is created; "window code only in the platform module" is met by `engine/rendering-raylib`.
  2. Until the ECS exists, `Application` calls `ISimulation::runTick()` for each tick and `IFrameRenderer::renderFrame()` once per render frame. An `Application` built from a simulation alone can only run headless.
  3. `engine::enableLeakReportAtExit()` (`engine/core/src/leak_check.cpp`) is the one place with compiler-specific code: under `_MSC_VER` and `_DEBUG` it turns on the C runtime leak report and also sends it to the standard error output; in every other build it does nothing.
  4. The example application is `sandbox`, not a `demo/` folder.
- **Rationale:** The chosen options change no architecture rule and keep the headless build free of raylib. `[further rationale not given]`
- **Consequences:** needs confirmation from Işık Dönger in the PB-011 pull request (Q-019). The ECS scheduler will implement `ISimulation`. `runSmokeWindow` is removed and `sandbox` runs on `Application`. Leaks are checked automatically only in MSVC Debug builds.
- **Docs updated:** `architecture/engine-layer.md`, `troubleshooting/environment-setup.md`, `troubleshooting/common-errors.md`, `agents/state/*`.
- **Course documents affected (for humans):** none known. The ClickUp text of PB-011 still names `engine/platform` and `demo/main.cpp`.
- **Revisit when:** `engine/rendering` (renderer interface and null renderer) or `engine/ecs` is created, or the team wants sanitizers in CI.
- **Supersedes / Related:** R-4, DEC-007, DEC-022, DEC-024, DEC-037, Q-016, Q-019, PB-011, TASK-006.
- **Amendments:**

### DEC-047 — PB-012: the game loop follows srs.fr.1.2 to 1.6 with an exact integer accumulator and the tick counter in `engine/core`
- **Date:** 2026-11-10
- **Status:** Proposed
- **Proposed by:** claude-code (PB-012 session), from srs.fr.1.2 to 1.6  **Recorded by:** claude-code (PB-012 session)
- **Context:** `architecture/engine-layer.md` had the cap on ticks per render frame only as `[PROPOSED: cap 5]`. The SRS fixes the loop (srs.fr.1.2 to 1.6), but it is a course document and the decision log had no entry. The ClickUp text of PB-012 says "simulation frame counter" and `EngineContext`, gives `maxStepsPerFrame` no value and does not say how time is represented.
- **Options considered:**
  1. Limit on catching up: (a) at most 5 ticks per render frame, drop the time of the further whole ticks, keep the part of a tick (srs.fr.1.3, 1.4); (b) clamp the frame time, as the PB-008 spike does (0.25 s). For comparison, Unity 6.5 also stops catching up beyond a maximum time, and the fixed-step group of its Entities 6.5 package also defaults to 1/60 s. (a) is proposed because the SRS states it.
  2. Accumulator: (a) exact integers, nanoseconds times 60; (b) `double` seconds; (c) a tick rounded to 16 666 667 ns. In one second, (b) runs 59 ticks at 144 frames per second and (c) runs 59 at every frame rate. (a) is proposed.
  3. Counter: a tick counter in the loop, passed to `ISimulation::runTick(tick)`, instead of a "frame number" in an `EngineContext`. DEC-042 item 2 already fixes the word tick.
- **Decision (proposed):**
  1. `FixedTimestep` in `engine/core` implements 1(a) with `maxTicksPerRenderFrame = 5` and reports when the warning "Simulation is running slower than real time" is due, at most once per second of real time. Until a logger exists, `Application` writes it to the standard error output.
  2. The accumulator is exact (2(a)).
  3. `Tick` is `std::uint64_t` in `engine/core/tick.hpp`. `Application` keeps the counter: 0 when a run starts and one more after every completed tick; simulated time is the counter times 1/60 s (srs.fr.1.5, 1.6).
  4. `FrameClock` (the task's `Time`) reads `std::chrono::steady_clock` once per render frame; nothing in a tick reads real time. A headless run uses no clock and needs a maximum number of ticks (srs.fr.7.4, 7.7).
- **Rationale:** It follows the SRS as written and keeps "60 ticks are 1.0 s" true at every frame rate.
- **Consequences:** needs confirmation from Işık Dönger in the PB-012 pull request (Q-020). Not built yet: pause, resume and step (srs.fr.1.16 to 1.18), `RunError` handling (srs.fr.1.11), the headless run settings (srs.fr.7.x).
- **Docs updated:** `architecture/engine-layer.md`, `troubleshooting/common-errors.md` (E-007, E-031), `troubleshooting/environment-setup.md`, `testing/test-checklists.md`, `agents/state/*`.
- **Course documents affected (for humans):** none; this follows SRS 6.1.
- **Revisit when:** pause and step are built (they change what the accumulator keeps), or a logger exists.
- **Supersedes / Related:** DEC-004, DEC-042, DEC-046, R-5, R-6, E-006, E-007, E-031, Q-020, PB-012, TASK-006.
- **Amendments:**

### DEC-049 — PB-025: the physics interface is `IPhysicsAdapter`, bodies are addressed by `BodyId`, and `Vec2` and `EntityId` live in `engine/core`
- **Date:** 2026-11-11
- **Status:** Accepted
- **Decided by:** Zeynep Ertürk (owner of PB-025; physics adapter area, DEC-024) chose the three options on 2026-11-10; Işık Dönger (project manager, reviewer of PR #33) accepted them on 2026-11-11  **Recorded by:** claude (PB-025 review session)
- **Human's words:** Işık Dönger: "I approve the first one, second and third also seems correct to me" (the three choices listed in PR #33).
- **Context:** The ClickUp text of PB-025 (PB-025.2) names the interface `IPhysicsWorld`, while `design/physics-adapter-design.md`, R-3 and DEC-003 always say Physics Adapter. The design doc addressed bodies by `BodyId` but let `DestroyBody` take either an entity or a `BodyId`, and had no entity-to-body lookup, which `PhysicsSyncIn` needs (DEC-041). `Vec2` and `EntityId` were only designed (`ecs-api-design.md` §4), not written, and the physics target must not depend on an ECS target that does not exist yet.
- **Options considered:**
  1. Name: (a) `IPhysicsAdapter`, as in the design doc and the rules; (b) `IPhysicsWorld`, as in ClickUp. Zeynep chose (a).
  2. Addressing: (a) every call takes a `BodyId`, plus `findBody(EntityId)`; (b) every call takes an `EntityId` and the adapter keeps the mapping inside. Zeynep chose (a).
  3. Shared types: (a) `Vec2` and `EntityId` in `engine/core`, copied from `ecs-api-design.md` §4; (b) in `engine/physics`; (c) wait for `engine/ecs`. Zeynep chose (a).
- **Decision:**
  1. The interface is `engine::physics::IPhysicsAdapter` in the header-only `engine::physics` target, which links only `engine::core`.
  2. Bodies are addressed by `BodyId` (64-bit, so the Box2D implementation can pack a `b2BodyId` into it). `createBody(EntityId, BodyDesc)` returns it, `findBody(EntityId)` looks it up and `destroyBody(BodyId)` removes it. Collision events and raycasts report `EntityId`, not `BodyId`.
  3. `engine::Vec2` and `engine::EntityId` are defined in `engine/core` (`vec2.hpp`, `entity_id.hpp`); the ECS reuses them.
- **Rationale:** One name across the rules, the design doc and the code. A `BodyId` avoids a map lookup in every call, and the lookup the sync systems need is one explicit call. `engine/core` is the one target both physics and the ECS already depend on, so neither depends on the other (R-2). `[further rationale not given]`
- **Consequences:** PB-026 implements `Box2DPhysicsAdapter` against this interface in `engine/physics-box2d/`. The ECS (Phase 1) must include `engine/core/entity_id.hpp` instead of defining its own `EntityId`. The ClickUp subtask PB-025.2 still says `IPhysicsWorld`.
- **Docs updated:** `design/physics-adapter-design.md` (§2 and the decisions line), `agents/state/current-status.md`, `agents/state/task-board.md`, `agents/state/handoff-log.md` (HND-020).
- **Course documents affected (for humans):** none known. The ClickUp text of PB-025.2 names `IPhysicsWorld`.
- **Revisit when:** PB-026 shows that `BodyId` addressing does not fit Box2D v3, or `engine/ecs` is created and wants `EntityId` elsewhere.
- **Supersedes / Related:** DEC-003, DEC-014, DEC-024, DEC-041, R-2, R-3, PB-025, PB-026, TASK-018.
- **Amendments:**
