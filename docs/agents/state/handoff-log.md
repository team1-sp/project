# Handoff Log

**Append-only. Newest entry at the bottom.** Never edit earlier entries; add a correcting entry instead. Template: [../templates/handoff.md](../templates/handoff.md). Read the last ~5 entries at session start. This file keeps only the newest entries: older ones are moved, unchanged, to [../../history/handoff-archive/](../../history/handoff-archive/) by `tools/rotate_handoffs.py` (DEC-152.1), which you run when `tools/check_ids.py` says the file holds more than 15 entries.

---

### HND-152.1 — Agent docs read in tiers (decision index, handoff rotation, problems by area)
- **Date:** 2026-11-24
- **Agent:** claude-code/opus-5.5/6r0a
- **Work:** PR #152 (no issue)
- **Goal:** cut what every agent session reads at boot, as agreed with Işık Dönger (DEC-152.1).
- **Result:** Partial — the agent-docs part is done; the bridge to the `team1-sp/docs` repo (requirements in this repo, a deliverable sync ledger, an SDD map) is waiting for the user's review and will follow in this PR.
- **Files changed:** see the `Docs updated` list of DEC-152.1.
- **Verified:** `python3 tools/check_ids.py .`, `python3 tools/check_md_links.py .` and `python3 tools/check_boot.py .` pass; `check_ids.py` fails as expected on a 144-character index row, a superseded row in Active, a missing problem index row, an id in both the live and archive handoff files, and 16 live handoffs; `rotate_handoffs.py` then moves the extra entries.
- **Not verified:** the CI run itself (pull request `--base` mode) until the PR's checks finish.
- **Next:** after the user reviews the bridge proposal, add `docs/requirements/`, `docs/sync/deliverable-changes.md` and `docs/sync/sdd-map.md` as DEC-152.2.
- **Traps:** run `tools/rotate_handoffs.py` only when `check_ids.py` asks for it; it rewrites the live file's top. Decision rows over 120 characters fail CI.
- **Records:** DEC-152.1, HND-152.1.

---

### HND-152.2 — Requirements owned by the project; docs repository syncs from it
- **Date:** 2026-11-24
- **Agent:** claude-code/opus-5.5/6r0a
- **Work:** PR #152 (no issue)
- **Goal:** the bridge agreed with Işık Dönger (DEC-152.2): project owns the requirements, the docs repository rebuilds its sections with a script.
- **Result:** Done, awaiting review. `docs/requirements/` seeded from team1-sp/docs `2ddcc2e`; `tools/check_requirements.py` in CI. The docs-repository side (markers, `tools/sync_from_project.py`, `AGENTS.md`) is a patch for the humans, not committed there.
- **Verified:** rendering `docs/requirements/` with the draft sync script into an SRS copy with empty markers reproduced 6.1, 6.3, 6.4 and 7.1–7.6 byte for byte; `--check` reports nothing out of date against this branch and reports 6.1 after one requirement edit; `check_requirements.py` passes and fails on a wrong id and a missing `Use cases` line; `check_ids.py`, `check_md_links.py`, `check_boot.py` (48.0 KB) pass.
- **Not verified:** the Word build from the synced Markdown (not part of this PR).
- **Next:** the user applies the docs-repository patch and adds their docs agent workflow; SDD sources once its outline exists; decide whether requirement ids stay renumbered without gaps or become stable.
- **Traps:** the requirement files are large; read only the epic of your story. Do not edit team1-sp/docs.
- **Records:** DEC-152.2, HND-152.2.

---

### HND-152.3 — Continuous requirement ids kept by a script
- **Date:** 2026-11-24
- **Agent:** claude-code/opus-5.5/6r0a
- **Work:** PR #152 (no issue)
- **Result:** Done, awaiting review. `tools/renumber_requirements.py` (DEC-152.3) and its `--check` in CI.
- **Verified:** on a copy, one placeholder inserted in group 1 → 19 ids renumbered in 42 files, code comments followed (srs.fr.1.4 → 1.5), `check_requirements.py` and `--check` pass afterwards; deleting a referenced requirement makes the script stop and list the references. On this branch `--check` passes after fixing two stale `srs.fr.6.10` references.
- **Next:** none for this PR beyond review.
- **Records:** DEC-152.3, HND-152.3.

