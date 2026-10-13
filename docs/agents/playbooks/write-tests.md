# Playbook — Write Tests

Strategy and checklists: [testing/test-strategy.md](../../testing/test-strategy.md), [testing/test-checklists.md](../../testing/test-checklists.md), [testing/headless-testing.md](../../testing/headless-testing.md).

## Rules of thumb

1. **Tests run headless.** No window, no GPU. If a test needs rendering, it tests the render *data* (what would be drawn where), not pixels — except in explicitly marked visual tests.
2. **Deterministic.** Fixed timestep, fixed inputs, fixed random seed. Never depend on wall-clock time.
3. **One behaviour per test.** Name it `<Unit>_<Condition>_<Expected>` (or the language's convention in `coding-standards.md`).
4. **Test each link of the chain separately**, then one integration test across it.
5. **Arrange–Act–Assert**; build the world explicitly (entities + components) in the test; don't rely on global state.
6. **Assert on data and events**, not on log text.
7. **Failing first.** Show that the test fails without the change when possible (note it in the task).

## Picking the level

| What you're verifying | Level | Example |
| --- | --- | --- |
| A component/system in isolation | Unit | MovementSystem updates Velocity from input |
| Entity ↔ body mapping, write-back | Adapter | After Step, PositionComponent equals body position |
| Several systems in the loop | Integration (headless) | Player walks into wall: collision event fired, position stops |
| Whole demo game with an agent | Scenario | Agent reaches goal; injected bug "door opens without key" is detected |
| Performance requirement | Benchmark | ≥ 600 headless steps/s (ip.nfr.1.2) on reference hardware |

## Checklist before saying "tests written"

- [ ] I ran them (command + result in the task).
- [ ] They fail when the behaviour is broken (mutate once to check).
- [ ] They do not depend on test order or machine speed.
- [ ] They are listed/mapped to a requirement ID in the test or in `test-checklists.md`.
