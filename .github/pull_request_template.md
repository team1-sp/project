<!-- Title: Conventional Commit; the scope is the story id or the issue number, e.g. "feat(US-1.1): Run every simulation tick in a fixed phase order" or "fix(#142): Door opens without the key"; no scope without an issue, e.g. "ci: Update the agent workflow" -->

Closes #<!-- issue number; a docs, ci or chore branch without an issue deletes this line (DEC-056) -->

## What and why

## How it was verified
<!-- Commands you ran and their results. -->

## Docs updated
<!-- Which docs changed, or "none needed". -->

## Decisions for review
<!-- Every decision-log entry this PR adds or changes, one line each: "DEC-### - what was decided (approval: review of this PR | conversation, who, date)". Entries approved through review are written Accepted and bind only when this PR is merged; ask for a change or a rejection in the review (DEC-052). Write "none" if there are none. -->

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
