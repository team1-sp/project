---
last_updated: 2026-11-24
updated_by: claude-code/opus-5.5/6r0a
---

# Open Questions

Questions waiting for a human or another agent. **Every material uncertainty is also said to the human directly** (`DECISION NEEDED`, see `AGENT-PROTOCOL.md`); this file is the durable copy. Add new ones at the bottom as `Q-<GitHub #>.<counter>` (DEC-148.1). When a human answers: fill `Answer`, set status `Answered`, record the decision as `Accepted` in `history/decision-log.md` and propagate it (`playbooks/record-decision.md`). This file holds **only open questions**, so it stays small enough to read at every boot. When a question is answered or becomes obsolete, set its status and move the entry, unchanged apart from `Status` and `Answer`, to the bottom of [../../history/questions-archive.md](../../history/questions-archive.md) in the same PR (DEC-152.1).

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
- **Answered by:** project name → Q-005 (answered 2026-10-24: Aegis, DEC-034); branch rules → DEC-027; agent permissions → DEC-025; task tracking → DEC-028; layout/hygiene/CI/naming → DEC-022, `git-workflow.md`, PB-001.
- **Answer:** —

### Q-016 — Which demo game do we build?
- **To:** human:team
- **Asked by:** claude-code (demo-undecided session) on 2026-10-27
- **Status:** Open
- **Context:** Phase 9 needs a demo game with deliberately injectable bugs, used as ground truth for bug detection and the AI assistant (ip.fr.12.*). Earlier docs assumed a 2D maze with a player, walls, a door, a key and a goal (`games/demo-maze/`, `design/demo-game-spec.md`), but the team never agreed on it. Işık Dönger had both removed on 2026-10-27; the build's window check moved to `sandbox/`. Door/key wording elsewhere in the docs is only an example, not the demo.
- **Needed:** the game (genre, entities, win condition) and the bugs it must be able to inject. Record the answer in the decision log, then write the spec under `design/` and create `games/<name>/`.
- **Answer:** —

### Q-017 — May the PB-008 spike link Box2D and raylib directly?
- **To:** human:isik
- **Asked by:** claude (PB-008 session) on 2026-11-01, for Zeynep Ertürk
- **Status:** Open
- **Context:** R-3/R-4 keep Box2D in `engine/physics-box2d/` and raylib in `engine/rendering-raylib/`. The PB-008 spike in `sandbox/` links both directly, kept in separate files (DEC-037, `Proposed`).
- **Options:** (A) accept the exception for this spike only; (B) move the spike's Box2D code into `engine/physics-box2d/` and its drawing into `engine/rendering-raylib/`.
- **Answer:**
