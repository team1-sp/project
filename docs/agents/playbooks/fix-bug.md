# Playbook — Fix a Bug

Companion docs: [troubleshooting/debugging-playbook.md](../../troubleshooting/debugging-playbook.md) (technical method) and [history/problem-log.md](../../history/problem-log.md) (memory).

## Steps

1. **Search memory.** Grep `problem-log.md` and `common-errors.md` for the exact error text, the component name and the symptom. If a solution exists, try it and add an `Occurrences` line. Done if it works.
2. **Reproduce.** Make a failing test or a deterministic reproduction (fixed inputs, fixed seed, headless if possible). A bug you cannot reproduce is not fixed, only hidden. Record the reproduction command in the task.
3. **Localise along the chain.** Input → ECS → Physics sync → Physics step → ECS write-back → Collision events → Gameplay → Telemetry → Rendering. Inspect data at each link (telemetry, logs, debugger, assertions). Find the *first* link where the data is wrong.
4. **Classify the layer owner.** Wrong decision → gameplay system. Wrong position/collision → adapter or physics config. Wrong picture with right data → rendering. Wrong data in the right place → whoever wrote the component.
5. **Hypothesise one cause, test it, change one thing.** Two attempts per approach (protocol rule), then escalate.
6. **Fix at the root.** No symptom patches in rendering for a physics problem, etc.
7. **Lock it with a test.** The reproduction test now passes and stays in the suite.
8. **Check neighbours.** Same bug class elsewhere? (e.g. another system iterating while removing components.)
9. **Log it.** Append `PRB-###` with exact error text, root cause, solution, prevention. Update `common-errors.md` if it is a general pitfall.
10. **Handoff.**

## Do not

- "Fix" by loosening a test or an assertion.
- Add sleeps/frame delays to hide ordering bugs; fix the ordering.
- Change the fixed timestep to make a collision problem go away (R-5); consider tunnelling, collider size, or continuous collision instead.
- Fix a bug in the demo game that is a **deliberately injected** ground-truth bug (ip.fr.12.2). Those are controlled by configuration.
