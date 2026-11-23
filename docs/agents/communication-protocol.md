# Agent Communication Protocol

Agents never talk to each other directly. They communicate **only through files in this repository**. This document defines the channels, formats and conflict rules so that Model A can leave a message that Model B (different vendor, no shared memory) can act on.

## 1. Channels

| Need | Channel | Writing style |
| --- | --- | --- |
| "Where are we?" | `state/current-status.md` | Increment-level snapshot. Edit only what your change makes wrong; per-PR progress goes in the handoff (DEC-149.1). |
| "What should be done, and by whom?" | the [GitHub project](https://github.com/orgs/team1-sp/projects/3) and the issue's assignee | Not a repo file (DEC-149.1). The old task board was deleted (DEC-152.1); its history is in git. |
| "What did the last agent do, what must I know?" | `state/handoff-log.md` | **Append-only**, newest at the bottom; older entries are moved to `history/handoff-archive/` (DEC-152.1). |
| "I need a human / another agent to answer" | `state/open-questions.md` | Add `Q-<#>.<n>` (and also say it to the human in your reply — never only in the file); when answered, fill `Answer` + status and move the entry to `history/questions-archive.md`. |
| "A human decided something" | `history/decision-log.md` | Log `Accepted` (approval in the conversation, or in the review of the PR that adds it; DEC-052), then propagate to every document: [playbooks/record-decision.md](playbooks/record-decision.md). |
| "What is the plan?" | the [GitHub project](https://github.com/orgs/team1-sp/projects/3) and `state/current-status.md` | `plan.md` was deleted (DEC-152.1). |
| "What did we decide and why?" | `history/decision-index.md`, then the entry in `history/decision-log.md` | **Append-only. The single source of truth** (DEC-017). Read the index; open an entry only to trace it back (DEC-152.1). |
| "Has this problem happened before?" | `history/problems/` (one file per area) | **Append-only.** Read only when a problem occurs. |
| "Project knowledge" | `architecture/`, `design/`, `testing/`… | Edit per ownership table in docs README. |

## 2. Identity

Every entry is signed with an agent ID: `<tool>/<model>/<short-session-id>` (e.g. `cursor/gpt/3e91`, `claude-code/sonnet-5.5/a1b2`, `gemini-cli/pro/77aa`, `human/arda-s`). Roles are *hats*, not identities — one agent can wear several:

| Role | Responsibility |
| --- | --- |
| Planner | Breaks goals into GitHub issues and keeps the project board honest |
| Implementer | Builds a claimed task |
| Tester | Writes/runs tests, reproduces bugs |
| Reviewer | Reads another agent's finished task against `definition-of-done.md` |
| Doc-keeper | Makes docs match reality, prunes stale state |

Rule: an agent MUST NOT be the only reviewer of its own task when another agent or human is available. If none is, mark the task `Review (self)` and say so in the handoff.

## 3. Task lifecycle and claims

Work is tracked on GitHub, not in a repo file (DEC-149.1). A task is a GitHub issue; its status is the card on the [GitHub project](https://github.com/orgs/team1-sp/projects/3).

- **Claim:** the issue's assignee is the member the agent works for, and the work branch exists (creating it moves the card to In progress). Never work on an issue that has another assignee or an open branch unless a human hands it over.
- **Blocked:** comment on the issue and add a `Q-…` entry; the `dependencies` CI job also blocks a PR whose issue still has open "blocked by" issues.
- **Review / Done:** the draft PR becomes ready for review; merging it closes the issue and moves the card to Done.

Tasks recorded before DEC-149.1 (`TASK-001` to `TASK-025`) are in the git history of `docs/history/task-board-archive.md`, deleted by DEC-152.1.

## 4. Handoff entries

Every session that did *anything* ends with a handoff entry (template: [templates/handoff.md](templates/handoff.md)). It answers six questions for the next reader:

1. What was the goal and what is the result?
2. What changed (files)?
3. What was **verified**, and how (exact commands)? What was **not** verified?
4. What is the state of the repo (builds? tests?)?
5. What should happen next?
6. What traps or surprises did you find?

Keep it under ~25 lines. Link to issue, decision and problem ids instead of copying their content.

## 5. Addressing messages

To ask a specific party something, write it in `open-questions.md` with `To:` set to one of:

- `human:<name>` (e.g. `human:arda-s`) — decisions, access, priorities
- `human:any` — anyone on the team
- `agent:any` — any agent (e.g. "please review #80")
- `agent:<role>` — e.g. `agent:tester`

Humans and agents resolve questions by changing status to `Answered` and filling the answer. Whoever sees a human's answer turns it **immediately** into an `Accepted` decision-log entry if it is a decision, and propagates it (`playbooks/record-decision.md`). Agents never answer decision questions themselves.

## 6. Concurrency and conflicts

Several agents may work at the same time in different tools. There is no lock server; use these habits:

1. **Re-read just before writing** any shared state file. If it changed since you first read it, merge — do not overwrite.
2. **Append-only files:** append at the end; never reorder.
3. **IDs** (DEC-148.1): a new entry's id is `<PREFIX>-<GitHub #>.<counter>`, where the number is the issue your pull request closes, or your pull request's own number for no-issue work: `DEC-80.1`, `HND-80.2`, `PRB-147.1`. Count up from `.1` within your work; look only at your own number's entries, not the highest id in the file. Tasks are GitHub issues and have no `TASK` id (DEC-149.1). Old 3-digit ids stay as they are. Rules: [documentation-rules.md §4](../guidelines/documentation-rules.md#4-entry-ids).
4. **One task = one owner.** Different tasks should touch different files. If two tasks must touch the same file, note it in both tasks (`Touches:`) and sequence them.
5. **Git:** work on a branch named after the backlog item (`feat/pb-026-short-name`, see `guidelines/git-workflow.md`). Resolve merge conflicts in state files by keeping *both* sides' entries.
6. **If state files disagree with the repo**, trust the repo, then fix the state file and note it in your handoff.

## 7. Entry formats

All logs use a header per entry so they can be searched with plain text tools:

```
### PRB-142.1 — Short title
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

1. Look in `problems/` / `common-errors` / `decision-log`.
2. Try one alternative approach (two-attempt rule).
3. Tell the human (`DECISION NEEDED`), write a `Q-###` addressed to the right party and mark the task `Blocked`.
4. Move to a different `Ready` task.
5. Never "wait" inside a session; the board and the question are the waiting mechanism.
