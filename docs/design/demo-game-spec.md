# Demo Game Specification

Purpose: a small game that exercises every engine feature **and** serves as ground truth for automated testing and the AI assistant (ip.fr.12.1, 12.2). Roadmap phase 9.

## 1. Contents (ip.fr.12.1)

Player, walls, a goal, a door, a key (golden key). A maze/platformer style 2D level.

```
 ┌──────────────────────────────────────┐
 │ W W W W W W W W W W W W W W W W W W │
 │ W P .  .  .  K  .  . D  .  .  . G W │   P player   K key   D door (needs K)
 │ W W W W W W W W W W W W W W W W W W │   G goal     W wall
 └──────────────────────────────────────┘
```
(Illustrative only; real layout not decided.)

## 2. Entities and components

| Entity | Components |
| --- | --- |
| Player | Position, Velocity, Collider(dynamic), Sprite, Inventory, Health(optional), PlayerControlled |
| Wall | Position, Collider(static), Sprite |
| Key | Position, Collider(sensor), Sprite, Item(`GoldenKey`) |
| Door | Position, Collider(static while closed), Sprite, DoorState, RequiredItem(`GoldenKey`) |
| Goal | Position, Collider(sensor), Sprite |

## 3. Rules (gameplay systems)

- Walk with keyboard (or agent actions). Collide with walls.
- Touch key → pick up, added to Inventory, key entity removed (`KeyPickedUpEvent`).
- Press Interact near door: with `GoldenKey` → `DoorOpenedEvent`, `IsOpen=true`, collider removed/disabled; without → `DoorOpenFailedEvent` (ip.fr.6.3, 6.4).
- Reach goal → `GoalReachedEvent` (ip.fr.6.5).

## 4. Injectable bugs (ground truth, ip.fr.12.2, ip.fr.9.2)

Selected by configuration flag; default build has none.

| Flag | Bug | Expected detection |
| --- | --- | --- |
| `bug.missingWallCollider` | One wall segment has no collider | `B-NO-COLLIDER` |
| `bug.doorWithoutKey` | Door opens even if inventory lacks key | `B-DOOR-NO-KEY` |
| `bug.unreachableGoal` | Goal placed behind solid walls | `B-GOAL-UNREACHABLE` |
| `bug.playerExitsMap` | Hole in boundary wall | `B-OUT-OF-BOUNDS` |

Every detection must report class, frame number and entity IDs (ip.fr.9.3). Each flag has at least one automated scenario test that expects exactly that detection, and a "no bug" scenario that expects none (no false positives).

## 5. Used for

- Integration tests (full chain).
- Automated Playtest Agent training/evaluation.
- Semantic graph acceptance (`semantic-layer.md` §5).
- AI assistant evaluation (`ai-assistant-layer.md` §7).
- Performance requirement ip.nfr.1.2 (≥ 600 headless steps/s) and ip.nfr.1.1 (≥ 60 FPS, ≤ 200 entities).

## 6. Do not

- Add features to the demo that the engine does not need to demonstrate.
- "Fix" an injected bug in code; flags control them.
