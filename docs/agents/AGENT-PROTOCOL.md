# AGENT-PROTOCOL — How to Approach the Project Every Time

Follow this procedure literally, in order. It is deliberately boring: the goal is that any model, with no memory of earlier sessions, behaves the same way and leaves the project in a state the next model can pick up.

Certainty tags used below: `[VERIFIED]` `[ASSUMPTION]` `[TBD]` `[PROPOSED]`.

---

## The Truth Rule (read this first)

1. **Humans decide. Agents consult.** A decision is binding only when a **human** has made it, after the agent surfaced the uncertainty and options, and it is recorded in `history/decision-log.md` as `Accepted`.
2. **The `Accepted` entries in the decision log are the single source of truth** for what the project does, uses and believes. Where any other document disagrees — architecture, design, guidelines, state files, tests, code comments, **and the course documents** — the decision log wins.
3. **Course documents are not authoritative.** The `team1-sp/docs` repository (Initial Plan, SRS, SDD) exists for the course and is rebuilt from this repository when a deliverable is submitted. The requirements (`srs.fr.*`, `srs.nfr.*`, epics and stories) are owned by [`docs/requirements/`](../requirements/README.md) here (DEC-152.2); the older `ip.fr.*` ids are a reference snapshot. If a requirement conflicts with an `Accepted` decision, the decision wins and the requirement is changed here, in the same PR, with a decision a human approves.
4. **Uncertainty is never resolved silently.** If you are unsure about something that matters (scope, technology, behaviour, priority, a conflict between documents, an unclear instruction), you MUST tell the human clearly — see "Raising uncertainty" below — and wait for their decision before building on it.
5. **A human decision must propagate everywhere.** Once a decision is accepted, every document, state file, task and test that depended on the old belief is updated to match — see [playbooks/record-decision.md](playbooks/record-decision.md). A decision that is only in the log, while other docs still say otherwise, is not finished.

---

## Raising uncertainty (how to ask)

When you hit anything uncertain that affects the work:

1. **Stop building on it.** Do not guess, do not pick "the likely answer", do not tag `[ASSUMPTION]` and continue silently.
2. **Say it plainly to the human in your reply** (not buried in a doc). Use this shape:

```
DECISION NEEDED — <one-line topic>
What I am unsure about: <1–2 sentences>
Why it matters: <what depends on it>
Options:
  A) <option> — pros / cons
  B) <option> — pros / cons
My recommendation (optional): <A/B + reason>. The decision is yours.
What I will do meanwhile: <unblocked work, or "nothing; blocked">
```

