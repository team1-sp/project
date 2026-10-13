# Code Review

Applies to humans and agents. Reviewer ≠ author whenever possible. An agent reviewing another agent's work is encouraged (use a different model if available — it catches different mistakes).

## Review order

1. **Read the task** (goal, acceptance criteria) and the diff summary.
2. **Architecture first:** invariants R-1…R-9. A violation blocks the review regardless of how well it works.
3. **Correctness:** does it meet each acceptance criterion? Edge cases: empty queries, entity destroyed mid-frame, zero velocity, two collisions in one step, first frame.
4. **Determinism and headless safety:** no wall-clock, no render dependence, iteration order stable.
5. **Tests:** meaningful, fail when behaviour breaks, run headless, mapped to requirements.
6. **Telemetry/observability:** new state/events visible? cost acceptable?
7. **Readability:** names, size, comments explain why, follows `coding-standards.md`.
8. **Docs/records:** updated? problem/decision entries present?
9. **Dependencies:** any new one has a decision entry.

## How to write feedback

- Label each comment: `blocker`, `should`, `nit`, `question`.
- Be specific: file/line, what is wrong, suggested fix.
- Verify claims by running/inspecting, not only reading.
- Approve only when blockers are resolved. Record the outcome in the task (`Review: approved by <id> on <date>` or `changes requested`).

## Review checklist (copy into the task)

```
Reviewer: <id>   Date: YYYY-MM-DD
[ ] Invariants respected (R-1..R-9)
[ ] Acceptance criteria met (each verified how?)
[ ] Tests run by reviewer: <command> → <result>
[ ] Determinism/headless OK
[ ] Telemetry impact OK
[ ] Docs and records updated
[ ] No scope creep
Verdict: approved | changes requested
```

## Agents reviewing agents

- Do not trust the author's handoff "Verified" section: re-run at least the main command.
- If you cannot run code, say so and limit the review to static checks; do not approve as "tested".
