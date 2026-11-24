# Questions Archive

**Moved, not edited.** Answered and obsolete questions from [../agents/state/open-questions.md](../agents/state/open-questions.md), kept for tracing back. Agents do not read this file at boot. Newest moves at the bottom.

---

### Q-015 — Task IDs: TASK-### (docs) or PB-### (ClickUp)?
- **To:** human:isik
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-16
- **Status:** Answered 2026-11-22 (DEC-149.1)
- **Context:** GitHub branches, PR titles and issues use ClickUp's `PB-###` ids (DEC-028, `git-workflow.md`). The agent docs still use `TASK-###` on `agents/state/task-board.md`, in commit trailers (`Refs: TASK-###`) and in playbooks. Two id systems for the same work will drift.
- **Options (not decisions):** (A) ClickUp is the only task list; `task-board.md` keeps only a short pointer plus a `PB` column mapping; agent docs say `PB-###`. (B) Keep `task-board.md` as an agent-facing mirror, each `TASK-###` carries its `PB-###`. (C) Leave as is. I lean (B) now (smallest change, agents keep a file they can read offline) and (A) later.
- **Answer:** neither: ClickUp was replaced by GitHub issues (DEC-050), and the task board and TASK ids were retired; tasks are GitHub issues (DEC-149.1).

### Q-005 — Project / product name?
- **To:** human:team
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-12
- **Status:** Answered 2026-10-24 → DEC-034
- **Context:** Needed for namespaces, binary names and repo naming.
- **Answer:** Işık Dönger: "project name is Aegis decided by the team."

### Q-013 — What should "LLM-instructive components" contain?
- **To:** human:isik
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-13
- **Status:** Answered for Phase 1 (2026-11-05, DEC-042). Was deferred by the human ("too early", 2026-10-15).
- **Context:** DEC-018: the custom ECS exists so components are more LLM-instructive via the semantic layer.
- **Options (not decisions):** (A) minimal: component name + per-field name/type; (B) plus human-readable purpose and per-field description/unit/range; (C) plus declared owner/readers and example values. Declared in C++20 by a registration function per component vs macro/table. I lean (B)/(C) because the assistant must cite meaning, not just values — your call.
- **Answer:** 2026-11-05, Işık Dönger, for Phase 1 (DEC-042 item 10): a registration builder (no macros) with component and field descriptions, units, and readBy/writtenBy derived from system declarations. Valid ranges, examples and notes may be added later without changing the API (`ecs-api-design.md` §11, §14).

### Q-019 — Are the PB-011 structure choices accepted?
- **To:** human:isik
- **Asked by:** claude-code (PB-011 session) on 2026-11-10, for Arda Karabulut
- **Status:** Answered 2026-11-11 → DEC-046
- **Context:** PB-011's ClickUp text conflicts with the accepted layout in four places. Arda Karabulut chose: the window in `engine/rendering-raylib` behind an `IWindow` interface in `engine/core` (not a new `engine/platform`); `ISimulation` and `IFrameRenderer` as the loop's seams until the ECS exists; one guarded MSVC-only call for the leak check; `sandbox` as the example application. Recorded as DEC-046, `Proposed`.
- **Options:** (A) accept DEC-046 as it is; (B) change a point in the PB-011 review.
- **Answer:** (A), 2026-11-11, Işık Dönger after merging PR #35: "I want you to update decision from pr 35 and 37 as accepted since they are only proposed now." DEC-046 is `Accepted`.

### Q-020 — Is the fixed-timestep loop of PB-012 accepted as the SRS states it?
- **To:** human:isik
- **Asked by:** claude-code (PB-012 session) on 2026-11-10, for Arda Karabulut
- **Status:** Answered 2026-11-11 → DEC-047
- **Context:** `engine-layer.md` had the cap on ticks per render frame only as `[PROPOSED: cap 5]`; the SRS fixes it (srs.fr.1.3, 1.4) but the decision log had no entry. PB-012 implements srs.fr.1.2 to 1.6 with an exact integer accumulator and a tick counter in `engine/core`. Recorded as DEC-047, `Proposed`.
- **Options:** (A) accept DEC-047 as it is; (B) change a point in the PB-012 review.
- **Answer:** (A), 2026-11-11, Işık Dönger after merging PR #37: "I want you to update decision from pr 35 and 37 as accepted since they are only proposed now." DEC-047 is `Accepted`.

### Q-018 — Which Python version should `ai-services` pin?
- **To:** human:arda-s
- **Asked by:** codex/gpt-5/pb006 on 2026-11-04
- **Status:** Answered 2026-11-04 → DEC-039
- **Context:** GitHub issue #20 requires a pinned Python version, but no accepted decision or repository configuration named one. HND-013 explicitly called Python 3.12 an assumption until PB-006 chose the version.
- **Options:** (A) Python 3.12 — mature ecosystem support and already used provisionally by CI; (B) Python 3.13 — newer runtime, with a greater chance that future AI dependencies lag support.
- **Recommendation:** Python 3.12 for the initial skeleton.
- **Answer:** Arda Saritas: "proceed with 3.12".

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
