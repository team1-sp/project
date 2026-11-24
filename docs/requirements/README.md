# Requirements

**This folder owns Aegis's requirements** (DEC-152.2): the 36 epics, 291 user stories and 1203 functional requirements with their use-case and Initial Plan mapping, and the non-functional requirements. The course deliverables in `team1-sp/docs` (SRS sections 6.1, 6.3, 6.4 and 7.1 to 7.6, later the SDD) are rebuilt from these files by a script in that repository. The docs repository reads from here; nothing here reads from there.

Agents read this folder **on demand** ([reading tiers](../agents/README.md#reading-tiers-dec-1521)): open the epic file of the story you work on. Never read the whole folder.

## Layout

| Path | Holds | Becomes in the SRS |
| --- | --- | --- |
| [conventions.md](conventions.md) | Id format, keywords, system names, screen codes, ticks and frames | 6.1 Conventions |
| `functional/ep-<nn>-<slug>.md` | One epic: its statement, intro, stories and requirements | 6.1 group `srs.fr.<n>`, 6.3 epic table, 6.4 rows |
| `non-functional/<name>.md` | One non-functional section (`srs.nfr.<n>.*`) | 7.1 to 7.6 |

An epic file looks like this (the order and the line shapes are fixed):

```markdown
---
epic: EP-01
group: srs.fr.1
part: engine            # engine | editor: which half of SRS 6.1 the group is in
epic_title: ...         # only when the 6.3 epic title differs from the group title
---

# EP-01 Engine Core and Game Loop

**Epic:** As a game developer, I want ..., so that ...

Optional intro paragraphs of the group.

## US-1.1 Render frames and simulation ticks

**Story:** As a game developer, I want ..., so that ...

*Screens: C-01*            (optional, Editor groups)

- **srs.fr.1.1** In normal mode, the Engine shall ...
  - Use cases: UC-06, UC-07; Initial Plan: ip.fr.1.1
```

A story's heading is the subsection title in SRS 6.1; its acceptance criteria in 6.3 are its requirements, first to last. `Initial Plan: -` means none. `tools/check_requirements.py` (CI) checks every rule of this layout.

## Changing a requirement

1. A requirement change is a decision: **humans decide** (DEC-017). Code work that finds a requirement wrong or missing proposes the change in its PR: edit the requirement here and log a `DEC` that quotes the old and the new wording, listed under "Decisions for review" (DEC-052). It binds only when a human merges the PR.
2. Keep every line shape above. One requirement is one line.
3. **Ids are numbered from 1 without gaps** in each group, the SRS convention, and a script keeps them that way (DEC-152.3). Agents follow [playbooks/change-requirements.md](../agents/playbooks/change-requirements.md). Never renumber by hand:
   - **Add:** put the line where it belongs with a placeholder id in its group, `- **srs.fr.14.new** ...` (`srs.fr.14.new2` for a second one), plus its `Use cases` line.
   - **Remove:** delete its two lines, and point any reference to it at the requirement that replaces it.
   - **Move:** move the two lines to the other epic file.
   - Then run `python tools/renumber_requirements.py`. It numbers every group in file order, rewrites every reference across the repository in one pass (code, tests, docs; not `docs/history/`, the handoff log or `docs/research/`), and adds the old → new table to [renumbering.md](renumbering.md). It refuses to run while a removed id is still referenced, and lists where. CI runs it with `--check`.
4. Do not edit `team1-sp/docs`. Its Markdown and Word files are rebuilt from here only when a deliverable is submitted (S-3).

## How the docs repository uses this folder

`team1-sp/docs` holds the deliverable Markdown with marked regions (`<!-- GENERATED:srs-fr-list -->` … `<!-- /GENERATED -->`). Its `tools/sync_from_project.py --project <checkout of this repo>` rewrites only those regions from these files, with no AI, and records the project commit in `SYNCED_FROM`. `--check` lists the deliverable sections that are out of date. Text outside the markers (section intros, GenAI usage notes) stays as people wrote it. The seed of this folder was taken from `team1-sp/docs` at commit `2ddcc2e` and rebuilds those SRS sections byte for byte.

## Change table

| Date | Change | Author |
| --- | --- | --- |
| 2026-11-24 | Created from SRS 6.1, 6.3, 6.4 and 7.1 to 7.6 (DEC-152.2) | claude-code/opus-5.5/6r0a |
