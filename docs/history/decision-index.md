# Decision Index

**Agents read this file at boot instead of the decision log** (DEC-152.1). Each row is one line: the decision in at most 120 characters. Open the full entry in [decision-log.md](decision-log.md) by its id only when you need its reasons, scope or wording (tracing back).

Rules (checked by `tools/check_ids.py`):
- Every `### DEC-…` entry in the decision log has exactly one row here, and every row an entry.
- The Decision column is one line of at most 120 characters. The full text belongs in the entry.
- A new decision adds its row at the bottom of **Active**. When a decision is superseded or rejected, its row moves to **Archived** in the same PR, with the new status.

Statuses: `Proposed` (waiting for an answer; binds nobody) · `Accepted` (binding) · `Rejected` · `Superseded by DEC-…`.

## Active

| ID | Decision | Status | Date |
| --- | --- | --- | --- |
| DEC-001 | Three-layer architecture with one-way dependencies | Accepted | 2026-10-12 (extracted) |
| DEC-002 | ECS is the central data hub; gameplay decisions live in gameplay systems | Accepted | 2026-10-12 (extracted) |
| DEC-003 | Physics is accessed only through a Physics Adapter | Accepted | 2026-10-12 (extracted) |
| DEC-004 | Fixed physics timestep 1/60 s and fixed frame order | Accepted | 2026-10-12 (extracted) |
| DEC-005 | Headless mode is a core requirement | Accepted | 2026-10-12 (extracted) |
| DEC-007 | Graphics framework is accessed through an abstraction | Accepted | 2026-10-12 (extracted) |
| DEC-008 | Telemetry is recorded every frame inside the loop | Accepted | 2026-10-12 (extracted) |
| DEC-009 | Adopt this docs structure and agent protocol | Accepted | 2026-10-12 |
| DEC-010 | MCP is an optional later interoperability layer, not the first integration | Proposed | 2026-10-12 |
| DEC-013 | Programming language is C++ | Accepted | 2026-10-13 |
| DEC-014 | Physics library is Box2D, behind the Physics Adapter | Accepted | 2026-10-13 |
| DEC-015 | Graphics framework is raylib, behind the rendering abstraction | Accepted | 2026-10-13 |
| DEC-016 | Product scope is 2D only; 3D is future work and the architecture keeps it possible | Accepted | 2026-10-13 |
| DEC-017 | Humans decide, agents consult; the decision log is the single truth; course docs are not authoritative | Accepted | 2026-10-13 |
| DEC-018 | ECS is custom-built so components can be LLM-instructive through the semantic layer | Accepted | 2026-10-13 |
| DEC-019 | C++20 (its platform part was decided in DEC-029) | Accepted | 2026-10-13 |
| DEC-020 | Box2D version is v3.1.1 (C API) | Accepted | 2026-10-13 |
| DEC-022 | One repository for code and engineering docs; `team1-sp/docs` holds course deliverables only | Accepted | 2026-10-15 |
| DEC-023 | Build system CMake, test framework GoogleTest | Accepted | 2026-10-15 |
| DEC-024 | Işık Dönger reviews every PR; area owners are second code owners | Accepted | 2026-10-15 |
| DEC-026 | The semantic layer has its own top-level folder, separate from engine and ai-services | Accepted | 2026-10-15 |
| DEC-027 | Trunk-based git workflow; branch protection deferred, rules kept as a team agreement | Accepted | 2026-10-15 |
| DEC-029 | Develop on Windows and macOS (Windows first); engine and tool run on both; games for Windows only | Accepted | 2026-10-15 |
| DEC-030 | Compilers: whatever each platform needs; none pinned now | Accepted | 2026-10-15 |
| DEC-031 | Coding standards, Definition of Done and clang-format 19 accepted | Accepted | 2026-10-23 |
| DEC-032 | Only Işık Dönger and the advisor approve PRs; Işık's own PRs need no approval; `main` ruleset on | Accepted (advisor part superseded by DEC-151.1) | 2026-10-23 |
| DEC-033 | Raylib version is 6.2 | Accepted | 2026-10-24 |
| DEC-034 | The product name is Aegis | Accepted | 2026-10-24 |
| DEC-036 | World units are meters with y up; 32 pixels per meter; conversion only in the renderer | Accepted | 2026-11-01 |
| DEC-037 | The PB-008 spike in `sandbox/` may link Box2D and raylib directly (R-3/R-4 exception, spike only) | Proposed | 2026-11-01 |
| DEC-038 | ADR-002: the C++ to Python bridge is gRPC/protobuf behind AgentBridge | Accepted | 2026-11-04 |
| DEC-039 | `ai-services` uses Python 3.12 with pinned Python project tooling | Accepted | 2026-11-04 |
| DEC-040 | The physics body owns velocity; the ECS Velocity component mirrors it and carries pending writes | Accepted | 2026-11-05 |
| DEC-041 | The ECS and the Physics Adapter meet only in the PhysicsSyncIn and PhysicsSyncOut systems | Accepted | 2026-11-05 |
| DEC-042 | Pre-implementation ECS gaps settled: events, ticks, Phase 1 components, input, time, RNG, ids, errors | Accepted | 2026-11-05 |
| DEC-043 | About 20 % of the whole project is built this semester; the ECS builds its Phase 1 slice | Accepted | 2026-11-05 |
| DEC-044 | Coding standards amended for the ECS design: naming, tags, declarations, errors, RNG | Accepted | 2026-11-05 |
| DEC-045 | No own LLM; users bring their own model provider and key; only free-tier or local use is guaranteed | Accepted | 2026-11-06 |
| DEC-046 | PB-011: window in `rendering-raylib` behind `IWindow`; loop uses `ISimulation` and `IFrameRenderer` | Accepted | 2026-11-10 |
| DEC-047 | PB-012: loop follows srs.fr.1.2 to 1.6, at most 5 ticks per frame, exact integer accumulator | Accepted | 2026-11-10 |
| DEC-048 | The editor hosts `engine::Application` and owns the window; `sandbox` is a dev/CI host; games never host it | Accepted | 2026-11-11 |
| DEC-049 | PB-025: `IPhysicsAdapter`, bodies addressed by `BodyId` via `findBody(EntityId)`; `Vec2`/`EntityId` in core | Accepted | 2026-11-11 |
| DEC-050 | Backlog moves from ClickUp to one GitHub project; branches use the issue number and move the card | Accepted | 2026-11-11 |
| DEC-051 | US-1.1: a frame collects input first via `IWindow::collectInput()`; raylib uses custom frame control | Accepted | 2026-11-12 |
| DEC-052 | A PR's decisions are written `Accepted`, listed for review and bind on merge; `Proposed` only while asked | Accepted | 2026-11-12 |
| DEC-053 | Agents commit after a human approves the change, then push and update the draft PR; humans merge | Accepted | 2026-11-12 |
| DEC-054 | Draft PRs are opened with `tools/open_pr.py`; no workflow creates commits or PRs | Accepted | 2026-11-12 |
| DEC-055 | Branches merge `main` (never rebase) before push and review; collided ids are renumbered by the later branch | Accepted | 2026-11-12 |
| DEC-057 | US-1.2: `parseRunOptions()` gives run mode and settings; a `RunError` ends the run with outcome `Error` | Accepted | 2026-11-15 |
| DEC-058 | Agents ask before committing only on a member's computer; in a cloud copy they commit and push freely | Accepted | 2026-11-15 |
| DEC-059 | US-1.3: each tick runs the seven `TickPhase`s via `ISimulation::runPhase()`; input comes from `IInputSource` | Accepted | 2026-11-16 |
| DEC-060 | US-1.4: `Application` pause, resume and step act at the next tick boundary; one tick per step | Accepted | 2026-11-16 |
| DEC-061 | Agents work in a cloud copy by default; on a member's computer only when the work needs it or they ask | Accepted | 2026-11-16 |
| DEC-062 | `Epic close` closes an epic when all sub-issues are closed; with later stories left it gets `increment-done` | Accepted | 2026-11-16 |
| DEC-063 | US-14.1: Editor UI is Dear ImGui (docking) via rlImGui; the window-free menu model lives in `editor_core` | Accepted | 2026-11-16 |
| DEC-064 | US-14.1: Editor look follows the Figma design: embedded fonts, design palette, Font Awesome toolbar icons | Accepted | 2026-11-16 |
| DEC-065 | srs.fr.14.15 reworded: a dragged-out panel floats inside the main window, not as a separate OS window | Accepted | 2026-11-17 |
| DEC-066 | US-14.2: the status bar is built from a window-free `EditorStatus` model in `editor_core` | Accepted | 2026-11-17 |
| DEC-067 | US-11.2: read-only assistant core with AgentBridge streaming, cancellable answers, user-local chats | Accepted | 2026-11-22 |
| DEC-068 | US-24.1: structured logs owned by a window-free Console model; non-mutating projection, injected seams | Accepted | 2026-11-22 |
| DEC-069 | US-33.5: AI/privacy settings via a window-free controller and AgentBridge health and model operations | Accepted | 2026-11-22 |
| DEC-070 | US-11.1: grounded answers with evidence seams, typed citations and a citation-checking AgentBridge decorator | Accepted | 2026-11-22 |
| DEC-071 | US-33.2: AI chat state is a window-free model with an injectable sender; answers run on a worker thread | Accepted | 2026-11-22 |
| DEC-148.1 | Record ids are `<PREFIX>-<GitHub #>.<counter>`; append-only logs merge with `union`; `check_ids.py` enforces it | Accepted | 2026-11-22 |
| DEC-149.1 | Task board retired: tasks are GitHub issues, assignee is the claim; `current-status.md` is increment-level | Accepted | 2026-11-22 |
| DEC-151.1 | Işık Dönger is the only code owner and approver; the advisor is no longer a code owner | Accepted | 2026-11-22 |
| DEC-152.1 | Agents read a one-line decision index; handoffs rotate; problems split by area; closed records go to archives | Accepted | 2026-11-24 |
| DEC-152.2 | The project owns the requirements (`docs/requirements/`); the docs repo rebuilds deliverables from it by script | Accepted | 2026-11-24 |
| DEC-152.3 | Requirement ids stay continuous; `tools/renumber_requirements.py` renumbers them and every reference | Accepted | 2026-11-24 |
| DEC-152.4 | Requirement edits made in the docs repo come back as a normal project PR made by `propose_to_project.py` | Accepted | 2026-11-24 |

