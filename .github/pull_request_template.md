<!-- Title: Conventional Commit; the scope is the story id or the issue number, e.g. "feat(US-1.1): Run every simulation tick in a fixed phase order" or "fix(#142): Door opens without the key" -->

Closes #<!-- issue number; for chore/ci PRs without an issue, add the no-issue label instead -->

## What and why

## How it was verified
<!-- Commands you ran and their results. -->

## Docs updated
<!-- Which docs changed, or "none needed". -->

## Definition of Done
<!-- Full list: docs/process/definition-of-done.md -->
- [ ] Acceptance criteria in the issue are met
- [ ] Builds from a clean checkout; tests added or updated and passing
- [ ] Tests run headless (no window or GPU)
- [ ] Architecture rules R-1 to R-9 respected (`docs/agents/rules.md`)
- [ ] No Box2D or raylib types outside their adapter targets
- [ ] C++ formatted with clang-format 19; no new compiler warnings, no secrets, no large binaries
- [ ] Docs and decision log updated where the change made them inaccurate

After merging, the issue closes and its card moves to **Done** on the project by itself.
