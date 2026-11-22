# Template — Handoff entry

Append to the bottom of `state/handoff-log.md`. Max ~25 lines. Link IDs instead of copying content. The id is `HND-<n>.<k>`: `<n>` is the issue your PR closes (or the PR's number for no-issue work), `<k>` counts this work's handoffs from 1 (DEC-148.1).

```markdown
### HND-<n>.<k> — Short title
- **Date:** YYYY-MM-DD
- **Agent:** <tool>/<model>/<session-id>
- **Task:** TASK-<issue #> or the issue/PR number
- **Goal:** what you set out to do.
- **Result:** Done | Partial | Blocked | Failed — one sentence.
- **Files changed:** list (or "none").
- **Verified:** exact commands run and what they showed. (Write "nothing executed" if so.)
- **Not verified:** what you could not or did not check.
- **Repo state:** builds? tests passing? uncommitted changes? branch name?
- **Next:** numbered, concrete steps for the next agent.
- **Traps:** surprises, wrong turns, things that look right but are not.
- **Records:** PRB-###, DEC-###, Q-### created in this session.

---
```

Good handoff test: a different model, with no memory, reads only this entry and the task, and can continue without asking you anything.
