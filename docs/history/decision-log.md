# Decision Log

**Append-only. This log is the single source of truth.** Every architectural, technological, scope or process decision, with rationale, so nobody has to re-argue it. Where any other document (including the course documents under `senior-project/docs/`) disagrees with an `Accepted` entry here, this log wins and the other document must be updated (project docs) or flagged for humans (course docs). Template: [../agents/templates/decision-entry.md](../agents/templates/decision-entry.md). Procedure: [../agents/playbooks/record-decision.md](../agents/playbooks/record-decision.md).

Statuses: `Proposed` (a question is open in the conversation and nobody has answered it yet; binds nobody) · `Accepted` (binding) · `Rejected` · `Superseded by DEC-###`.
**Humans decide; agents consult.** An entry is `Accepted` only because a human decided it (see `Decided by`). A human approves a decision in one of two places (DEC-052): in the conversation, or in the review of the pull request that adds the entry. An entry added in a pull request is written `Accepted` from the start: the reviewer treats it as proposed, and it binds only once a human merges the PR. On `main`, `Accepted` therefore always means a human approved it, in the conversation or in the review. Writing it `Accepted` saves the extra commit that would otherwise flip `Proposed` to `Accepted` after approval; it does not let an agent decide anything. To reverse a decision, add a new entry that supersedes it.

## Index

The one-line index of every entry is [decision-index.md](decision-index.md). Agents read the index at boot and open an entry here only by its id, when they need to trace a decision back. A new entry adds its index row there in the same PR (`tools/check_ids.py`).


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
- **Status:** Superseded by DEC-053
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
- **Status:** Superseded by DEC-050
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
- **Amendments:** 2026-11-22 — the advisor is no longer a code owner; Işık Dönger is the only one (DEC-151.1).

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
- **Status:** Superseded by DEC-050
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
- **Amendments:** 2026-11-23 — widened by DEC-46.3: the hierarchy and the entity and component enabled flags are built in Phase 1 (US-2.1, US-2.2).

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
- **Status:** Accepted
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
- **Amendments:** 2026-11-11 — accepted by Işık Dönger after merging PR #35 (Q-019): "I want you to update decision from pr 35 and 37 as accepted since they are only proposed now." Item 4 is clarified by DEC-048: `sandbox` is a developer and CI test host, the editor hosts the application, and the demo game is built inside the editor. Recorded by claude (PB-122 thread).

### DEC-047 — PB-012: the game loop follows srs.fr.1.2 to 1.6 with an exact integer accumulator and the tick counter in `engine/core`
- **Date:** 2026-11-10
- **Status:** Accepted
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
- **Amendments:** 2026-11-11 — accepted by Işık Dönger after merging PR #37 (Q-020): "I want you to update decision from pr 35 and 37 as accepted since they are only proposed now." Recorded by claude (PB-122 thread).

### DEC-048 — The editor hosts `engine::Application` and owns the main window
- **Date:** 2026-11-11
- **Status:** Accepted
- **Decided by:** Işık Dönger   **Recorded by:** claude (PB-122 thread)
- **Human's words:** reviewing PR #35: "shouldnt this task be to initialize the main window that the engine will run in, aka the editor. Demo is a game that will be developed within the editor." Asked for the layout, then: "okey you can go ahead and open pr."
- **Context:** DEC-046 item 4 named `sandbox` "the example application", and the PB-011 text named `demo/main.cpp`, which read as if a game hosts the engine. `editor/` was a placeholder that only printed its version and did not link the renderer.
- **Options considered:**
  1. The editor hosts `Application` and opens the main window; `sandbox` stays as a test host. Chosen.
  2. Keep `sandbox` as the only window application until the editor shell (PB-110).
- **Decision:**
  1. `engine::Application` stays in `engine/core` and knows no editor and no game (DEC-046).
  2. The `editor` executable is the application that hosts the engine: it builds `Application` with `RaylibWindow` and owns the main window. Editor panels draw in the render frame (srs.fr.1.13).
  3. `sandbox` is a developer and CI test host, not the product and not a stand-in for the demo.
  4. Games, including the demo game (Q-016), are content built and run inside the editor; they never host the engine.
  5. The editor opens a window, so like `sandbox` it is built only when `ENGINE_WITH_RAYLIB` is on; the `headless` preset builds neither (R-4, R-6).
- **Rationale:** The SRS puts Editor panels in the render frame (srs.fr.1.13) and the run-mode switch in the Editor (srs.fr.1.8), so the editor is the host.
- **Consequences:** PB-122 moves `editor/` onto `Application`. PB-110 (editor shell) builds its panels on that window. Rule S-6 no longer lists editor tooling as out of scope.
- **Docs updated:** `agents/rules.md` (S-6), `architecture/engine-layer.md`, `guidelines/repository-structure.md`, `troubleshooting/environment-setup.md`, `agents/state/*`.
- **Course documents affected (for humans):** none known.
- **Revisit when:** the editor shell (PB-110) or the run-mode switch (srs.fr.1.8) is built.
- **Supersedes / Related:** clarifies DEC-046 item 4; DEC-029, R-4, R-6, Q-016, PB-110, PB-122.
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
- **Docs updated:** `design/physics-adapter-design.md` (§2 and the decisions line), `agents/state/current-status.md`, `agents/state/task-board.md`, `agents/state/handoff-log.md` (HND-021).
- **Course documents affected (for humans):** none known. The ClickUp text of PB-025.2 names `IPhysicsWorld`.
- **Revisit when:** PB-026 shows that `BodyId` addressing does not fit Box2D v3, or `engine/ecs` is created and wants `EntityId` elsewhere.
- **Supersedes / Related:** DEC-003, DEC-014, DEC-024, DEC-041, R-2, R-3, PB-025, PB-026, TASK-019.
- **Amendments:**

### DEC-050 — The backlog moves from ClickUp to one GitHub project; branches use the issue number; the branch starts the task
- **Date:** 2026-11-11
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)  **Recorded by:** claude (issue #126)
- **Human's words:** "Okey, lets go with Github Projects with one project. But I am going to ask you to create all the issues and everything from stracth instead of copying clickup. Because main reason I wanted to do this was because we needed an overhaul and clickup wasnt easily altered."; "my problem is actions should not be creating pr's, only the branch."; "feat/us-15.2 would be clear"; "I do not like this poll idea ... branch creation should be instant since project status will be changed right before starting implementation."
- **Context:** ClickUp was hard to change and its backlog had drifted from the SRS. The Issue start workflow opened a draft PR for every new `PB-` issue, which failed with "GitHub Actions is not permitted to create or approve pull requests".
- **Decision:**
  1. Planning moves to one GitHub project, team1-sp #3 (Iterative development template). Epics (`EP-xx`) and user stories (`US-x.y`) come from SRS A Section 6.3; each story is a sub-issue of its epic and its functional requirements are its acceptance criteria. Increment 1 stories (SRS B) are issues in the Increment 1 milestone; later stories stay as checklists in their epics until planned. Iteration, Estimate, Size and Priority are project fields.
  2. One branch per user story, named `<type>/us-<x.y>-<short-description>` (`feat/us-15.2-build-settings-file-menu`); other issues use `<type>/<issue>-<short-description>`; `<type>/pb-<nnn>-…` stays valid for branches started before this decision. A colon cannot appear in a git branch name, so the PR-title form `feat(US-15.2): …` cannot be used for branches. The `branch-names` ruleset enforces the same pattern.
  3. PR titles use the story id as the Conventional Commit scope (`feat(US-1.1): …`), or the issue number for other issues (`fix(#142): …`).
  4. The developer creates the branch (instant), and that starts the task: the Issue start workflow runs on the branch creation, moves the issue's card to In progress with the `PROJECT_TOKEN` secret, assigns the creator if nobody is assigned and comments the checkout command. The PR fill workflow copies `Closes #<issue>`, labels, milestone, assignees and a Conventional Commit title from the issue into a new PR. No workflow creates branches or PRs. (Creating the branch from the status was rejected: project status changes do not trigger workflows, so it would need polling.)
- **Rationale:** GitHub issues are the backlog itself, so nothing has to be synchronised, and AI agents can edit them directly. Opening PRs from Actions needs a repository setting the team does not want and a PR with an empty start commit.
- **Consequences:** Someone with org access must add the `PROJECT_TOKEN` repository secret; until then cards are not moved, but the comment, assignment and PR filling still work. If a GitHub branch-name ruleset exists, its pattern must be updated to match `tools/check_pr.py`. ClickUp is no longer updated; old `PB-` ids remain in history and in older docs.
- **Docs updated:** `CONTRIBUTING.md`, `guidelines/git-workflow.md`, `process/development-process.md`, `.github/pull_request_template.md`, `.github/ISSUE_TEMPLATE/1-backlog-item.yml`, `.github/labels.yml`, `.github/workflows/issue-start.yml`, `.github/workflows/pr-fill.yml`, `tools/start_task.py`, `tools/fill_pr.py`, `tools/check_pr.py`.
- **Course documents affected (for humans):** the Initial Plan names ClickUp as the planning tool (§4.2).
- **Revisit when:** the team needs more than one project.
- **Supersedes / Related:** supersedes DEC-028 and the scope rule of DEC-035; related DEC-024, DEC-027, DEC-032.
- **Amendments:** 2026-11-12 — the PR fill workflow of point 4 is replaced by `tools/open_pr.py`, which the agent or developer runs after the first push (DEC-054). The rest of point 4 stands.

### DEC-051 — US-1.1: a render frame collects input first; raylib is built with custom frame control
- **Date:** 2026-11-12
- **Status:** Accepted
- **Decided by:** Işık Dönger   **Recorded by:** claude (US-1.1 thread)
- **Human's words:** "You can log the decision as accepted, push and then create the pr." (2026-11-12, after reviewing the US-1.1 change)
- **Proposed by:** claude (US-1.1 thread), from srs.fr.1.1
- **Context:** srs.fr.1.1 orders a normal-mode render frame as (1) collect keyboard input, (2) run the due ticks, (3) render, (4) present. Since PB-011 (DEC-046) the loop ran ticks, `beginFrame`, `renderFrame`, `endFrame`, and `RaylibWindow::endFrame()` called raylib's `EndDrawing()`, which presents, waits for the frame-rate cap and only then polls input. Input was therefore collected at the end of the previous frame, before the wait: the first frame's ticks saw no collected input, and input arriving during the wait reached the ticks one frame late. raylib cannot poll a second time at the start of the frame, because `PollInputEvents()` copies the current key state into the previous one and would erase the key-press edges of the first poll.
- **Options considered:**
  1. Keep `EndDrawing()` as it is and document that the poll at the end of frame N is step 1 of frame N + 1. No raylib change, but the order is only true by argument, cannot be seen in the interface or tested, and keeps up to one frame of extra input latency.
  2. Build raylib with `SUPPORT_CUSTOM_FRAME_CONTROL=1` (a documented raylib build flag; `config.h` honours a compile definition) and give `IWindow` an explicit `collectInput()`. `EndDrawing()` then only flushes the draw batch; `RaylibWindow` calls `PollInputEvents()` in `collectInput()`, and `SwapScreenBuffer()` plus the frame-rate wait in `presentFrame()`. raylib's own frame timing (`GetFrameTime()`, `GetFPS()`) is no longer updated; the engine does not use it (its loop measures time with `FrameClock`).
- **Decision:** option 2. `IWindow` has `collectInput()`, `beginFrame()` and `presentFrame()` (renamed from `endFrame()`); `Application` calls them in the srs.fr.1.1 order, and the game loop tests check that order.
- **Rationale:** the order of srs.fr.1.1 holds by construction and is tested; input is collected after the frame-rate wait, right before the ticks that use it.
- **Consequences:** raylib code outside `RaylibWindow` must present and poll itself: the PB-008 spike (`sandbox/src/box2d_raylib.cpp`) now calls `PollInputEvents()`, `SwapScreenBuffer()` and `WaitTime()` and measures its frame time with `GetTime()`. Zeynep Ertürk (`engine/rendering-raylib`) should still look at the window wrapper in the US-1.1 pull request. The warning of srs.fr.1.4 still goes to the standard error output until a logger and the Console exist; pause (srs.fr.1.3 "while not paused") is US-1.4.
- **Docs updated:** `architecture/engine-layer.md`, `testing/test-checklists.md`, `agents/state/handoff-log.md`, `agents/state/current-status.md`.
- **Course documents affected (for humans):** none; this follows SRS 6.1.
- **Revisit when:** a raylib upgrade changes custom frame control, or the renderer moves off raylib.
- **Supersedes / Related:** DEC-033, DEC-046, DEC-047, R-4, srs.fr.1.1, srs.fr.5.1, US-1.1 (#41).

### DEC-052 — A decision is `Proposed` only while it waits for an answer in the conversation; a decision a PR carries is written `Accepted` and approved in the review
- **Date:** 2026-11-12
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)   **Recorded by:** claude (ci/131 thread)
- **Human's words:** "Change the workflow so that decisions are only logged as proposed when waiting for approval through the conversation. Otherwise it should be logged as approved and wait for approval in the review part. This change is not to allow for ai agents to make the decisions themself with bypassing the human!! This is to solve the requiring an additional commit to change the proposed log to accepted." (2026-11-12)
- **Context:** Agents wrote every decision their change carried as `Proposed`, even though the reviewer of the pull request approves or rejects it anyway. When the reviewer agreed, another commit was needed only to flip the entry to `Accepted` (for example DEC-046 and DEC-047 after PRs #35 and #37, and DEC-051 before PR #130 was opened).
- **Options considered:**
  1. Keep `Proposed` until a human approves, then commit the flip. Simple, but costs a commit and a review round for every accepted decision.
  2. Write the entry `Accepted` in the PR and make the review the approval step: the reviewer treats it as proposed, and the merge accepts it.
- **Decision:** option 2. `Proposed` means a question is open in the conversation and nobody has answered it. A decision the human made in the conversation is `Accepted`, with their words. A decision a pull request carries is also written `Accepted` (no extra marker, so nothing has to be removed at merge), is listed under "Decisions for review" in the PR body, and binds only once a human merges the PR. A rejected or changed entry is fixed on the branch before the merge. Humans still decide (DEC-017): an agent puts open choices to the human first, and an unlisted entry is not approved by the merge.
- **Rationale:** the review already is a human approval step, and every PR is merged by a human (DEC-025), so nothing reaches `main` as `Accepted` without a human. The flip commit added no information.
- **Consequences:** the PR template has a "Decisions for review" section that reviewers check (`process/code-review.md`). Entries may still change on their branch until the merge; once on `main` they are append-only as before.
- **Docs updated:** `history/decision-log.md` (header), `agents/templates/decision-entry.md`, `agents/playbooks/record-decision.md`, `agents/rules.md` (P-3, section D), `agents/AGENT-PROTOCOL.md` (steps 20 to 21a), `agents/model-compatibility.md`, `agents/communication-protocol.md`, `process/code-review.md`, `.github/pull_request_template.md`.
- **Course documents affected (for humans):** none.
- **Revisit when:** a decision that skipped the conversation turns out to need wider discussion than a review gives.
- **Supersedes / Related:** refines DEC-017 and DEC-025; related DEC-050.

### DEC-053 — AI agents ask before committing, not before pushing
- **Date:** 2026-11-12
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)   **Recorded by:** claude (ci/131 thread)
- **Human's words:** "Requiring approval before pushing should be removed. Because after commiting commits cannot be altered and changes will require new commit anyway so pushing isnt effective. Commits should require approval." (2026-11-12)
- **Context:** DEC-025 made agents ask before every push. By then the commit already exists, and since history is never rewritten (S-2), any requested change is a new commit whether or not the first one was pushed. The approval protected nothing and delayed CI and the draft PR.
- **Decision:** An AI agent shows the human its change and commits only after the human approves it. Once a commit is approved, the agent pushes it and opens or updates the draft PR without asking again. Merging `main` into the branch with no conflicts, or with only state and history files resolved by the rules of DEC-055, is committed without asking again; a merge that needed any other resolution is shown first. A human approves and merges every PR. Agents never push to `main`, force-push or rewrite history.
- **Rationale:** the approval belongs where the change becomes permanent, which is the commit.
- **Consequences:** CI runs and the draft PR exists as soon as the human approves the change.
- **Docs updated:** `guidelines/git-workflow.md`, `CONTRIBUTING.md`, `agents/AGENT-PROTOCOL.md` (step 23a), `agents/state/current-status.md`.
- **Course documents affected (for humans):** none.
- **Revisit when:** a pushed commit causes harm that an earlier approval would have stopped.
- **Supersedes / Related:** supersedes DEC-025; related S-2, DEC-054, DEC-055.

### DEC-054 — Agents open the draft PR with `tools/open_pr.py`; the PR fill workflow is removed
- **Date:** 2026-11-12
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)   **Recorded by:** claude (ci/131 thread)
- **Human's words:** "Pr should not be required to be opened manually. ... either after branch creation github actions push an initial commit ... or creation of both branch and pr should be moved to agents instead of github."; on Claude's recommendation of the second: "In point 2, I approve your proposal." (2026-11-12)
- **Context:** Since DEC-050 a developer opened each PR by hand and the PR fill workflow filled it, but agents already opened and filled their own PRs, so the workflow did nothing. GitHub cannot open a PR for a branch with no commits beyond `main`.
- **Options considered:**
  1. After the branch is created, a workflow pushes a "task started" commit and opens a draft PR. Needs the "Allow GitHub Actions to create pull requests" setting rejected in DEC-050 (or a personal token, which makes the token owner the PR author); a PR opened with the workflow token does not trigger CI or other workflows; every local branch falls behind its remote at once; and the start commit edits shared docs, which adds merge conflicts.
  2. The agent (or developer) opens the PR after the first push, with a local tool that fills it from the issue.
