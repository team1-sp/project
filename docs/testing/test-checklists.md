# Test Checklists

Source: Senior Project Guide "Testing Checklist", extended with requirement IDs. Tick items by linking the test name (`[x] test name — file`). Items stay in this file; progress is also visible in tests.

## ECS (ip.fr.2.*)

- [ ] Entity creation returns unique IDs (2.1)
- [ ] Entity deletion removes it from queries; stale handle detected (2.1)
- [ ] Component add / remove (2.2)
- [ ] Component read / modify (2.2)
- [ ] System selects the correct entities for its query (2.4) — e.g. MovementSystem selects Player/Enemy, not Wall/Door
- [ ] Systems run once per frame in defined, repeatable order (2.3)
- [ ] Query iteration order deterministic
- [ ] Structural change during iteration is safe/deferred

## Physics via adapter (ip.fr.3.*)

- [ ] Dynamic body is affected by gravity (3.3)
- [ ] Static body does not move (3.2)
- [ ] Kinematic body moves only by code (3.2)
- [ ] Collider produces a collision (3.2)
- [ ] Collision event generated with correct entity IDs (3.6)
- [ ] Raycast finds the correct entity / none (3.7)
- [ ] Fixed timestep consistent: two runs with the same inputs give identical trajectories (1.2)
- [ ] Entity↔body mapping correct both ways (3.4)
- [ ] Write-back: PositionComponent == body position after Step (3.5)
- [ ] DestroyBody on entity destruction leaves no orphan body

## Rendering data (ip.fr.4.*)

- [ ] Sprite drawn at the entity's PositionComponent (4.1)
- [ ] Sprite position equals physics body position (3.5 + 4.1)
- [ ] Door sprite matches DoorState (open/closed) (4.2)
- [ ] RenderSystem does not modify simulation state (4.3) — compare state hash before/after
- [ ] Camera follows selected entity (4.4, should)
- [ ] Simulation continues when rendering is disabled (7.1)

## Gameplay (ip.fr.6.*)

- [ ] Movement input updates VelocityComponent (6.1)
- [ ] Key pickup adds to Inventory (6.2)
- [ ] Door opens with key + Interact in range → DoorOpenedEvent, IsOpen=true (6.3)
- [ ] Door stays closed without key → DoorOpenFailedEvent (6.4)
- [ ] Goal reached → GoalReachedEvent (6.5)
- [ ] Physics code never decides door state (6.6) — review check

## Integration chain

- [ ] Input → ECS → Physics → ECS → Rendering: each link asserted separately
- [ ] Walk right into wall: velocity set → body moves → collision event → position stops → render data matches

## Headless (ip.fr.7.*)

- [ ] No window and no render function invoked (7.1)
- [ ] Accepts agent input stream (7.2)
- [ ] Same results as normal mode for identical inputs (7.3)
- [ ] Run limited by max frames or terminated by goal/failure event (7.4)
- [ ] Test report produced (outcome, frames, bugs) (7.5)

## Telemetry (ip.fr.8.*)

- [ ] Per-frame state recorded (8.1)
- [ ] Events recorded with frame number (8.2)
- [ ] Frame and physics-step timings recorded (8.3)
- [ ] Export file matches documented schema (8.4)
- [ ] Recording can be switched off (8.5); off ⇒ no overhead/alloc
- [ ] Overhead ≤ 10% (ip.nfr.1.3)

## Bug detection (ip.fr.9.*, 12.2)

- [ ] `B-NO-COLLIDER`, `B-DOOR-NO-KEY`, `B-GOAL-UNREACHABLE`, `B-OUT-OF-BOUNDS` each detected with class, frame, entity IDs
- [ ] Clean build: zero detections (no false positives)

## Semantic graph (ip.fr.10.*)

- [ ] Nodes for entities, components, systems, events (10.1)
- [ ] Edges: has, reads/writes, collidedWith, triggered (10.2)
- [ ] Built from telemetry only (10.3)
- [ ] Query interface answers the acceptance sketch (10.4)
- [ ] 10,000-frame recording processed < 60 s (ip.nfr.1.4)

## AI assistant (ip.fr.11.*)

- [ ] Answers "Why did the door not open?" with cited frames/entities (11.2, 11.3)
- [ ] Says "unavailable" when evidence is missing (11.5)
- [ ] Never modifies engine/source (11.6)
- [ ] No external transmission unless enabled (ip.nfr.2.1)
- [ ] Response time target (ip.nfr.1.5)

## Performance (ip.nfr.1.*) — reference hardware `[TBD]`

- [ ] ≥ 60 FPS average with up to 200 entities, normal mode (1.1)
- [ ] ≥ 600 simulation steps/s headless, demo game (1.2)
