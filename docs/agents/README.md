# agents/ — Agent-Only Documentation

This folder exists so that **different AI agents and different models** — Claude, GPT, Gemini, local models, in any tool (IDE assistants, CLI agents, chat) — can work on the same project without a human relaying context between them.

The documents here are the *shared memory and message bus*. If it is not written here (or in `history/`), the next agent does not know it.

## The one rule above all others

**Humans decide; agents consult.** Anything uncertain is put to the human clearly (`DECISION NEEDED`, see `AGENT-PROTOCOL.md`). The human's decision is logged `Accepted` in `history/decision-log.md` and becomes the single truth; every other document is then updated to match (`playbooks/record-decision.md`). Course documents under `senior-project/docs/` are not authoritative.

## Reading order (every session, no exceptions)

1. [AGENT-PROTOCOL.md](AGENT-PROTOCOL.md) — the procedure. Follow it literally.
2. [rules.md](rules.md) — hard rules and architecture invariants.
3. [state/current-status.md](state/current-status.md) — where the project is right now.
4. [state/handoff-log.md](state/handoff-log.md) — the last ~5 entries, newest at the bottom.
5. [state/open-questions.md](state/open-questions.md) — things waiting for a human.
6. [../history/decision-index.md](../history/decision-index.md), the **Active** table — the current truth, one line per decision.
7. Your GitHub issue; what is planned and who has it is on the [GitHub project](https://github.com/orgs/team1-sp/projects/3) (DEC-149.1).
8. Then only the architecture/design/testing docs relevant to your task.

## Reading tiers (DEC-152.1)

Every session pays for what it reads, so the docs are split into three tiers. Read a file only in its tier's situation.

| Tier | When an agent reads it | Files |
| --- | --- | --- |
| **Boot** | Every session, steps 1 to 6 above. Kept small: `tools/check_boot.py` holds these files (without the handoff log) under 50 KB, and `tools/check_ids.py` keeps the live handoff log at 15 entries or fewer. | this README, `AGENT-PROTOCOL.md`, `rules.md`, `state/current-status.md`, `state/handoff-log.md` (last ~5 entries), `state/open-questions.md`, `history/decision-index.md` |
| **On demand** | When the task points to it | a decision's full entry in `history/decision-log.md` (to trace it back: reasons, scope, wording); `history/problems/` (when a problem occurs, before debugging); `architecture/`, `design/`, `testing/`, `guidelines/`, `troubleshooting/`; playbooks and templates for the kind of work |
| **Human-only** | Never, unless a human asks or you trace something back that the other tiers do not explain | `history/handoff-archive/`, `history/questions-archive.md`, `history/genai-usage-log.md`, `history/adr-*.md`, `research/`, `process/roadmap.md`. Each Markdown file says so in a line under its title. |

Keep the boot files small: closed questions, old handoffs and superseded decisions move to their archives, details stay in the full entries, and the index row of a decision is one line.

## Contents

| File | Purpose | Written by |
| --- | --- | --- |
| [AGENT-PROTOCOL.md](AGENT-PROTOCOL.md) | Start-of-session, work loop, end-of-session procedure | maintainers |
| [rules.md](rules.md) | MUST / MUST NOT rules and invariants | maintainers |
| [communication-protocol.md](communication-protocol.md) | File-based messaging, claims, handoffs, conflicts, IDs | maintainers |
| [model-compatibility.md](model-compatibility.md) | Writing/reading conventions that work for any model | maintainers |
| [state/current-status.md](state/current-status.md) | Increment-level snapshot: phase, what exists, health, traps | any agent, only when something on it changes |
| [state/handoff-log.md](state/handoff-log.md) | Append-only session handoffs; only the newest entries (`tools/rotate_handoffs.py`) | any agent, every session |
| [state/open-questions.md](state/open-questions.md) | Open questions for humans / other agents; answered ones move to `history/questions-archive.md` | any agent / human |
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
