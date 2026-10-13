# Git Workflow

Status: `[PROPOSED]` — confirm with the team (Q-006). The repo keeps contribution history per member (course requirement shown in the docs repo README), so **commit authorship matters**.

## 1. Branching

| Branch | Purpose | Who merges |
| --- | --- | --- |
| `main` | Always builds; tests pass once code exists | Humans only |
| `task/TASK-###-short-name` | One task, one branch | Author opens PR; a human reviews and merges |
| `docs/...` | Docs-only changes | Same |

Agents MAY commit to `task/*` and `docs/*` branches if the human allows it (Q-006). Agents MUST NOT push to `main`, force-push, or rewrite history (S-2).

## 2. Commits

- Small, one logical change each.
- Message format:

```
<area>: <imperative summary, ≤ 72 chars>

<why, not what — optional body>

Refs: TASK-###, DEC-###
```

  `area` ∈ `ecs`, `physics`, `adapter`, `render`, `input`, `gameplay`, `telemetry`, `headless`, `semantic`, `ai`, `build`, `docs`, `test`.
- When an AI agent made the commit, keep the tool's co-author trailer if it adds one. Do not remove other people's attribution.
- Never commit: secrets, large binaries, build output, personal paths.

## 3. Pull requests

- Link the task; describe what/why; list how it was verified (commands); list docs updated.
- Checklist from `process/definition-of-done.md`.
- At least one reviewer other than the author (a different agent or a human) when possible (`process/code-review.md`).

## 4. Merge conflicts

- State files (`agents/state/*`, `history/*`): keep **both** sides' entries; re-number IDs if they collide.
- Code: the author of the later branch resolves; re-run tests afterwards.

## 5. Tags / releases

Increment deliverables (first increment, final) get an annotated tag `inc-1`, `final` `[TBD names]`, aligned with the sprint review.

## 6. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Proposed | claude-code/sonnet-5.5/012r |
