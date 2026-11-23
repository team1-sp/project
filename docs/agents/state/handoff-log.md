# Handoff Log

**Append-only. Newest entry at the bottom.** Never edit earlier entries; add a correcting entry instead. Template: [../templates/handoff.md](../templates/handoff.md). Read the last ~5 entries at session start. This file keeps only the newest entries: older ones are moved, unchanged, to [../../history/handoff-archive/](../../history/handoff-archive/) by `tools/rotate_handoffs.py` (DEC-152.1), which you run when `tools/check_ids.py` says the file holds more than 15 entries.

---

### HND-035 — AI Assistant entry points pushed for review
- **Date:** 2026-11-22
- **Agent:** codex/gpt-5/118-assistant
- **Task:** TASK-024, GitHub #118
- **Goal:** Commit and publish the human-approved US-33.1 implementation.
- **Result:** Commit `0c96323` pushed to `feat/118-open-ai-assistant`; draft PR #145 opened with
  `Closes #118`, the acceptance checklist and verification results.
- **Files changed:** no implementation changes after HND-034; live task/status records now point to
  PR #145.
- **Verified:** latest `origin/main` remained the branch base; pre-commit ID/link/diff checks passed;
  GitHub accepted the branch and created the draft PR.
- **Not verified:** GitHub CI and human review are pending.
- **Repo state:** branch pushed and tracking `origin/feat/118-open-ai-assistant`; draft PR #145 open.
- **Next:** watch CI, address review findings, and let a human mark ready/merge when approved.
- **Traps:** none beyond HND-034.
- **Records:** no new decision, problem or open question.

---

### HND-036 — Merge `main` (US-33.5, US-11.1) into the US-33.1 branch
- **Date:** 2026-11-22
- **Agent:** claude-code/opus-5.5/118-merge
- **Task:** TASK-024, GitHub #118, PR #145
- **Result:** Resolved conflicts with PRs #143 and #144. The US-33.1 placeholder Project Settings
  shell is removed; AI > AI Settings... uses US-33.5's `canOpenAiPrivacySettings()` /
  `openAiPrivacySettings()`, and `AssistantEntryPoints::openAiSettings()` requests are forwarded by
  `MainWindowView` to the same AI and Privacy window. `IEditorActions::openAiSettings()` is gone.
  Records renumbered: this story is TASK-024 and HND-034/035, since `main` used TASK-022/023 and
  HND-032/033.
- **Verified:** see the PR #145 merge commit.
- **Next:** CI and human review of PR #145.

---

### HND-037 — Restore the AI Settings entry point after the US-33.5 merge
- **Date:** 2026-11-22
- **Agent:** codex/gpt-5/118-assistant
- **Task:** TASK-024, GitHub #118, PR #145
- **Goal:** Fix AI > AI Settings... doing nothing when the Editor has no project-backed settings
  controller yet.
- **Result:** Project Settings and AI Settings remain available and open the AI and Privacy shell;
  without a project controller the shell says `No project open`, while an injected US-33.5
  controller continues to render the full settings controls. The status-bar AI target uses the
  same action.
- **Verified:** targeted Editor CTest 29/29, complete headless CTest 237/237, the macOS Editor build,
  clang-format 19, include policy, record IDs, documentation links and `git diff --check` passed.
- **Not verified:** interactive clicking through the standalone raylib window because the
  computer-use harness could not bind to that process; CI and Windows compilation remain pending.
- **Repo state:** the human approved the completed diff for commit and push to
  `feat/118-open-ai-assistant` under DEC-053/058.
- **Next:** check PR #145 CI and address any review findings.
- **Traps:** do not construct a settings controller with a fake path merely to enable the command;
  project opening owns the real per-user project path and bridge factory.
- **Records:** PRB-003 repeat occurrence; no new decision or open question.

---

