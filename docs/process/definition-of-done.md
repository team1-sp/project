# Definition of Done

A task is `Done` only if **all** applicable items hold.

## Code tasks

- [ ] Acceptance criteria in the task are met and checked by someone/something other than the wording of the code.
- [ ] Builds from a clean checkout with the documented command (once build system exists).
- [ ] New/changed behaviour has automated tests; all tests pass; commands and results recorded in the task.
- [ ] Tests run **headless** (no window/GPU).
- [ ] The chain was verified link by link where relevant (Input → ECS → Physics → ECS → Rendering).
- [ ] Architecture invariants R-1…R-9 respected (reviewer checked).
- [ ] No library/framework types leaked outside adapter / rendering implementation.
- [ ] Headless and normal mode produce identical simulation results for the affected feature (if applicable).
- [ ] Telemetry updated for new state/events; overhead considered (ip.nfr.1.3).
- [ ] No new compiler warnings; formatting per `coding-standards.md`.
- [ ] No secrets, no stray files, no large binaries.

## Docs

- [ ] Every doc made inaccurate by the change is updated (architecture/design/testing/troubleshooting).
- [ ] New pitfalls → `common-errors.md`; solved problems → `problem-log.md`.
- [ ] Decisions → `decision-log.md`.

## Records

- [ ] Task entry updated with result and verification.
- [ ] `current-status.md` updated.
- [ ] Handoff entry appended.
- [ ] GenAI log updated if course text was generated.

## Review

- [ ] Reviewed by someone other than the author (agent or human), or marked `Review (self)` with a reason.
- [ ] Merged to `main` by a human (S-2).

## Docs-only tasks

Accuracy verified against the repo, links checked, change table updated, README folder map updated if files were added/removed.

## Not Done if

Tests were not run; "works on my machine" only; a TODO hides a known failure without a `Blocked`/new task; the doc contradicts the code.
