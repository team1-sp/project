---
last_updated: 2026-10-13
updated_by: claude-code/sonnet-5.5/012r
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

### Q-005 — Project / product name?
- **To:** human:team
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-12
- **Status:** Open
- **Context:** Needed for namespaces, binary names and repo naming.
- **Answer:** —

### Q-007 — C++ standard, platforms, Raylib version, compilers
- **To:** human:isik
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-13
- **Status:** Open — the human answered with propositions and clarified "still open" covers all of it (2026-10-13): C++20, development on Windows+macOS with Windows-only target (DEC-019, Proposed), Raylib v6.0 (DEC-021, Proposed).
- **Still unclear:** (0) Which of the proposals will the team adopt, and by when? (1) Compilers/toolchain per OS (e.g. MSVC on Windows, Clang on macOS?). (2) Must the engine **build and run** on macOS during development, or is macOS only for editing/docs? (3) What would settle C++20, the platform split and Raylib v6.0 (e.g. a team check, a build test)? Say "decided" for any of them and I will record it.
- **Answer:** —

### Q-009 — Build system and test framework?
- **To:** human:isik
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-13
- **Status:** Open — **deferred by the human** ("to be decided later", 2026-10-13). Blocks TASK-005; agents must not choose. If the proposed Windows+macOS development (DEC-019) is adopted, it must work on both.
- **Answer:** —

### Q-013 — What should "LLM-instructive components" contain?
- **To:** human:isik
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-13
- **Status:** Open → blocks TASK-010 only
- **Context:** DEC-018: the custom ECS exists so components are more LLM-instructive via the semantic layer.
- **Options (not decisions):** (A) minimal: component name + per-field name/type; (B) plus human-readable purpose and per-field description/unit/range; (C) plus declared owner/readers and example values. Declared in C++20 by a registration function per component vs macro/table. I lean (B)/(C) because the assistant must cite meaning, not just values — your call.
- **Answer:** —

### Q-006 — Should agents be allowed to commit to feature branches?
- **To:** human:isik
- **Asked by:** claude-code/sonnet-5.5/012r on 2026-10-12
- **Status:** Open
- **Context:** `guidelines/git-workflow.md` proposes: agents may commit to `task/*` branches, never to `main`; humans merge.
- **Answer:** —

---

## Answered

### Q-003 — Status of the benchmark?
- **Status:** Answered 2026-10-13: finished; no notable difference, as expected; team decided on code complexity and ease of use (DEC-015 amendment). TASK-001 Done.

### Q-008 — Which Box2D version?
- **Status:** Answered 2026-10-13 → **v3.1.1** (DEC-020).

### Q-010 — Why Raylib; does the team agree?
- **Status:** Answered 2026-10-13: whole team agreed on Raylib — best compromise of ease of use and functionality (DEC-015 amendment). Team agreement on DEC-013/014/016 not explicitly stated; treated as agreed unless a human says otherwise.

### Q-012 — ECS: custom or library?
- **Status:** Answered 2026-10-13 → **custom ECS**, so components are LLM-instructive via the semantic layer (DEC-018). Follow-up: Q-013.

### Q-001 — Is C++ or C# the implementation language?
- **Status:** Answered 2026-10-13 → **C++** (DEC-013). Also decided: Box2D (DEC-014), Raylib (DEC-015), 2D only (DEC-016).

---

## Archive

(none yet)
