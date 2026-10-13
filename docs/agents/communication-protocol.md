# Agent Communication Protocol

Agents never talk to each other directly. They communicate **only through files in this repository**. This document defines the channels, formats and conflict rules so that Model A can leave a message that Model B (different vendor, no shared memory) can act on.

## 1. Channels

| Need | Channel | Writing style |
| --- | --- | --- |
| "Where are we?" | `state/current-status.md` | Overwrite (snapshot). Keep `last_updated`, `updated_by`. |
| "What should be done, and by whom?" | `state/task-board.md` | Edit your own task rows/entries. |
| "What did the last agent do, what must I know?" | `state/handoff-log.md` | **Append-only**, newest at the bottom. |
| "I need a human / another agent to answer" | `state/open-questions.md` | Add `Q-###` (and also say it to the human in your reply — never only in the file); answer by filling `Answer` + status `Answered`. |
| "A human decided something" | `history/decision-log.md` | Log `Accepted`, then propagate to every document: [playbooks/record-decision.md](playbooks/record-decision.md). |
| "What is the plan?" | `state/plan.md` | Edit; log big changes in the change table. |
| "What did we decide and why?" | `history/decision-log.md` | **Append-only. The single source of truth** (DEC-017). |
| "Has this problem happened before?" | `history/problem-log.md` | **Append-only.** |
| "Project knowledge" | `architecture/`, `design/`, `testing/`… | Edit per ownership table in docs README. |

## 2. Identity

Every entry is signed with an agent ID: `<tool>/<model>/<short-session-id>` (e.g. `cursor/gpt/3e91`, `claude-code/sonnet-5.5/a1b2`, `gemini-cli/pro/77aa`, `human/arda-s`). Roles are *hats*, not identities — one agent can wear several:

| Role | Responsibility |
| --- | --- |
| Planner | Breaks goals into tasks, keeps `plan.md` and the board honest |
| Implementer | Builds a claimed task |
| Tester | Writes/runs tests, reproduces bugs |
| Reviewer | Reads another agent's finished task against `definition-of-done.md` |
| Doc-keeper | Makes docs match reality, prunes stale state |

Rule: an agent MUST NOT be the only reviewer of its own task when another agent or human is available. If none is, mark the task `Review (self)` and say so in the handoff.

## 3. Task lifecycle and claims

Statuses: `Backlog` → `Ready` → `Claimed` → `Review` → `Done`; side states `Blocked`, `Dropped`.

```
Backlog ──(has acceptance criteria + no open human decision it depends on)──▶ Ready
Ready   ──(agent writes claim)──▶ Claimed
Claimed ──(work finished, tests run)──▶ Review
Review  ──(reviewer / human OK)──▶ Done
Claimed ──(needs answer)──▶ Blocked ──(answered)──▶ Ready
```

**Claim format** (in the task entry): `Claimed by: <agent-id> on <YYYY-MM-DD>`.
**Stale claim:** a claim with no related handoff/commit activity for **3 days** (agent sessions are short-lived) may be taken over. The taker MUST add a handoff entry saying "took over stale claim from X" and read X's last handoff first.
**Never** work on a task claimed by someone else; add a comment in the task or a question instead.

## 4. Handoff entries

Every session that did *anything* ends with a handoff entry (template: [templates/handoff.md](templates/handoff.md)). It answers six questions for the next reader:

1. What was the goal and what is the result?
2. What changed (files)?
3. What was **verified**, and how (exact commands)? What was **not** verified?
4. What is the state of the repo (builds? tests?)?
5. What should happen next?
6. What traps or surprises did you find?

Keep it under ~25 lines. Link to task/decision/problem IDs instead of copying their content.

## 5. Addressing messages

To ask a specific party something, write it in `open-questions.md` with `To:` set to one of:

- `human:<name>` (e.g. `human:arda-s`) — decisions, access, priorities
- `human:any` — anyone on the team
- `agent:any` — any agent (e.g. "please review TASK-012")
- `agent:<role>` — e.g. `agent:tester`

Humans and agents resolve questions by changing status to `Answered` and filling the answer. Whoever sees a human's answer turns it **immediately** into an `Accepted` decision-log entry if it is a decision, and propagates it (`playbooks/record-decision.md`). Agents never answer decision questions themselves.

## 6. Concurrency and conflicts

Several agents may work at the same time in different tools. There is no lock server; use these habits:

1. **Re-read just before writing** any shared state file. If it changed since you first read it, merge — do not overwrite.
2. **Append-only files:** append at the end; never reorder.
3. **IDs:** take the next number by reading the highest existing ID in the file right before writing. If two agents collide on an ID, the later writer renumbers *its own* entry and fixes its references.
4. **One task = one owner.** Different tasks should touch different files. If two tasks must touch the same file, note it in both tasks (`Touches:`) and sequence them.
5. **Git:** work on a branch named after the task (`task/TASK-012-short-name`, see `guidelines/git-workflow.md`). Resolve merge conflicts in state files by keeping *both* sides' entries.
6. **If state files disagree with the repo**, trust the repo, then fix the state file and note it in your handoff.

## 7. Entry formats

All logs use a header per entry so they can be searched with plain text tools:

```
### PRB-004 — Short title
```

Fields are `**Field:** value` lines. Tags go on a `**Keywords:**` line (comma-separated, include exact error strings and component names). See [templates/](templates/).

## 8. Front matter for live state files

```
---
last_updated: YYYY-MM-DD
updated_by: <agent-id>
---
```

Update both whenever you change the file.

## 9. Escalation ladder

1. Look in `problem-log` / `common-errors` / `decision-log`.
2. Try one alternative approach (two-attempt rule).
3. Tell the human (`DECISION NEEDED`), write a `Q-###` addressed to the right party and mark the task `Blocked`.
4. Move to a different `Ready` task.
5. Never "wait" inside a session; the board and the question are the waiting mechanism.