---

### HND-152.4 — Agent playbook for requirement changes
- **Date:** 2026-11-24
- **Agent:** claude-code/opus-5.5/6r0a
- **Work:** PR #152 (no issue)
- **Result:** Done, awaiting review. `playbooks/change-requirements.md` gives agents the add / edit / remove / move steps with placeholder ids and the renumber script; rule P-13, AGENT-PROTOCOL step 10, record-decision step 7 and the requirements README point to it, so requirement changes need no hand edits.
- **Verified:** `check_md_links.py`, `check_ids.py`, `check_boot.py` (48.8 KB), `check_requirements.py`, `renumber_requirements.py --check` pass.
- **Records:** HND-152.4 (part of DEC-152.3).

---

### HND-152.5 — Requirement changes from the docs repository
- **Date:** 2026-11-24
- **Agent:** claude-code/opus-5.5/6r0a
- **Work:** PR #152 (no issue)
- **Result:** Done, awaiting review (DEC-152.4). Project side: requirements README and change-requirements playbook. Docs side: `propose_to_project.py`, `srs_regions.py`, region fingerprints in `sync_from_project.py`, `AGENTS.md`, in `/mnt/project-files/docs-bridge/docs-repo-bridge.patch` (two commits, applies with `git am` on docs `2ddcc2e`).
- **Verified:** on copies: docs edits (reword, add with placeholder + 6.4 row, remove, NFR reword) → project branch with DEC-999.1, renumber/format/id checks pass → sync rebuilds the regions with final ids, `--check` clean. Sync refuses while an edited region is unproposed.
- **Not verified:** `--push` against GitHub (needs a real PR number).
- **Next:** after #152 merges, the user applies the patch, runs `sync_from_project.py` once to create SYNCED_FROM, and pushes the docs repo.
- **Records:** DEC-152.4, HND-152.5.

---

---

### HND-48.1 — Systems, ordering, queries and the ECS behind the game loop (US-2.3)
- **Date:** 2026-11-24
- **Agent:** claude-code/opus-5.5/us-2.3
- **Work:** US-2.3 #48 on `feat/us-2.3-register-systems-query-entities`, stacked on the US-2.2 branch; pull request not opened yet
- **Goal:** srs.fr.2.13 to 2.18 with the scope of DEC-48.1: register systems, run them once per tick in their phase and order, groups and before/after rules, enable/disable, declarations checked on every access, timings, queries that skip disabled entities and components, and the ECS behind `ISimulation`.
- **Result:** Done. New: `ISystem`, `SystemDescription`, `SystemAccess`, `SystemContext`, `SystemScheduler` (groups, stable order, cycle refusal, manual order, enable/disable, timings, command playback per phase), `World::each`, `single`, `createSingleton`, the query-visible enabled flag, `GameplayRandom` (PCG32), `Schema` with derived readBy/writtenBy, `EcsSimulation`. `CommandBuffer` checks the recording system's declaration and names it in warnings.
- **Files changed:** `engine/ecs/**`, `tests/unit/ecs/**`, `tests/CMakeLists.txt`, `docs/design/ecs-api-design.md`, `docs/architecture/engine-layer.md`, `docs/agents/state/current-status.md`, `docs/history/decision-log.md`, `docs/history/decision-index.md`.
- **Verified:** `windows-debug` (MSVC 19.44) and `headless` presets with `-DENGINE_WARNINGS_AS_ERRORS=ON` build clean; CTest 407/407 and 394/394; a headless `Application` run with `EcsSimulation` calls a system once per tick and ends with the outcome Error on an undeclared access; PCG32 matches the reference outputs for seed 42; clang-format 19.1.7 dry run clean; `tools/check_ids.py`, `check_md_links.py`, `check_includes.py`, `check_boot.py`, `check_requirements.py` and `renumber_requirements.py --check` pass.
- **Not verified:** GCC and Apple Clang (CI runs them).
- **Repo state:** builds and tests pass; committed only after the member approves (DEC-058).
- **Next:**
  1. US-2.4 on a branch stacked on US-2.2: `.scene` files as JSON through nlohmann/json.
  2. US-2.8: the event bus, `publish`/`subscribe` in `SystemContext`, handler `runsIn` in the schema, event type registration.
  3. Hosts (editor, sandbox) switch to `EcsSimulation` in their own stories; the physics stories set its PhysicsStep function.