### HND-039 — Check record ids locally with git hooks (#147)
- **Date:** 2026-11-22
- **Agent:** claude-code/opus-5.5/147-hooks
- **Task:** GitHub #147 (no issue)
- **Result:** Added `.githooks/pre-commit`, which runs `tools/check_ids.py` and stops a commit when two record entries share an id, and `.githooks/pre-merge-commit`, which runs the same check for merges git completes without conflicts (those skip `pre-commit`). Documented `git config core.hooksPath .githooks` in `CONTRIBUTING.md` and `git-workflow.md` §4, and added `.githooks/` to `repository-structure.md`.
- **Verified (macOS, git 2.53, Python 3.14):** in a scratch clone, a conflict-free merge giving two entries TASK-999 stopped before the merge commit, a plain commit adding a duplicate stopped, and a clean commit passed; `check_ids.py` and `check_md_links.py` pass.
- **Not verified:** Windows Git Bash (the hooks are POSIX sh and fall back to `python`).
- **Next:** each member and agent copy runs `git config core.hooksPath .githooks` once. The hooks catch collisions at sync time but do not prevent them; ids that cannot collide (for example issue number plus counter) would, and are a team decision.
- **Traps:** the check reads the working tree, so a duplicate in unstaged changes also blocks a commit. HND-039 skips HND-034 to HND-038, already used by open PRs #145 and #146.

---

### HND-040 — AI Assistant chat, context chips and Stop (US-33.2)
- **Date:** 2026-11-22
- **Agent:** claude-code/opus-5.5/119-chat
- **Task:** TASK-025, GitHub #119, srs.fr.33.5 to 33.12
- **Goal:** composer, Send/Stop, context chips, New Chat and History over the #71/#72/#118 public interfaces.
- **Result:** Done locally on `feat/119-ai-chat-context`, stacked on `feat/118-open-ai-assistant` merged with `feat/71-grounded-assistant-answers` (commit `694ad0a`; agent-record conflicts kept both entries, #71's is TASK-023/HND-033 on `main`). Added `AssistantChat`, `ContextChipSet`, `GroundedChatQuestionSender`, `ChatStoreLoader`, `GroundedContextSizeEstimator`, `EditorContextItemProvider` and the ImGui `AssistantChatPanel`; #71, #72 and #118 headers unchanged.
- **Verified:** headless and macOS Debug builds with warnings as errors, CTest 258/258 and 271/271; GCC 15 headless with warnings as errors 258/258 after merging #145's CI fix 7a22193; new tests repeated 200 times; clang-format 19.1.7, `check_includes`, `check_ids`, `check_md_links`, `git diff --check`; `editor --max-ticks 2` exits 0.
- **Not verified:** ThreadSanitizer (instrumented binary crashed in gtest discovery), clicking through the panel, Windows/CI, a live AI service.
- **Repo state:** committed and pushed to `feat/119-ai-chat-context` with the member's approval (DEC-058); draft PR opened against `main`.
- **Next:** human review of PR #146 after PR #145 merges.
- **Traps:** do not read the answer message while `isAnswering()`; `NoUserArea` returns an empty path, so plugging in a real bridge before a project path exists would write chats relative to the working directory (PRB-006).
- **Records:** DEC-071, PRB-007.

---

### HND-041 — Merge `main` and the resolved US-33.1 branch into the US-33.2 branch
- **Date:** 2026-11-22
- **Agent:** claude-code/opus-5.5/119-merge
- **Task:** TASK-025, GitHub #119, PR #146
- **Result:** Merged `feat/118-open-ai-assistant` at `a6af34c` (which already contains `main` with
  PRs #143 and #144) instead of `main` directly, so the US-33.1/US-33.5 overlap keeps HND-036's
  resolution. `MainWindowView` takes `AssistantChat&` on top of that constructor; US-33.1's
  placeholder draft box stays replaced by `AssistantChatPanel`. Grounded assistant files equal
  `main`. Records renumbered: US-33.2 is TASK-025, DEC-071 and HND-040, since `main` and #145 use
  DEC-069/070, TASK-022 to 024 and HND-032 to 037, and #147 uses HND-039.
- **Verified:** see the PR #146 merge commit.
- **Next:** CI and human review of PR #146; merge after PR #145.

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
