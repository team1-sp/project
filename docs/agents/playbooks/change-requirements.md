# Playbook — Change Requirements

How an agent adds, edits, removes or moves a requirement owned by [`docs/requirements/`](../../requirements/README.md) (DEC-152.2, DEC-152.3, DEC-153.1). The human's part is the decision: a requirement change is listed for review and binds only when a human merges the PR (P-3, DEC-052).

**Feature work does not edit the epic files and does not renumber.** It writes the change into a pending change file; the requirement files are updated later, in one pull request, by a script that applies every pending change, renumbers and rewrites every reference (DEC-153.1).

## Before you change anything

1. Have a reason: a human decision in the conversation, or a change your PR needs that you will list under "Decisions for review". Never change a requirement silently.
2. Find the ids you need with a search (`grep -n "srs.fr.14.1[0-9]" docs/requirements/functional/ep-14-*.md`); open the epic file only around them. Never read the whole folder.

## In a feature PR: write the change

Create or extend `docs/requirements/changes/<n>.md` (`<n>` = the issue your PR closes, or the PR's own number for no-issue work), in the form of [changes/README.md](../../requirements/changes/README.md):

| Change | Heading | Lines |
| --- | --- | --- |
| **Add** | `## Add after <id>` or `## Add at end of US-<x.y>` (or `non-functional/<file>.md`) | the requirement line with id `RC-<n>.<k>`, plus its `Use cases` line |
| **Edit** | `## Reword <id>` | the requirement line with its own id and the new text |
| **Remove** | `## Remove <id>` | `References: <replacing id>` or `References: none` |
| **Move** | `## Move <id> after <id>` | none |

In your code, tests and docs, cite a new requirement as `RC-<n>.<k>`; existing ones by their current id. Then:

```sh
python tools/apply_requirement_changes.py --check .   # your file applies cleanly; every RC- id you cite is defined
```

Log a `DEC` that quotes the old and the new wording (with `RC-` ids for new requirements) and points to the change file; list it under "Decisions for review".

## Applying the pending changes (one PR, when the team asks)

Before a docs sync for a deliverable, or whenever a human asks, open one no-issue PR that runs:

```sh
python tools/apply_requirement_changes.py   # applies every pending file, renumbers 1..n, rewrites every reference, deletes the files
python tools/check_requirements.py .
```

If it refuses, it changed nothing and names the cause: a target id that no longer exists, two files changing one requirement, or a removed requirement still cited with `References: none`. Fix the change file and run it again. Its `HND` lists the applied files; the old → new table is in [renumbering.md](../../requirements/renumbering.md).

If your branch was open during an apply, merge `main`: CI then fails on any `RC-` id or old id your branch still cites. Replace them with the final ids from `renumbering.md`.

## Changes proposed from the docs repository

A PR made by `tools/propose_to_project.py` in `team1-sp/docs` (branch `docs/<n>-…`, `DEC-<n>.1` "Decided by" a meeting or member) edits the requirement files directly and renumbers, like an apply PR; `tools/renumber_requirements.py` also rewrites the ids inside pending change files, so they stay valid. Review it like any requirement change; if CI fails, fix it on that branch ([requirements README](../../requirements/README.md#changes-that-start-in-the-docs-repository), DEC-152.4).

## Not covered

User stories (`## US-x.y`) are not renumbered: they name GitHub issues and branches. Adding, removing or splitting a story is a backlog change: ask the human first ([record-decision.md](record-decision.md)).
