# Contributing

The short version of how we work. Details and reasons: [docs/guidelines/git-workflow.md](docs/guidelines/git-workflow.md).

## Starting a task

1. In ClickUp, set the task to **in progress** and press **Create GitHub issue** on it. Keep the PB id in the issue title (for example `PB-025 Define the physics adapter interface`).
2. Within a minute, the **Issue start** workflow creates the branch and a draft pull request for the issue, and comments on the issue with the checkout command:

   ```
   git fetch origin
   git switch feat/pb-025-define-the-physics-adapter-interface
   ```

   Branch names follow `<type>/pb-<nnn>-<short-description>`. For a bug without a PB item, create the branch yourself: `fix/<issue>-<short-description>`.
   Types: `feat`, `fix`, `test`, `refactor`, `docs`, `ci`, `chore`, `research`. Lowercase, words joined with `-`.
3. Commit and push to that branch. The draft PR already says `Closes #<issue>`.

## Pull requests

- The PR title is a [Conventional Commit](https://www.conventionalcommits.org/en/v1.0.0/) and becomes the commit on `main`:
  `feat(phys): define the physics adapter interface (PB-025)`. The scope is the area label without `area:`.
- Keep PRs under about 400 changed lines.
- Fill in the PR template, including how you verified the change.
- Mark the PR **ready for review** when CI is green. Reviewers come from [.github/CODEOWNERS](.github/CODEOWNERS).
- One approval from Işık Dönger or the advisor, Leyla Sezer (the code owners) is needed; area teammates are welcome to review too, but their approval does not count. Işık's own PRs need no approval. Reviews happen within 1 working day; comments are labelled `blocker`, `should`, `nit` or `question`, and only `blocker` holds up approval.
- The author squash-merges. Afterwards, set the ClickUp task to **done**.

## Code style and Definition of Done

- C++ follows [docs/guidelines/coding-standards.md](docs/guidelines/coding-standards.md) and is formatted with **clang-format 19** using [.clang-format](.clang-format). Install it with `pip install clang-format==19.1.7` and turn on format-on-save; CI rejects unformatted C++.
- A task is done when it meets [docs/process/definition-of-done.md](docs/process/definition-of-done.md). The PR template carries the short version.

## Rules for `main`

- No direct pushes, no force pushes. Every change goes through a squash-merged PR with an approval and a green `ci-ok` check.
- A ruleset on `main` enforces this (DEC-032): PR required, one code-owner approval, squash merge only, `ci-ok` must pass, no force pushes or deletion. Repository admins may bypass it only through a PR, which is how Işık merges his own PRs without review. The **Main guard** workflow still marks any commit that reached `main` without a merged PR.

## AI agents

An agent pushes a branch only after a human approves that push. A human approves and merges every PR an agent opens. See `docs/agents/rules.md` (S-2).

## Issues

- One issue per top-level ClickUp backlog item, titled like the task (`PB-025 Define the physics adapter interface`); sub-items are a checklist inside it.
- Every issue has one assignee, `area:*` labels, a `priority:*` label and a milestone (Increment 1, Increment 2, Final Product).
- Bugs you find along the way get their own issue (use the Bug form).
- Close issues through a merged PR, or as "not planned" with a one-line reason.
