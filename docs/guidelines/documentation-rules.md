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
| What we learned from spikes and prototypes | `research/` |
| How we fix | `troubleshooting/` |
| What happened / why we chose | `history/` (append-only) |
| Agent coordination | `agents/` |
| Course deliverables | **not here** (`senior-project/docs/`); not authoritative, not edited by agents |

## 4. Entry IDs

New entries (DEC-148.1) are `<PREFIX>-<GitHub #>.<counter>`: `DEC-80.1`, `HND-80.2`, `PRB-142.1`, `Q-80.1`, `GEN-80.1`. The GitHub number is the issue the pull request closes (`Closes #80`), or the pull request's own number for `docs`/`ci`/`chore` work with the `no-issue` label (`DEC-148.1`). The counter starts at 1 for each prefix and number and counts up within that work. Tasks are GitHub issues and get no record id (DEC-149.1). Issues and pull requests share one number sequence, so two branches can never produce the same id.

Entries written before DEC-148.1 keep their 3-digit ids (`DEC-071`, `HND-042`) and are never renumbered. `tools/check_ids.py` accepts both forms, and in pull request CI fails when a new id does not use the pull request's number.

## 4a. Reading tiers, indexes and archives

Agents pay for every file they read, so the agent docs have three tiers ([agents/README.md](../agents/README.md#reading-tiers-dec-1521), DEC-152.1): **boot** files read every session, **on-demand** files read when the task points to them, and **human-only** files that agents skip. Rules that keep the boot files small:

- **Decisions:** the full entry goes in `history/decision-log.md`; its one-line row (at most 120 characters) goes in `history/decision-index.md`, which is what agents read. Superseded and rejected rows move to the index's Archived table.
- **Handoffs:** the live `agents/state/handoff-log.md` keeps the newest entries; `tools/rotate_handoffs.py` moves older ones unchanged to `history/handoff-archive/<YYYY-MM>.md`.
- **Questions:** `agents/state/open-questions.md` holds only open questions; answered or obsolete ones move to `history/questions-archive.md`.
- **Problems:** one file per area under `history/problems/` with an index in its README, read only when a problem occurs.
- Moving an entry unchanged to its archive is not an edit (P-6). `tools/check_ids.py` checks ids across live and archive files; `tools/check_boot.py` checks the boot budget.

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
