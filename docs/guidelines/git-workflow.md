# Git Workflow

Status: **Accepted** (DEC-024, DEC-025, DEC-027, 2026-10-15). The repo keeps contribution history per member (course requirement), so **commit authorship matters**. Short version for contributors: [CONTRIBUTING.md](../../CONTRIBUTING.md).

## 1. Branching

Trunk-based: `main` is the only long-lived branch. No `develop` branch.

| Branch | Rule |
| --- | --- |
| `main` | Always builds and passes tests. No direct pushes, no force pushes, no deletion. Changes only by squash-merged PR |
| Work branches | From the latest `main`, one backlog item (or one bug) per branch, merged within the sprint, deleted after merge |
| Tags `inc-1`, `inc-2`, `final` | Annotated tags on `main` at each increment review, created by a repo admin |

Branch names: `<type>/<issue>-<short-description>`, lowercase, words joined with `-`, for example `feat/41-run-every-simulation-tick-in-a-fixed` (DEC-050). Branches started before DEC-050 may still use `<type>/pb-<nnn>-…`. `tools/check_pr.py` enforces this pattern:

```
^(feat|fix|test|refactor|docs|ci|chore|research)/([0-9]+|pb-[0-9]{3}(\.[0-9]+)?)-[a-z0-9]+(-[a-z0-9]+)*$
```

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

**Starting a task:** move the issue to **In progress** on the [GitHub project](https://github.com/orgs/team1-sp/projects/3) (DEC-050). The `Issue start` workflow (`tools/start_task.py`) checks the project every 10 minutes and creates the branch from `main` for each open issue in progress that has no branch yet, links it to the issue and comments the checkout command. It never opens a pull request: the developer opens it after the first push. Running the workflow by hand with an issue number starts that issue at once. Epics get no branch. The type comes from the issue labels: `type:research` gives `research/`, `type:bug` gives `fix/`, `area:devops` gives `ci/`, docs-only gives `docs/`, otherwise `feat/`. Reading the project needs the `PROJECT_TOKEN` secret (see the workflow file).

**Agents** push a branch only after a human approves that push; a human approves and merges every agent PR (DEC-025, S-2). Agents never push to `main`, force-push or rewrite history.

## 2. Commits and PR titles

PRs are squash-merged, so **the PR title becomes the commit on `main`**. It must be a [Conventional Commit](https://www.conventionalcommits.org/en/v1.0.0/), with the story id as the scope, or the issue number for issues that are not SRS stories (DEC-050, replacing the PB id of DEC-035):

```
feat(US-1.1): Run every simulation tick in a fixed phase order
ci(#126): Move the repo workflow from ClickUp to GitHub Projects
fix(#142): Door opens without the key
```

Types: `feat`, `fix`, `test`, `refactor`, `perf`, `docs`, `build`, `ci`, `chore`, `research`, `revert`. Commits inside a branch are free-form. Squash merging keeps the PR author as the commit author; co-authors stay as `Co-authored-by:` trailers. When an AI agent made a commit, keep the tool's co-author trailer. Never commit secrets, large binaries, build output or personal paths.

## 3. Pull requests

- Body: `Closes #<issue>` (or the `no-issue` label for chore/ci), what and why, how it was verified, docs updated, and the Definition of Done checklist (template in `.github/pull_request_template.md`).
- Open as a draft early; mark ready for review when CI is green. Keep PRs under about 400 changed lines.
- Review: 1 approval from a code owner (`.github/CODEOWNERS`, DEC-032). The only code owners are Işık Dönger and the advisor, Leyla Sezer (`@leylasezer196`), so every other member's PR needs one of them. Area teammates are listed in CODEOWNERS as people to ask for a review; their approval does not count toward merging. Işık's own PRs need no approval: repository admins merge them through the ruleset's PR bypass. Review practice: `process/code-review.md`.
- All CI green (`ci-ok`) and conversations resolved before merging. The author squash-merges; the issue closes and moves to Done on the project.

**Enforcement:** the repo is public, so GitHub rulesets work on the free plan. The `main` ruleset (DEC-032) requires a PR, one code-owner approval, squash merge and a green `ci-ok`, and blocks force pushes and deletion. Bypass: repository admins and organization owners, only through a PR. The `Main guard` workflow still flags commits that reached `main` without a merged PR.

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
| 2026-10-23 | Only Işık and the advisor approve; Işık's PRs need no approval; `main` ruleset (DEC-032) | claude-code (session 0171) |
| 2026-10-15 | Issue start automation (branch and draft PR from ClickUp issues) | claude-code (session 0171) |
| 2026-10-24 | PR title scope is the PB id: `<type>(PB-<nnn>): …` (DEC-035) | claude-code (PB-002 session) |
| 2026-10-27 | Only `fix/` branches may use a bare issue number; pattern written out | claude-code |
| 2026-11-11 | GitHub Projects replaces ClickUp: issue-number branches, story-id PR scopes, Issue start creates only the branch when an item is In progress (DEC-050) | claude (issue #126) |
