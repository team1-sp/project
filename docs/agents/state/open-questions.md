---
last_updated: 2026-10-27
updated_by: claude-code (demo-undecided session)
---

# Open Questions

Questions waiting for a human or another agent. **Every material uncertainty is also said to the human directly** (`DECISION NEEDED`, see `AGENT-PROTOCOL.md`); this file is the durable copy. Add new ones at the bottom with the next `Q-###`. When a human answers: fill `Answer`, set status `Answered`, record the decision as `Accepted` in `history/decision-log.md` and propagate it (`playbooks/record-decision.md`). Keep answered questions until archived — do not delete.

Statuses: `Open` · `Answered` · `Obsolete`.

---

## Open

### Q-002 — Which sprint is current, and what are the dates?
- **To:** human:arda-s (Scrum Master)
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-12
- **Status:** Open
- **Context:** Plan dates are unknown. Gantt is in ClickUp. Needed to date the plan and size tasks.
- **Answer:** —

### Q-004 — Are the extracted "Accepted" decisions DEC-001…DEC-008 and DEC-012 correct?
- **To:** human:isik
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-12
- **Status:** Open (partly answered: user said the docs "look good" on 2026-10-13; DEC-001…008 not individually confirmed)
- **Context:** These were extracted from the guide and Initial Plan, not individually voted.
- **Answer:** —

### Q-014 — Left over from the GitHub repository setup
- **To:** human:isik (consult the team)
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-15; reduced 2026-10-16 after reconciling with GitHub main
- **Status:** Open — most items were answered on 2026-10-15 (DEC-022…028). Still open:
  1. **Visibility and branch protection.** *Protection answered:* the "Protect Main" ruleset is active (DEC-032; checked through the GitHub API on 2026-10-27, PB-004). Whether the repo stays public is still open. `team1-sp/project` is now **public** (clone works without sign-in), but DEC-027 and `CONTRIBUTING.md` say branch protection is unavailable (private repo, free plan). Public repositories can use branch protection/rulesets on the free plan `[ASSUMPTION — check GitHub docs/settings]`. Is the repo meant to stay public? If yes, should protection be switched on now (same rules as DEC-027)? Also: everything in `docs/` (names, decisions, course context) is now publicly readable.
  2. **License.** No `LICENSE` file exists. A public repository without a license is "all rights reserved" by default. Which license (or none on purpose)? Raylib is zlib and Box2D is MIT, so any choice works.
  3. **Access.** GitHub usernames/roles of the 5 members and read access for the advisor (Leyla Sezer), if the repo goes private again.
  4. **Git LFS** for assets: yes or no? [no until needed]
  5. *(Answered: the `build` job runs on Windows, macOS and headless Linux since PB-002.)* **Windows and macOS in CI.** `.github/workflows/ci.yml` runs every job on `ubuntu-latest` only, and the layout lists Windows presets only. Once code exists, builds must run on Windows and macOS (DEC-029). Add them with PB-002, or earlier? [with PB-002]
- **Answered by:** project name → Q-005 (answered 2026-10-24: Aegis, DEC-034); branch rules → DEC-027; agent permissions → DEC-025; task tracking → DEC-028; layout/hygiene/CI/naming → DEC-022, `git-workflow.md`, PB-001.
- **Answer:** —

### Q-015 — Task IDs: TASK-### (docs) or PB-### (ClickUp)?
- **To:** human:isik
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-16
- **Status:** Open
- **Context:** GitHub branches, PR titles and issues use ClickUp's `PB-###` ids (DEC-028, `git-workflow.md`). The agent docs still use `TASK-###` on `agents/state/task-board.md`, in commit trailers (`Refs: TASK-###`) and in playbooks. Two id systems for the same work will drift.
- **Options (not decisions):** (A) ClickUp is the only task list; `task-board.md` keeps only a short pointer plus a `PB` column mapping; agent docs say `PB-###`. (B) Keep `task-board.md` as an agent-facing mirror, each `TASK-###` carries its `PB-###`. (C) Leave as is. I lean (B) now (smallest change, agents keep a file they can read offline) and (A) later.
- **Answer:** —

### Q-005 — Project / product name?
- **To:** human:team
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-12
- **Status:** Answered 2026-10-24 → DEC-034
- **Context:** Needed for namespaces, binary names and repo naming.
- **Answer:** Işık Dönger: "project name is Aegis decided by the team."

### Q-013 — What should "LLM-instructive components" contain?
- **To:** human:isik
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-13
- **Status:** Open — **deferred by the human** ("too early", 2026-10-15). Do not raise until a human does. Blocks TASK-010 only.
- **Context:** DEC-018: the custom ECS exists so components are more LLM-instructive via the semantic layer.
- **Options (not decisions):** (A) minimal: component name + per-field name/type; (B) plus human-readable purpose and per-field description/unit/range; (C) plus declared owner/readers and example values. Declared in C++20 by a registration function per component vs macro/table. I lean (B)/(C) because the assistant must cite meaning, not just values — your call.
- **Answer:** —

### Q-016 — Which demo game do we build?
- **To:** human:team
- **Asked by:** claude-code (demo-undecided session) on 2026-10-27
- **Status:** Open
- **Context:** Phase 9 needs a demo game with deliberately injectable bugs, used as ground truth for bug detection and the AI assistant (ip.fr.12.*). Earlier docs assumed a 2D maze with a player, walls, a door, a key and a goal (`games/demo-maze/`, `design/demo-game-spec.md`), but the team never agreed on it. Işık Dönger had both removed on 2026-10-27; the build's window check moved to `sandbox/`. Door/key wording elsewhere in the docs is only an example, not the demo.
- **Needed:** the game (genre, entities, win condition) and the bugs it must be able to inject. Record the answer in the decision log, then write the spec under `design/` and create `games/<name>/`.
- **Answer:** —

---

## Answered

### Q-006 — Should agents be allowed to commit to feature branches?
- **Status:** Answered 2026-10-15: agents push only after a human approves that push; humans approve and merge every PR (DEC-025).

### Q-009 — Build system and test framework?
- **Status:** Answered 2026-10-15 → **CMake + GoogleTest** (DEC-023).

### Q-003 — Status of the benchmark?
- **Status:** Answered 2026-10-13: finished; no notable difference, as expected; team decided on code complexity and ease of use (DEC-015 amendment). TASK-001 Done.

### Q-008 — Which Box2D version?
- **Status:** Answered 2026-10-13 → **v3.1.1** (DEC-020).

### Q-010 — Why Raylib; does the team agree?
- **Status:** Answered 2026-10-13: whole team agreed on Raylib — best compromise of ease of use and functionality (DEC-015 amendment). Team agreement on DEC-013/014/016 not explicitly stated; treated as agreed unless a human says otherwise.

### Q-012 — ECS: custom or library?
- **Status:** Answered 2026-10-13 → **custom ECS**, so components are LLM-instructive via the semantic layer (DEC-018). Follow-up: Q-013.

### Q-007 — C++ standard, platforms, compilers, Raylib version
- **Status:** Answered 2026-10-13…15 → platforms DEC-029, compilers DEC-030, C++20 DEC-019, Raylib v6.0 DEC-021 (the last two accepted 2026-10-15).

### Q-001 — Is C++ or C# the implementation language?
- **Status:** Answered 2026-10-13 → **C++** (DEC-013). Also decided: Box2D (DEC-014), Raylib (DEC-015), 2D only (DEC-016).

---

## Archive

(none yet)