### Still open, no entry yet (do not decide silently)

what "LLM-instructive components" means concretely (Q-013, deferred by the human) · license · GitHub access, visibility and branch protection (see Q-014) · reference hardware for the performance targets · telemetry file format (Proposed in design doc, not yet decided). See `agents/state/open-questions.md`.
| DEC-46.1 | ADR-001: components are stored in sparse sets, one independent pool per type | Accepted | 2026-11-05 |
| DEC-46.2 | The PB-010 design decisions D-2 to D-29 (`ecs-api-design.md` §2) are the ECS design | Accepted | 2026-11-05 |
| DEC-46.3 | US-2.1 and US-2.2 also build the hierarchy and the entity and component enabled flags | Accepted | 2026-11-23 |
| DEC-46.4 | US-2.1: ID reuse rules, run start, command buffer errors, created-then-destroyed parents | Accepted | 2026-11-23 |
| DEC-47.1 | The built-in components have no enabled flag of their own | Accepted | 2026-11-23 |
| DEC-47.2 | US-2.2 field kinds add UInt8, UInt32 and Tag; managed and FixedArray fields come later | Accepted | 2026-11-23 |
| DEC-47.3 | Position and Velocity are engine components registered by `registerCoreComponents()` | Accepted | 2026-11-23 |

## Archived

Superseded and rejected decisions. Agents skip this table unless they trace a decision back.

| ID | Decision | Status | Date |
| --- | --- | --- | --- |
| DEC-006 | MVP is 2D; architecture must not block 3D | Superseded by DEC-016 | 2026-10-12 (extracted) |
| DEC-011 | AI orchestration in Python, engine-facing contract via gRPC/protobuf | Superseded by DEC-038 (protocol) and DEC-039 (Python) | 2026-10-12 |
| DEC-012 | Graphics framework selection is decided by the shader benchmark | Superseded by DEC-015 | 2026-10-12 (extracted) |
| DEC-021 | Raylib version is v6.0 | Superseded by DEC-033 | 2026-10-13 |
| DEC-025 | AI agents push only after a human approves; humans approve and merge every PR | Superseded by DEC-053 | 2026-10-15 |
| DEC-028 | GitHub issues are created from ClickUp tasks with ClickUp's GitHub integration | Superseded by DEC-050 | 2026-10-15 |
| DEC-035 | PR titles put the PB id in the Conventional Commit scope | Superseded by DEC-050 | 2026-10-24 |