- **Traps:** the agent boot files are at the 50 KB budget (`tools/check_boot.py`); this branch shortened the US-2.1/2.2 index rows to fit DEC-48.1. A system that breaks the order is not added: `add` throws.
- **Records:** DEC-48.1, HND-48.1.
### HND-153.1 — Requirement changes batched through pending change files
- **Date:** 2026-11-24
- **Agent:** claude-code/opus-5.5/6r0a
- **Work:** PR #153 (no issue)
- **Goal:** take requirement editing and renumbering out of feature PRs while keeping continuous ids, as Işık Dönger asked (DEC-153.1).
- **Result:** Done, awaiting review. `tools/apply_requirement_changes.py` (new) applies `docs/requirements/changes/<n>.md` files, rewrites `RC-` ids and removed ids, and runs the renumber script; CI runs its `--check`. Playbook, README, P-13 and the protocol updated.
- **Verified:** on a copy, one change file (2 adds, NFR add, reword, remove with replacement, cross-epic move) applied: 12 ids renumbered in 8 files, every reference rewritten, the file deleted; `check_requirements.py`, `renumber_requirements.py --check`, `apply_requirement_changes.py --check`, `check_md_links.py`, `check_boot.py` pass on the branch.
- **Not verified:** CI on the PR until its checks finish; the docs-repository warning for pending files (follow-up there).
- **Next:** the docs repository's `sync_from_project.py --check` should warn while project change files are pending.
- **Traps:** an apply PR renumbers; branches open during it fail CI until they use the final ids from `renumbering.md`.
- **Records:** DEC-153.1, HND-153.1.

---

### HND-158.1 — Merge commits for PRs with stacked PRs, and a reminder workflow
- **Date:** 2026-11-24
- **Agent:** claude-code/opus-5.5/158-stacks
- **Work:** PR #158 (no issue)
- **Goal:** stop stacked PRs from conflicting with `main` after the PR below them is squash-merged, as Işık Dönger chose (DEC-158.1).
- **Result:** Done, awaiting review. New `tools/flag_stacks.py` and `.github/workflows/stack-flag.yml` (label `has-stacked-prs` plus one comment per stacked PR); the rule is in `git-workflow.md` §4, CONTRIBUTING, AGENT-PROTOCOL 23a and the PR template.
- **Files changed:** `tools/flag_stacks.py`, `.github/workflows/stack-flag.yml`, `.github/labels.yml`, `.github/pull_request_template.md`, `CONTRIBUTING.md`, `docs/guidelines/git-workflow.md`, `docs/agents/AGENT-PROTOCOL.md`, `docs/history/{decision-log,decision-index}.md`.
- **Verified:** `flag_stacks.py` dry run against the repository (no stacks open now); its stack detection on a fake list (two PRs on one base, a chain, a fork head ignored); `check_ids.py`, `check_boot.py`, `check_md_links.py`.
- **Not verified:** the workflow on GitHub until a stacked PR is opened (it can also be run by hand from the Actions tab).
- **Next:** the `Protect Main` ruleset must allow the "Merge" method (it allows only squash now); the repository's default merge commit title should be the PR title. Both are admin settings.
- **Traps:** the agent boot files sit at the 50 KB budget; 23a and two index rows were shortened to fit. DEC-46.1 to DEC-48.1 rows had landed below the "Still open" paragraph of the index (outside the table) after the ECS stack merges; moved back into Active.
- **Records:** DEC-158.1, HND-158.1.

---

