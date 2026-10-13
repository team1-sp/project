# Glossary — Canonical Terminology

Use these terms consistently in code, comments, docs and prompts. Do **not** substitute synonyms.

## Architecture terms

| Term | Definition |
| --- | --- |
| **Physics library** | The external library (Box2D, DEC-014) that performs physics calculations. |
| **Physics engine / physics layer** | The engine's physics subsystem, which wraps the physics library via an adapter. |
| **Physics adapter / middleware** | The abstraction layer between ECS/engine and the physics library. |
| **Graphics framework** | The external framework (Raylib, DEC-015) used for rendering. |
| **Rendering layer / system** | The engine's rendering subsystem. |
| **ECS** | Entity Component System — the data architecture pattern. |
| **Entity** | Unique ID representing a game object. No data or behaviour. |
| **Component** | Pure data container in ECS. |
| **System** | Logic that processes entities with specific components. |
| **World** | Container managing all entities, components and systems. |
| **Query** | Filter selecting entities by component composition. |
| **Scene** | Container for the lights, sound, environment, UI and entities of one part of the game. |
| **Game loop** | The main update cycle of the engine. |
| **Telemetry** | Runtime state, event and performance data recorded each frame. |
| **Headless mode** | Running the engine without rendering (no window), for automated testing. |
| **Semantic graph** | Structured representation of engine runtime data for AI consumption. |
| **Semantic Architecture Layer** | Middle layer: data, graph, relationships. |
| **AI Assistant Layer** | Top layer: architecture reading, LLM model, user assistant. |
| **Development agent** | An AI model helping to build the product (see `agents/`). |
| **Product agent** | A runtime component of the product: Automated Playtest Agent or AI Assistant Layer. |
| **Automated Playtest Agent** | Product component (RL agent) that controls the player in headless mode to find bugs. |
| **Ground-truth bug** | A bug deliberately injected into the demo game by configuration, used to validate detection. |
| **Chain** | Input → ECS → Physics → ECS → Rendering; each link must be observable. |

## Physics concepts

| Term | Definition |
| --- | --- |
| Rigid body | Object with mass, position, velocity and collision shape whose shape does not deform. |
| Static body | Does not move (walls, ground). |
| Dynamic body | Simulated by forces and gravity (player, projectiles). |
| Kinematic body | Moved by code, not by forces (moving platforms). |
| Collider / shape | Simplified invisible mathematical boundary used for collisions (box, circle, polygon). Need not equal the visual model. |
| Gravity | Constant downward force. |
| Force | Push/pull applied over time. |
| Impulse | Instantaneous change in velocity. |
| Collision detection | Checking whether two shapes overlap or touch. |
| Broad phase | Cheap approximate test that eliminates pairs that cannot collide. |
| Narrow phase | Precise test for pairs that passed the broad phase. |
| Collision normal | Direction of the collision response. |
| Contact point | Where two colliders touch. |
| Penetration depth | How much two colliders overlap. |
| Restitution | Bounciness: energy preserved after collision. |
| Friction | Resistance to sliding. |
| Joint / constraint | Connection limiting relative motion of two bodies (hinge, spring, distance). |
| Raycast | Invisible ray from a point in a direction, reporting what it hits. |
| Fixed timestep | Physics updates at constant intervals (1/60 s) regardless of render FPS. |

## Rendering concepts

| Term | Definition |
| --- | --- |
| Rendering | Converting game data to images (GPU maths → 2D picture). |
| Polygon / mesh | Basic flat shape (usually triangle) / collection of vertices, edges, faces forming a model. |
| Bitmap | Pixel-grid image data. |
| Sprite | A 2D bitmap used for a character, item, bullet, etc. |
| Texture | Image applied to a surface. |
| Shader | GPU program (vertex / fragment) describing how an object interacts with light, camera, effects. |
| Material | Configuration describing which shader and which parameters render an object. |
| Framebuffer | Buffer holding the image before it is presented on screen. |

## Names used in the examples

Entities: `PlayerEntity`, `WallEntity`, `DoorEntity`, `GoalEntity`, `EnemyEntity`, key (`GoldenKey`).
Components: `PositionComponent`, `VelocityComponent`, `ColliderComponent`, `SpriteComponent`, `InventoryComponent`, `HealthComponent`, `DoorStateComponent`, `RequiredItemComponent`.
Systems: `InputSystem`, `MovementSystem`, `PhysicsSystem`, `CollisionSystem`, `GameplaySystem`/`InteractionSystem`, `RenderSystem`, `TelemetrySystem`.
Events: `CollisionEvent`, `DoorOpenedEvent`, `DoorOpenFailedEvent`, `GoalReachedEvent`.
