# Playbook — Record a Human Decision and Propagate It

Use whenever a human decides something: answers a `DECISION NEEDED`, answers a `Q-###`, changes or confirms an earlier decision, or states a fact about the project ("the tech stack is X").

Principle: the decision log is the single source of truth. A decision is finished only when **every** document, task and test agrees with it.

## Who may do what

| Action | Human | Agent |
| --- | --- | --- |
| Decide | Yes | **No** — agents consult: present uncertainty, options, recommendation |
| Write the entry as `Accepted` | Yes | **Yes, but only when recording an explicit human decision**, citing who decided, when, and their words |
| Write a `Proposed` entry | Yes | Yes (binds nobody) |
| Edit course documents (`senior-project/docs/`) | Yes | No (S-3) — list them for humans |

If you are not sure the human actually *decided* (vs. "maybe", "I think", "probably"), ask once more: "Shall I record this as the decision?"

## Steps

1. **Capture the decision exactly.** Who decided, when, in what words, what it replaces. Quote them (short).
2. **Check for contradictions with other `Accepted` decisions.** If the new decision conflicts with one, supersede it explicitly (step 4) rather than leaving two truths.
3. **Write the entry** in `history/decision-log.md` from [../templates/decision-entry.md](../templates/decision-entry.md) with status `Accepted`, `Decided by`, the quote, and a `Docs updated` list (filled at step 6). Add it to the index.
4. **Supersede.** For each entry the new decision replaces, change its status line to `Superseded by DEC-###` (the only status edit allowed; you may also append a dated line to its `Amendments` field — e.g. a human later supplies the rationale or a pinned version — but never change what was decided). Resolved `Proposed` entries become `Accepted` or `Rejected` (with the human's reason).
5. **Find everything that depends on the old belief.** Search the repo docs for the old term and for the topic (e.g. old technology names, `[TBD]` markers, "Q-###" references, the old rule text). Check at least:
   - `architecture/` (tech-stack, overview, layer docs, glossary)
   - `design/` (adapter, ECS, telemetry, demo)
   - `guidelines/`, `process/`, `testing/`, `troubleshooting/`
   - `agents/rules.md` (invariants, rule text), `agents/state/*` (status, plan, task board, open questions), playbooks, templates
   - `history/problem-log.md` / `common-errors.md` entries that become obsolete (mark `obsolete`, do not delete)
   - code, tests and comments (if code exists)
6. **Update them all** to the new decision. Replace `[TBD]` markers, remove obsolete conditional sections, unblock tasks (`Blocked → Ready`), close the answered `Q-###` (`Answered`, link the `DEC-###`), update `current-status.md` and `plan.md`. Record the list in the entry's `Docs updated` field.
7. **Course documents drift.** Anything in `senior-project/docs/` (Initial Plan etc.) that now disagrees: do **not** edit. Add the list to the entry's `Course documents affected (for humans)` field and mention it in your reply. They are for the course, not the truth.
8. **Create follow-up work** the decision implies (new `TASK-###`, new `Q-###` for questions it opens).
9. **Verify propagation:** search again for the old term; the only remaining mentions must be in `history/` (past entries) or clearly marked as history.
10. **Tell the human** in 2–4 lines: what was recorded, what was updated, which course documents they may want to update, and what new questions it raised.
11. Handoff entry lists the decision IDs.

## Example (abridged)

Human: "Physics library is Box2D."
- DEC-014 `Accepted`, decided by `<human>`, quote.
- Old `[TBD]` in `tech-stack.md`, `physics-adapter-design.md`, `current-status.md`, `plan.md`, `task-board.md`, `open-questions.md` updated; BEPUphysics mentions kept only as "considered" history in the decision entry.
- Course documents affected: Initial Plan §… mentions "Box2D or BEPUphysics" → human to update.
- New question: Box2D version (v2.4 C++ vs v3 C)?

## Checklist

```
[ ] Quote and decider recorded      [ ] Index row added
[ ] Conflicting/old entries superseded
[ ] All project docs/state/tasks updated (list in "Docs updated")
[ ] Old term search shows only history mentions
[ ] Q-### closed, follow-ups created
[ ] Course documents listed for humans (not edited)
[ ] Human told
```
