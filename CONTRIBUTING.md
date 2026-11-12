# Contributing

The short version of how we work. Details and reasons: [docs/guidelines/git-workflow.md](docs/guidelines/git-workflow.md).

## Starting a task

The backlog is the [GitHub project](https://github.com/orgs/team1-sp/projects/3) (DEC-050). Epics (`EP-xx`) and user stories (`US-x.y`) come from the SRS; each story is a sub-issue of its epic, and its functional requirements are its acceptance criteria.

1. Create the branch for your issue, from **Development → Create a branch** on the issue (edit the suggested name) or by pushing it. Name it after the story: `<type>/us-<x.y>-<short-description>`, for example `feat/us-15.2-build-settings-file-menu`. For an issue that is not a story (a bug, a chore), use its number: `fix/142-door-opens-without-key`. Documentation, pipeline and tooling work needs no issue: name the branch `docs/…`, `ci/…` or `chore/…` with a short description (`ci/update-agent-workflow`), leave out `Closes`, and title the PR without a scope (`ci: Update the agent workflow`) (DEC-056). Types: `feat`, `fix`, `test`, `refactor`, `docs`, `ci`, `chore`, `research`. Lowercase, words joined with `-`; one branch per story. The `branch-names` ruleset rejects other names.
2. The **Issue start** workflow reacts at once. If the story is still blocked by open issues, it only comments which ones and leaves the card alone. Otherwise it moves the card to **In progress** on the project, assigns you if nobody is assigned, and comments the checkout command on the issue.
3. Commit and push, then run `python tools/open_pr.py` on the branch. It opens a draft PR with `Closes #<issue>`, the issue's labels, milestone and assignees, and a title like `feat(US-1.1): Run every simulation tick in a fixed phase order` (DEC-054). GitHub needs at least one commit before a PR can exist. No workflow creates branches, commits or pull requests for you.
4. Keep the branch in step with `main`: merge `origin/main` into it (no rebase) before each push and before asking for review, and whenever the PR gets the `needs-sync` label. Rules for resolving conflicts: [git-workflow.md §4](docs/guidelines/git-workflow.md#4-merge-conflicts) (DEC-055).

## Pull requests

- The PR title is a [Conventional Commit](https://www.conventionalcommits.org/en/v1.0.0/) and becomes the commit on `main`:
  `feat(US-1.1): Run every simulation tick in a fixed phase order`. The scope is the story id, or the issue number for other issues: `fix(#142): Door opens without the key` (DEC-050). A docs, ci or chore PR without an issue has no scope (DEC-056).
- Keep PRs under about 400 changed lines.
- Fill in the PR template, including how you verified the change.
- Mark the PR **ready for review** when CI is green. Reviewers come from [.github/CODEOWNERS](.github/CODEOWNERS).
- One approval from Işık Dönger or the advisor, Leyla Sezer (the code owners) is needed; area teammates are welcome to review too, but their approval does not count. Işık's own PRs need no approval. Reviews happen within 1 working day; comments are labelled `blocker`, `should`, `nit` or `question`, and only `blocker` holds up approval.
- The author squash-merges. The issue closes and its card moves to **Done** on the project by itself.

## Code style and Definition of Done

- C++ follows [docs/guidelines/coding-standards.md](docs/guidelines/coding-standards.md) and is formatted with **clang-format 19** using [.clang-format](.clang-format). Install it with `pip install clang-format==19.1.7` and turn on format-on-save; CI rejects unformatted C++.
- A task is done when it meets [docs/process/definition-of-done.md](docs/process/definition-of-done.md). The PR template carries the short version.

## Rules for `main`

- No direct pushes, no force pushes. Every change goes through a squash-merged PR with an approval and a green `ci-ok` check.
- A ruleset on `main` enforces this (DEC-032): PR required, one code-owner approval, squash merge only, `ci-ok` must pass, no force pushes or deletion. Repository admins may bypass it only through a PR, which is how Işık merges his own PRs without review. The **Main guard** workflow still marks any commit that reached `main` without a merged PR.

## AI agents

An agent commits only after a human approves the change; once approved, it pushes and opens or updates the draft PR without asking again (DEC-053). A human approves and merges every PR an agent opens. See `docs/agents/rules.md` (S-2).

## Issues

- SRS stories are issues titled with their id (`US-17.2 Play, pause and step the game in the editor`), added as a sub-issue of their epic (`EP-17 ...`). Stories not planned yet stay as a checklist in their epic until they are planned.
- Every story has one assignee, an `area:*` label, its `uc:*` labels and a milestone (Increment 1, Increment 2, Final Product). Iteration, Estimate, Size and Priority are set on the project.
- Bugs you find along the way get their own issue (use the Bug form).
- Close issues through a merged PR, or as "not planned" with a one-line reason.
