# Template — Task entry

Copy into `state/task-board.md` (summary row **and** entry). Next ID = highest existing + 1.

```markdown
### TASK-### — Short imperative title
- **Status:** Backlog | Ready | Claimed | Review | Done | Blocked | Dropped
- **Claimed by:** <agent-id> on YYYY-MM-DD        (omit until claimed)
- **Serves:** ip.fr.x.y, ip.nfr.x.y, DEC-###     (requirement / decision IDs)
- **Depends on:** TASK-###, Q-###, DEC-###
- **Touches:** paths or modules (for conflict avoidance)
- **Goal:** one or two sentences.
- **Acceptance criteria:**
  - [ ] observable, testable condition
  - [ ] ...
- **Plan:** (written by the claimer before starting)
  - step
- **Verification:** exact commands / manual steps that prove it.
- **Result:** (written at the end; what changed, what was verified, what was not)
- **Links:** PRB-###, DEC-###, HND-###
```

Rules:
- Acceptance criteria must be checkable by someone who did not do the work.
- If you cannot write acceptance criteria, the task is not `Ready`.
- Keep a task to ≤ 1 day of agent work; split otherwise.
