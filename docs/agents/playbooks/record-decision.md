# Playbook — Record a Human Decision and Propagate It

Use whenever a human decides something: answers a `DECISION NEEDED`, answers a `Q-###`, changes or confirms an earlier decision, or states a fact about the project ("the tech stack is X").

Principle: the decision log is the single source of truth. A decision is finished only when **every** document, task and test agrees with it.

## Who may do what

| Action | Human | Agent |
| --- | --- | --- |
| Decide | Yes | **No** — agents consult: present uncertainty, options, recommendation |
| Write the entry as `Accepted` | Yes | **Yes, but only for a decision a human approves**: one they made in the conversation (cite who, when, their words), or one the agent's pull request lists under "Decisions for review" (see below) |
| Write a `Proposed` entry | Yes | Yes, while waiting for the human's answer in the conversation (binds nobody) |
| Edit course documents (`senior-project/docs/`) | Yes | No (S-3) — list them for humans |

If you are not sure the human actually *decided* (vs. "maybe", "I think", "probably"), ask once more: "Shall I record this as the decision?"

## Proposed or Accepted?

DEC-052. Every decision needs a human's approval; the status only says **where** that approval happens. The pull request review is an approval step like the conversation: the reviewer treats every new entry in a PR as proposed, and asks for a change or rejects it like any other part of the diff. Writing the entry `Accepted` on the branch means that when the reviewer agrees, merging the PR is enough, with no extra commit to flip `Proposed` to `Accepted`. **It does not let an agent decide.** Whatever is open still goes to the human (P-3, P-4); nothing an agent chose binds anyone before a human merges it.

| Situation | Status |
| --- | --- |
| You asked in the conversation (`DECISION NEEDED`, `Q-###`) and the human has not answered yet | `Proposed` |
| The human answered in the conversation | `Accepted` (with who, when, their words) |
| The decision is part of your pull request, and the PR body lists it under "Decisions for review" | `Accepted` (binds when a human merges the PR; the reviewer reads it as proposed) |
| A reviewer rejects it, or asks for a different choice | Change or remove the entry on the branch in a new commit (it is not on `main` yet, so this is not a rewrite of history). Add a `Rejected` entry only when the reason is worth keeping so nobody raises it again |
| The PR is opened while a question is still open in the conversation | `Proposed`, and the PR body says so; the human answers before the merge, or merges knowing it stays `Proposed` |

Rules for the review route:

- List every new or changed entry under **Decisions for review** in the PR body, one line each, and name them in your reply to the human. An entry the PR does not list is not approved by the merge.
- Put a real choice between options to the human in the conversation first (`DECISION NEEDED`) when you cannot continue without it, or when it reaches beyond the PR (architecture, scope, a new dependency, S-5). The review route is for decisions the change itself carries, such as how an interface is named or how a module reads an SRS rule.
- Fill `Decided by` honestly: the human who chose it in the conversation, or "the PR reviewer" when the PR is the first time a human sees it.

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
[ ] Review-route entries listed under "Decisions for review" in the PR
[ ] Conflicting/old entries superseded
[ ] All project docs/state/tasks updated (list in "Docs updated")
[ ] Old term search shows only history mentions
[ ] Q-### closed, follow-ups created
[ ] Course documents listed for humans (not edited)
[ ] Human told
```
