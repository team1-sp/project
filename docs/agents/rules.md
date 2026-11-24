# Agent Rules

Keywords: **MUST** / **MUST NOT** are absolute. **SHOULD** means "do unless you have a written reason". Rules R-1…R-9 are architecture invariants (they restate accepted decisions; see `history/decision-log.md`). **The `Accepted` entries of the decision log are the single source of truth** (DEC-017): if this file or any other document disagrees with them, the decision log wins and the document must be fixed.

## A. Architecture invariants (DEC-001 … DEC-008, DEC-014, DEC-015)

| # | Rule | Why | Source |
| --- | --- | --- | --- |
| R-1 | Dependencies flow one way: **AI Assistant → Semantic Architecture → Engine**. The Engine layer MUST NOT import, call or know about the Semantic or AI layers. It only *emits* data (components, events, telemetry). | Keeps the engine usable and testable alone; the AI layer is replaceable. | DEC-001 |
| R-2 | **ECS is the central data hub.** Physics, rendering, gameplay and telemetry communicate only by reading/writing components and emitting events. Physics MUST NOT call rendering and vice-versa. | Enables headless mode, observability, the semantic graph. | DEC-002 |
| R-3 | The engine MUST NOT call the physics library (Box2D, DEC-014) directly. All physics goes through the **Physics Adapter** interface. Box2D types MUST NOT leak outside the adapter implementation. | Swap the library later without rewriting the engine; keeps a future 3D path open. | DEC-003, DEC-014 |
| R-4 | Same rule for the graphics framework (Raylib, DEC-015): only the rendering layer touches it, through an abstraction. Raylib MUST NOT be initialised in headless mode. | Same reason; headless needs no window/GL context. | DEC-007, DEC-015 |
| R-5 | Physics advances with a **fixed timestep (1/60 s)** independent of render FPS. | Determinism (ip.fr.1.2, ip.fr.7.3). | DEC-004 |
| R-6 | The simulation MUST run identically **with and without rendering** (headless). Nothing in input → ECS → physics → gameplay → telemetry may depend on a window, GPU or wall-clock render time. | Automated playtesting (ip.fr.7.*). | DEC-005 |
| R-7 | **Gameplay decisions belong to gameplay systems**, never to physics (e.g. "does the door open?" — ip.fr.6.6). Physics only provides positions and collisions. | Clear responsibility; correct diagnosis by the AI layer. | DEC-002 |
| R-8 | Components are **pure data** (no behaviour). Systems hold behaviour. Entities are IDs. | ECS definition. | DEC-002 |
| R-9 | Frame order is fixed: input → gameplay input processing → physics sync → physics step → collision events → gameplay rules → telemetry → rendering → present (ip.fr.1.1). A change needs a decision entry. | Reproducibility. | DEC-004 |

## B. Process rules

| # | Rule |
| --- | --- |
| P-1 | MUST follow [AGENT-PROTOCOL.md](AGENT-PROTOCOL.md) each session. |
| P-2 | MUST work only on an issue assigned to the member you work for (or a no-issue `docs`/`ci`/`chore` PR); MUST say in the handoff and on the issue when you leave it unfinished (DEC-149.1). |
| P-3 | MUST NOT make or silently change a decision. **Humans decide; agents consult** (DEC-017). Surface uncertainty with a `DECISION NEEDED` block and log it `Proposed` while you wait for the answer. Write `Accepted` only for a decision a human approves: one they made in the conversation (who, when, words), or one your PR lists under "Decisions for review" (binding only once a human merges it; DEC-052). Then propagate it everywhere (`playbooks/record-decision.md`). |
| P-4 | MUST NOT resolve any open choice yourself — anything marked `[TBD]` or not covered by an `Accepted` decision (currently e.g. ECS implementation, build system, Box2D version, test framework). Raise it with the human; write against the abstraction only if that is genuinely neutral. Decided stack: C++ (DEC-013), Box2D (DEC-014), Raylib (DEC-015), 2D only (DEC-016). |
| P-5 | MUST NOT claim something works unless you ran it. State clearly what was and was not executed. |
| P-6 | MUST NOT edit or delete past entries in `history/` or `handoff-log.md`. Append a correcting entry (decision entries also allow a dated `Amendments` line). Moving an entry unchanged to its archive file (`tools/rotate_handoffs.py`, answered questions, a split problem file) is allowed (DEC-152.1). |
| P-7 | MUST record every solved problem in its area file under `history/problems/` with the exact error text. |
| P-8 | SHOULD keep changes small: one task, one reviewable diff. No drive-by refactors. |
| P-9 | MUST update documentation that your change invalidates, in the same task. After any human decision, update **every** affected document (`playbooks/record-decision.md`). |
| P-11 | MUST tell the human about every material uncertainty or document conflict, in your reply — not only in a file. Course documents (Initial Plan etc.) are not authoritative; the decision log is. |
| P-12 | SHOULD do code and repository docs work in your own cloud copy of the repository; MUST work on a member's computer only when the files, tools or signed-in accounts exist only there, or the member asks, and there MUST ask before every commit and push (DEC-058, DEC-061; `guidelines/git-workflow.md` §1). |
| P-10 | MUST NOT leave the repo with failing build/tests without an issue describing it. |
| P-13 | MUST change requirements only through [playbooks/change-requirements.md](playbooks/change-requirements.md): new ones get a placeholder id (`srs.fr.<n>.new`), removed ones are deleted, and `tools/renumber_requirements.py` renumbers and rewrites references. MUST NOT renumber ids by hand (DEC-152.3). |

## C. Safety and scope rules

| # | Rule |
| --- | --- |
| S-1 | MUST NOT commit secrets (API keys, tokens, `.env`). The AI layer reads keys from environment/config that is git-ignored. |
| S-2 | MUST NOT push to `main`/force-push/rewrite git history unless a human explicitly instructs it in the task (see `guidelines/git-workflow.md`). |
| S-3 | MUST NOT modify files outside this repository — in particular the `team1-sp/docs` course-deliverable repository — unless a human explicitly asks for it. Those documents are built from this repository (requirements from [`docs/requirements/`](../requirements/README.md), DEC-152.2) when a deliverable is submitted; they are **not** the source of truth. Change the source here instead. |
| S-4 | MUST NOT write text *for course deliverables* without logging it in `history/genai-usage-log.md` and marking it per `guidelines/genai-usage-rules.md`. Unmarked AI text in a graded document is treated as plagiarism by the course. |
| S-5 | MUST NOT add heavy dependencies, download assets, or call external AI services without a decision entry. ip.nfr.2.1: no source/telemetry is sent to an external AI service unless the user enabled it. |
| S-6 | MUST NOT expand scope: **2D only** (DEC-016) — no 3D work, not even experiments; also no networking etc. — unless a human decides otherwise. Keep seams so 3D stays possible later; do not build it. |
| S-7 | When unsure whether an action is destructive or irreversible: don't; ask. |

## D. What agents MAY do without asking

- Comment on the issue they are working on.
- Append to `history/*`, `handoff-log.md`, `open-questions.md`.
- Edit `testing/`, `troubleshooting/` and `agents/state/` docs.
- Add tests, fix bugs inside the issue they work on, update docs made stale by their change.
- Propose decisions (`Proposed` while the question is open in the conversation, binds nobody), record human decisions as `Accepted` with propagation, and write a decision their PR carries as `Accepted`, listed under "Decisions for review" (the human approves or rejects it in the review; DEC-052).
- Ask the human questions at any time; that is part of the job.
