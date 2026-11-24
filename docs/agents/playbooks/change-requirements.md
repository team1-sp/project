# Playbook — Change Requirements

How an agent adds, edits, removes or moves a requirement in [`docs/requirements/`](../../requirements/README.md) (DEC-152.2, DEC-152.3). Agents do every step themselves, including renumbering; no human edits files by hand. The human's part is the decision: a requirement change is listed for review and binds only when a human merges the PR (P-3, DEC-052).

## Before you change anything

1. Open only the epic file of the group you change (`functional/ep-<nn>-*.md`) or the one non-functional file. Never read the whole folder.
2. Have a reason: a human decision in the conversation, or a change your PR needs that you will list under "Decisions for review". Never change a requirement silently.

## Operations

Each requirement is exactly two lines; keep their shape ([format](../../requirements/README.md#layout)).

| Operation | What you do | Id to write |
| --- | --- | --- |
| **Add** | Insert the two lines where the requirement belongs in its story. | A placeholder in the group: `srs.fr.<n>.new`, then `.new2`, `.new3` for more. Elsewhere in the PR (code, tests, docs) cite the same placeholder. |
| **Edit** | Change the text on its line. Keep the id. | unchanged |
| **Remove** | Delete its two lines. Point every reference to it at the requirement that replaces it, or remove the reference. | none |
| **Move** | Cut its two lines and paste them into the other story or epic file. | keep the old id; the script gives it the new group's number |
| **Non-functional** | The same, in `non-functional/<name>.md`: list lines `- **srs.nfr.<n>.<m>** ...` or table rows `\| srs.nfr.<n>.<m> \| ...`. | placeholder `srs.nfr.<n>.new` |

Then, in the same PR:

```sh
python tools/renumber_requirements.py      # renumbers 1..n in file order, rewrites every reference, logs old -> new
python tools/check_requirements.py .       # the format the docs repository reads
```

If the renumber script stops with "is not a requirement", a removed id is still cited at the listed lines: fix those references and run it again. Commit the rewritten files with your change; do not edit ids by hand.

## Record it

- Log a `DEC` that quotes the old and the new wording (with the new ids after renumbering) and list it under "Decisions for review".
- Do not touch `team1-sp/docs` (S-3). Its SRS sections are rebuilt from these files at submission.

## Changes proposed from the docs repository

A PR made by `tools/propose_to_project.py` in `team1-sp/docs` (branch `docs/<n>-…`, `DEC-<n>.1` "Decided by" a meeting or member) already ran the renumber and format checks. Review it like any requirement change; if CI fails, fix it on that branch with this playbook ([requirements README](../../requirements/README.md#changes-that-start-in-the-docs-repository), DEC-152.4).

## Not covered

User stories (`## US-x.y`) are not renumbered: they name GitHub issues and branches. Adding, removing or splitting a story is a backlog change: ask the human first ([record-decision.md](record-decision.md)).