- **Decision:** option 2. `tools/open_pr.py` opens a draft PR for the current branch with `Closes #<issue>`, the issue's labels, milestone and assignees, and a Conventional Commit title; agents run it right after their first push. `.github/workflows/pr-fill.yml` and `tools/fill_pr.py` are removed. The Issue start workflow still runs on branch creation. No workflow creates branches, commits or PRs.
- **Rationale:** the PR is opened by the person (or the agent working for them), so authorship is right and CI runs; nothing needs a repository setting or a bot commit.
- **Consequences:** `docs`, `ci` and `chore` changes without an issue are named after their own PR number (`ci/131-update-agent-workflow`, PR title `ci(#131): Update agent workflow`, `no-issue` label, no `Closes` line); `open_pr.py` does this when the branch number has no issue (Işık Dönger, 2026-11-12: "make sure everything works with ci/pr_number-description branch name and ci(pr-number): description pr name"). A PR opened by hand is no longer filled; `pr-hygiene` (`tools/check_pr.py`) still fails it without a valid title and `Closes #<issue>`.
- **Docs updated:** `guidelines/git-workflow.md`, `CONTRIBUTING.md`, `agents/AGENT-PROTOCOL.md` (step 23a), `agents/state/current-status.md`, `tools/issue_ref.py`; DEC-050 amended.
- **Course documents affected (for humans):** none.
- **Revisit when:** humans keep opening PRs by hand and forgetting the fields.
- **Supersedes / Related:** replaces the PR fill part of DEC-050; related DEC-053.

### DEC-055 — Branches keep in step with `main` by merging it; collided ids are caught in CI; conflicting PRs are flagged
- **Date:** 2026-11-12
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)   **Recorded by:** claude (ci/131 thread)
- **Human's words:** "There should be addition to workflow to account for merge conflicts happening because of pr merges while working on the pr."; on Claude's proposal: "In point 3, I approve your proposal but I have a question. This does not solve merge conflicts after ready-for-review ... do you have any proposal to mitigate this." (2026-11-12). The conflict flag answers that question and is approved in the review of this PR.
- **Context:** PRs merged while another branch is open cause conflicts that the reviewer resolved during review, mostly in docs. Two branches that append to the same log can also merge without any git conflict and still give two entries the same id.
- **Decision:**
  1. Before starting, the agent lists open PRs and the files they touch and tells the human about overlap in code or design docs.
  2. The branch merges `origin/main` (never rebases) before every push, before ready for review and when flagged, then builds and tests again.
  3. In state and history files both sides are kept and the branch being synced renumbers its own colliding ids. Code and design-doc conflicts are resolved only when both sides fit together; otherwise the agent asks the human.
  4. `tools/check_ids.py` in the `docs` CI job fails on a duplicate DEC, HND, PRB, Q or TASK id, or a decision or problem entry missing from its index.
  5. The `Conflict flag` workflow (`tools/flag_conflicts.py`) runs after each merge to `main` and on pushes to PRs: a PR that conflicts with `main` gets the `needs-sync` label and one comment, and the label is removed once it no longer conflicts. The author syncs; the reviewer only re-checks the merge commit; a PR with `needs-sync` is not merged.
- **Rationale:** conflicts are found by the author, early and in small pieces, instead of by the reviewer at the end.
- **Consequences:** the `needs-sync` label is added to `.github/labels.yml`. The ruleset option "require branches to be up to date before merging" stays off unless the team turns it on.
- **Docs updated:** `guidelines/git-workflow.md` §4, `CONTRIBUTING.md`, `agents/AGENT-PROTOCOL.md` (steps 14a, 23a), `.github/labels.yml`, `.github/workflows/ci.yml`, `.github/workflows/conflict-flag.yml`.
- **Course documents affected (for humans):** none.
- **Revisit when:** conflicts still reach review regularly.
- **Supersedes / Related:** related DEC-053, DEC-054, P-6.

