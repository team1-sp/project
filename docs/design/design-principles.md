# Design Principles

Principles that guide every design choice. When two conflict, the earlier one wins.

1. **Observability over cleverness.** Every state change and event must be inspectable. If a design hides data from telemetry, redesign it. The product's value is that the AI can see inside the engine.
2. **Data, not objects.** Components are plain data; systems are functions over data (R-8). No inheritance hierarchies for game objects.
3. **One owner per piece of state.** For each component field, exactly one system writes it per frame (documented in the system's `writes` set). Multiple writers need an explicit order and a reason.
4. **Determinism.** Same inputs → same results (ip.fr.7.3). No wall-clock reads, unordered iteration that affects results, or uncontrolled randomness (seeded RNG only) inside the simulation.
5. **Simulation never depends on presentation.** Nothing in input→gameplay→telemetry may require a window/GPU (R-6).
6. **Seams at the right places.** Abstractions exist where we know we may swap or test: Physics Adapter, rendering abstraction, input action interface, telemetry sink. Do not abstract anything else "just in case".
7. **Declare, don't infer.** Systems declare the components they read and write; components and events have documented schemas. The semantic graph is built from declarations plus runtime evidence.
8. **Smallest thing that works for a 2D MVP.** Prefer the practical solution over theoretically perfect (student project, fixed timeline). Add 3D-ready seams, not 3D features.
9. **Fail loudly in development.** Assertions on invariants (e.g. entity has body but no collider, body without entity). Silent fixes hide bugs from the playtest agent and the AI.
10. **Errors are data.** Detected problems become events/telemetry records (bug class, frame, entity IDs — ip.fr.9.3), not just log lines.

## Review questions for any design

- What are the `reads`, `writes`, `emits` of each new system?
- Where in the frame order does it run, and why there?
- What does it add to telemetry, and what does it cost (ip.nfr.1.3)?
- Does it work identically in headless mode?
- Which library/framework types does it leak, and where?
- How is it tested without a window?
- What will the semantic graph be able to say about it?

## Design doc process

Non-trivial designs get a short design doc from [templates/design-doc-template.md](templates/design-doc-template.md), saved in `design/`, linked from this folder's list in the docs README, and any technology/architecture choice inside it logged in `history/decision-log.md`.
