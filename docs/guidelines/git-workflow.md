# Git Workflow

Status: **Accepted** (DEC-024, DEC-027, 2026-10-15; DEC-053 to DEC-055, 2026-11-12). The repo keeps contribution history per member (course requirement), so **commit authorship matters**. Short version for contributors: [CONTRIBUTING.md](../../CONTRIBUTING.md).

## 1. Branching

Trunk-based: `main` is the only long-lived branch. No `develop` branch.

| Branch | Rule |
| --- | --- |
| `main` | Always builds and passes tests. No direct pushes, no force pushes, no deletion. Changes only by squash-merged PR |
| Work branches | From the latest `main`, one backlog item (or one bug) per branch, merged within the sprint, deleted after merge |
| Tags `inc-1`, `inc-2`, `final` | Annotated tags on `main` at each increment review, created by a repo admin |

Branch names (DEC-050), lowercase, words joined with `-`, one branch per user story:

- `<type>/us-<x.y>-<short-description>` for an SRS user story, for example `feat/us-15.2-build-settings-file-menu`.
- `<type>/<issue>-<short-description>` for any other issue, for example `fix/142-door-opens-without-key`.
- `<type>/pb-<nnn>-…` stays valid for branches started before DEC-050.
- `docs/<short-description>`, `ci/<short-description>` or `chore/<short-description>` for documentation, pipeline or tooling work that has no issue (DEC-056), for example `ci/update-agent-workflow`. Its PR needs no `Closes` line and its title has no scope: `ci: Update the agent workflow`. These types may still use an issue when there is one.

`tools/issue_ref.py` (used by `tools/check_pr.py`) and the repository's `branch-names` ruleset enforce this pattern:

```
^((feat|fix|test|refactor|docs|ci|chore|research)/(us-[0-9]+\.[0-9]+|[0-9]+|pb-[0-9]{3}(\.[0-9]+)?)-[a-z0-9]+(-[a-z0-9]+)*|(docs|ci|chore)/[a-z][a-z0-9]*(-[a-z0-9]+)*)$
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

**Starting a task:** create the story's branch, from the issue's **Development → Create a branch** button (edit the suggested name to the pattern above) or by pushing it (DEC-050). Creating the branch is the start signal: the `Issue start` workflow (`tools/start_task.py`) runs on the branch creation, moves the issue's card to **In progress** on the [GitHub project](https://github.com/orgs/team1-sp/projects/3) (unless it is already further along), assigns the branch creator if nobody is assigned and comments the checkout command. Moving the card needs the `PROJECT_TOKEN` secret (see the workflow file). **Opening the pull request** (DEC-054): after the first commit is pushed, run `python tools/open_pr.py` on the branch. It opens a **draft** PR filled from the issue: `Closes #<issue>`, labels, milestone, assignees, and a Conventional Commit title (`--title` overrides it; `--body-file` gives the body, otherwise the PR template is used). Agents run it themselves right after their first push; nobody has to open the PR by hand. GitHub cannot open a PR for a branch with no commits beyond `main`, so the PR waits for the first commit. No workflow creates branches, commits or pull requests: a PR opened by a workflow would not trigger CI, and a bot "task started" commit would put every local branch behind its remote. A PR opened another way is still checked by the `pr-hygiene` CI job (`tools/check_pr.py`), which fails without a valid title and `Closes #<issue>`. Epics get no branch; work on their stories.

**Dependencies:** stories have "blocked by" links. Creating a branch for a story whose blockers are still open only gets a warning comment, and the `dependencies` CI job (`tools/check_blockers.py`) fails while any blocker is open, so `ci-ok` and the `main` ruleset block the merge. After a blocker closes, re-run the job or push again.

**Agents** commit only after a human approves the change (DEC-053, replacing the push approval of DEC-025): a commit cannot be changed afterwards without rewriting history, so the approval belongs before the commit. Once a commit is approved, the agent pushes it and opens or updates the draft PR without asking again. A human still approves and merges every PR (S-2). Agents never push to `main`, force-push or rewrite history.

## 2. Commits and PR titles