### DEC-057 — US-1.2: run options in `engine/core`, run errors end the run in the game loop, Editor parts wait for their stories
- **Date:** 2026-11-15
- **Status:** Accepted (carried by the US-1.2 pull request; binds once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-1.2 pull request   **Recorded by:** claude (US-1.2 thread)
- **Human's words:** none yet; listed under "Decisions for review" in the PR.
- **Proposed by:** claude (US-1.2 thread), from srs.fr.1.7 to 1.11 and DEC-042
- **Context:** US-1.2 (#42) asks for srs.fr.1.7 to 1.11. Normal and headless mode (srs.fr.1.7) and closing the window (srs.fr.1.10) were built in PB-011 and PB-012. Missing were the command-line run mode and settings of a built game (srs.fr.1.8, 7.33), the fullscreen state of the standalone window (srs.fr.1.9) and the handling of unrecoverable run errors (srs.fr.1.11), which DEC-042 had already given the shape "throw `engine::RunError`, only the game loop catches it". The rest of those requirements needs Editor features that are other stories: the mode switch (srs.fr.17.19, US-17.3, not in Increment 1), Play mode and Error Pause (US-17.2, US-24.1), the Game view (US-20.1) and the project's Rendering settings (srs.fr.27.23, US-27.4).
- **Decision:**
  1. `engine/core/run_options.hpp` has `RunOptions` and `parseRunOptions()` for the options of srs.fr.7.33: `--headless`, `--max-ticks N` (1 to 1 000 000, default 10 000 in a headless run), `--seed S` (0 to 4 294 967 295, srs.fr.7.32) and `--input-recording F`. An unknown option or an invalid value gives an error naming the option; the caller writes it to the standard error output, runs no tick and exits non-zero. `parseRunOption()` reads one option, so an application with options of its own (the sandbox's `--fps`) can combine both. The built game (US-28.x) will use it; the sandbox uses it now. The editor does not take `--headless`: its mode comes from the mode switch.
  2. `--input-recording` is parsed, but the sandbox refuses it with "input recording playback is not built yet" and runs no tick, until playback (srs.fr.5.12 onwards) exists. The seed is parsed and printed; nothing draws random numbers until the gameplay generator (srs.fr.7.5) exists.
  3. `engine/core/run_error.hpp` has `engine::RunError` (a `std::runtime_error`). `Application::runOneTick()` is the only place it is caught. It writes `error: tick N: <message>` to the standard error output (there is no logger yet), runs no further tick, does not render or present the frame in progress, and `run()` returns exit code 1. The failed tick does not count as executed. Other exceptions are bugs and are not caught. This applies in headless mode and in normal mode outside the Editor; Error Pause in Play mode comes with US-17.2.
  4. `Application::outcome()` returns a `RunOutcome`: `NotRun`, `WindowClosed`, `MaxTicks` or `Error`, with the report names "Not run", "Window closed", "Max ticks" and "Error" (`runOutcomeName()`). The headless names follow srs.fr.7.16; `Stopped`, `Failure` and `Objective reached` come with US-7.2.
  5. `WindowConfig` has `fullscreen`, which `RaylibWindow::open` turns into raylib's `FLAG_FULLSCREEN_MODE`. Who fills it from the project's Rendering settings is US-27.4 and US-28.x.
  6. The sandbox gets `--seed`, `--input-recording`, `--fullscreen` and `--fail-at-tick N` (tick N throws `RunError`, so CI can check srs.fr.1.11 on a real process), and prints the run's mode, seed and outcome.
- **Rationale:** everything the engine needs for the two run modes is built and tested now, behind the interfaces the Editor and the built game will call, without building Editor features that belong to other stories.
- **Consequences:** srs.fr.1.8 (Editor mode switch), 1.9 (Rendering settings, Game view) and 1.11 (Error Pause in Play mode) stay partly open after US-1.2; the PR says which parts. A logger (Console, US-24.1) replaces the standard error output later.
- **Docs updated:** `architecture/engine-layer.md`, `testing/test-checklists.md`, `testing/headless-testing.md`, `agents/state/current-status.md`, `agents/state/handoff-log.md`.
- **Course documents affected (for humans):** none; this follows SRS 6.1.
- **Revisit when:** the logger, Play mode or the build pipeline exists.
- **Supersedes / Related:** DEC-042 (item 8), DEC-046, DEC-047, DEC-048, US-1.2 (#42).

### DEC-058 — Agents ask before committing only on a member's computer; in a cloud copy they commit and push without asking
- **Date:** 2026-11-15
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager)   **Recorded by:** claude (US-1.2 thread)
- **Human's words:** "if worked on the local commit/pushes should ask for permissions. otherwise it should be auto commit/pushed" (2026-11-15)
- **Context:** DEC-053 made every agent ask before committing. Agents running in a cloud session work in their own clone of the repository: their commits change nothing on a member's computer and reach `main` only through a PR that a human reviews and merges, so the approval added a wait without protecting anything. An agent working on a member's own computer changes that member's working copy and local branches.
- **Decision:** An agent working on a member's own computer (folder access or a session on the device) shows the change and commits and pushes only after that member approves. An agent working in its own cloud copy of the repository commits and pushes to the story branch without asking, then opens or updates the draft PR with `tools/open_pr.py`. Everything else in DEC-053 stays: a merge of `main` that needed a resolution beyond the state and history file rules of DEC-055 is shown first; a human approves and merges every PR; agents never push to `main`, force-push or rewrite history.
- **Rationale:** the approval belongs where an agent's commit touches a member's own files; in a cloud copy the PR review is the approval step.
- **Consequences:** cloud agents' changes appear on the branch and in the draft PR as soon as they are built and tested.
- **Docs updated:** `guidelines/git-workflow.md` §1, `CONTRIBUTING.md`, `agents/AGENT-PROTOCOL.md` (step 23a), `agents/state/current-status.md`.
- **Course documents affected (for humans):** none.
- **Revisit when:** a cloud agent's push causes harm that an approval would have stopped.
- **Supersedes / Related:** refines DEC-053; related DEC-052, DEC-054, DEC-055, S-2.

### DEC-059 — US-1.3: the game loop runs the seven tick phases itself and gives every tick its input action state
- **Date:** 2026-11-16
- **Status:** Accepted (carried by the US-1.3 pull request; binds once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-1.3 pull request   **Recorded by:** claude (US-1.3 thread)
- **Human's words:** none yet; listed under "Decisions for review" in the PR.
- **Proposed by:** claude (US-1.3 thread), from srs.fr.1.12 to 1.15, srs.fr.5.5 to 5.7, 5.11 and DEC-041, DEC-042
- **Context:** US-1.3 (#43) asks that every tick runs the same seven phases in the same order in normal and headless mode (srs.fr.1.12, 1.13), that the input application phase gives the tick the input action state of its input source (srs.fr.1.14) and that a frame without a tick still renders the last completed tick (srs.fr.1.15, already met in US-1.1). `ISimulation::runTick(tick)` left the phase order to whoever implements it, so nothing in the loop guaranteed it. `TickPhase` was only designed, inside `engine/ecs` (`ecs-api-design.md` §8), which does not exist yet. Nothing in the engine held input actions or input sources. Key bindings and the Input Map are US-5.1; focus loss is US-5.3; recordings are US-5.4.
- **Options considered:**
  1. Who runs the phases: (a) the game loop calls `ISimulation::runPhase(phase, context)` once per phase; (b) keep `runTick()` and let the ECS run its phases inside it. (a) is chosen: the loop owns the order, both modes reach the simulation through one function, and the ECS `SystemScheduler::runPhase()` maps onto it one to one.
  2. Where input transitions come from: (a) an `IInputSource` interface that the loop asks once per tick, with a `KeyboardQueue` for normal mode and `NoInputSource` for None; (b) pass input through `IWindow`. (a) is chosen: recordings and the agent (srs.fr.5.3, 5.11, 9.51) become sources too, and headless mode, which has no window, uses the same path.
- **Decision:**
  1. `engine/core/tick_phase.hpp` has `enum class TickPhase` (InputApplication, GameplayInput, PhysicsSyncIn, PhysicsStep, PhysicsSyncOut, GameplayRules, TelemetryRecording), `tickPhases` in execution order and `tickPhaseName()` with the names the schema uses. The ECS uses this type instead of defining its own.
  2. `ISimulation::runTick(tick)` is replaced by `runPhase(TickPhase, const TickContext&)`. `Application::runOneTick()`, the only function both modes use to run a tick, calls it for each phase of `tickPhases` in order. `TickContext` holds the tick number and the tick's `InputActionState`. The PhysicsStep phase is still called: the ECS-backed simulation steps the Physics Adapter there and runs no systems (DEC-041 said "the game loop calls `adapter.step(1/60)`"; the loop now does so through this call).
  3. A phase that throws `engine::RunError` ends the tick there: the later phases of that tick do not run and the tick is not counted (srs.fr.1.11 as in DEC-057).
  4. `engine/core/input_actions.hpp` has the Phase 1 actions of DEC-042 item 4 (`InputAction`: MoveUp, MoveDown, MoveLeft, MoveRight, Interact), `InputTransition` and `InputActionState`, whose `applyTick()` follows srs.fr.5.6 and 5.7. The Input Map (US-5.1, srs.fr.5.22 onwards) replaces the fixed list.
  5. `engine/core/input_source.hpp` has `IInputSource::takeTransitions(tick, out)`, `NoInputSource` and `KeyboardQueue` (srs.fr.5.5: transitions of frames that run no tick stay queued; the first tick afterwards takes them all). Before phase 1 of each tick, `Application` takes the transitions from the run's source and applies them to the run's `InputActionState`. `ApplicationConfig::inputSource` selects a source; without one, normal mode uses the keyboard queue (`Application::keyboardQueue()`) and headless mode uses None (srs.fr.7.3). While another source is active, the keyboard queue is emptied every render frame (srs.fr.5.11). A new run starts with nothing held and nothing queued.
  6. Nothing pushes keyboard transitions yet: turning raylib keys into actions needs the default bindings and the Input Map (srs.fr.5.1, 5.28; US-5.1). Focus-lost markers (US-5.3), recording playback (US-5.4) and the agent source come with their stories as `IInputSource` implementations or additions.
  7. The sandbox prints `tick phases run: N`, so CI checks seven phases per tick on a real process.
- **Rationale:** the phase order is fixed in one place that both run modes share, so they cannot drift apart, and every later input source plugs into the same tick input path.
- **Consequences:** `editor`, `sandbox` and the tests implement `runPhase()`. `ecs-api-design.md` §8 now points to `engine/core` for `TickPhase`. US-5.1 fills the keyboard queue from the window.
- **Docs updated:** `architecture/engine-layer.md`, `design/ecs-api-design.md`, `design/ecs-design.md`, `design/physics-adapter-design.md`, `testing/test-checklists.md`, `agents/state/current-status.md`, `agents/state/handoff-log.md`.
- **Course documents affected (for humans):** none; this follows SRS 6.1.
- **Revisit when:** the ECS scheduler, the Input Map or recording playback is built.
- **Supersedes / Related:** DEC-041, DEC-042 (items 2, 4), DEC-046 (item 2), DEC-047, DEC-057, US-1.3 (#43).

### DEC-060 — US-1.4: pause, resume and step in the game loop
- **Date:** 2026-11-16
- **Status:** Accepted (carried by the US-1.4 pull request; binds once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-1.4 pull request   **Recorded by:** claude (US-1.4 thread)
- **Human's words:** none yet; listed under "Decisions for review" in the PR.
- **Proposed by:** claude (US-1.4 thread), from srs.fr.1.16 to 1.18, 17.12 to 17.14, 24.6 and DEC-047
- **Context:** US-1.4 (#44) asks that a pause takes effect at the next tick boundary, drops the whole ticks still due and keeps the part of a tick (srs.fr.1.16), adds no real time while paused (srs.fr.1.17) and measures real time again from the resume, so the pause neither simulates paused time nor causes the slow-simulation warning (srs.fr.1.18). Step (srs.fr.17.14) runs one tick without the accumulator. The Pause and Step buttons, Ctrl+Shift+P and Ctrl+Alt+P belong to Play mode (US-17.2), and Error Pause to US-24.1; this story gives them the engine calls. `FixedTimestep` already holds only the part of a tick between frames: the whole ticks of a frame live in the loop's `FrameTicks`.
- **Options considered:**
  1. How the paused time is kept out: (a) keep reading the frame clock every frame, give the timestep nothing while paused, and on resume take off the part of the next frame time that passed before the resume; (b) restart the frame clock on resume. (a) is chosen: frame times stay true for FPS and render frame time, and `FrameClock::sinceFrameStart()` is the only addition.
  2. Where the pause is checked: (a) before every tick of a frame and once after the frame's ticks; (b) only between frames. (a) is chosen: srs.fr.24.6 needs a pause asked for inside a tick to stop the frame's remaining ticks.
- **Decision:**
  1. `Application::requestPause()` asks for a pause. The loop checks the request before each tick of a render frame and after the frame's ticks; at that boundary the pause takes effect: the frame's ticks that have not started are dropped (they are the accumulator's whole ticks) and `FixedTimestep` keeps the part of a tick. Render frames go on, collecting input and rendering the last completed tick.
  2. While paused, the frame clock is still read every frame, but `FixedTimestep::advance()` is not called, so no real time and no warning interval reach it.
  3. `resume()` ends the pause and stores `FrameClock::sinceFrameStart()`, the paused part of the next frame time, which the next frame takes off before it calls `advance()`. A `resume()` before the pause has taken effect cancels the request.
  4. `requestStep()` while paused counts one step; the next render frame runs one tick per step through `runOneTick()` and renders it, without calling the timestep. A step can reach `maxTicks` or throw `RunError` like any tick.
  5. The calls act only during a normal-mode `run()` and return false otherwise: headless runs have no Pause (srs.fr.17.28 enables it only in Play mode). A new run starts unpaused. They are called on the loop's thread, from input collection, from a tick or from rendering; the Editor's controls come with US-17.2 and Error Pause with US-24.1.
- **Rationale:** the timestep stays exact and unchanged: pause and step only decide whether it is called and with how much time, so no paused time and no rounding enter the accumulator.
- **Consequences:** Play mode's Pause, Step and the toolbar counter (US-17.2) and Error Pause (US-24.1) call these functions. The sandbox has no pause control.
- **Docs updated:** `architecture/engine-layer.md`, `testing/test-checklists.md`, `agents/state/current-status.md`, `agents/state/handoff-log.md`.
- **Course documents affected (for humans):** none; this follows SRS 6.1.
- **Revisit when:** the Editor's Play mode controls or Error Pause are built, or the game loop moves to another thread than the Editor's UI.
- **Supersedes / Related:** DEC-047, DEC-057, DEC-059, US-1.4 (#44).

### DEC-061 — Agents do code work in a cloud copy by default, on a member's computer only when the work needs it
- **Date:** 2026-11-16
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager) asked for the rule; the wording below is for review in the ci/136 pull request   **Recorded by:** claude (ci/136 thread)
- **Human's words:** "Adding a rule when the agents work on the local and when work on the cloud." (2026-11-16)
- **Context:** Agents can work in two places: in their own cloud copy of `team1-sp/project` (a Claude Code cloud session), or on a member's own computer, through folder access or a Remote Control session. DEC-058 already says how commits differ between the two, but nothing said which place to pick.
- **Options considered:**
  1. Cloud copy by default — cheaper and faster to start, can build, run the tests, commit, push and open the PR itself; the member only runs `git pull`. Cannot reach files that live only on the member's computer, local tools or signed-in accounts.
  2. Member's computer by default — uses the member's own setup, but every commit and push waits for their approval (DEC-058), the computer must stay on, and the agent changes their working copy.
- **Decision:** Code and repository docs work happens in the agent's own cloud copy by default: build, test, commit, merge `origin/main`, push and open the draft PR there, without asking (DEC-058); the member then runs `git pull`. An agent works on a member's computer only when (a) the files live only there, such as the Word deliverables in `docs/deliverables` on the member's computer, (b) the work needs a tool, program or signed-in account that exists only there (Visual Studio, a Windows-only check, the member's GitHub or ClickUp login), or (c) the member asks for it. On the member's computer it prefers folder access for reading and editing files, uses Remote Control only to run commands there, and commits or pushes only after the member approves (DEC-058). In a cloud session `tools/open_pr.py` fails, because it uses `gh repo view` and GraphQL is blocked there (HTTP 403): the agent opens the draft PR with the GitHub `create_pull_request` tool, with the same title and body `open_pr.py` would build, then sets labels, milestone and assignee with `gh api -X PATCH repos/team1-sp/project/issues/<pr>`.
- **Rationale:** most work is code and docs in the repository, which the cloud copy handles end to end without waiting on anyone; the member's computer is used only for what nothing else can reach.
- **Consequences:** members `git pull` the branch to look at cloud work locally. Work that needs Windows-only checks still relies on CI's Windows job or a member's computer.
- **Docs updated:** `guidelines/git-workflow.md` §1, `agents/AGENT-PROTOCOL.md` (steps 5a, 23a), `agents/rules.md` (P-12), `CONTRIBUTING.md`, `agents/state/current-status.md`.
- **Course documents affected (for humans):** none.
- **Revisit when:** cloud sessions can run GraphQL (then `open_pr.py` works there too), or a cloud copy cannot build something the team needs.
- **Supersedes / Related:** related DEC-054, DEC-058.

### DEC-062 — The `Epic close` workflow closes an epic once all of its stories are done, and marks it `increment-done` while only later stories are left
- **Date:** 2026-11-16
- **Status:** Accepted
- **Decided by:** Işık Dönger (Project Manager), 2026-11-16, in the conversation (option 2 chosen); the rest of the details are for review in the ci/136 pull request   **Recorded by:** claude (ci/136 thread)
- **Human's words:** "Adding an action that auto closes user epic issues that are sub-issues are closed." (2026-11-16); on epics with later stories: chose "Keep open, add increment-done" (2026-11-16)
- **Context:** Epics (`EP-xx`, label `type:epic`) are parent issues whose stories are GitHub sub-issues (DEC-050). Stories close when their PRs merge; epics stayed open until someone closed them by hand (EP-01 was closed by hand on 2026-11-16). Stories not planned yet are a "Later stories" checklist in the epic's body, not sub-issues.
- **Options considered:**
  1. Close as soon as all sub-issues are closed, reopen by hand when a later story is planned — simple, but the board says the epic is done while SRS stories are still left, and Actions cannot trigger on a sub-issue being linked, so reopening cannot be automated reliably.
  2. Keep the epic open while unchecked later stories remain, and mark the in-between state with an `increment-done` label — the board stays honest, nothing has to be reopened, and a label needs no `PROJECT_TOKEN`.
  3. Same as 2 without a label — nothing shows that the epic has no planned work left.
- **Decision:** option 2. `.github/workflows/epic-close.yml` runs `tools/close_epics.py` when an issue is closed, reopened or edited: it checks the issue itself if it is an epic, then its parent (found with the sub-issues API). For an open `type:epic` issue with at least one sub-issue: while any sub-issue is open, it stays open without `increment-done`; when all are closed but the "Later stories" section still has unchecked items (items that only link an issue are not counted), it stays open and gets `increment-done`; when all are closed and no later stories are left, it is commented on and closed as completed (and the check goes one level up). Run by hand (`workflow_dispatch`, optional dry run), it checks every open epic. Closed epics are never reopened. It uses the workflow's own token; the project's built-in "Item closed" workflow moves the card to Done, so it needs no `PROJECT_TOKEN`.
- **Rationale:** an epic covers all of its SRS stories, so it is closed only when all of them are done; the label still shows that an Increment's part of it is finished.
- **Consequences:** converting a later story to a sub-issue edits the epic's body, which runs the check and takes the label off. An epic with no sub-issues never changes by itself. EP-01 (#40) was closed by hand on 2026-11-16 with US-1.5 still in its later stories; under this rule it would be open with `increment-done`, and the workflow leaves it closed unless someone reopens it.
- **Docs updated:** `guidelines/git-workflow.md` §1, `CONTRIBUTING.md`, `.github/labels.yml`, `agents/state/current-status.md`.
- **Course documents affected (for humans):** none.
- **Revisit when:** later stories are planned as sub-issues up front, or GitHub Actions can trigger on sub-issue links.
- **Supersedes / Related:** related DEC-050.

### DEC-063 — US-14.1: the Editor's main window with Dear ImGui
- **Date:** 2026-11-16
- **Status:** Accepted (carried by the US-14.1 pull request; binds once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (library choice, 2026-11-17); the rest by the reviewer of the US-14.1 pull request   **Recorded by:** claude (US-14.1 thread)
- **Human's words:** Işık Dönger, US-14.1 thread, 2026-11-17, after seeing the themed editor: "okey it looks good. then I approve ImGui+rIlmGui."

- **Proposed by:** claude (US-14.1 thread), from srs.fr.14.1 to 14.6, 14.12 to 14.27, 15.15, 15.22 and the B-01, C-01 and D-01 to D-09 frames
- **Context:** US-14.1 (#77) asks for the main window with the menu bar, toolbar, the Default panel layout and the status bar (srs.fr.14.1), the project and scene in the title with "*" for unsaved changes (14.2), the unsaved-changes check before closing (14.3), the nine menus (14.4), disabled items in muted text with no hover effect (14.5) and Ctrl/Cmd shortcuts (14.6). raylib draws shapes and text but has no menus, tabs or docking, and no UI library had been chosen (tech-stack.md). Later stories need tab docking, splitting and floating panels (srs.fr.14.12 to 14.15). No project, scene or Unsaved Changes dialog (F-01) exists yet.
- **Options considered:**
  1. UI library: (a) Dear ImGui, docking branch, with the rlImGui backend for raylib; (b) raygui, raylib's own immediate-mode GUI; (c) our own widgets drawn with raylib. (a) is chosen: menus, disabled items, tabs and the dock builder are built in, and the docking stories reuse them; (b) and (c) have no menus or docking. The choice was offered to the human with (a) recommended.
  2. Where the backend lives: rlImGui includes raylib.h, so it is built in `engine/rendering-raylib` (R-4); the editor uses only `imgui.h`.
  3. What a not-yet-built menu item does: (a) show it disabled; (b) leave it out. (a) is chosen: the menus match D-01 to D-09 now, and srs.fr.14.5 already says an item is disabled while its condition is not met.
- **Decision:**
  1. Dear ImGui v1.93.0-docking and rlImGui (commit 5c17e70, no release tags) are fetched by `cmake/Dependencies.cmake` only when raylib is on. `imgui` is a static library target of the core files; `rlimgui` is built in `engine/rendering-raylib` without our warning flags, with the icon font left out. `engine::rendering_raylib::ImGuiRenderer` is the `IFrameRenderer` that owns the Dear ImGui context: it creates it in the first render frame, measures the frame time itself (custom frame control, DEC-051, leaves raylib's frame time unset) and releases it in `releaseGraphics()`.
  2. `editor_core` (always built, linked by the tests) holds the window-free parts: `Command` (label, shortcut, enabled condition, action), `shortcutText()` with Ctrl on Windows and Cmd on macOS (srs.fr.14.6; Exit is Alt+F4 / Cmd+Q, srs.fr.15.15), the nine menus and the C-01 toolbar from `buildMainMenu()` and `buildToolbar()`, and `EditorSession` with the project, active scene, unsaved items (srs.fr.15.24), the window title (srs.fr.14.2: "Aegis Editor 0.1 — <project> — <scene>*", following B-01) and `confirmClose()`.
  3. A command without an enabled condition is a feature not built yet and is drawn disabled (srs.fr.14.5); an empty submenu (Open Recent) is disabled too. Enabled now: File > Exit and Window > General's six panels (Ctrl+1 to Ctrl+6). The toolbar is all disabled until the Scene view and Play mode stories.
  4. `MainWindowView` (editor, Dear ImGui) draws the menu bar, the toolbar and the status bar as viewport side bars and the panels in a dock space with the Default layout of srs.fr.14.1, built with the dock builder at start: Hierarchy left, Scene and Game centre, Inspector right, Project and Console bottom. No imgui.ini is written until the layout stories save layouts. Enabled commands also run from their shortcuts.
  5. `engine/core`: `ApplicationConfig::confirmClose` is asked when the user asks to close the window and can keep it open; `Application::requestClose()` ends a normal-mode run after the render frame (File > Exit, after its own check); `Application::setWindowTitle()` and `IWindow::setTitle()` change the title; `WindowConfig::escapeClosesWindow` lets the Editor keep Escape (Stop Headless Run, D-06); `IFrameRenderer::releaseGraphics()` runs right before the window closes.
  6. `EditorSession::confirmClose()` returns true with nothing unsaved. With unsaved items it keeps the Editor open and flags a request for the Unsaved Changes dialog (F-01), which closes the Editor itself when it comes with US-15.x. Stopping a simulation operation first (srs.fr.15.25) comes with Play mode.
- **Rationale:** Dear ImGui is the common choice for engine editors, covers srs.fr.14.5 and the later docking requirements out of the box, and keeps raylib inside its adapter target. Keeping the menu model and session out of Dear ImGui lets the tests check srs.fr.14.2 to 14.6 without a window.
- **Consequences:** two new pinned dependencies (changing their versions is a decision, coding-standards §6). The Editor has no project until the Project Hub (US-13.x), so it shows "No project open". Fonts and colours follow in DEC-064. Docking by drag, floating panels and saved layouts (srs.fr.14.12 to 14.27) build on this dock space.
- **Docs updated:** `architecture/tech-stack.md`, `architecture/engine-layer.md`, `agents/state/current-status.md`, `agents/state/handoff-log.md`.
- **Course documents affected (for humans):** none; this follows SRS 6.1.
- **Revisit when:** panels in separate operating-system windows are wanted again (rlImGui has no multi-viewport support, DEC-065).
- **Supersedes / Related:** DEC-033, DEC-048, DEC-051, US-14.1 (#77).

### DEC-064 — US-14.1: the Editor's theme follows the Figma design
- **Date:** 2026-11-16
- **Status:** Accepted (carried by the US-14.1 pull request; binds once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-14.1 pull request   **Recorded by:** claude (US-14.1 thread)
- **Human's words:** Işık Dönger, US-14.1 thread, 2026-11-17: "first build on top of the already built ImGui and adds the theme to make it look like the figma design so I can see how far off it will be."
- **Proposed by:** claude (US-14.1 thread), from the B-01, B-02 and C-01 frames and the palette of `aegis-figma-update-6`
- **Context:** the first version used Dear ImGui's stock dark style and its ASCII-only pixel font, so it looked unlike the design. The design uses Inter (Regular, Semi Bold) and JetBrains Mono, a grey palette with a #3a79bb accent, 3 px corners and icons on the toolbar.
- **Options considered:**
  1. Fonts: (a) commit the three TTF files (about 1.1 MB, SIL Open Font License) and embed them in the executable; (b) download the release archives at configure time (Inter's is 34 MB); (c) load them from files beside the executable. (a) is chosen: it builds offline, keeps `editor.exe` a single file, and `.gitattributes` already treats `*.ttf` as binary assets.
- **Decision:**
  1. `editor/assets/fonts/` holds Inter 4.1 Regular and Semi Bold and JetBrains Mono 2.304 Regular with their licenses and a README naming the sources. `cmake/EmbedFile.cmake` (`embed_file()`) turns each into a generated C++ array linked into `editor`.
  2. `editor/src/theme.cpp` (`applyTheme()`) loads the fonts at 13 px and sets Dear ImGui's colours and sizes from the design's palette: panels #282828, bars #232323, tabs #2d2d2d / #3c3c3c with a #3a79bb top line, selection #2c5d87, text #e6e6e6, 3 px corners. `ImGuiRenderer` takes a setup callback for this and scales fonts and sizes by the display's DPI scale.
  3. rlImGui's Font Awesome 6 icons are turned back on and merged into Inter Regular; the toolbar uses them, laid out as in C-01 (tools left, mode switch, run buttons and tick counter centre, AI Assistant and Layout right). The status bar shows the mode, the FPS and the AI service state (offline) at the right.
  4. Dear ImGui's dock window-menu button is hidden; the design's ⋮ menu (E-07) comes with the docking stories.
- **Rationale:** shows how close Dear ImGui gets to the design (asked by the human) with styling only, no custom widgets.
- **Consequences:** the window's title bar stays the operating system's, not the design's custom one. Disabled controls are drawn at 45 % opacity (srs.fr.14.5), so the toolbar looks fainter than the design, which shows them enabled.
- **Docs updated:** `editor/assets/fonts/README.md`, `agents/state/handoff-log.md`.
- **Course documents affected (for humans):** none.
- **Revisit when:** the design changes its fonts or palette, or a light theme is wanted.
- **Supersedes / Related:** DEC-063, US-14.1 (#77).

### DEC-065 — srs.fr.14.15: floating panels stay inside the main window
- **Date:** 2026-11-17
- **Status:** Accepted
- **Decided by:** Işık Dönger   **Recorded by:** claude (US-14.1 thread)
- **Human's words:** US-14.1 thread, 2026-11-17: "I also decide changing srs.fr.14.15 wording to floating panel inside the main window."
- **Proposed by:** claude (US-14.1 thread)
- **Context:** srs.fr.14.15 said that a panel tab dragged outside the main window becomes a floating window. With Dear ImGui and rlImGui (DEC-063) that needs multi-viewport support, which rlImGui lacks because raylib manages one window. The options put to the human were (a) reword the requirement to a floating panel inside the main window, (b) replace rlImGui with Dear ImGui's GLFW and OpenGL backends (about 3 to 5 days, more risk on macOS), (c) leave the requirement out of scope.
- **Decision:** (a). srs.fr.14.15 now reads: "When the user drags a panel tab out of its dock and drops it where no dock accepts it, the Editor shall show the panel as a floating panel inside the main window." Dear ImGui's docking already behaves this way.
- **Rationale:** costs nothing, keeps the requirement testable and keeps option (b) open for a later increment.
- **Consequences:** the docking story that holds srs.fr.14.15 is met by Dear ImGui's floating docked windows. A panel cannot be moved to another monitor without the main window spanning it.
- **Docs updated:** `history/decision-log.md`. Agents in this repository do not change the SRS or other deliverables in team1-sp/docs; the team updates the SRS wording there.
- **Course documents affected (for humans):** SRS A, srs.fr.14.15 (Section 6.1).
- **Revisit when:** separate operating-system windows are wanted, which needs option (b).
- **Supersedes / Related:** DEC-063.

### DEC-066 — US-14.2: the status bar shows the Editor's state from a window-free model
- **Date:** 2026-11-17
- **Status:** Accepted (carried by the US-14.2 pull request; binds once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-14.2 pull request   **Recorded by:** claude (US-14.2 thread)
- **Human's words:** Işık Dönger, US-14.2 thread, 2026-11-17: "Can you start on the US-14.2."
- **Proposed by:** claude (US-14.2 thread), from srs.fr.14.7 to 14.11 and the status bars of the B-01, B-02, B-04, B-08 and C-08c frames and the B-01 and D-06 spec cards
- **Context:** US-14.2 (#78) asks the status bar for the last Console message (14.7), the mode, FPS and entity count (14.8), the AI service dot (14.9), a red REC while telemetry records (14.10) and the active debug flags in orange (14.11). None of the sources exist yet: the Console log is US-24.1, Play mode EP-17, scenes and entities EP-02/EP-18, the AI service settings US-33.5, the Telemetry panel EP-30 and debug flags EP-12. The design shows no frame with debug flags on, and none with an empty message.
- **Options considered:**
  1. Click targets that are not built yet (Project Settings F-04, the Telemetry panel): (a) keep the item visible but not clickable until its story, like a disabled menu item (srs.fr.14.5); (b) add empty action methods now. (a) is chosen: no dead calls.
  2. Where the debug flags go: (a) first in the right-hand group, before REC; (b) in the message area. (a) is chosen: the message area changes with every log line, so the flags would be hidden.
- **Decision:**
  1. `editor_core` gets `EditorStatus` (last Console message, `EditorMode` Edit/Play/Paused/Headless, headless ticks per second, optional entity count, `AiServiceState` and service name, telemetry recording, active debug flag names) and `buildStatusBar()`, which turns it into a `StatusBar`: the message at the left and, at the right, debug flags, REC, mode, FPS, entities and AI service, in this order. Each `StatusItem` has its text, a muted or warning colour, an optional dot and a `Command` for its click. The stories that own each source fill `EditorStatus`; the Editor owns one in `main.cpp`.
  2. Texts follow the design: "Edit mode", "Play mode", "Paused", "Headless run"; "FPS —" in Edit mode, the render frame rate in Play mode and while paused, "FPS 2 840 ticks/s" in a headless run; "Entities 24", "Entities —" with no scene; "AI service: offline" (grey #6a6a6a dot), "AI service: connected" or "AI service: local · connected" (green #6bcb3c dot); "REC telemetry" with a red #e04a4a dot; flag names joined by commas in #e0b03c. Numbers are grouped in threes with a space. No message is shown while the Console has none.
  3. Clicking the message opens the Console panel (srs.fr.14.7). The AI service and REC items name their targets but stay unclickable until US-33.5 and EP-30 add them. A clickable item highlights under the mouse, shows a hand cursor and its target as a tooltip. A message too long for the room left is cut with "...".
  4. `MainWindowView` draws the bar 22 px high in #2a2a2a with a #1a1a1a line above and #a8a8a8 text, from the B-01 palette.
- **Rationale:** the tests check srs.fr.14.7 to 14.11 without a window, and each later story only sets one field.
- **Consequences:** a fresh Editor shows "Edit mode · FPS — · Entities — · AI service: offline" and no message. srs.fr.14.9 and 14.10's clicks are met once US-33.5 and EP-30 enable them.
- **Docs updated:** `agents/state/current-status.md`, `agents/state/handoff-log.md`.
- **Course documents affected (for humans):** none; this follows SRS 6.1.
- **Revisit when:** the design adds a frame with debug flags on, or the Console story wants severity icons in the status bar.
- **Supersedes / Related:** DEC-063, DEC-064, US-14.1 (#77), US-14.2 (#78).

### DEC-067 — US-11.2 uses a read-only assistant core, AgentBridge streaming and user-local chats
- **Date:** 2026-11-22
- **Status:** Accepted (carried by the US-11.2 pull request; binds once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-11.2 pull request   **Recorded by:** codex/gpt-5/us112
- **Context:** US-11.2 (#72) requires a read-only assistant, manual Semantic Graph design
  suggestions, configured gRPC responses through AgentBridge, cancellation and user-local chat
  persistence. `main` had only the transport-neutral synchronous playtest `AgentBridge`; the
  Semantic Graph, project path service, assistant core and production gRPC contract did not exist.
  The issue explicitly permits only the smallest stable interfaces for those gaps and excludes the
  chat UI (#119), grounded diagnosis (#71) and settings UI (#121).
- **Options considered:**
  1. Assistant placement: (a) a window-free top-level C++ `assistant/` target above the bridge and
     Semantic layer seams; (b) put orchestration in `editor_core`; (c) put it only in the Python AI
     service. (a) keeps the product layer independent from the UI while the configured C++ client can
     call the separate Python service. (b) couples the layer to the Editor and (c) cannot enforce the
     local client-side read-only/storage boundary.
  2. Responses: (a) server-streaming `Answer` returning text chunks, opaque citations and category
     names, with cancellation on the stream; (b) a unary answer. (a) preserves partial answers and
     maps Stop to gRPC `ClientContext::TryCancel()` without a second transport.
  3. Project paths: (a) inject `UserProjectPaths::userLocalArea()` and append
     `assistant/chats`; (b) invent a concrete hidden-directory name now. (a) makes shared settings
     unreachable and leaves the concrete cross-platform project layout to its owning story.
- **Decision:**
  1. `aegis::assistant` is a window-free C++ target. `ReadOnlyAssistant` receives only a const
     `SemanticGraphView`, read-only `DataTransmissionPolicy`, `AgentBridge` and `ChatStore`; it
     receives no engine state, project-source writer or general project filesystem interface and
     rejects disallowed data categories before starting the bridge. Design suggestions are
     `MessageContent::DesignSuggestion` text with no apply operation.
  2. The production `AgentBridge` protobuf service has the existing `Exchange` RPC and a
     server-streaming `Answer` RPC. The transport-neutral C++ interface exposes a cancellable
     `AssistantResponseStream`; optional `engine::bridge_grpc` owns all gRPC/protobuf types and maps
     cancellation to `TryCancel()`. Default builds keep communication dependencies out (DEC-038).
  3. Chat format `AEGIS_CHAT 1` stores every message's role, state, content kind, text, citations and
     string data-category names under `<userLocalArea>/assistant/chats`. Saves replace valid files
     through temporary/backup paths and refuse to overwrite corrupt or unsupported data.
- **Rationale:** This is the smallest vertical slice that enforces every US-11.2 boundary while
  leaving the UI, evidence gathering, citation semantics, service/model settings and concrete
  project-local directory policy to their owning stories.
- **Consequences:** The future project layer supplies `UserProjectPaths`; the Semantic layer
  implements the read-only view; #121 supplies a validated address/model and constructs the gRPC
  bridge; #119 runs/cancels operations and displays persisted history; #71 interprets structured
  citations. New citation kinds and data categories require no format change because their values
  are stored as strings.
- **Docs updated:** `architecture/ai-assistant-layer.md`, `architecture/semantic-layer.md`,
  `architecture/tech-stack.md`, `guidelines/repository-structure.md`,
  `testing/test-checklists.md`, `agents/state/task-board.md`, `agents/state/current-status.md`,
  `agents/state/handoff-log.md`.
- **Course documents affected (for humans):** none; this implements SRS A srs.fr.11.8–11.12.
- **Revisit when:** #71 fixes concrete citation schemas, the project-local path owner defines the
  directory layout, or AgentBridge streaming/deadline evidence requires a wire revision.
- **Supersedes / Related:** DEC-001, DEC-038, DEC-039, DEC-045, TASK-020, US-11.2 (#72).

### DEC-068 — US-24.1: structured Console logging and navigation contracts
- **Date:** 2026-11-22
- **Status:** Accepted (carried by the US-24.1 pull request; binds once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-24.1 pull request   **Recorded by:** codex/gpt-5/104-console
- **Proposed by:** codex/gpt-5/104-console, from srs.fr.24.1 to 24.11 and issue #104
- **Context:** The Editor had an empty Console panel and `EditorStatus.lastConsoleMessage`, while `Application` wrote warnings and errors directly to stderr. US-24.1 needs one structured entry format, non-destructive view operations, Error Pause through the existing game loop, Clear on Play before US-17.2's controller exists, and testable source/entity activation before the related Preferences and Hierarchy stories exist.
- **Options considered:**
  1. Store Console entries in the ImGui view, or own them in a window-independent `ConsoleModel`. The model is chosen so headless tests cover all state and projection behavior.
  2. Collapse by message text alone, or require every structured field to match. Full equality is chosen so messages at different ticks or with different stacks, sources or entities never lose important inspection context.
  3. Search with exact case, or ignore case. Case-insensitive substring search is chosen to match the Hierarchy search rule and normal editor search behavior.
  4. Call operating-system/editor and selection code directly, or inject interfaces. `IExternalCodeEditor` and `IEntityNavigator` are chosen; the production launcher runs the configured command without a shell and entity navigation updates shared selection state.
- **Decision:** `engine::LogEntry` is the canonical owning record: severity, complete message, optional tick, stack trace, source location and entity. `ConsoleModel` owns entries in insertion order on the Editor/game-loop thread, supplies counters and index-based filtered/collapsed rows without mutating entries, updates `EditorStatus`, and exposes `onPlayStarted()`/`onPlayStopped()` until #85 connects the final Play controller. Collapse compares every `LogEntry` field and search is a case-insensitive substring of the message. The first ticked Error of a Play run calls an injected callback wired to `Application::requestPause()`.
- **Rationale:** this keeps Dear ImGui out of state/filtering tests, preserves all diagnostic context, reuses the accepted tick-boundary pause mechanism, avoids global mutable logger state and prevents tests from opening external applications.
- **Consequences:** `ApplicationConfig::logSink` optionally receives structured entries and stderr remains the fallback for command-line/headless use. The Editor's external code-editor command defaults to `code` and can be set with `AEGIS_CODE_EDITOR`; it receives `--goto file:line`. US-17.2 (#85) must call the two lifecycle methods at its existing Play transition, and future Hierarchy/Inspector/Scene work observes `EntityNavigation::selectedEntity()`.
- **Docs updated:** `design/console-logging.md`, `architecture/engine-layer.md`, `guidelines/repository-structure.md`, `testing/test-checklists.md`, agent state and handoff.
- **Course documents affected (for humans):** none; this implements SRS A srs.fr.24.1 to 24.11.
- **Revisit when:** US-17.2 introduces the Play controller, Project Preferences owns the editor command, or entity selection gains scene-qualified/stable references.
- **Supersedes / Related:** DEC-057, DEC-060, DEC-063, DEC-066, TASK-021, GitHub #104, #85, #88, #123.

### DEC-069 — US-33.5 uses a window-free settings controller and AgentBridge service discovery
- **Date:** 2026-11-22
- **Status:** Accepted (carried by the US-33.5 pull request; binds once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-33.5 pull request   **Recorded by:** codex/gpt-5/121-ai-privacy
- **Context:** US-33.5 (#121) needs independently testable address validation, asynchronous health
  and model discovery, address-race protection, privacy policy and per-user project persistence.
  Merged #141 supplied an address-bound `AgentBridge` answer stream but no health or model operation.
  The general Project Settings framework, Project Hub and per-question consent dialog are separate
  stories.
- **Options considered:**
  1. Call gRPC from Dear ImGui, or extend the transport-neutral AgentBridge seam. The seam is chosen
     so UI/model tests use fakes and protocol types remain inside the optional transport target.
  2. Let asynchronous callbacks mutate controller state, or poll typed completions carrying request
     identifiers and addresses. Polling is chosen so mutations stay on the UI thread and stale
     completions are deterministically rejected.
  3. Store these values with shared project settings, or under an injected current-user project
     area. The user-local area is required by srs.fr.33.57 and prevents privacy choices from entering
     version control.
- **Decision:** `AiPrivacySettingsController` owns all window-independent state and persists
  `AEGIS_AI_PRIVACY 1` beneath `<user-local-project>/settings/ai-privacy.settings` through temporary
  and backup replacement. `AiPrivacySettings` implements #72's `DataTransmissionPolicy` and exposes
  per-category consent need for #120. `AgentBridge` adds synchronous, deadline-taking
  `checkHealth()` and `listModels()` operations; `AgentBridgeRequestRunner` executes them
  asynchronously through an injected address-bound bridge factory and clock. The production bridge
  uses wire-compatible `grpc.health.v1.Health/Check` plus `AgentBridge.ListModels`; UI code sees only
  transport-neutral results. Dear ImGui renders the controller and the Edit menu, AI menu and status
  bar share one availability-checked action.
- **Rationale:** this is the smallest complete slice that keeps protocol code out of the UI, makes
  every timing/race transition fakeable, reuses #72's service boundary and stores no credentials.
- **Consequences:** Project-opening code supplies the user-local project root and a production bridge
  factory when that lifecycle lands. #120 reads `requiresConsent()` but owns the general send/dialog
  sequence. A successful health check triggers a fresh five-second model-list request; changing the
  address clears the model/connection and both address-bound permissions.
- **Docs updated:** `design/ai-privacy-settings.md`, `architecture/ai-assistant-layer.md`,
  `guidelines/repository-structure.md`, `testing/test-checklists.md`, agent state and handoff.
- **Course documents affected (for humans):** none; this follows srs.fr.33.28–33.33, 33.57 and
  srs.nfr.4.11.
- **Revisit when:** Project Hub supplies default service addresses, the cross-platform project path
  owner standardizes the concrete user-local root, or #120 connects the send pipeline.
- **Supersedes / Related:** DEC-038, DEC-045, DEC-067, TASK-022, GitHub #121, #120.

### DEC-070 — US-11.1: grounded answers, typed citations and an AgentBridge decorator
- **Date:** 2026-11-22
- **Status:** Accepted (part 1 decided in the conversation; parts 2–6 carried by the US-11.1 pull request, binding once a human merges it, DEC-052)
- **Decided by:** ardasaritas (part 1, in the US-11.1 agent session, 2026-11-22); the reviewer of the US-11.1 pull request (parts 2–6)   **Recorded by:** claude-code/opus-5.5/us111
- **Human's words:** asked how to proceed because #72's `AssistantRequest` has no field for evidence, attached context or earlier messages and `ReadOnlyAssistant` persists citations as they stream, the human chose "Decorator, no API change".
- **Context:** US-11.1 (#71) needs grounded answers with structured citations (srs.fr.11.1–11.7). DEC-067 left citation semantics, evidence and diagnosis to #71 and fixed the AgentBridge request as `question`, `semanticGraphStructure` and `dataCategories`. `main` has no Semantic Graph, telemetry recording, debug flags or EP-33 context items yet. The task forbids changing the shared #72 APIs concurrently.
- **Options considered:**
  1. Transport of grounded context: (a) an `AgentBridge` decorator that fills the existing `semanticGraphStructure` string with a versioned grounded-context document and filters the response stream; (b) stop and ask for new `AssistantRequest`/proto fields and a `ReadOnlyAssistant` hook; (c) domain logic only, no transport. (a) delivers every criterion without editing a shared header or the proto.
  2. Citation form: (a) typed C++ values serialised as fixed-order `key=value` lines under the kinds `state`, `entity`, `source`; (b) parse citations from answer text. (a) keeps citations as data (DEC-067 stores kind/payload strings).
  3. Evidence access: (a) narrow const query interfaces (`SemanticGraphQuery`, `RunTelemetryQuery`, `AssetCatalogQuery`, `ProjectSourceReader`) returning copies; (b) give the assistant engine or recording objects. (a) keeps the assistant read-only (DEC-067) and testable with fakes.
- **Decision:**
  1. `GroundedAgentBridge` decorates the configured `AgentBridge` for one question: it puts the `AEGIS_GROUNDED_CONTEXT 1` document in `semanticGraphStructure`, streams the deterministic diagnosis first, passes only AI-service citations that decode as typed citations and match the retrieved evidence (re-encoded canonically, the rest counted in a note), keeps only data categories the request carried, and refuses a request for another question. `GroundedAssistant::ask()` drives it through `ReadOnlyAssistant`, so cancellation and `ChatStore` persistence are #72's.
  2. Citations: `StateCitation{run, Start | tick}`, `EntityCitation{id, name, source scene, runtime-created, optional run with optional Start | tick}`, `SourceCitation{attached file, line from 1, content hash}`. Payloads: `run=…`/`point=start|tick:N`; `id`, `name`, `scene`, `runtime`, optional `run`, `point`; `file`, `line`, `hash`. Values escape only `%`, LF, CR. Decoding is strict. Start is never tick 0.
  3. Content hash: `sha256:` + 64 lowercase hex digits of the file bytes at question time, from a local FIPS 180-4 implementation (no new dependency, S-5).
  4. Context and consent: only the question, earlier messages of the chat, and the attached items are read. An attached item whose data category the policy does not allow, a Source code path outside `Scripts/` (or with `.`/`..`/empty segments, backslash, drive or absolute prefix), an unreadable file or more than 200 KB of sources refuses the whole question before anything is sent. Earlier messages carrying a now-denied category become `[earlier message left out: contains <category>]`; included messages pass their categories on (srs.fr.33.46, 33.56, 33.59). Categories: `semantic-graph`, `telemetry` (Last run, event nodes, collided with / triggered edges, run log entries), `source-code` (Source code, file names in graph nodes, script assets, stack traces). Last run defaults to the last 600 ticks; Start records are always kept.
  5. Debug flags (srs.fr.11.3): a flag is named as the cause only when the report does not say "not applied", its debug-injection record exists, and a retrieved record shows its effect: a position sample of another entity inside a removed collider's unrotated bounds (no collider, out of bounds); an `UnlockedEvent` for the target whose `requiredItem` is not in `inventory` (unlock without item); an event involving an added ring obstacle and another entity (unreachable objective). Otherwise the answer says not applied, not recorded, not exercised, or that the needed records are missing.
  6. A sampled value change is attributed only to a recorded write in the interval; declared writers of the component become a hypothesis prefixed "Hypothesis (not established by the evidence):", and the missing write record is named.
- **Rationale:** every #71 criterion is met with the #72 seams unchanged; the deterministic part makes the debug-flag and abstention rules testable without a model.
- **Consequences:** the Python AI service (later story) reads the grounded document from `semantic_graph_structure` and returns citations in the payload form above. If the wire later gets dedicated context fields, only `GroundedAgentBridge` changes. The Semantic layer, telemetry recording, asset catalogue and EP-33 chips implement the query seams and context items in their stories; until then they are exercised with fakes. Field-by-field removal of denied data (srs.fr.33.54), consent dialogs and request preview UI remain EP-33/#121.
- **Docs updated:** `architecture/ai-assistant-layer.md`, `architecture/semantic-layer.md`, `guidelines/repository-structure.md`, `testing/test-checklists.md`, `agents/state/task-board.md`, `agents/state/current-status.md`, `agents/state/handoff-log.md`, `history/problem-log.md`.
- **Course documents affected (for humans):** none; this implements SRS A srs.fr.11.1–11.7.
- **Revisit when:** the AgentBridge request gains structured context fields, the telemetry recording defines its record schema (srs.fr.8.29), or the demo game fixes the event and payload names used for unlocks and collisions.
- **Supersedes / Related:** DEC-038, DEC-067, DEC-069 (US-33.5 settings and `checkHealth()`/`listModels()`), TASK-020, TASK-023, US-11.1 (#71), #119, #121.

### DEC-071 — US-33.2: AI Assistant chat state, sending seam and threading
- **Date:** 2026-11-22
- **Status:** Accepted (part 1 decided in the conversation; parts 2–6 carried by the US-33.2 pull request, binding once a human merges it, DEC-052)
- **Decided by:** ardasaritas (part 1, stacking on the open #71 and #118 PRs, in the US-33.2 agent session, 2026-11-22); the reviewer of the US-33.2 pull request (parts 2–6)   **Recorded by:** claude-code/opus-5.5/119-chat
- **Human's words:** asked whether to wait for PRs #144 and #145 or build on them, the human answered "they are done, let's stack, continue".
- **Context:** US-33.2 (#119) needs the composer, Stop, chips, New Chat and History (srs.fr.33.5–33.12) over #71's `GroundedAssistant`/`ContextItem`, #72's `AnswerOperation`/`ChatStore` and #118's panel, without changing those interfaces. `AnswerOperation::run()` blocks while it writes the answer into the shared `Chat` under a private mutex, so another thread cannot safely read that message while it streams. #118's `IConsentAwareQuestionSender` returns nothing and carries only Selection and Last run, so it cannot report a rejection or a running answer.
- **Options considered:** (a) a new editor-owned sending seam returning a running-answer handle, worker thread per answer, draw the running answer only after it ends; (b) ask #72 for a thread-safe answer snapshot to show text while it streams; (c) extend #118's sender. (a) meets every criterion without touching shared headers; live streaming text is not one of srs.fr.33.5–33.12.
- **Decision:**
  1. Stack #119 on the open #71 (#144) and #118 (#145) branches; #144 has since merged.
  2. `AssistantChat` in `editor_core` owns the draft, chips, open chat and History; Dear ImGui only draws it. It sends only through `IChatQuestionSender`; production `GroundedChatQuestionSender` calls `GroundedAssistant::ask()` and runs the returned `AnswerOperation` on its own thread; Stop calls `AnswerOperation::cancel()` once. One answer at a time; New Chat and History wait until it ends.
  3. While an answer runs, the panel draws every message except the one being written, with a spinner and Stop; the full answer (or the stopped part) appears when the operation ends.
  4. Whitespace-only messages are empty. A rejected send keeps the draft and chips; a successful send clears the draft and keeps the chips for the next question. An entry point's Selection replaces only the Selection chip.
  5. A chip's size is `renderContextDocument()` with the item minus without it, both with an empty diagnosis (US-11.1 rewords its diagnosis when evidence is attached, which would hide the item's bytes). Measuring ignores the data-sharing policy because nothing is sent.
  6. History lists the chats of the current Editor session and reopens them with `ChatStore::load()`; citations stay `{kind, payload}` and are decoded only for display.
- **Rationale:** every criterion is met with #71, #72 and #118 interfaces unchanged and the state model testable without a window.
- **Consequences:** live streaming text needs a thread-safe answer snapshot on `AnswerOperation` (a #72 change). Listing chats saved in earlier Editor sessions needs a `ChatStore` listing operation. Graph nodes, Source code, Asset and Component type chips are attachable once their panels provide a selection. US-33.3 decorates `IChatQuestionSender` for consent and offline rules; US-33.5 supplies the bridge, model and policy.
- **Docs updated:** `architecture/ai-assistant-layer.md`, `guidelines/repository-structure.md`, `agents/state/task-board.md`, `agents/state/current-status.md`, `agents/state/handoff-log.md`, `history/problem-log.md`.
- **Course documents affected (for humans):** none; this implements SRS A srs.fr.33.5–33.12.
- **Revisit when:** #72 gains an answer snapshot or chat listing, or the consent story needs to intercept sends differently.
- **Supersedes / Related:** DEC-067, DEC-070, TASK-020, TASK-024, TASK-023, US-33.1 (#118), US-11.1 (#71), #120, #121.

### DEC-148.1 — Record ids carry the GitHub number, and append-only logs merge with `union`
- **Date:** 2026-11-22
- **Status:** Accepted (approach agreed in the conversation; details carried by PR #148, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (in the conversation "Review Arda's four AI assistant PRs", 2026-11-22); the reviewer of PR #148   **Recorded by:** claude-code/opus-5.5/148-ids
- **Human's words:** asked "is it better to be branch numbering or issue numbering?", was told the issue number (or the PR number when there is no issue), and answered "okey then create the thread for this pr."
- **Context:** parallel PRs append to the same logs and pick "highest id + 1", so two branches regularly take the same DEC, HND, PRB or TASK number (PR #33 vs #35/#37/#39, #145 vs #146, #147). Every sync then means renumbering entries and their references by hand, and the shared logs conflict on every merge with `main` even though both sides only appended. `check_ids.py` (DEC-055) and the `.githooks` hooks (#147) catch the collision but do not prevent it.
- **Options considered:** (a) keep sequential ids and renumber on collision (today); (b) story number in the id (`DEC-15.1.1`), which non-story work does not have and reads badly next to a counter; (c) the branch's GitHub number: the issue the PR closes, or the PR's own number. Issues and PRs share one number sequence, so (c) is unique across all work and links to it.
- **Decision:**
  1. New entries in the decision, problem, handoff, open-questions and GenAI logs use `<PREFIX>-<GitHub #>.<counter>`: `DEC-80.1`, `HND-80.2`, `PRB-142.1`, `Q-80.1`, `GEN-80.1`. The number is the issue the PR closes (`Closes #n`), or the PR's own number for `docs`/`ci`/`chore` work with the `no-issue` label. The counter starts at 1 for each prefix and number.
  2. Tasks are one per issue and use the issue number alone: `TASK-80`. Işık Dönger chose this over dropping TASK ids for new work (in the conversation, 2026-11-22).
  3. Existing entries keep their 3-digit ids; nothing is renumbered, and references to them stay valid.
  4. `.gitattributes` sets `merge=union` for the pure append logs only: `docs/agents/state/handoff-log.md`, `docs/history/decision-log.md` and `docs/history/problem-log.md`. `task-board.md`, `current-status.md` and `open-questions.md` are edited in place and are left out, because a union merge there would keep both the old and the new version of an edited line.
  5. `tools/check_ids.py` accepts both forms and rejects any other heading id. It keeps the duplicate and index checks. In pull request CI (`--base HEAD^1`), every id the PR adds must use one of its numbers (a `Closes #n` issue, or the PR number with `no-issue`). The `.githooks` hooks run the same checks without the PR-number match, which needs the PR.
- **Rationale:** two branches can no longer produce the same id, so syncing no longer means renumbering, and an id tells you which issue or PR to open. Union merges remove the remaining append-only conflicts for local merges, and the duplicate check catches the one way they can go wrong.
- **Consequences:** `merge=union` works only for a local `git merge origin/main`; GitHub's web conflict editor and the mergeability that the `Conflict flag` workflow reads ignore it, so a PR may still get `needs-sync` and is synced locally as before. If one side edits the last line of a log in place (for example the last index row's status) while the other appends after it, union keeps both versions; `check_ids.py` reports the doubled index row. Union also merges the identical `---` separator lines both sides add, so two new handoff entries may end up without a separator between them; that is cosmetic. The index is no longer in numeric order across old and new ids; it stays in merge order. Conflicts in `current-status.md` and `task-board.md` remain and are a separate, later decision (update them only after merging, or retire the task board in favour of the GitHub project).
- **Docs updated:** `guidelines/documentation-rules.md` §4, `guidelines/git-workflow.md` §4, `agents/communication-protocol.md` §6–7, `agents/AGENT-PROTOCOL.md`, `agents/templates/*`, `history/genai-usage-log.md` entry format, `CONTRIBUTING.md`, `.github/pull_request_template.md`, `.gitattributes`, `.githooks/pre-commit`, `.github/workflows/ci.yml`, `tools/check_ids.py`.
- **Course documents affected (for humans):** none.
- **Revisit when:** the team retires the task board, or ids from different PRs still collide.
- **Supersedes / Related:** refines DEC-055 (collided record ids are no longer expected for new entries); DEC-050, DEC-052, PR #147.
- **Amendments:** 2026-11-22 — part 2 (`TASK-<issue #>`) is superseded by DEC-149.1, which retires the task board and TASK ids.

### DEC-149.1 — Tasks are GitHub issues; the task board is retired and current-status.md is increment-level
- **Date:** 2026-11-22
- **Status:** Accepted (approach decided in the conversation; details carried by PR #149, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (in the conversation "Make record IDs collision-free", 2026-11-22); the reviewer of PR #149   **Recorded by:** claude-code/opus-5.5/148-ids
- **Human's words:** "updating on the main is not a option." Then: "I think retiring task-board.md and rewriting the current-status.md is the best approach."
- **Context:** after DEC-148.1, record ids no longer collide, but `task-board.md` and `current-status.md` still conflict between parallel PRs because every PR edits them in place (status rows, the "In progress / next" list, `last_updated`). Updating them only on `main` after merging is ruled out. The task board was written before DEC-050 as the agents' lock (claims without a lock server); since DEC-050 the GitHub issue's assignee, one branch per story and the Issue start workflow do that job, so the board only mirrors the project. Q-015 (TASK vs PB ids) was still open.
- **Options considered:** (a) update both files only after merging, on `main` (ruled out by the human); (b) keep both and live with the conflicts; (c) retire the task board and make `current-status.md` a page that rarely changes.
- **Decision:**
  1. Tasks are GitHub issues on the project (team1-sp #3). An agent's claim is the issue's assignee (the member it works for) plus the work branch; blocked work is a comment on the issue plus a `Q-…` entry; Review and Done follow the PR and the merge. No new `TASK` ids are created (this supersedes DEC-148.1 part 2).
  2. `docs/agents/state/task-board.md` moves to `docs/history/task-board-archive.md`, frozen, so `TASK-000` to `TASK-025` references still resolve. `templates/task.md` is removed. `check_ids.py` still checks the archive's ids.
  3. `current-status.md` holds only increment-level content: phase, what exists per area, health, and what every new agent must know. Its per-story "In progress / next" list, decision lists and `last_updated` header are removed. A PR edits it only when the change makes something on it wrong; per-PR progress goes in the handoff entry.
  4. AGENT-PROTOCOL, rules P-2/P-10 and section D, the communication protocol, the agents README, the development process, the record-decision playbook and the templates follow from 1–3. Q-015 is answered by this entry.
- **Rationale:** both files stop being edited by every PR, so they stop conflicting, and agents and humans read status in one place (the project) instead of two that drift.
- **Consequences:** agents need GitHub access to see what is planned and claimed; offline sessions read the issue text the human gives them. `plan.md` is stale (still Phase 1) and should be revisited separately. `open-questions.md` is still edited in place, but rarely.
- **Docs updated:** `agents/state/current-status.md` (rewritten), `history/task-board-archive.md` (moved, header), `agents/templates/task.md` (removed), `agents/AGENT-PROTOCOL.md`, `agents/rules.md`, `agents/README.md`, `agents/communication-protocol.md`, `agents/playbooks/record-decision.md`, `agents/templates/{handoff,problem-entry,decision-entry}.md`, `agents/state/open-questions.md` (Q-015), `design/templates/design-doc-template.md`, `process/development-process.md`, `guidelines/{documentation-rules,git-workflow}.md`, `docs/README.md`, `CONTRIBUTING.md`, `tools/check_ids.py`.
- **Course documents affected (for humans):** none known; SRS/Initial Plan text about a repo task board, if any, is for the humans to check.
- **Revisit when:** agents need an offline task list again, or `current-status.md` starts changing in most PRs.
- **Supersedes / Related:** DEC-148.1 (part 2), DEC-050, DEC-055, Q-015.
- **Amendments:** 2026-11-24 — the archive file of part 2, `history/task-board-archive.md`, is deleted by DEC-152.1; the `TASK-0nn` entries stay in git history.

### DEC-151.1 — Işık Dönger is the only code owner
- **Date:** 2026-11-22
- **Status:** Accepted (carried by PR #151, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (in the conversation "Make record IDs collision-free", 2026-11-22)   **Recorded by:** claude-code/opus-5.5/148-ids
- **Human's words:** asked whether removing `leylasezer` "as a codeowner and a reviewer" needs a PR or a repository setting, then: "you can now go ahead and also create the pr for codeowners change".
- **Context:** DEC-032 made Işık Dönger and the advisor, Leyla Sezer (`@leylasezer196`), the only code owners, so GitHub requested the advisor's review on every PR.
- **Decision:** `.github/CODEOWNERS` lists only `@isikdonger`. The `main` ruleset stays as it is (one code-owner approval, admin bypass through a PR), so every other member's PR needs Işık's approval and Işık's own PRs still need none. The advisor's repository access and Product Owner role are not changed by this entry.
- **Rationale:** `[rationale not given beyond the words above]`
- **Consequences:** GitHub no longer requests the advisor's review automatically; the advisor can still review when asked, but that approval does not count toward merging. Merges of other members' PRs depend on Işık alone.
- **Docs updated:** `.github/CODEOWNERS`, `CONTRIBUTING.md`, `guidelines/git-workflow.md`.
- **Course documents affected (for humans):** none known.
- **Revisit when:** another approver is needed (for example while Işık is unavailable).
- **Supersedes / Related:** supersedes the advisor part of DEC-032; DEC-024.

### DEC-46.1 — ADR-001: the ECS stores components in sparse sets, one independent pool per component type
- **Date:** 2026-11-05 (recorded 2026-11-23)
- **Status:** Accepted (carried by the US-2.1 pull request, binding once a human merges it, DEC-052)
- **Decided by:** Arda Karabulut (owner of PB-010, which proposed it) and Işık Dönger (approved and merged PR #28 on 2026-11-05); recorded at Arda Karabulut's request on 2026-11-23   **Recorded by:** claude-code/opus-5.5/us-2.1
- **Human's words:** Arda Karabulut, 2026-11-23, asked how to record ADR-001 and the PB-010 D-decisions: "İki giriş" (two entries: ADR-001, then the D-table).
- **Context:** PB-010.1 compared storage models for the custom ECS (DEC-018) at our scale: scenes up to 1,000 entities, 200 active, at least 600 headless ticks per second ([research/ecs-storage-research.md](../research/ecs-storage-research.md)). PR #28 merged the research and the API design built on it (D-1), but ADR-001 and the D-decisions were never written into this log: PB-010.3 has no issue. US-2.1 is the first code that depends on them.
- **Options considered:**
  1. Sparse sets: one `ComponentPool<T>` per type, O(1) add and remove that touch one pool, one packed array per type (EnTT).
  2. Archetypes: one table per component combination, the fastest multi-component iteration, but every add or remove moves the entity with all its components (Unity Entities 6.5).
  3. A "big matrix" of arrays indexed by entity, with holes.
- **Decision:** Components are stored in sparse sets: one independent `ComponentPool<T>` per component type, with no groups and no chunks. Every pool is kept in ascending `EntityId` (D-3).
- **Rationale:** the simplest correct storage for a student team; cheap adds and removes for gameplay roles and tags; every component type is one packed, enumerable array for the Semantic Graph export; at our scale iteration speed does not decide the choice (research §8, §9).
- **Consequences:** a multi-component query walks the smallest pool and probes the others. Groups, pagination, pointer stability and multithreading are left out; they can be added later without changing the API. The ideas taken from Unity (command buffers, tags, singleton components, cleanup through observers) do not depend on the storage model (research §9).
- **Docs updated:** none; the research and the design already describe it.
- **Course documents affected (for humans):** none known.
- **Revisit when:** profiling shows multi-component iteration dominating a tick, or the entity counts grow by orders of magnitude.
- **Supersedes / Related:** DEC-018, DEC-043, DEC-46.2; D-1; PB-010 (PR #28).

### DEC-46.2 — The PB-010 design decisions D-2 to D-29 are the ECS design
- **Date:** 2026-11-05 (recorded 2026-11-23)
- **Status:** Accepted (carried by the US-2.1 pull request, binding once a human merges it, DEC-052)
- **Decided by:** Arda Karabulut (the PB-010 design choices) and Işık Dönger (review of PR #28, whose changes were applied; approved and merged on 2026-11-05); recorded at Arda Karabulut's request on 2026-11-23   **Recorded by:** claude-code/opus-5.5/us-2.1
- **Human's words:** Arda Karabulut, 2026-11-23: "İki giriş" (see DEC-46.1).
- **Context:** `design/ecs-api-design.md` §2 lists the decisions D-1 to D-29, made from Arda Karabulut's answers in PB-010.2. Işık Dönger's review of PR #28 changed five of them (sort at playback D-3, `{offset, FieldType, count}` fields D-6, structural-power declarations D-21, handler phases D-22, agent I/O D-25) and settled the related gaps in DEC-040 to DEC-044. They were to be logged in PB-010.3, which never got an issue.
- **Options considered:** (a) one entry per decision; (b) one entry for the whole table, referring to §2; (c) each decision with the story that builds it. Arda Karabulut chose (b).
- **Decision:** D-2 to D-29, as written in `design/ecs-api-design.md` §2, are the design every ECS story follows; D-1 is DEC-46.1. Each story builds the parts it needs. Changing one of them later needs its own entry that supersedes that item. The event bus choices E-1 to E-5 (`ecs-event-bus-design.md` §2) are recorded with US-2.8.
- **Rationale:** one place in this log that binds the design the ECS stories follow, without copying 28 table rows.
- **Consequences:** D-26 (build phases) is read together with DEC-043 and DEC-46.3. The design's SRS references follow the final SRS A of 2026-11-14 in §2, §4, §5, §7 to §9, §12 and §13; the event bus document is updated with US-2.8.
- **Docs updated:** `design/ecs-api-design.md` (SRS numbers), `design/ecs-design.md` (SRS numbers in §1).
- **Course documents affected (for humans):** none known.
- **Revisit when:** an ECS story finds that one of the decisions does not work as designed.
- **Supersedes / Related:** DEC-018, DEC-040 to DEC-044, DEC-46.1, DEC-46.3; PB-010 (PR #28).

### DEC-46.3 — US-2.1 and US-2.2 build the hierarchy and the entity and component enabled flags
- **Date:** 2026-11-23
- **Status:** Accepted (carried by the US-2.1 pull request, binding once a human merges it, DEC-052)
- **Decided by:** Arda Karabulut (owner of US-2.1 and US-2.2), 2026-11-23   **Recorded by:** claude-code/opus-5.5/us-2.1
- **Human's words:** asked whether the two stories build only the Phase 1 slice or everything their acceptance criteria list, he chose "Hepsini yap" (build all of it).
- **Context:** DEC-043 limits the ECS of this semester to the Phase 1 slice of `ecs-api-design.md` §17, which listed the hierarchy (`setParent`, recursive destroy) and the entity and component enabled flags under "Later". The Increment 1 stories US-2.1 (srs.fr.2.4, 2.5) and US-2.2 (srs.fr.2.11) require them, and other Increment 1 stories build on them: the Hierarchy panel, the Inspector's enabled checkboxes, rendering of enabled entities and the demo loop. Phase 1's own Collect rule disables entities (srs.fr.12.2), so §17 was inconsistent on its own.
- **Options considered:** (a) build everything the two stories list; (b) build Phase 1 only and leave srs.fr.2.4, part of 2.5 and 2.11 unchecked; (c) the enabled flags now and the hierarchy later.
- **Decision:** (a). US-2.1 builds the hierarchy and the entity enabled flag; US-2.2 builds the component enabled flags, as `ecs-api-design.md` §4 to §6 and §9 design them. DEC-043 still holds for everything else.
- **Rationale:** both stories can close with all of their acceptance criteria met, and the stories that build on them are not held back by a follow-up. `[further rationale not given]`
- **Consequences:** §17 lists these items under Phase 1. Skipping disabled entities and components in systems and queries comes with US-2.3 (srs.fr.2.15, 2.18). Whether US-2.3, US-2.4 and US-2.12 build more than Phase 1 is decided when each starts.
- **Docs updated:** `design/ecs-api-design.md` §17, `design/ecs-design.md` §2, DEC-043 (Amendments line).
- **Course documents affected (for humans):** none.
- **Revisit when:** the semester's scope is cut again.
- **Supersedes / Related:** amends DEC-043 and D-26; DEC-46.2; US-2.1 #46, US-2.2 #47.

### DEC-46.4 — US-2.1: ID reuse rules, run start, command buffer errors and created-then-destroyed parents
- **Date:** 2026-11-23
- **Status:** Accepted (carried by the US-2.1 pull request, binding once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-2.1 pull request   **Recorded by:** claude-code/opus-5.5/us-2.1
- **Context:** the design (§4, §9) leaves a few details of entity creation and the command buffer open; US-2.1 needs an answer for each to meet srs.fr.2.1 to 2.4.
- **Decision:**
  1. An Editor-range ID may be given to an entity whenever no loaded entity uses it, because Undo and Revert restore an entity with its own ID (srs.fr.2.105); keeping the Editor from giving a discarded ID to a different entity is the Editor's watermark (srs.fr.2.62, US-2.12). A runtime-range ID is used at most once per run and only after `allocateRuntimeId()` reserved it (srs.fr.2.2, 2.63).
  2. `World::beginRun()` restarts the runtime IDs and is refused while an entity with a runtime ID is alive, because its ID would be given out again (srs.fr.2.2).
  3. A `CommandBuffer` refuses a stale handle when the command is recorded (`RunError`, coding standard rule 9). At playback, a command that no longer applies is skipped with a warning to the World's `ILogSink`, or to the standard error output when the World has none.
  4. An entity created and destroyed in the same buffer never appears and no observer hears of it (§9), unless the buffer also parents an entity to it; then it is created and destroyed for real, so that the child is destroyed with it (srs.fr.2.4).
  5. When an entity is destroyed, every observer hears about the whole subtree, parents before children and children in order, before any of its components are removed.
- **Rationale:** each closes a gap without changing the designed API, and none lets an ID reach a second entity.
- **Consequences:** a host starts a run with `beginRun()` on a World with no runtime entities left, for example after restoring the Play mode snapshot.
- **Docs updated:** `design/ecs-api-design.md` §9 (the created-then-destroyed row).
- **Course documents affected (for humans):** none.
- **Revisit when:** the snapshot design (D-24) or the scene loader needs other ID rules.
- **Supersedes / Related:** D-2, D-4, D-5, D-29; DEC-46.2.

### DEC-47.1 — The built-in components have no enabled flag of their own
- **Date:** 2026-11-23
- **Status:** Accepted (carried by the US-2.2 pull request, binding once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-2.2 pull request   **Recorded by:** claude-code/opus-5.5/us-2.2
- **Context:** srs.fr.2.11 gives every component except Position and Velocity an enabled flag. The design stores an entity's name, tag, layer and parent as the built-in components Name, EntityInfo and Hierarchy (D-12), which the SRS describes as properties of the entity (srs.fr.2.5) and which the Inspector shows in the entity's header, not as component foldouts with their own checkbox (srs.fr.22.1).
- **Options considered:** (a) the built-ins have no flag, like Position and Velocity; the entity's enabled flag covers them; (b) they get a flag like any other component, which would let a system "disable" an entity's name or parent link.
- **Decision:** (a). Name, EntityInfo and Hierarchy have no enabled flag; `World::setComponentEnabled` refuses them, as it refuses Position and Velocity.
- **Rationale:** they are the entity's own data, read even for disabled entities (the Hierarchy panel, scene saving), and a disabled parent link would break srs.fr.2.4 and 2.5.
- **Consequences:** queries treat them as always enabled; only the entity flag hides them (US-2.3).
- **Docs updated:** `design/ecs-api-design.md` §6.
- **Course documents affected (for humans):** none.
- **Supersedes / Related:** D-12, D-13; DEC-46.3.

### DEC-47.2 — The field kinds of US-2.2 and when the others come
- **Date:** 2026-11-23
- **Status:** Accepted (carried by the US-2.2 pull request, binding once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-2.2 pull request   **Recorded by:** claude-code/opus-5.5/us-2.2
- **Context:** the field types of `ecs-api-design.md` §11 have no kind for the built-in fields `EntityInfo.layer` (`std::uint8_t`), `EntityInfo.tag` (`TagId`) and `Hierarchy.childCount` (`std::uint32_t`). §17 leaves managed components for later, and the first `FixedArray` field is the Inventory of the demo loop.
- **Decision:**
  1. `FieldType` gains `UInt8`, `UInt32` and `Tag`. A `FieldValue` holds both unsigned kinds as `std::uint32_t` and writing checks the range.
  2. Enum fields need an `enum class` with an unsigned underlying type; a `FieldValue` holds the value as `std::int64_t`. `TypeRegistry::enumeration<E>(name, description)` takes a description, because every enum is registered with one (`ecs-design.md` §8).
  3. US-2.2 registers unmanaged (trivially copyable) components only, and `CommandBuffer::add` copies their bytes when the command is recorded. `String`, `List` and `FixedArray` fields, and managed components, come with the first component that needs them.
  4. Looking a type up by its C++ type also checks that the type registered under that name is the same C++ type, so two structs with one `typeName` cannot reach each other's pool.
- **Rationale:** the built-ins are reflected like every other component (srs.fr.2.12, scene files in US-2.4) without changing their types; the rest keeps US-2.2 to what its acceptance criteria need.
- **Consequences:** whoever adds the first managed or `FixedArray` field extends `FieldType`, `FieldValue`, `readField`/`writeField` and the command buffer's value storage.
- **Docs updated:** `design/ecs-api-design.md` §11.
- **Course documents affected (for humans):** none.
- **Supersedes / Related:** D-6, D-14; DEC-043.

### DEC-47.3 — Position and Velocity are engine components registered with `registerCoreComponents()`
- **Date:** 2026-11-23
- **Status:** Accepted (carried by the US-2.2 pull request, binding once a human merges it, DEC-052)
- **Decided by:** the reviewer of the US-2.2 pull request   **Recorded by:** claude-code/opus-5.5/us-2.2
- **Context:** srs.fr.2.11 names Position and Velocity as the components without an enabled flag, so US-2.2 needs them. Physics, gameplay and rendering all use them (`ecs-design.md` §2); no story owns their definition.
- **Decision:** `PositionComponent` (x, y, rotation, scaleX, scaleY) and `VelocityComponent` (x, y), with the fields of `ecs-design.md` §2, live in `engine/ecs/core_components.hpp`, and `registerCoreComponents(TypeRegistry&)` registers them as always enabled. They are not built-in: an entity has them only when they are added. Write tracking for them (D-27) comes with the physics synchronization systems.
- **Rationale:** every module that uses them already depends on `engine::ecs`, so no module depends on another for them (R-2).
- **Consequences:** a host registers them before `finalize()`, like the other engine components.
- **Docs updated:** none beyond `ecs-design.md` §2, which already lists them.
- **Course documents affected (for humans):** none.
- **Supersedes / Related:** D-13, D-27, DEC-040, DEC-041.
### DEC-152.1 — Agent docs are read in tiers: a one-line decision index, rotated handoffs, problems by area, archives for closed records
- **Date:** 2026-11-24
- **Status:** Accepted (decided in the conversation; details carried by PR #152, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (in the conversation "Bridge project docs and docs repo", 2026-11-24); the reviewer of PR #152   **Recorded by:** claude-code/opus-5.5/6r0a
- **Human's words:** "If the whole document is read make the rule so one line explanations are mandetory and long explanation is when tracking-back is required." Then: "I agree on all points for decision-log … for problem-log I think the whole document is important, but maybe it can be re-worked to only be looked at when a problem occurs and not always or problems regarding different parts can be made a seperate files under one problems/ folder … plan.md and task-board.md should be deleted. Others can be made human-only. open-questions your suggestions works."
- **Context:** an agent's boot read was about 65 KB and grew with every PR: the decision-log index alone was 22 KB with rows of up to 330 characters, the handoff log was 67 KB (the protocol said "read the last 5", but tools often load the whole file), and open-questions held answered questions. `plan.md` still described Phase 1. The team will keep working with agents next semester, so the cost compounds.
- **Options considered:** (a) keep the files and rely on "read only part of it"; (b) split each growing record into a small live file agents read and an archive they skip; (c) delete old records.
- **Decision:**
  1. **Decision index.** The index moves out of `decision-log.md` into `history/decision-index.md`, which agents read at boot instead of the log. Each row is one line, the decision in at most 120 characters. Superseded and rejected rows sit in an Archived table that agents skip. Agents open a full entry in `decision-log.md` by its id only to trace a decision back. `check_ids.py` fails on a missing or extra row, a row over 120 characters, or a closed row in Active (or an open one in Archived).
  2. **Handoff rotation.** The live `handoff-log.md` keeps the newest entries. `tools/rotate_handoffs.py` moves older entries unchanged to `history/handoff-archive/<YYYY-MM>.md`, keeping 10; `check_ids.py` fails when the live log holds more than 15, which is when a PR runs the script. No workflow commits the rotation (DEC-054).
  3. **Problems by area.** `history/problem-log.md` becomes `history/problems/` with one append-only file per area (`build-and-tooling`, `engine`, `ai`, `editor`; split further when a file passes about 30 KB) and an index in its README. Agents read it only when a problem occurs, never at boot.
  4. **Open questions.** `open-questions.md` keeps only open questions; answered and obsolete ones move unchanged to `history/questions-archive.md`.
  5. **Deleted:** `agents/state/plan.md` (stale; the GitHub project and `current-status.md` cover it) and `history/task-board-archive.md` (its entries stay in git history).
  6. **Reading tiers.** `agents/README.md` lists three tiers: boot, on demand and human-only. Human-only files (`history/handoff-archive/`, `history/questions-archive.md`, `history/genai-usage-log.md`, ADRs, `research/`, `process/roadmap.md`) carry an audience line under their title. `tools/check_boot.py` keeps the boot files other than the handoff log under 50 KB, in CI.
  7. Moving an entry unchanged to its archive file is not an edit of a past entry (P-6). `check_ids.py` checks each id across its live and archive files.
- **Rationale:** agents read what they need for the session and nothing that only explains the past, so the boot cost stays flat while the records keep growing. Nothing is lost: full decisions, old handoffs and answered questions stay in the repository.
- **Consequences:** the boot set is about 45 KB plus the last handoffs (it was about 65 KB). Reaching the 30 KB first suggested would need `AGENT-PROTOCOL.md` (15 KB) and `rules.md` (7 KB) trimmed, which is a separate decision. An agent that needs a reason must now open the entry, one extra read. Parallel PRs that both rotate make the same change and merge cleanly.
- **Docs updated:** `history/decision-index.md` (new), `history/decision-log.md` (index moved), `history/problems/` (new, from `problem-log.md`), `history/handoff-archive/` (new), `history/questions-archive.md` (new), `agents/state/{handoff-log,open-questions}.md`, `agents/{README,AGENT-PROTOCOL,rules,communication-protocol}.md`, `agents/playbooks/{record-decision,fix-bug}.md`, `agents/templates/{decision-entry,problem-entry}.md`, `agents/state/current-status.md`, `guidelines/{documentation-rules,git-workflow}.md`, `process/{development-process,definition-of-done,roadmap}.md`, `troubleshooting/*`, `research/*.md`, `history/{genai-usage-log,adr-002-cpp-python-communication}.md`, `docs/README.md`, `.gitattributes`, `.github/workflows/ci.yml`, `tools/{check_ids,check_boot,rotate_handoffs}.py`.
- **Course documents affected (for humans):** none.
- **Revisit when:** the boot set passes its budget again, or agents keep needing an archived file at boot.
- **Supersedes / Related:** DEC-149.1 (its archive file is deleted), DEC-148.1, DEC-017, DEC-054, P-6.

### DEC-152.2 — The project repository owns the requirements; the docs repository rebuilds the deliverables from it with a script
- **Date:** 2026-11-24
- **Status:** Accepted (decided in the conversation; details carried by PR #152, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (in the conversation "Bridge project docs and docs repo", 2026-11-24); the reviewer of PR #152   **Recorded by:** claude-code/opus-5.5/6r0a
- **Human's words:** "project/docs definietly needs to be the owner and docs/ only extend from it. But main idea is that we need three layers of seperation … And we need to find a way to make these seperations as static as possible to not need genAI to cut usage." Then: "I think the converter should live in the docs/ if possible. Because since its the one that extends project/docs it should be the one calling instead of project/docs sending."
- **Context:** the SRS (team1-sp/docs) and the code drifted apart, and course rules (S-3, the user's rule of 2026-11-16) kept agents from touching the deliverables, so requirement changes from code work were recorded only as decisions. Instructors receive the deliverables at the demo and the final presentation, so they must match the final work. Next semester has no new deliverables except keeping these current.
- **Options considered:** (a) keep the SRS as the source and record drift as decisions; (b) a ledger of pending deliverable changes written by every PR; (c) the project owns the data, and the docs repository rebuilds its sections from it with a deterministic script only when submitting.
- **Decision:**
  1. **Three layers.** (1) `team1-sp/project` docs are the source: engineering docs, decisions, archives, and the data the deliverables reuse in a fixed format. (2) The converter lives in `team1-sp/docs` and pulls from a project checkout. (3) The deliverables are rebuilt in `team1-sp/docs` only when they are submitted.
  2. **Requirements live in `docs/requirements/`**: `conventions.md`, one file per epic under `functional/` (front matter, epic, stories as `## US-x.y` headings with `**Story:**`, each requirement one line with a `Use cases: …; Initial Plan: …` line), and one file per non-functional section under `non-functional/`. Seeded from team1-sp/docs commit `2ddcc2e` (36 epics, 291 stories, 1203 FRs, 49 NFRs); rendering it back gives SRS 6.1, 6.3, 6.4 and 7.1–7.6 byte for byte.
  3. **A requirement change is made here**, in the PR whose work needs it, with a decision a human approves (DEC-052). This replaces "record SRS changes from code work as decisions only": the decision still records it, and the text now changes here as well. Agents still never edit `team1-sp/docs` (S-3).
  4. Ids stay numbered from 1 without gaps (the SRS convention); a PR that removes or inserts a requirement renumbers the group and updates the references.
  5. `tools/check_requirements.py` (CI) keeps the layout the converter needs. Agents read only the epic file of their story (on demand).
  6. The docs-repository side (marked regions, `tools/sync_from_project.py`, `--check`, `SYNCED_FROM`, Word build, a short agent guide) is the humans' change to `team1-sp/docs`; a draft is handed over with this PR, not committed there.
- **Rationale:** normal work keeps one source true and pays nothing for the deliverables; the deliverables can be brought up to date at any time by a script, so AI is needed only for prose that exists only in the deliverables.
- **Consequences:** about 520 KB of requirements move into this repository (never read at boot). The SRS Word file still needs its format rules applied when it is rebuilt. Renumbering keeps the SRS convention but changes ids that code and tests cite (about 300 references today); stable ids would avoid that and are a separate decision. The SDD gets its own fixed-format sources when its outline exists.
- **Docs updated:** `requirements/` (new), `agents/{rules,AGENT-PROTOCOL,README}.md`, `agents/playbooks/record-decision.md`, `agents/state/current-status.md`, `guidelines/documentation-rules.md` §3 and §8, `docs/README.md`, `.github/workflows/ci.yml`, `tools/check_requirements.py` (new).
- **Course documents affected (for humans):** SRS 6.1, 6.3, 6.4 and 7.1–7.6 get GENERATED markers and are rebuilt from this repository by the docs-repository script.
- **Revisit when:** the SDD outline exists, or renumbering ids causes trouble in code.
- **Supersedes / Related:** DEC-017, DEC-022, DEC-152.1, S-3; the user's rule of 2026-11-16 on SRS changes from code work (refined by part 3).

### DEC-152.3 — Requirement ids stay continuous, and a script renumbers them and their references
- **Date:** 2026-11-24
- **Status:** Accepted (decided in the conversation; details carried by PR #152, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (in the conversation "Bridge project docs and docs repo", 2026-11-24); the reviewer of PR #152   **Recorded by:** claude-code/opus-5.5/6r0a
- **Human's words:** "I think all ids do need to be continous, but I also do not like rebuilding ids each time." Then: "can you make a script to run for renumbering automatically instead of an agent rebuilding it, that would work for me if it will keep the usage down enough."
- **Context:** DEC-152.2 part 4 kept the SRS rule that ids run 1..n without gaps, which renumbers later ids on every insertion or removal; about 300 references in code, tests and docs cite these ids. Stable ids with "Removed" lines and a batch renumber at submission were offered and declined.
- **Options considered:** (a) stable ids with gaps; (b) renumber in a batch at submission; (c) continuous ids at all times, renumbered by a script in the PR that changes a requirement.
- **Decision:** (c). A requirement is added with a placeholder id (`srs.fr.<n>.new`), removed by deleting its lines, or moved between epic files; `tools/renumber_requirements.py` then numbers every `srs.fr` and `srs.nfr` group in file order, rewrites every changed id in one pass across tracked text files except `docs/history/`, the handoff log and `docs/research/`, and appends the old → new table to `docs/requirements/renumbering.md`. It writes nothing while a removed id is still referenced. CI runs it with `--check`, which also fails on any reference to a requirement that does not exist. User stories (`US-x.y`) are not renumbered: they name GitHub issues and branches.
- **Rationale:** the SRS keeps its continuous numbering, and nobody (human or agent) spends effort on renumbering: one command, no AI.
- **Consequences:** a PR that inserts a requirement early in a group touches every file that cites the later ids (a test run showed 42 files for one insertion in group 1); two PRs that change the same group at once conflict in those lines, and the later one runs the script again after merging `main`. History keeps old ids; `renumbering.md` traces them. The first `--check` found three stale references (two `srs.fr.6.10` in design docs, fixed to srs.fr.12.8; the research notes are excluded as history).
- **Docs updated:** `requirements/README.md`, `agents/AGENT-PROTOCOL.md`, `design/{ecs-design,physics-adapter-design}.md`, `.github/workflows/ci.yml`, `tools/renumber_requirements.py` (new), `tools/check_requirements.py`.
- **Course documents affected (for humans):** none; the SRS keeps continuous ids.
- **Revisit when:** renumbering PRs collide often.
- **Supersedes / Related:** refines DEC-152.2 part 4.

### DEC-152.4 — Requirement changes made in the docs repository come back as an ordinary project PR, made by a script
- **Date:** 2026-11-24
- **Status:** Accepted (decided in the conversation; details carried by PR #152, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (in the conversation "Bridge project docs and docs repo", 2026-11-24); the reviewer of PR #152   **Recorded by:** claude-code/opus-5.5/6r0a
- **Human's words:** "we have worked on only the update from project/docs to docs/. But how about update from docs/ to project/docs. For example I was going to get to creating a workflow for updating the docs for V2 submission based on the meeting notes, but that also does touch requirement which lives and owned by project/docs." Then: "I want it to be added now".
- **Context:** DEC-152.2 made the project the owner of the requirements and the docs repository a consumer. Deliverable work (instructor feedback, meeting notes, the SRS V2) starts in the docs repository and can change requirements.
- **Decision:**
  1. Ownership does not move. The docs repository may edit the generated SRS regions with the project's rules (placeholders for new requirements, deleted lines for removed ones, a 6.4 row for each new one; no new or removed stories), and must then propose the edit.
  2. `tools/propose_to_project.py` (docs repository) parses the edited regions back into `docs/requirements/` on a branch `docs/<n>-…` from `main`, runs `tools/renumber_requirements.py` and `tools/check_requirements.py`, writes `DEC-<n>.1` (who decided, the source, every added, removed and reworded id), its index row and `HND-<n>.1`, and commits; `--push` opens the draft PR with `tools/open_pr.py`. No AI is involved.
  3. These PRs follow the normal rules: `no-issue` docs PR named after its number, CI, human review and merge; on a member's computer the push waits for the member (DEC-058). No special rule is added.
  4. Safety: the script refuses when the project's requirements changed after the docs repository's `SYNCED_FROM` commit. `SYNCED_FROM` stores a fingerprint of each generated region; `sync_from_project.py` refuses to overwrite a region edited since the last sync until it is proposed (or `--discard`ed), and a proposal marks its edits as sent so the sync after the merge replaces them with the final ids.
- **Rationale:** one owner and one review path for requirements, whichever repository the change starts in, with no hand renumbering and no AI cost.
- **Consequences:** verified end to end on copies: one rewording, one addition, one removal and one NFR rewording in the docs Markdown became a project branch (25 ids renumbered in 43 files, all checks passing, `DEC-999.1` written), and the sync afterwards rebuilt the docs regions with the final ids and nothing left out of date. The docs repository needs `gh` signed in (or `--number`) to know the coming PR number.
- **Docs updated:** `requirements/README.md`, `agents/playbooks/change-requirements.md`; docs-repository side (patch): `tools/propose_to_project.py`, `tools/srs_regions.py`, `tools/sync_from_project.py`, `AGENTS.md`.
- **Course documents affected (for humans):** none directly; the docs-repository patch adds the scripts.
- **Revisit when:** the SDD gets generated regions (it needs its own parser), or proposals often collide with project changes.
- **Supersedes / Related:** DEC-152.2, DEC-152.3, DEC-058.

### DEC-48.1 — US-2.3 builds system groups, ordering rules and system enable/disable; how queries see enabled flags
- **Date:** 2026-11-24
- **Status:** Accepted (carried by the US-2.3 pull request, binding once a human merges it, DEC-052)
- **Decided by:** Arda Karabulut (owner of US-2.3) for items 1 and 2, 2026-11-23; the reviewer of the US-2.3 pull request for items 3 to 6   **Recorded by:** claude-code/opus-5.5/us-2.3
- **Human's words:** asked whether US-2.3 builds only the Phase 1 scheduler or everything srs.fr.2.13 to 2.18 need, he chose "Hepsini yap" (build all of it); asked whether a disabled parent also hides its children, which the SRS does not say, he chose "Sadece kendisi" (only the entity itself).
- **Context:** srs.fr.2.14 runs systems "in the tick phase of its group and in the order computed by srs.fr.2.50", and 2.15 skips disabled systems; `ecs-api-design.md` §17 had only registration order in Phase 1 (DEC-043). The design (§7 rule 5) made a disable visible to queries "from the next system", while the final srs.fr.2.116 says "from the next query".
- **Decision:**
  1. US-2.3 builds system groups (srs.fr.2.48), runsBefore/runsAfter rules with a stable order that keeps the manual order where the rules allow (2.49, 2.50), refusal of cycles and of rules against the phase order with the last valid order kept (2.51), and enabling and disabling systems and groups (2.15, 2.52), as `ecs-api-design.md` §8 designs them. This widens DEC-043 for these items.
  2. An entity's enabled flag affects only that entity; its descendants keep their own flags.
  3. A disable is visible to queries from the next query, never inside a running one (srs.fr.2.116): the World applies the pending changes when an outermost query starts. Component enabled flags take effect for queries at once.
  4. A system whose rules would break the order is not added: `SystemScheduler::add` throws `RunError` with the cycle or the rule and its two phases, and the last valid order stays. `setManualOrder` returns the reason instead. A rule that names no known system or group is ignored.
  5. Systems declare publications and subscriptions now; checking them, and registering event types, comes with the event bus (US-2.8). The query builder's filters (`with`, `without`, tag, layer; srs.fr.2.56) and write tracking for Position and Velocity (D-27) come with their stories.
  6. `EcsSimulation` implements `ISimulation` (DEC-059): each phase runs its systems, then its command playback; the PhysicsStep phase calls a step function the physics stories set (DEC-041). The editor and the sandbox keep their own simulations until their stories switch to it. `buildSchema` gives the schema of srs.fr.8.29 in memory; the telemetry writer stores it.
- **Rationale:** items 1 and 2 are the owner's choices; items 3 to 6 follow the final SRS and keep the designed API.
- **Consequences:** §17 lists groups, ordering rules and enable/disable under Phase 1. US-2.8 adds `publish`/`subscribe` to `SystemContext` and the subscriptions' `runsIn` to the schema.
- **Docs updated:** `design/ecs-api-design.md` §7, §8, §17; `architecture/engine-layer.md` §1; `agents/state/current-status.md`.
- **Course documents affected (for humans):** none.
- **Supersedes / Related:** amends DEC-043 and D-26; D-10, D-11, D-16, D-28; DEC-46.3, DEC-059.
### DEC-153.1 — Feature work writes requirement changes to pending files; one batch PR applies them and renumbers
- **Date:** 2026-11-24
- **Status:** Accepted (decided in the conversation; details carried by PR #153, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (in the conversation "Plan docs repo formatting rules and agent workflow", 2026-11-24); the reviewer of PR #153   **Recorded by:** claude-code/opus-5.5/6r0a
- **Human's words:** "can we make it so requirement changes is also static, only the what will be changed is written and we can sync when we want. I do not think requirements needs to be up-to-date in a moment." Then: "Continious ids are non-negotiable. [...] it will now only be renumbered when the change is applied? If then yes thats fine [...] Renumbering can be required to run directly after applying changes. And my flag for you is this applying changes should also include changing every reference".
- **Context:** under DEC-152.3 every feature PR that adds or removes a requirement edits a large epic file (ECS 53 KB, Physics 30 KB) and renumbers its group, which rewrites references in up to 56 code and test files and the project docs; parallel PRs that cite the same group conflict.
- **Decision:**
  1. Feature work writes the change in `docs/requirements/changes/<n>.md` (`<n>` = the issue or no-issue PR number): `Add after <id>` / `Add at end of US-x.y`, `Reword <id>`, `Remove <id>` with `References: <replacement>|none`, `Move <id> after <id>`. New requirements get a change id `RC-<n>.<k>`, which code, tests and docs cite until applied. Epic files are not edited and nothing is renumbered in feature PRs.
  2. `tools/apply_requirement_changes.py` applies every pending file in number order in one PR, when the team wants (at the latest before a docs sync for a deliverable): it applies the edits, points references to removed requirements at their replacements, turns every `RC-` id into a placeholder and runs `tools/renumber_requirements.py` right after, which numbers each group 1..n and rewrites every reference; it logs the table in `renumbering.md`, deletes the applied files, and writes nothing when any check fails.
  3. Ids stay continuous in the requirement files at all times (DEC-152.3 holds); they only change in apply PRs and docs-repository proposals. The renumber script also rewrites ids inside pending change files.
  4. CI runs `apply_requirement_changes.py --check`: every pending file applies cleanly and every cited `RC-` id has a pending definition, so a branch open during an apply fails until it uses the final ids.
- **Rationale:** feature PRs carry a few lines instead of a large-file edit and a repository-wide renumber; renumbering conflicts happen once per batch instead of once per change; the SRS still sees continuous ids, and every reference is rewritten by script.
- **Consequences:** between applies the requirement files lag behind accepted changes; the docs repository should apply first before syncing for a deliverable. A test on a copy applied one file with two adds, an NFR add, a reword, a removal with a replacement and a cross-epic move: 12 ids renumbered in 8 files, references (including `RC-` ids and the removed id) rewritten, `check_requirements.py` and `renumber_requirements.py --check` passing.
- **Docs updated:** `requirements/README.md`, `requirements/changes/README.md` (new), `agents/playbooks/change-requirements.md`, `agents/rules.md` (P-13), `agents/AGENT-PROTOCOL.md`, `.github/workflows/ci.yml`, `tools/apply_requirement_changes.py` (new), `tools/renumber_requirements.py`.
- **Course documents affected (for humans):** none; the SRS keeps continuous ids. The docs repository's sync should warn while change files are pending (docs-repository follow-up).
- **Revisit when:** pending files pile up long enough that agents work against stale requirement text.
- **Supersedes / Related:** refines DEC-152.3 (when renumbering runs); DEC-152.2, DEC-152.4.

### DEC-158.1 — A pull request with other PRs stacked on it is merged with a merge commit
- **Date:** 2026-11-24
- **Status:** Accepted (decided in the conversation; carried by PR #158, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (project chat, 2026-11-24, choosing option 1: "option 1 is fine"; then "settings are fine. you can start on the ci")   **Recorded by:** claude-code/opus-5.5/158-stacks
- **Human's words:** "now can we circle back on making sure stack merge conflicts do not happen again. option 1 is fine"
- **Context:** the ECS stack #154 → #155 → #156 hit merge conflicts on files that were already identical, because #154 and #155 were squash-merged: `main` got each lower branch as one new commit while the next branch up still held the original commits. The fix each time was merging `main` into the stacked branch and keeping its side.
- **Decision:**
  1. A PR that has other open PRs stacked on it (their base is its branch) is merged with **Create a merge commit**, titled with the PR title (a Conventional Commit). Every other PR is still squash-merged.
  2. A stacked PR's body says `Stacked on #<n>`; the stack is merged from the bottom up. After the lower branch is deleted, GitHub moves the stacked PR's base to `main`.
  3. The `Stack flag` workflow (`tools/flag_stacks.py`) labels each open PR with an open PR stacked on it `has-stacked-prs` and comments the rule once per stacked PR; the label is removed when nothing is stacked on it. It is a reminder: GitHub cannot choose a PR's merge method.
- **Rationale:** a merge commit keeps the lower branch's commits on `main`, so the stacked branch shares history with `main` and has nothing to conflict on. Keeping squash for every other PR keeps `main` mostly one commit per PR.
- **Consequences:** `main` gets a merge commit plus the branch's own commits for each stacked base PR. The `main` ruleset must allow the merge method "Merge" besides "Squash" (repository settings already allow merge commits). `Main guard` still passes, since those commits belong to the merged PR.
- **Docs updated:** `guidelines/git-workflow.md` (§1, §2, §3, §4 "Stacked pull requests"), `CONTRIBUTING.md`, `agents/AGENT-PROTOCOL.md` (23a), `.github/pull_request_template.md`, `.github/labels.yml`.
- **Revisit when:** stacks become rare enough that the reminder is noise, or GitHub lets a ruleset pick the merge method per PR.
- **Supersedes / Related:** refines DEC-032 (squash-only `main`) and DEC-055 (merge conflicts).

### DEC-160.1 — The run mode with a window is called "windowed mode" in the requirements
- **Date:** 2026-11-24
- **Status:** Accepted (decided in the conversation; carried by PR #160, binding once a human merges it, DEC-052)
- **Decided by:** Işık Dönger (saved point, 2026-11-17; asked for in the project chat, 2026-11-24)   **Recorded by:** claude-code/srs-v2 thread
- **Human's words:** "I want you to add it to requirement change md." (project chat, 2026-11-24, about this rename and the srs.fr.14.15 rewording of DEC-065)
- **Context:** the requirements call the run mode that opens a window and runs render frames "normal mode", against headless mode. The word says nothing about what the mode is. "Editor mode" was rejected, because exported games and the sandbox run in this mode too.
- **Decision:** the requirements say "windowed mode" (and "windowed-mode") instead of "normal mode"; the Editor's mode switch offers Windowed and Headless. The change is queued in `requirements/changes/160.md` (srs.fr.1.1, 1.3, 1.7, 1.9, 1.10, 1.14, 4.1, 4.10, 4.20, 5.5, 5.7, 5.11, 5.13, 7.6, 17.9, 17.19) with the srs.fr.14.15 rewording of DEC-065, and the render frame convention in `requirements/conventions.md` is reworded now.
- **Rationale:** names the mode by what it does, and fits every host that runs it.
- **Consequences:** code, architecture docs and UI frames still say "normal mode" / "Normal" until a follow-up renames them (for example `Application` comments and the B-frames' mode switch).
- **Docs updated:** `requirements/changes/160.md`, `requirements/conventions.md`, `requirements/README.md` (example), `history/decision-log.md`, `history/decision-index.md`.
- **Course documents affected (for humans):** SRS A 6.1 (the listed ids and the Conventions), through the next docs sync.
- **Revisit when:** a third run mode with a window is added.
- **Supersedes / Related:** DEC-065, DEC-153.1.
### DEC-53.1 — Physics bodies are created without shapes; the ECS link lives in `engine/physics-sync`
- **Date:** 2026-11-26
- **Status:** Accepted (decided in the conversation; carried by the US-3.1 PR, binding once a human merges it, DEC-052)
- **Decided by:** Zeynep Ertürk (owner of US-3.1, #53)   **Recorded by:** claude (US-3.1 session)
- **Human's words:** chose "A) Change the menu: 'make a body', then 'add a shape to it' (or remove or change one) as separate steps" over "B) Keep one shape per body for now"; then "New folder engine/physics-sync" over "Inside engine/physics" for the PhysicsSyncIn/Out systems.
- **Context:** DEC-049's `createBody(EntityId, BodyDesc)` made a body with exactly one shape. The requirements give an entity several collider components on one body (srs.fr.3.2, srs.fr.3.8, srs.fr.3.30, srs.fr.3.33) and polygon colliders (srs.fr.3.5).
- **Decision:** `BodyDesc` holds only the body (type, transform, mass, gravity scale, damping, fixed rotation). A new `ShapeDesc` describes one collider (box, circle or convex polygon of up to 8 vertices; offset, friction, restitution, sensor). `createBody` makes a body without shapes; `addShape(BodyId, ShapeDesc) → ShapeId` and `removeShape(ShapeId)` add and remove shapes, and the adapter sets the mass data again after each (srs.fr.3.33, srs.fr.3.46). `ShapeId` is opaque and 64-bit, like `BodyId`. `setBodyType` changes a body's type. The PhysicsSyncIn and PhysicsSyncOut systems live in a new target `engine/physics-sync` (`engine::physics_sync`), which links the ECS and `engine::physics` but not Box2D, so the interface target stays free of the ECS.
- **Rationale:** matches Box2D (shapes are attached to bodies) and the later stories (shape changes and disabled colliders, srs.fr.3.25, srs.fr.3.30) without another interface change.
- **Consequences:** PhysicsSyncIn creates the body, then one shape per enabled collider component, and keeps the collider → `ShapeId` mapping. Shape updates in place (srs.fr.3.25) are not in the interface yet.
- **Docs updated:** `design/physics-adapter-design.md`, `design/ecs-design.md`, `guidelines/repository-structure.md`, `agents/state/current-status.md`.
- **Supersedes / Related:** refines DEC-049 (the interface); DEC-003, DEC-041.
