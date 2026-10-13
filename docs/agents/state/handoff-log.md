# Handoff Log

**Append-only. Newest entry at the bottom.** Never edit or delete earlier entries; add a correcting entry instead. Template: [../templates/handoff.md](../templates/handoff.md). Read the last ~5 entries at session start.

---

### HND-001 — Docs scaffold created
- **Date:** 2026-10-12
- **Agent:** claude-code/sonnet-5.5/012r
- **Task:** TASK-000
- **Goal:** Create developer docs and the agent coordination system under `project/docs/`.
- **Result:** Done, awaiting human review.
- **Files changed:** entire `project/docs/` tree (new).
- **Verified:** Directory listing after write. Cross-checked terminology and flow against the Senior Project Guide, the Project Context Prompt and Initial Plan §2 (requirements IDs). No code exists, so nothing was built or run.
- **Not verified:** Whether the team agrees with every `Accepted` decision in the decision log (they were extracted from the guide and Initial Plan, not formally voted). Status of the shader benchmark implementations.
- **Repo state:** No code. `project/` contains README, .gitignore, docs.
- **Next:** (1) Humans review `decision-log.md` DEC-001…DEC-009 and change anything wrong; (2) answer Q-001…Q-004 in `open-questions.md`; (3) finish TASK-001 benchmark; (4) an agent may then prepare decision briefs (TASK-002…004).
- **Traps:** Technology choices are intentionally `[TBD]`. Do not start engine code before TASK-005.

---

### HND-002 — Feedback round: roles, stack decision, decision-authority protocol
- **Date:** 2026-10-13
- **Agent:** claude-code/sonnet-5.5/012r
- **Task:** TASK-009
- **Goal:** Apply the project manager's four corrections: (1) Işık Dönger is Project Manager; (2) full names; (3) stack decided: C++, Box2D, Raylib, 2D only with 3D possible later; (4) humans decide, uncertainty always surfaced, decision log is the single truth, course docs not authoritative.
- **Result:** Done.
- **Files changed:** `AGENT-PROTOCOL.md` (rewritten), new `playbooks/record-decision.md`, `templates/decision-entry.md`, `rules.md`, `communication-protocol.md`, `model-compatibility.md`, `agents/README.md`, `decision-log.md` (DEC-013…017; DEC-006, DEC-012 superseded), `tech-stack.md` (rewritten), state files, coding standards, repo structure, environment setup, common errors, engine layer, glossary, physics adapter, headless testing, benchmark protocol, roadmap, test strategy, development process, documentation rules, README. The user had already edited `README.md` and `process/development-process.md` for names/roles; their versions were used as the base.
- **Verified:** relative links checked by script; grep for old terms (C#, MonoGame, BEPUphysics, "Arda K.", "Zeynep" alone) leaves mentions only in history/decision entries. No code exists; nothing built.
- **Not verified:** that the whole team agrees with the stack (Q-010); rationale for each choice (not given).
- **Next:** humans answer Q-007, Q-008, Q-009, Q-012 → record + propagate → TASK-005.
- **Traps:** Raylib needs a window/GL context for loading textures — keep it out of headless mode. Box2D v2.4 and v3 have different APIs (Q-008). The Initial Plan still says "C++ or C#" etc.; it is not authoritative and was not edited.
- **Records:** DEC-013…017, Q-007…Q-010, Q-012, TASK-009.

---

### HND-003 — Round-2 answers recorded and propagated
- **Date:** 2026-10-13
- **Agent:** claude-code/sonnet-5.5/012r
- **Task:** TASK-011
- **Goal:** Record Işık Dönger's answers to Q-012, Q-009, Q-007, Q-008, Q-003, Q-010 and update every document (record-decision playbook).
- **Result:** Done; two items intentionally open.
- **Decided / recorded:** DEC-018 custom ECS; DEC-019 C++20, dev Windows+macOS, Windows-only target; DEC-020 Box2D v3.1.1; amendments on DEC-013/014/015. DEC-021 (Raylib v6.0) left `Proposed` — the human wrote "still open". Q-009 recorded as deferred. New Q-013.
- **Files changed:** decision log + entry template (Amendments field), record-decision playbook, tech-stack, coding-standards, repository-structure, physics-adapter-design, ecs-design, semantic-layer, common-errors, environment-setup, test-strategy, benchmark-protocol, roadmap, state files.
- **Not verified:** Box2D v3.1.1 / Raylib v6.0 API details (from memory, `[ASSUMPTION]`); team agreement on DEC-013/014/016.
- **Next:** human answers rest of Q-007 and Q-013; Q-009 when ready → TASK-005, TASK-010.
- **Traps:** Box2D v3 is a C API with ids; most tutorials are v2.4. Do not start the skeleton: build system deferred.
- **Records:** DEC-018…021, Q-013, TASK-010, TASK-011.

---

### HND-004 — Correction: C++20/platforms are proposals, not decisions
- **Date:** 2026-10-13
- **Agent:** claude-code/sonnet-5.5/012r
- **Task:** TASK-011 (follow-up)
- **What happened:** In HND-003 I recorded DEC-019 (C++20, platforms) as Accepted, reading "still open" as applying only to Raylib. Işık Dönger clarified that "still open" covered the whole Q-007 answer. DEC-019 is now Proposed (with an Amendment describing my error); docs that assumed C++20/platforms were reverted to "proposed".
- **Also recorded:** the team agrees on C++, Box2D and 2D-only (amendments on DEC-013, DEC-014, DEC-016).
- **Lesson:** when an answer contains a hedge, confirm its scope before recording `Accepted` (record-decision.md: "If you are not sure the human decided, ask once more").
- **Records:** DEC-013, DEC-014, DEC-016, DEC-019, DEC-021, Q-007.

---
