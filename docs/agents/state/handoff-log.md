# Handoff Log

**Append-only. Newest entry at the bottom.** Never edit earlier entries; add a correcting entry instead. Template: [../templates/handoff.md](../templates/handoff.md). Read the last ~5 entries at session start. This file keeps only the newest entries: older ones are moved, unchanged, to [../../history/handoff-archive/](../../history/handoff-archive/) by `tools/rotate_handoffs.py` (DEC-152.1), which you run when `tools/check_ids.py` says the file holds more than 15 entries.

---

### HND-042 — Merge `main` (US-33.1 #145, #147) into the US-33.2 branch
- **Date:** 2026-11-22
- **Agent:** claude-code/opus-5.5/119-merge-main
- **Task:** TASK-025, GitHub #119, PR #146
- **Result:** `main` holds #145 squash-merged, so its commits look new to git. Merged `main` with
  only the changes `main` has beyond `a6af34c` (the US-33.1 version already in this branch): #145's
  fix `462ca5a` (AI Settings opens the shell without a project) and #147's git hooks. Code applied
  without conflicts. Renumbered this branch's handoff entries to HND-040/HND-041 because `main`
  uses HND-037 (#145) and HND-039 (#147).
- **Verified:** see the merge commit's CI run.
- **Next:** CI and human review of PR #146.

---

### HND-148.1 — Collision-free record ids and union merges for the append-only logs
- **Date:** 2026-11-22
- **Agent:** claude-code/opus-5.5/148-ids
- **Task:** PR #148 (no issue)
- **Goal:** stop parallel PRs from taking the same record id, and stop append-only logs from conflicting on local merges (DEC-148.1).
- **Result:** Done, awaiting review of PR #148.
- **Files changed:** `tools/check_ids.py`, `.github/workflows/ci.yml`, `.gitattributes`, `.githooks/pre-commit`, `.github/pull_request_template.md`, `CONTRIBUTING.md`, `docs/guidelines/{documentation-rules,git-workflow}.md`, `docs/agents/{AGENT-PROTOCOL,communication-protocol}.md`, `docs/agents/templates/*`, `docs/history/{decision-log,genai-usage-log}.md`.
- **Verified:** `check_ids.py` on the repo; with fake pull_request events (matching and wrong numbers, old-form, malformed and duplicate ids); a local merge of two branches that both append to `handoff-log.md` completes without a conflict.
- **Not verified:** the CI step on GitHub until this PR's run.
- **Next:** review of PR #148. TASK ids stay as `TASK-<issue #>` (Işık Dönger, 2026-11-22). `current-status.md`/`task-board.md` conflicts are a separate later decision.
- **Traps:** this is the first entry in the new form, so the index now mixes old and new ids.
- **Records:** DEC-148.1, HND-148.1.

---

### HND-149.1 — Retire the task board and make current-status.md increment-level
- **Date:** 2026-11-22
- **Agent:** claude-code/opus-5.5/148-ids
- **Work:** PR #149 (no issue), stacked on PR #148
- **Goal:** stop `task-board.md` and `current-status.md` from conflicting between parallel PRs (DEC-149.1).
- **Result:** Done, awaiting review of PR #149.
- **Files changed:** see the `Docs updated` list of DEC-149.1.
- **Verified:** `tools/check_ids.py .` and `tools/check_md_links.py .` pass; grep for `task-board`/`TASK-` leaves only history and the archive.
- **Not verified:** the new current-status.md "What exists" table was written from the folder layout and the decision log, not from a build.
- **Next:** merge #148 first, then #149. `plan.md` is stale and needs its own update.
- **Traps:** claims are now the issue's assignee; don't recreate a task file.
- **Records:** DEC-149.1, HND-149.1, Q-015 answered.

---

### HND-151.1 — Remove the advisor as a code owner
- **Date:** 2026-11-22
- **Agent:** claude-code/opus-5.5/148-ids
- **Work:** PR #151 (no issue)
- **Result:** Done, awaiting review. `.github/CODEOWNERS` lists only `@isikdonger` (DEC-151.1); CONTRIBUTING and git-workflow updated.
- **Verified:** `tools/check_ids.py` and `tools/check_md_links.py` pass.
- **Not verified:** that GitHub stops requesting the advisor's review; check on the next PR after merge.
- **Next:** if the advisor should also lose repository access, that is a repository setting (Settings → Collaborators), not part of this PR.
- **Records:** DEC-151.1, HND-151.1.

---

### HND-46.1 — Entities, IDs, the hierarchy and the command buffer (US-2.1)
- **Date:** 2026-11-23
- **Agent:** claude-code/opus-5.5/us-2.1
- **Work:** US-2.1 #46 on `feat/us-2.1-create-destroy-entities`; pull request not opened yet
- **Goal:** srs.fr.2.1 to 2.5: create and destroy entities at runtime with IDs that are never given to another entity, destruction requests that take effect at the end of the phase, destroying with all descendants, and a name, enabled flag, tag, layer and optional parent for every entity.
- **Result:** Done. New target `engine::ecs` (`engine/ecs/`, links only `engine::core`): `Entity`, `EntityTable`, the sorted sparse-set `ComponentPool<T>`, `FixedString`, `AssetId`, `TagId`, the built-in Name, EntityInfo and Hierarchy components, `World` (create, destroy, find, idOf, enabled flag, setParent, beginRun, observers, log sink) and `CommandBuffer` (createEntity, destroy, setParent, playback).
- **Files changed:** `engine/ecs/**`, `engine/CMakeLists.txt`, `engine/core/include/engine/core/entity_id.hpp` (SRS number), `tests/unit/ecs/**`, `tests/CMakeLists.txt`, `docs/design/ecs-api-design.md`, `docs/design/ecs-design.md`, `docs/agents/state/current-status.md`, `docs/guidelines/repository-structure.md`, `docs/history/decision-log.md`.
- **Verified:** `windows-debug` (MSVC 19.44) and `headless` presets with `-DENGINE_WARNINGS_AS_ERRORS=ON` build clean; CTest 332/332 and 319/319; clang-format 19.1.7 dry run clean; `tools/check_ids.py`, `tools/check_md_links.py` and `tools/check_includes.py` pass.
- **Not verified:** GCC and Apple Clang (CI runs them); a slot's generation wrapping around (it takes 2^32 destructions of one slot; the code retires such a slot).
- **Repo state:** builds and tests pass; committed only after the member approves (DEC-058).
- **Next:**
  1. US-2.2 on `feat/us-2.2-add-remove-components`, stacked on this branch: the type registry and reflection, general `add`/`remove`/`get`/`has`/`tryGet`, component enabled flags (DEC-46.3) and `CommandBuffer` add/remove; `World` then takes `const TypeRegistry&` (design §5).
  2. US-2.3 backs `ISimulation::runPhase()` with the ECS and adds queries that skip disabled entities (srs.fr.2.15); decide there whether a disabled parent also hides its children, which the SRS does not say.
  3. US-2.8 moves `ecs-event-bus-design.md` to the final SRS numbers and records E-1 to E-5.
- **Traps:** the ECS design docs were written against the SRS draft of 2026-11-05; the final SRS (2026-11-14) is renumbered from about srs.fr.2.61 on (runtime IDs 2.63, destroy timing 2.116, ID range 2.104). A `CommandBuffer` must be played back: the entities it created stay alive without components until then. `World` can be neither copied nor moved, because its pool list points at its own members.
- **Records:** DEC-46.1, DEC-46.2, DEC-46.3, DEC-46.4, HND-46.1.

---

### HND-47.1 — Components, their descriptions and enabled flags (US-2.2)
- **Date:** 2026-11-23
- **Agent:** claude-code/opus-5.5/us-2.2
- **Work:** US-2.2 #47 on `feat/us-2.2-add-remove-components`, stacked on the US-2.1 branch; pull request not opened yet
- **Goal:** srs.fr.2.6 to 2.12: add, remove, read and change components at runtime, at most one per type, component enabled flags except for Position and Velocity, and a semantic description for each component type.
- **Result:** Done, except the "systems that read and write it" part of srs.fr.2.12, which is derived from system declarations and comes with US-2.3 (D-8). New: `TypeId`, `TypeRegistry` with component and enum builders, the member-count check and `finalize()`; `FieldInfo`, `readField`, `writeField`; `PositionComponent`, `VelocityComponent` and `registerCoreComponents()`. `World` now takes `const TypeRegistry&`, makes one pool per type and has `add`, `remove`, `has`, `get`, `tryGet`, `setComponentEnabled`, `isComponentEnabled`, `pool(TypeId)` and `forEachComponent`; `CommandBuffer` has `add` and `remove`; observers hear about added and removed components.
- **Files changed:** `engine/ecs/**`, `tests/unit/ecs/**`, `tests/CMakeLists.txt`, `docs/design/ecs-api-design.md`, `docs/agents/state/current-status.md`, `docs/history/decision-log.md`.
- **Verified:** `windows-debug` (MSVC 19.44) and `headless` presets with `-DENGINE_WARNINGS_AS_ERRORS=ON` build clean; CTest 367/367 and 354/354; clang-format 19.1.7 dry run clean; `tools/check_ids.py`, `tools/check_md_links.py` and `tools/check_includes.py` pass.
- **Not verified:** GCC and Apple Clang (CI runs them), in particular the member counter of `type_registry.hpp` under GCC.
- **Repo state:** builds and tests pass; committed only after the member approves (DEC-058).
- **Next:**
  1. US-2.3 on a branch stacked on this one: systems, groups and ordering, queries that skip disabled entities and components, access checks, timings, the schema's readBy/writtenBy, and the ECS behind `ISimulation::runPhase()` (DEC-46.3 scope, decided for US-2.3 on 2026-11-23).
  2. US-2.4 on a branch stacked on this one: `.scene` files as JSON through nlohmann/json, using `forEachComponent`, `readField` and `writeField`.
- **Traps:** the registry and every World that uses it must agree: a World keeps a reference to its registry and one pool per registered type. `World::add` sorts the pool before it returns, so take the returned reference after the call, not a pointer from before it.
- **Records:** DEC-47.1, DEC-47.2, DEC-47.3, HND-47.1.
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