PRs are squash-merged, so **the PR title becomes the commit on `main`**. It must be a [Conventional Commit](https://www.conventionalcommits.org/en/v1.0.0/), with the story id as the scope, or the issue number for issues that are not SRS stories (DEC-050, replacing the PB id of DEC-035):

```
feat(US-1.1): Run every simulation tick in a fixed phase order
ci(#126): Move the repo workflow from ClickUp to GitHub Projects
fix(#142): Door opens without the key
```

Types: `feat`, `fix`, `test`, `refactor`, `perf`, `docs`, `build`, `ci`, `chore`, `research`, `revert`. Commits inside a branch are free-form. Squash merging keeps the PR author as the commit author; co-authors stay as `Co-authored-by:` trailers. When an AI agent made a commit, keep the tool's co-author trailer. Never commit secrets, large binaries, build output or personal paths.

## 3. Pull requests

- Body: `Closes #<issue>` (not needed on a `docs/`, `ci/` or `chore/` branch without an issue, DEC-056; elsewhere the `no-issue` label is the exception), what and why, how it was verified, docs updated, and the Definition of Done checklist (template in `.github/pull_request_template.md`).
- Open as a draft early; mark ready for review when CI is green. Keep PRs under about 400 changed lines.
- Review: 1 approval from a code owner (`.github/CODEOWNERS`, DEC-032). The only code owners are Işık Dönger and the advisor, Leyla Sezer (`@leylasezer196`), so every other member's PR needs one of them. Area teammates are listed in CODEOWNERS as people to ask for a review; their approval does not count toward merging. Işık's own PRs need no approval: repository admins merge them through the ruleset's PR bypass. Review practice: `process/code-review.md`.
- All CI green (`ci-ok`) and conversations resolved before merging. The author squash-merges; the issue closes and moves to Done on the project.

**Enforcement:** the repo is public, so GitHub rulesets work on the free plan. The `main` ruleset (DEC-032) requires a PR, one code-owner approval, squash merge and a green `ci-ok`, and blocks force pushes and deletion. Bypass: repository admins and organization owners, only through a PR. The `Main guard` workflow still flags commits that reached `main` without a merged PR.

## 4. Merge conflicts

Other PRs get merged while a branch is open, so `main` moves under it (DEC-055). The branch's author keeps it in step; the reviewer does not resolve conflicts.

**Before starting:** list the open PRs and the files they touch (`gh pr list --state open --json number,title,files`). If one touches the same code or design docs, tell the human before you begin, so the order can be agreed; shared logs (`history/*`, `agents/state/*`) do not count.

**Sync** by merging `origin/main` into the branch (`git fetch origin` then `git merge origin/main`), never by rebasing, so nothing pushed is rewritten:

- before every push,
- before marking the PR ready for review,
- whenever the PR gets the `needs-sync` label (see below).

After the merge, build and run the tests again before pushing.

**Resolving:**

- State and history files (`agents/state/*`, `history/*`, `testing/test-checklists.md`): keep **both** sides' entries. If both added the same id (DEC, HND, PRB, Q, TASK), the branch being synced renumbers its own entries and every reference to them. The `docs` CI job runs `tools/check_ids.py`, which fails on a duplicate id or a decision or problem entry missing from its index, because two appends can merge without any git conflict and still collide.
- Code and design docs: resolve only when both sides clearly fit together, then test. When they contradict each other (two designs, two decisions), stop and ask the human; do not pick a side.
- An agent commits a merge without asking again when it had no conflicts, or only conflicts in state and history files resolved as above. A merge that needed any other resolution is shown to the human before it is committed (DEC-053).

**After ready for review:** the `Conflict flag` workflow (`tools/flag_conflicts.py`) runs after every merge to `main`. Each open PR that now conflicts gets the `needs-sync` label and one comment; the label goes away by itself once a push resolves it. The author (or the agent working on the PR) syncs as above and pushes; the reviewer then only needs to look at the merge commit, not the whole PR again. A PR with `needs-sync` is not merged.

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
| 2026-11-11 | GitHub Projects replaces ClickUp: issue-number branches, story-id PR scopes; creating a branch moves the card to In progress and new PRs are filled from the issue (DEC-050) | claude (issue #126) |
| 2026-11-12 | `docs/`, `ci/` and `chore/` branches may have no issue: `ci/<short-description>`, PR title without scope (DEC-056) | claude (ci/131) |
| 2026-11-12 | Agents ask before committing instead of before pushing (DEC-053); `tools/open_pr.py` opens the filled draft PR and the PR fill workflow is removed (DEC-054); merge-conflict rules, `check_ids.py` and the `needs-sync` flag (DEC-055) | claude (ci/131) |
