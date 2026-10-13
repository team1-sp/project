# Documentation Rules

## 1. Principles

1. **Docs are part of the product.** A change is not done if it leaves a doc wrong (P-9).
2. **One source of truth per fact.** Link instead of copying. **Decisions live in `history/decision-log.md` and are the truth** (humans decide, DEC-017); state in `agents/state/`. The Initial Plan's `ip.fr.*` IDs are a reference snapshot, not truth.
3. **Write for the next reader who has no context** — human or model.
4. **Tell the truth about status:** `[VERIFIED] [ASSUMPTION] [TBD] [PROPOSED]`. Never document planned behaviour as if it exists.

## 2. Format

- Markdown, UTF-8, LF line endings preferred.
- One `# H1` per file (the title). Headings in order.
- Tables for comparisons, fenced code blocks for commands/diagrams/schemas, ASCII diagrams (they work everywhere).
- Relative links (`../design/ecs-design.md`). Check them when moving files.
- Files < ~400 lines. Split by topic.
- English. Terms from the glossary.
- No emojis, no marketing tone.

## 3. Where things go

| Content | Location |
| --- | --- |
| What the system is | `architecture/` |
| How a part is designed | `design/` (use template) |
| Rules for writing code/git/docs | `guidelines/` |
| How the team works | `process/` |
| How we verify | `testing/` |
| How we fix | `troubleshooting/` |
| What happened / why we chose | `history/` (append-only) |
| Agent coordination | `agents/` |
| Course deliverables | **not here** (`senior-project/docs/`); not authoritative, not edited by agents |

## 4. Entry IDs

`TASK-###`, `DEC-###`, `PRB-###`, `HND-###`, `Q-###`, `GEN-###`. Zero-padded to 3 digits, sequential, never reused, never renumbered after publication (except the collision rule in `communication-protocol.md`).

## 5. Change tables

Docs that are edited over time (architecture, design, guidelines, process, testing) end with a `Change table` (date, change, author). Append-only logs don't need one.

## 6. Ownership and precedence

See the ownership table in `docs/README.md`. When documents disagree: **`Accepted` decision log > everything else.** After a human decision, all project documents are updated to match (`agents/playbooks/record-decision.md`). Agents never change human-owned content on their own judgement: they ask, then record the human's decision.

## 7. Review checklist for doc changes

- [ ] Accurate against the repo/code today and consistent with the decision log?
- [ ] Links work?
- [ ] Glossary terms?
- [ ] Certainty tags where unsure?
- [ ] Change table updated?
- [ ] Any state/index file that lists it updated (README folder map)?

## 8. Relationship to the course documents

The Initial Plan, SRS, SDD, etc. are graded deliverables with their own rules (see `genai-usage-rules.md`). Content may be *derived* from this folder only by humans (or with GenAI disclosure). Do not paste text from here into a deliverable without following the disclosure rules.
