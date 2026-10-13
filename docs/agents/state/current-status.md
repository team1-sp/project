---
last_updated: 2026-10-13
updated_by: claude-code/sonnet-5.5/012r
---

# Current Status

Snapshot of the project. Overwrite this file each session (keep it short). History belongs in `handoff-log.md`. **The truth is the `Accepted` entries in `history/decision-log.md`.**

## One-line summary

Pre-implementation. Architecture is defined and the core tech stack is **decided: C++, Box2D, Raylib, 2D only (3D possible later)**. No engine code exists yet; next is the repo skeleton, which waits on a few open choices (build system, ECS approach, versions).

## Phase

Roadmap phase **1 — Project and technology decisions**: core stack decided (DEC-013…016, 018…020); remaining open choices listed below.

## What exists `[VERIFIED 2026-10-12/13 by directory listing]`

| Item | State |
| --- | --- |
| `project/` repo | `README.md`, `.gitignore`, and this `docs/` folder. No source code. |
| `senior-project/docs/` (course deliverables) | Initial Plan sections mostly written. **Not authoritative; not edited by agents.** It may still say "C++ or C#" etc. (see "Course documents affected" in the decision entries) |
| `senior-project/shader-test/` | Graphics benchmark spec, shared assets, per-library folders. Benchmark finished (no notable difference; team chose by code complexity and ease of use). Raylib decided (DEC-015) |
| Process | Scrum, 2-week sprints, weekly progress meeting, sprint review with advisor |

## Decisions status (details: `history/decision-log.md`)

- **Accepted:** three-layer architecture; ECS as data hub; Physics Adapter; fixed 1/60 s timestep; headless mode; telemetry in loop; graphics abstraction; docs/agent system; **C++ (DEC-013); Box2D (DEC-014); Raylib (DEC-015); 2D only, 3D future (DEC-016); humans decide / decision log is the truth (DEC-017); custom ECS (DEC-018); Box2D v3.1.1 (DEC-020).** Team agrees on C++, Box2D and 2D-only (confirmed 2026-10-13).
- **Proposed (not binding):** MCP later (DEC-010); Python + gRPC for the AI service (DEC-011); C++20 + platforms: dev on Windows/macOS, Windows-only target (DEC-019, "still open"); Raylib v6.0 (DEC-021, "still open").
- **Open (asked of the human):** build system + test framework (deferred by the human, Q-009); C++ standard, platforms, compilers, Raylib version (Q-007 — all still open per the human); meaning of "LLM-instructive components" (Q-013); world units (TASK-008); project name (Q-005); sprint dates (Q-002); agent commits (Q-006).

## In progress / next

1. Human answers Q-007 (still open) and, when ready, Q-009 → then TASK-005 (repo skeleton) can start.
2. TASK-010 (custom ECS design with LLM-instructive metadata) once Q-013 is answered.
3. Then phase 2: window + game loop (TASK-006).

## Health

| Check | Result |
| --- | --- |
| Build | n/a (no code) |
| Tests | n/a (no code) |
| Docs | Scaffold created 2026-10-12; updated 2026-10-13 for the stack decision and the decision-authority rule |

## Things a new agent should know immediately

- **Humans decide; agents consult.** Put every real uncertainty to the human (`DECISION NEEDED`); log their decision as `Accepted`; update every doc (`playbooks/record-decision.md`).
- Course documents are not the truth; the decision log is.
- Stack is fixed: C++ / custom ECS / Box2D v3.1.1 (behind the adapter) / Raylib (behind the rendering abstraction). **2D only** — no 3D work. C++ standard, platforms and Raylib version are NOT decided (Q-007).
- Raylib must not be initialised in headless mode.
- The course requires disclosure of AI-generated text in deliverables (`guidelines/genai-usage-rules.md`).
- Project manager: Işık Dönger. Docs in this repo are English.
