# Pending Requirement Changes

Feature work does not edit the epic files or renumber ids (DEC-153.1). It writes what should change here, one file per pull request, and the requirement files are updated later, in one pull request, by `tools/apply_requirement_changes.py`. Ids stay continuous: the apply step renumbers right after applying and rewrites every reference.

## Writing a change file

Name it `<n>.md`, where `<n>` is the issue the pull request closes, or the pull request's own number for no-issue work (the same number as its `DEC-<n>.*`). Each change is a `##` heading followed by its lines. Ids are today's ids on `main`.

```markdown
# Requirement changes: #171

Decided by: DEC-171.1

## Add after srs.fr.0.15
- **RC-171.1** When the user ..., the Editor shall ...
  - Use cases: UC-03; Initial Plan: —

## Add at end of US-0.2
- **RC-171.2** The Editor shall ...
  - Use cases: UC-03; Initial Plan: —

## Add at end of non-functional/constraints.md
- **RC-171.3** The product shall ...

## Reword srs.fr.0.17
- **srs.fr.0.17** The new wording of the requirement.

## Remove srs.fr.0.4
References: srs.fr.0.5

## Move srs.fr.0.2 after srs.fr.0.3
```

(`srs.fr.0.*` is the example group; write real ids.)

| Change | Lines under the heading |
| --- | --- |
| `Add after <id>` / `Add at end of US-<x.y>` / `Add at end of non-functional/<file>.md` | The new requirement in the shape of its file, with the id `RC-<n>.<k>` (`k` = 1, 2, … in this file). A functional requirement has its `Use cases` line. `<id>` may be an `RC-` id added earlier. |
| `Reword <id>` | The requirement line with its own id and the new wording; add the `Use cases` line only to change it. |
| `Remove <id>` | `References: <replacing id>` (every reference is pointed there) or `References: none` (the apply refuses while anything still cites it). |
| `Move <id> after <id>` | none; it takes the number of its new place. |

Code, tests and docs cite a new requirement as `RC-<n>.<k>` until it is applied. User stories are not added or removed here: that is a backlog change for a human (playbook, "Not covered").

## Applying

When the team wants the requirement files current (at the latest before a docs sync for a deliverable), one pull request runs:

```sh
python tools/apply_requirement_changes.py      # applies every file here in number order, renumbers, rewrites references
python tools/check_requirements.py .
```

It applies the files, points references to removed requirements at their replacements, turns every `RC-` id into its final `srs.fr` or `srs.nfr` id across the repository, renumbers every group 1..n, adds the old → new table to [renumbering.md](../renumbering.md) and deletes the applied files. Nothing is written when a check fails. CI runs `--check` on every pull request: each file must apply cleanly and every cited `RC-` id must have a pending file.
