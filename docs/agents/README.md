# agents/ — Agent-Only Documentation

This folder exists so that **different AI agents and different models** — Claude, GPT, Gemini, local models, in any tool (IDE assistants, CLI agents, chat) — can work on the same project without a human relaying context between them.

The documents here are the *shared memory and message bus*. If it is not written here (or in `history/`), the next agent does not know it.

## The one rule above all others

**Humans decide; agents consult.** Anything uncertain is put to the human clearly (`DECISION NEEDED`, see `AGENT-PROTOCOL.md`). The human's decision is logged `Accepted` in `history/decision-log.md` and becomes the single truth; every other document is then updated to match (`playbooks/record-decision.md`). Course documents under `senior-project/docs/` are not authoritative.

## Reading order (every session, no exceptions)

1. [AGENT-PROTOCOL.md](AGENT-PROTOCOL.md) — the procedure. Follow it literally.
2. [rules.md](rules.md) — hard rules and architecture invariants.
3. [state/current-status.md](state/current-status.md) — where the project is right now.
4. [state/task-board.md](state/task-board.md) — what is claimed, ready, blocked, done.
5. [state/handoff-log.md](state/handoff-log.md) — the last ~5 entries, newest at the bottom.
6. [state/open-questions.md](state/open-questions.md) — things waiting for a human.
7. The index of [../history/decision-log.md](../history/decision-log.md) — the current truth.
8. Then only the architecture/design/testing docs relevant to your task.

## Contents

| File | Purpose | Written by |
| --- | --- | --- |
| [AGENT-PROTOCOL.md](AGENT-PROTOCOL.md) | Start-of-session, work loop, end-of-session procedure | maintainers |
| [rules.md](rules.md) | MUST / MUST NOT rules and invariants | maintainers |
| [communication-protocol.md](communication-protocol.md) | File-based messaging, claims, handoffs, conflicts, IDs | maintainers |
| [model-compatibility.md](model-compatibility.md) | Writing/reading conventions that work for any model | maintainers |
| [state/current-status.md](state/current-status.md) | Snapshot of progress | any agent, every session |
| [state/plan.md](state/plan.md) | Live plan: current phase, next steps | any agent / human |
| [state/task-board.md](state/task-board.md) | Tasks, owners, status | any agent / human |
| [state/handoff-log.md](state/handoff-log.md) | Append-only session handoffs | any agent, every session |
| [state/open-questions.md](state/open-questions.md) | Questions for humans / other agents | any agent / human |
| [playbooks/](playbooks/) | Approach per kind of work, incl. [record-decision](playbooks/record-decision.md) (human decision → log → update every doc) | maintainers |
| [templates/](templates/) | Entry formats to copy | maintainers |

## Two meanings of "agent" — do not confuse them

| Term | Meaning | Where described |
| --- | --- | --- |
| **Development agent** | An AI model helping *build* the product (you, reading this) | this folder |
| **Product agent** | A component *inside* the product: the **Automated Playtest Agent** (ip.fr.9.1) and the **AI Assistant Layer** (ip.fr.11.*) | `architecture/` |

Rules for product agents (e.g. "the AI Assistant shall not modify engine state", ip.fr.11.6) are product requirements. Rules for development agents are in [rules.md](rules.md).

## Agent identity

Sign every entry with an agent ID: `<tool>/<model>/<short-session-id>`, e.g. `claude-code/sonnet-5.5/a1b2`, `copilot/gpt/7f3c`, `human/isik`. If you do not know your model name, write `unknown-model`. Never impersonate a human or another agent. Project manager: Işık Dönger; address project-level questions to `human:isik` unless the topic clearly belongs to another member.
