# Git Workflow

Status: **Accepted** (DEC-024, DEC-025, DEC-027, 2026-10-15). The repo keeps contribution history per member (course requirement), so **commit authorship matters**. Short version for contributors: [CONTRIBUTING.md](../../CONTRIBUTING.md).

## 1. Branching

Trunk-based: `main` is the only long-lived branch. No `develop` branch.

| Branch | Rule |
| --- | --- |
| `main` | Always builds and passes tests. No direct pushes, no force pushes, no deletion. Changes only by squash-merged PR |
| Work branches | From the latest `main`, one backlog item (or one bug) per branch, merged within the sprint, deleted after merge |
| Tags `inc-1`, `inc-2`, `final` | Annotated tags on `main` at each increment review, created by a repo admin |

Branch names: `<type>/pb-<nnn>-<short-description>`, lowercase, words joined with `-`. Bugs without a PB item use the issue number: `fix/142-door-opens-without-key`.

| Type | Use for |
| --- | --- |
| `feat` | New behaviour |
| `fix` | Bug fix |
| `test` | Tests only |
| `refactor` | Restructure without behaviour change |
| `docs` | Docs only |
| `ci` / `chore` | Pipelines, build, tooling, dependencies |
| `research` | Notebooks, spikes, prototypes that may not merge |

The `pr-hygiene` CI job rejects other names (`tools/check_pr.py`).

**Agents** push a branch only after a human approves that push; a human approves and merges every agent PR (DEC-025, S-2). Agents never push to `main`, force-push or rewrite history.

## 2. Commits and PR titles

PRs are squash-merged, so **the PR title becomes the commit on `main`**. It must be a [Conventional Commit](https://www.conventionalcommits.org/en/v1.0.0/), scope = area label without `area:`:

```
feat(phys): define the physics adapter interface (PB-025)
fix(game): door opened without the key (#142)
```

Types: `feat`, `fix`, `test`, `refactor`, `perf`, `docs`, `build`, `ci`, `chore`, `research`, `revert`. Commits inside a branch are free-form. Squash merging keeps the PR author as the commit author; co-authors stay as `Co-authored-by:` trailers. When an AI agent made a commit, keep the tool's co-author trailer. Never commit secrets, large binaries, build output or personal paths.

## 3. Pull requests

- Body: `Closes #<issue>` (or the `no-issue` label for chore/ci), what and why, how it was verified, docs updated, and the Definition of Done checklist (template in `.github/pull_request_template.md`).
- Open as a draft early; mark ready for review when CI is green. Keep PRs under about 400 changed lines.
- Review: 1 approval from a code owner (`.github/CODEOWNERS`, DEC-024). Işık Dönger is the default owner of every path; area owners are second owners, so either approval counts. That also covers Işık's own PRs and days he cannot review within 1 working day. Review practice: `process/code-review.md`.
- All CI green (`ci-ok`) and conversations resolved before merging. The author squash-merges, then sets the ClickUp task to done.

**Enforcement:** branch protection is not available on our GitHub plan for a private repo (DEC-027). Until it is, these rules are a team agreement and the `Main guard` workflow flags commits that reached `main` without a merged PR.

## 4. Merge conflicts

- State files (`agents/state/*`, `history/*`): keep **both** sides' entries; re-number IDs if they collide.
- Code: the author of the later branch resolves by merging or rebasing `main` into their own branch, then re-runs tests.

## 5. Tags / releases

Increment deliverables get an annotated tag `inc-1`, `inc-2`, `final` on `main`, aligned with the sprint review.

## 6. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Proposed | claude-code/sonnet-5.5/012r |
| 2026-10-15 | Rewritten to the accepted repo plan: trunk-based, PB branch names, Conventional Commit PR titles, CODEOWNERS review, main guard | claude-code (session 0171) |