3. **Also record it** in `state/open-questions.md` (`Q-###`, `To: human:<name>` — the project manager is Işık Dönger unless the topic belongs to someone else) so it survives if the human is away or the chat ends. Mark the dependent task `Blocked` and move to other work.
4. **When the human answers** (in chat, in the file, or via another agent's handoff): follow [playbooks/record-decision.md](playbooks/record-decision.md) immediately — log it as `Accepted`, update every affected document, close the question.

An unattended session cannot ask live: do the preparatory work, write the question and the options, note on the issue that it is blocked, and stop that line of work.

If you are only *reasonably certain* (e.g. a small implementation detail inside an accepted design that no document covers), you may proceed — but state the choice in the task result and handoff so a human can overrule it. Anything touching architecture, scope, technology, requirements or process is never "small".

---

## Phase 0 — Boot (read before touching anything)

1. Read [README.md](README.md), [rules.md](rules.md), then `state/current-status.md`, the last ~5 entries of `state/handoff-log.md`, and `state/open-questions.md`.
2. Note the `last_updated` of each state file. If `current-status.md` is more than 7 days old, treat it as possibly stale and verify against the actual repo (file listing, `git log` if available) before trusting it.
3. Read the **Active table of `history/decision-index.md`** — this is the current truth, one line per decision. Anything `Accepted` binds you. Open a full entry in `history/decision-log.md` by its id only when you need its reasons, scope or exact wording (DEC-152.1). Do not read the archives at boot ([README.md](README.md#reading-tiers-dec-1521)).
4. Identify your **agent ID** (`<tool>/<model>/<short-session-id>`).
5. Find your task:
   - A human gave you a task → find its GitHub issue (a story `US-x.y`, a bug, a chore). Work without an issue is a `docs`/`ci`/`chore` PR with the `no-issue` label ([git-workflow.md](../guidelines/git-workflow.md)).
   - No instructions → ask the human; the [GitHub project](https://github.com/orgs/team1-sp/projects/3) shows what is planned and who has it. Never take an issue that has another assignee or an open branch (unless the human hands it over).
5a. **Pick where to work** (DEC-061, [git-workflow.md §1](../guidelines/git-workflow.md)): code and repository docs in your own cloud copy of the repository by default; a member's computer (folder access to edit files, Remote Control to run commands) only when the files, tools or signed-in accounts exist only there, or the member asks.
6. **Claim** the issue *before* starting work (DEC-149.1): the issue's assignee is the member you work for, and the work branch exists. Creating the branch moves the card to In progress (Issue start workflow). There is no task board and no `TASK` id.
6a. **Record ids** (DEC-148.1): every entry you add (DEC, HND, PRB, Q, GEN) is `<PREFIX>-<GitHub #>.<counter>`, where the number is the issue your PR closes, or the PR's own number for no-issue work: `HND-80.1`, `DEC-80.1`, `DEC-80.2`. Do not take "highest id + 1"; old 3-digit ids stay. CI rejects a new id with another number ([documentation-rules.md §4](../guidelines/documentation-rules.md#4-entry-ids)).

## Phase 1 — Understand (read-only)

7. Read the docs relevant to the task: the architecture doc for the layer you touch, the design doc for the component, `testing/` for how it will be verified.
8. **Search the memory before inventing anything:**
   - `history/decision-index.md` — search for the component / technology name, then open the matching entries in `history/decision-log.md`. If an `Accepted` decision exists, follow it. If you believe it is wrong, see "Disagreeing with a decision".
   - `history/problems/` — only when you hit a problem: search the area file (and the index in its README) for the error text, the component, the symptom. If a solution exists, apply it first.
   - `troubleshooting/common-errors.md` — anticipated pitfalls.
9. **Detect conflicts.** If a document contradicts an `Accepted` decision (including course documents), the decision wins. Do not "fix" course documents (S-3); list them for the human (see `record-decision.md`, "Course documents drift"). Fix project docs in the same task.
10. Read the requirements the task serves: your story's section in its epic file under [`docs/requirements/functional/`](../requirements/README.md) (only that file). If a requirement looks wrong or conflicts with a decision, that is uncertainty → raise it; to add, edit, remove or move a requirement, follow [playbooks/change-requirements.md](playbooks/change-requirements.md): placeholder ids (`srs.fr.<n>.new`) for new ones, delete the lines of removed ones, then run `tools/renumber_requirements.py`. Never renumber by hand.
11. Anything uncertain or only a human can answer → **Raising uncertainty** (above). Do not guess.

## Phase 2 — Plan (write it down)

12. Write a short plan inside the task entry: goal, files you expect to touch, how you will verify, risks. 3–8 bullets. A plan that is not written down does not exist for the next agent.
13. Check the plan against [rules.md](rules.md) invariants (layer boundaries, ECS as data hub, adapter, fixed timestep, headless). If the plan violates one → stop, go to "Disagreeing with a decision".
14. Prefer the **smallest vertical slice** that can be verified. One task = one reviewable change.
14a. **Check for overlapping work:** list the open PRs and the files they touch (`gh pr list --state open --json number,title,files`). If one changes the same code or design docs as your plan, tell the human before you start ([git-workflow.md §4](../guidelines/git-workflow.md#4-merge-conflicts), DEC-055).

## Phase 3 — Do

15. Make the change following [guidelines/coding-standards.md](../guidelines/coding-standards.md) and [guidelines/repository-structure.md](../guidelines/repository-structure.md).
16. Write or update tests in the same task ([playbooks/write-tests.md](playbooks/write-tests.md)). Verify the *chain*, not only the end result: Input → ECS → Physics → ECS → Rendering — each link observed separately.
17. Run the tests / the app. Record the exact command and the result. If you cannot run code in your environment, say so explicitly in your handoff — **never claim a test passed that you did not run.**
18. Stuck? Use the **two-attempt rule**: after two failed attempts with the same approach, stop, write what you tried and observed in the task, check `history/problems/` again, then either change approach (once) or raise it with the human. Do not loop.

## Phase 4 — Record (mandatory, even if the work failed)

19. **Problem hit and solved?** → append an entry to its area file in `history/problems/`, with an index row in its README, using [templates/problem-entry.md](templates/problem-entry.md). Include the exact error text so future searches find it.
20. **Human made a decision this session?** → [playbooks/record-decision.md](playbooks/record-decision.md): log it `Accepted` (with who decided, when, and their words), propagate to all documents.
21. **You think a decision is needed but the human has not decided?** → put the `DECISION NEEDED` block in your reply, log it `Proposed` (so it is visible) and add a `Q-###` in `open-questions.md`. A `Proposed` entry binds nobody. When the human answers, it becomes `Accepted`.
21a. **Your change carries a decision nobody had to answer first** (a name, a layout, how a module reads an SRS rule)? → log it `Accepted` and list it under "Decisions for review" in the PR body. The reviewer approves or rejects it with the rest of the diff, and it binds only once a human merges the PR (DEC-052). This saves a follow-up commit; it is not permission to decide open choices yourself (P-3, P-4).
22. **AI-generated text for a course deliverable?** → append to `history/genai-usage-log.md` (see [guidelines/genai-usage-rules.md](../guidelines/genai-usage-rules.md)).
23. **Docs affected?** Update the architecture/design/testing doc that your change made wrong. A change that makes a doc lie is not done.
23a. **Commit, push, PR** ([git-workflow.md](../guidelines/git-workflow.md), DEC-053 to DEC-055, DEC-058, DEC-061): working on a member's own computer, show them the change and commit and push only after they approve it; working in your own cloud copy of the repository, commit and push without asking. Merge `origin/main` into the branch, build and test again, then push. After the first push, open the draft PR with `python tools/open_pr.py`; in a cloud session it fails (GraphQL is blocked), so open it with the GitHub `create_pull_request` tool and set labels, milestone and assignee with `gh api -X PATCH` (DEC-061). Merge `main` again before marking it ready for review, and whenever the PR gets the `needs-sync` label.
24. Report the result in the PR body and, if the work is blocked, in a comment on the issue plus a `Q-…` entry. The card's status follows the branch, the PR and the merge.
25. Edit `state/current-status.md` only if your change makes something on it wrong (a new module or library, a new trap, a phase change, a health check you re-ran). Per-PR progress goes in your handoff entry, not there (DEC-149.1).

## Phase 5 — Hand off

26. Append a handoff entry to `state/handoff-log.md` using [templates/handoff.md](templates/handoff.md): what you did, what you verified (and how), what you did *not* verify, what to do next, traps, and any decisions awaiting a human.
27. If you stop before finishing, say so in the handoff and on the issue so a human can hand it on.
28. Final message to the human (if any): result in 2–4 sentences, files changed, and every `DECISION NEEDED` still open. No narration of steps.

---

## Disagreeing with a decision

You may not silently deviate from an `Accepted` decision, and you may not "fix" an architecture invariant because it is inconvenient. But if you believe a decision is wrong, outdated or blocking, you MUST say so — that is part of consulting.

1. Stop work that depends on it.
2. Tell the human using the `DECISION NEEDED` block: which `DEC-###`, what is wrong, evidence, the alternative, the cost of switching.
3. Record it as a `Proposed` entry referencing the original, plus a `Q-###` addressed to the human.
4. Continue with unrelated tasks. Until the human decides, the existing `Accepted` decision still binds.
5. If the human decides to change it: the new entry becomes `Accepted`, the old one `Superseded by DEC-###`, and **everything is updated to the new decision** (`record-decision.md`). If the human keeps the old decision, log that outcome (the `Proposed` entry becomes `Rejected`, with their reason) so nobody re-raises it blindly.

## Working under uncertainty

| Situation | Do |
| --- | --- |
| Library/API behaviour not in the repo | Check the dependency's pinned version and its docs. Verify by running code. If it cannot be verified and matters → raise it. |
| Requirement is ambiguous or looks outdated | Raise it (`DECISION NEEDED`). Course requirements are a reference snapshot, not truth. |
| Two documents contradict each other | The `Accepted` decision log wins over everything. Between non-decision documents: ask the human; do not pick. Fix project docs once the human answers; list course documents for the human to update. |
| A topic has no decision yet (marked `[TBD]` or simply undocumented) | Do not choose. Write against the abstraction (adapter / interface) if that is genuinely neutral; otherwise raise it and block. |
| You are about to touch something outside your task | Don't. Add a new task instead. |
| The human's instruction conflicts with an `Accepted` decision | Point out the conflict, ask whether they are changing the decision. If yes → record-decision playbook. |

## Quality bar (self-check before Phase 4)

- [ ] Does it run? (command + result recorded)
- [ ] Does a test fail without my change and pass with it?
- [ ] Did I respect layer dependencies (Engine ← Semantic ← AI, never the reverse)?
- [ ] Did I avoid direct physics-library / graphics-framework calls outside the adapter / render abstraction?
- [ ] Is simulation behaviour identical in headless and normal mode?
- [ ] Is the doc that describes this still true — and does nothing contradict the decision log?
- [ ] Did I raise every real uncertainty instead of resolving it myself?
- [ ] Could the next agent continue using only the handoff entry and the task?

## Anti-patterns (things models do that hurt this project)

- Resolving an uncertainty by guessing, then burying the guess in code or a doc.
- Treating a course document (Initial Plan, guide) as more authoritative than the decision log.
- Writing `Accepted` for something the human did not actually decide.
- Logging a decision but leaving other docs saying the old thing.
- Rewriting large files "to clean up" while doing a small task.
- Inventing library functions or versions instead of checking.
- Reporting success based on code that "should work".
- Putting gameplay decisions in physics code, or physics/rendering calls into each other (they talk only through ECS).
- Leaving state files stale. A stale `current-status.md` is worse than none.
- Deleting or editing past entries in `history/`.
