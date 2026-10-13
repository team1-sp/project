# Playbook — Implement a Feature

Use for any new capability (a component, a system, an adapter function, an exporter). Combine with [AGENT-PROTOCOL.md](../AGENT-PROTOCOL.md).

## Steps

1. **Locate the requirement.** Find the `ip.fr.*` IDs the feature serves (a reference snapshot). If none, or if it conflicts with a decision, ask the human whether it is in scope (rule S-6).
2. **Find the layer.** Engine / Semantic / AI. Put code only in that layer (rule R-1). Check [architecture/engine-layer.md](../../architecture/engine-layer.md).
3. **Identify the data.** Which components does it read? Which does it write? Which events does it emit? Write this as a 3-line contract in the task: `reads: …  writes: …  emits: …`. This contract also feeds the semantic graph later (ip.fr.10.2), so be precise.
4. **Check the design doc.** `design/ecs-design.md`, `physics-adapter-design.md`, `telemetry-and-events.md`. If the feature needs a new component/event type, update the design doc first (or in the same change).
5. **Write the test first** (or at least the acceptance test) — see [write-tests.md](write-tests.md). The test must be runnable headless.
6. **Implement the smallest slice.** One system or one adapter function at a time.
7. **Placement in the frame order.** If it is a system, decide where in the fixed frame order it runs (ip.fr.1.1) and write that in the design doc. Order is part of the contract.
8. **Telemetry.** If the feature changes state that a bug could hide in, record it (state or event) per `telemetry-and-events.md`. Respect ip.nfr.1.3 (≤ 10% overhead).
9. **Check invariants.** Run the self-check list from `AGENT-PROTOCOL.md`. Especially: no direct library calls outside adapter, no render dependence in simulation, no gameplay decisions in physics.
10. **Verify the chain.** Observe each link: input → component values → physics body values → component values → render data. A feature is not done because the picture looks right.
11. **Record & hand off** (Phase 4/5 of the protocol).

## Template for the feature contract (put in the task)

```
Feature: <name>
Layer: Engine | Semantic | AI
Serves: ip.fr.x.y
reads:  PositionComponent, VelocityComponent
writes: VelocityComponent
emits:  (none)
Runs:   after InputSystem, before physics sync
Tests:  <file/name>  (headless)
```

## Definition of a "small enough" slice

- One new system **or** one new component **or** one new adapter function.
- Reviewable in < 30 minutes.
- Has its own test.
