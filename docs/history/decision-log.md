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
| DEC-011 | AI orchestration in Python, engine-facing contract via gRPC/protobuf | Proposed | 2026-10-12 |
| DEC-012 | Graphics framework selection is decided by the shader benchmark | Superseded by DEC-015 | 2026-10-12 (extracted) |
| DEC-013 | Programming language is C++ | Accepted | 2026-10-13 |
| DEC-014 | Physics library is Box2D (behind the Physics Adapter) | Accepted | 2026-10-13 |
| DEC-015 | Graphics framework is Raylib (behind the rendering abstraction) | Accepted | 2026-10-13 |
| DEC-016 | Product scope is 2D only; 3D is future work, architecture keeps it possible | Accepted | 2026-10-13 |
| DEC-017 | Decision authority: humans decide, agents consult; decision log is the single truth; course docs are not authoritative | Accepted | 2026-10-13 |
| DEC-018 | ECS is custom-built (so components can be LLM-instructive via the semantic layer) | Accepted | 2026-10-13 |
| DEC-019 | C++20; development on Windows and macOS; engine app and games target Windows only (for now) | Proposed (human: "still open") | 2026-10-13 |
| DEC-020 | Box2D version is v3.1.1 (C API) | Accepted | 2026-10-13 |
| DEC-021 | Raylib version is v6.0 | Proposed (human said "still open") | 2026-10-13 |
| DEC-022 | One repository for code and engineering docs; `team1-sp/docs` holds course deliverables only | Accepted | 2026-10-15 |
| DEC-023 | Build system CMake, test framework GoogleTest | Accepted | 2026-10-15 |
| DEC-024 | Işık Dönger reviews every PR; area owners are second code owners | Accepted | 2026-10-15 |
| DEC-025 | AI agents push only after a human approves; humans approve and merge every PR | Accepted | 2026-10-15 |
| DEC-026 | The semantic layer lives in its own top-level folder, separate from engine and ai-services | Accepted | 2026-10-15 |
| DEC-027 | Trunk-based git workflow; branch protection deferred, rules kept as a team agreement | Accepted | 2026-10-15 |
| DEC-028 | GitHub issues are created from ClickUp tasks with ClickUp's GitHub integration | Accepted | 2026-10-15 |

### Decisions still open (no entry yet — do not decide silently)

C++ standard, platforms and Raylib version (DEC-019, DEC-021: Proposed) · compilers/toolchain · whether the engine must build/run on macOS or only be developed there · Raylib version confirmation (DEC-021) · what "LLM-instructive components" means concretely (Q-013) · world units / coordinate convention · project name · reference hardware for the performance targets · telemetry file format (Proposed in design doc, not yet decided). See `agents/state/open-questions.md`.

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

### DEC-011 — AI orchestration in Python; engine-facing contract via gRPC/protobuf
- **Date:** 2026-10-12
- **Status:** Proposed (not yet decided by a human; the engine language is now C++ — DEC-013 — so this is ready to be decided)
- **Context:** The guide says the assistant "may use an LLM via gRPC to a Python AI service" (ip.fr.11.8).
- **Decision (proposed):** Keep model orchestration/experiments in Python; define a strongly-typed protobuf contract between engine-side exporter and AI service. The engine side is C++ (DEC-013); the AI service language is still open.
- **Consequences:** Privacy rule ip.nfr.2.1 (no source/telemetry leaves the machine unless enabled).

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
- **Status:** Proposed (corrected from Accepted, see Amendments)
- **Proposed by:** Işık Dönger (Project Manager)  **Recorded by:** claude-code/sonnet-5.5/012r
- **Human's words:** "C++20, Development will be done on Windows and MacOS but engine app and games are for now only Windows targeted , Raylib v6.0, still open" — clarified later: "still open" covers the whole answer; these are propositions, not decisions.
- **Decision (proposed, binds nobody):** Language standard C++20. Team members develop on Windows and macOS. The shipped engine application and games only target Windows for now.
- **Consequences:** If adopted: code must not require anything newer than C++20, and Windows is the only run target; **open (Q-007, remaining):** does the code have to *build and run* on macOS for development, or is macOS only for editing/docs? That decides whether platform-specific APIs are forbidden and affects the build system (Q-009, deferred). Compiler/toolchain also open.
- **Docs updated:** `guidelines/coding-standards.md`, `troubleshooting/environment-setup.md`, `architecture/tech-stack.md`, `agents/state/*`.
- **Course documents affected (for humans):** any platform statement in the Initial Plan's required software/hardware.
- **Related:** DEC-013, Q-007, Q-009.
- **Amendments:** 2026-10-13 — recording error by claude-code/sonnet-5.5/012r: I first logged this as Accepted. Işık Dönger clarified that "still open" covered the entire Q-007 answer (C++20, platforms, Raylib v6.0). It stays Proposed until a human decides. C++ itself (DEC-013) is unaffected.

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
- **Status:** Proposed — the human wrote "Raylib v6.0, still open"; unclear whether "still open" refers to the version or to the rest of Q-007 (compilers etc.). Not binding until confirmed.
- **Proposed by:** claude-code/sonnet-5.5/012r from Işık Dönger's answer
- **Context:** Q-007. `[ASSUMPTION — a Raylib v6.0 release exists and matches the API you expect; verify before pinning.]`
- **Decision (proposed):** Pin Raylib v6.0.
- **Related:** DEC-015, Q-007.

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

---