### HND-53.1 — US-3.1: Box2D Physics Adapter, physics components and PhysicsSyncIn
- **Date:** 2026-11-26
- **Agent:** claude (US-3.1 session), working with Zeynep Ertürk
- **Work:** #53 (US-3.1), one PR in three parts: (1) adapter, (2) components, (3) PhysicsSyncIn.
- **Result:** Ready for review. (1) Interface split into bodies and shapes plus `setBodyType` (DEC-53.1); `Box2DPhysicsAdapter` implements every operation: mass per srs.fr.3.46, friction, restitution, sensors, gravity scale, damping, fixed rotation, offset boxes and circles, convex polygons (3 to 8 vertices; concave refused), basic begin/end contacts and closest-hit raycast; errors go to an optional `ILogSink`. (2) `RigidBody`, `BoxCollider`, `CircleCollider`, `PolygonCollider` in `engine/ecs/physics_components.hpp`, registered by `registerPhysicsComponents`. (3) `engine/physics-sync`: `PhysicsSync` (observer and body map) and `PhysicsSyncInSystem` create, reshape, retype and destroy bodies from the components (srs.fr.3.2, 3.8).
- **Verified (Linux, GCC, `headless` preset, `ENGINE_WARNINGS_AS_ERRORS=ON`):** builds with no warnings; `ctest` 422/422 (15 adapter, 3 component, 8 PhysicsSyncIn tests new); clang-format 19.1.7 clean; `check_includes.py`, `check_ids.py`, `check_boot.py`, `check_md_links.py` and the requirement checks pass.
- **Not verified:** Windows and macOS builds and the windowed (raylib) preset (CI).
- **Next:** US-3.2 (#54): PhysicsStep in the game loop, PhysicsSyncOut write-back, gravity from the Physics settings. US-3.3 refines events and raycast; US-3.6 handles disabled entities and components (each() skips them, so PhysicsSyncIn creates no body for them yet).
- **Traps:** `b2Rot_GetAngle` is approximate (research/box2d.md). The adapter always uses density 1 and overrides the mass. Changes to RigidBody or collider fields after the body exists are not applied yet (srs.fr.3.25, 3.57). The boot files were at the 50 KB budget: two current-status bullets were reworded shorter to fit the new index row.
- **Records:** DEC-53.1, HND-53.1.

---

### HND-54.1 — US-3.2: the ECS and the physics world in step
- **Date:** 2026-11-26
- **Agent:** claude (US-3.2 session), working with Zeynep Ertürk
- **Work:** #54 (US-3.2), stacked on the US-3.1 PR (#161).
- **Result:** Ready for review once #161 is merged. ECS write tracking as designed in D-27: `tracksWrites()` on Position and Velocity, a `pending_` array in `ComponentPool`, marks from a system's non-const `get`/`tryGet`/`each`, `writesUntracked<…>()` and `ctx.writeUntracked`, `hasPendingWrite`/`clearPendingWrite`. `engine/ecs/transform.hpp`: `compose` (srs.fr.3.49) and `localFromWorld`. PhysicsSyncIn creates bodies at world transforms with their Velocity, applies pending Position and Velocity writes and clears the marks, and moves static and kinematic children with their parent. New PhysicsSyncOutSystem writes position (local to the parent) and velocity back untracked, skipping marked components. `connectPhysics()` wires the observer, both systems and the 1/60 s PhysicsStep.
- **Decided by Zeynep:** parents are handled in this story (local conversion of srs.fr.3.12 and children following their parent), not left to US-3.7.
- **Verified (Linux, GCC, `headless` preset, `ENGINE_WARNINGS_AS_ERRORS=ON`):** builds with no warnings; `ctest` 441/441 (6 write-tracking, 4 transform, 9 sync tests new); clang-format 19.1.7 clean; doc and id checks pass.
- **Not verified:** Windows, macOS and windowed builds (CI). Gravity from the project's Physics settings (srs.fr.3.9): the settings do not exist yet (EP-27); gravity comes from `Box2DPhysicsConfig`.
- **Next:** US-3.4 (#56) and US-3.3 (#55), both unblocked once this merges.
- **Traps:** the CI blocker check fails for this PR until #53 closes. Merge #161 with a merge commit (it has a PR stacked on it, DEC-158.1). Spawn code that changes Position or Velocity after a body exists must call `World::markWritten` for the change to reach the body.
- **Records:** HND-54.1.
