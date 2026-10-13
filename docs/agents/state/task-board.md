---
last_updated: 2026-10-13
updated_by: claude-code/sonnet-5.5/012r
---

# Task Board

Statuses: `Backlog` · `Ready` · `Claimed` · `Review` · `Done` · `Blocked` · `Dropped`.
Rules: claim before working; one owner per task; see [communication-protocol.md](../communication-protocol.md). Entry template: [templates/task.md](../templates/task.md).

## Board (summary — keep in sync with entries below)

| ID | Title | Status | Owner / Claimed by | Depends on | Serves |
| --- | --- | --- | --- | --- | --- |
| TASK-000 | Create docs scaffold and agent protocol | Done | claude-code/sonnet-5.5/012r | — | process |
| TASK-001 | Graphics framework / shader benchmark (OpenGL, Raylib, SFML) | Done | humans | — | DEC-012 (superseded by DEC-015) |
| TASK-002 | Decide graphics framework | Done (DEC-015: Raylib) | human decision | — | ip.fr.4.* |
| TASK-003 | Decide programming language and physics library | Done (DEC-013 C++, DEC-014 Box2D) | human decision | — | ip.fr.3.* |
| TASK-004 | Decide ECS approach | Done (DEC-018: custom) | human decision | — | ip.fr.2.* |
| TASK-005 | Bootstrap `project/` skeleton (build system, folders, test runner, Raylib + Box2D linked, empty window) | Blocked | — | Q-009 (deferred), Q-007 (rest) | all |
| TASK-006 | Phase 2: window + game loop with fixed-timestep accumulator | Blocked | — | TASK-005 | ip.fr.1.1–1.4 |
| TASK-007 | Fill `environment-setup.md` once the toolchain is chosen | Blocked | — | TASK-005 | — |
| TASK-008 | Decide world units and coordinate convention (y-up vs y-down, pixels-per-meter) | Backlog — needs human decision | — | Q-008 | ip.fr.3.5, 4.1 |
| TASK-009 | Propagate the 2026-10-13 decisions (DEC-013…017) and role/name fixes through all docs | Done | claude-code/sonnet-5.5/012r | — | DEC-017 |
| TASK-010 | Design the custom ECS incl. LLM-instructive component metadata | Blocked — needs human input | — | Q-013 | ip.fr.2.*, 10.* |
| TASK-011 | Propagate round-2 decisions (DEC-018…021) | Done | claude-code/sonnet-5.5/012r | — | DEC-017 |

Blocked tasks become `Ready` when the dependency is `Done` **and** the related decision is `Accepted` (decided by a human).

## Entries

### TASK-000 — Create docs scaffold and agent protocol
- **Status:** Done (2026-10-12)
- **Claimed by:** claude-code/sonnet-5.5/012r
- **Result:** Folder tree per `docs/README.md`; seeded logs, state files, templates, playbooks. Reviewed by the project manager 2026-10-13 ("looks good", with four corrections → TASK-009).

### TASK-001 — Graphics framework / shader benchmark
- **Status:** Done (reported by Işık Dönger 2026-10-13): benchmark finished, no notable difference; team chose by code complexity and ease of use (DEC-015).
- **Spec:** `senior-project/shader-test/test-guideline.md`; summary in `testing/benchmark-protocol.md`.

### TASK-002 — Decide graphics framework
- **Status:** Done — human decision recorded as DEC-015 (Raylib), 2026-10-13.

### TASK-003 — Decide programming language and physics library
- **Status:** Done — DEC-013 (C++), DEC-014 (Box2D), 2026-10-13. Scope: DEC-016 (2D only).

### TASK-004 — Decide ECS approach
- **Status:** Done — DEC-018 (custom ECS), 2026-10-13. Follow-up: TASK-010.

### TASK-005 — Bootstrap project skeleton
- **Status:** Blocked by Q-009 (build system and test framework — deferred by the human; ask again when ready) and Q-007 (C++ standard, platforms, compilers, Raylib version — all still open per the human). Decided input: Box2D v3.1.1; C++20 only proposed.
- **Acceptance:** Clean checkout builds an executable that links Raylib and Box2D and opens/closes a window; the test runner runs one trivial test headless (no window); `repository-structure.md` updated to reality; `environment-setup.md` has exact commands.

### TASK-006 — Window + game loop
- **Status:** Blocked by TASK-005
- **Acceptance:** Window opens/closes cleanly (ip.fr.1.3); fixed-timestep accumulator for physics at 1/60 s decoupled from render (ip.fr.1.2); mode flag normal/headless parsed (ip.fr.1.4) — headless path must not initialise Raylib.

### TASK-007 — Environment setup doc
- **Status:** Blocked by TASK-005
- **Acceptance:** A new team member can build and run from a clean machine using only `environment-setup.md`.

### TASK-008 — World units and coordinate convention
- **Status:** Backlog — needs a human decision (present options; do not choose)
- **Why it matters:** Raylib's screen space is y-down; Box2D is y-up and works best in meters; mixing them is a classic bug (`common-errors.md` E-004, E-005).
- **Acceptance:** Accepted decision; documented in `design/physics-adapter-design.md`.

### TASK-009 — Propagate the 2026-10-13 decisions
- **Status:** Done (2026-10-13), claude-code/sonnet-5.5/012r
- **Result:** DEC-013…017 logged; AGENT-PROTOCOL rewritten (Truth Rule, Raising uncertainty, Disagreeing); `playbooks/record-decision.md` added; stack/scope updates applied across docs; Işık Dönger recorded as Project Manager; full names for Arda Karabulut and Zeynep Ertürk.
- **Not verified:** correctness of the team's agreement beyond the project manager's statement (Q-010).

### TASK-010 — Design the custom ECS (LLM-instructive components)
- **Status:** Blocked — needs the human's answer to Q-013
- **Serves:** DEC-018, ip.fr.2.*, ip.fr.10.*
- **Acceptance:** Design doc (from `design/templates/design-doc-template.md`) satisfying `design/ecs-design.md` §1 and the metadata decided in Q-013; accepted by the human.

### TASK-011 — Propagate round-2 decisions
- **Status:** Done (2026-10-13), claude-code/sonnet-5.5/012r. DEC-018, DEC-020 Accepted; DEC-019, DEC-021 Proposed (corrected 2026-10-13); docs updated; Q-003/Q-008/Q-010/Q-012 closed; Q-009 recorded as deferred; Q-007 narrowed; Q-013 added.
