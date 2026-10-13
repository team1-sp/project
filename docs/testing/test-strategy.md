# Test Strategy

The test framework and build integration are not chosen yet (Q-009, TASK-005); tests are C++. This file defines *what* we test and *how it is organised*, independent of tools.

## 1. Principles

1. **Test the chain, link by link.** Input → ECS → Physics → ECS → Rendering. When something is wrong on screen, find the first link whose data is wrong; don't only look at the final picture.
2. **Headless by default.** Tests never need a window or GPU. Rendering is tested via render *data* (what would be drawn where), with a small, explicit set of visual tests.
3. **Deterministic.** Fixed timestep, scripted inputs, seeded randomness. Never depend on wall-clock or machine speed.
4. **Requirements-traceable.** Each test maps to an `ip.fr.*` / `ip.nfr.*` ID (in the test name, a comment, or the checklist).
5. **Fast.** Unit and adapter tests run in seconds; scenarios in a minute or so; benchmarks separate.
6. **The demo game is ground truth.** Injected bugs verify the detector and the AI (ip.fr.12.2).

## 2. Levels

| Level | Scope | Location | Run when |
| --- | --- | --- | --- |
| Unit | ECS ops, single system with a hand-built World | `tests/unit/` | every change |
| Adapter | Entity↔body mapping, write-back, collision translation, raycast, fixed step | `tests/adapter/` | every change touching physics |
| Integration (headless) | Several systems in the loop for N frames | `tests/integration/` | every change |
| Scenario | Demo game + scripted/agent input; expects events or detected bugs | `tests/scenarios/` | before review, nightly if CI exists |
| Equivalence | Same input stream in normal vs headless → identical state/events (ip.fr.7.3) | `tests/integration/` | before review of loop/render changes |
| Performance | ip.nfr.1.1–1.4 | `tests/benchmarks/` | on request, before sprint review |
| AI evaluation | Assistant answers vs injected-bug ground truth | `tests/scenarios/ai/` | when AI layer exists |

## 3. What to test per layer

See [test-checklists.md](test-checklists.md) (ECS, physics, rendering data, integration, headless, telemetry, semantic, AI).

## 4. Test data and helpers

- Builders: `MakePlayer(world, pos)`, `MakeWall(...)`, `MakeDoor(...)` to keep tests short.
- Input scripts: list of `(frame, action)` — same format used by the Automated Playtest Agent's action interface (ip.fr.5.2).
- Golden telemetry files for scenarios (small, human-readable, versioned) — updated deliberately with a note in the PR.

## 5. Failure handling

- A failing test blocks `Done`. Do not delete or weaken tests to pass (fix-bug playbook).
- Flaky test = bug in determinism. Log a `PRB-###`, fix the cause, don't retry-until-green.

## 6. Reporting

A headless run produces a test report (ip.fr.7.5): outcome, frames executed, detected bugs. Test runs for sprint reviews should save this report as evidence `[TBD location]`.

## 7. Coverage expectations (MVP)

Not a numeric coverage target; the targets are: every checklist item has a test; every injected bug has a detection test and a "clean" no-false-positive test; each fixed bug has a regression test.

## 8. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Created from guide testing checklist and requirements | claude-code/sonnet-5.5/012r |
