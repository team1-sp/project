# Box2D v3.1.1 Research Notes (PB-008)

Status: **spike findings** for ClickUp PB-008 (Box2D v3 + raylib falling-box prototype). The units convention in section 3 is `Accepted` (DEC-036); everything else is a summary of how the pinned library behaves, checked against the v3.1.1 headers and the spike `[VERIFIED 2026-11-01]`. Code: `sandbox/src/falling_box_scene.cpp` (Box2D) and `sandbox/src/box2d_raylib.cpp` (raylib); tests: `tests/unit/sandbox/`.

Box2D is pinned at **v3.1.1** (DEC-020). box2d.org tracks the newest release, so check anything you read there against the pinned headers in `build/<preset>/_deps/box2d-src/include/box2d/` or the pinned docs in `_deps/box2d-src/docs/` (E-018).

## 1. The four ideas

| Idea | What it is | v3.1.1 API |
| --- | --- | --- |
| World | Owns every body, shape and joint; holds gravity; runs the simulation | `b2DefaultWorldDef()`, `b2CreateWorld(&def)`, `b2DestroyWorld(id)` |
| Body | A rigid body: position, rotation, velocity. No size or shape by itself. Type `b2_staticBody` (default, never moves), `b2_kinematicBody` (moved by velocity only), `b2_dynamicBody` (moved by forces, gravity and contacts) | `b2DefaultBodyDef()`, `b2CreateBody(worldId, &def)` |
| Shape | Geometry attached to a body: collision, density (mass), friction, restitution, event flags. A body can have several | `b2DefaultShapeDef()`, `b2MakeBox(halfW, halfH)`, `b2CreatePolygonShape(bodyId, &def, &polygon)` |
| Step | Advances the world by one time step: collision, integration, constraint solving | `b2World_Step(worldId, timeStep, subStepCount)` |

v3 is a C API. Every object is an **id** (`b2WorldId`, `b2BodyId`, `b2ShapeId`): a small value handle, not a pointer. Ids are copied freely; after an object is destroyed its id is stale, and `b2Body_IsValid(id)` / `b2Shape_IsValid(id)` tell you so. Reading state goes through functions: `b2Body_GetPosition`, `b2Body_GetRotation` (a `b2Rot`; `b2Rot_GetAngle` gives radians), `b2Body_GetLinearVelocity`, `b2Body_IsAwake`.

Minimal setup, as in the spike:

```cpp
b2WorldDef worldDef = b2DefaultWorldDef();
worldDef.gravity = b2Vec2{0.0f, -10.0f};
b2WorldId world = b2CreateWorld(&worldDef);

b2BodyDef boxDef = b2DefaultBodyDef();
boxDef.type = b2_dynamicBody;
boxDef.position = b2Vec2{12.5f, 12.0f};       // meters, y up
b2BodyId box = b2CreateBody(world, &boxDef);
b2Polygon square = b2MakeBox(0.5f, 0.5f);     // half extents: a 1 m x 1 m box
b2ShapeDef shapeDef = b2DefaultShapeDef();    // density 1, friction 0.6
b2CreatePolygonShape(box, &shapeDef, &square);

b2World_Step(world, 1.0f / 60.0f, 4);         // once per fixed step
```

A body's position is its origin. `b2MakeBox` centres the box on that origin, so for boxes the position is also the centre.

## 2. Step and substeps

- `timeStep` is our fixed `1/60 s` (R-5, DEC-004). The spike's loop adds the frame time to an accumulator and runs as many fixed steps as fit, so physics speed does not depend on the frame rate. The game loop owns the accumulator; the adapter only steps (`physics-adapter-design.md` section 4).
- `subStepCount` splits each step into smaller solver passes. **4 is Box2D's recommendation**: at 60 Hz that is 240 Hz inside the solver. More substeps make stacks and joints stiffer and more accurate but cost time; fewer are faster and softer. It is not the same as calling `Step` several times: contacts are found once per step, not once per substep.
- Same inputs give the same result: two scenes stepped side by side match bit for bit for 180 steps (`FallingBoxScene.TwoRuns_IdenticalTrajectory`).
- Bodies fall asleep after resting for a moment (`enableSleep` is on by default). A sleeping body costs nothing and wakes when touched or moved. The spike's box is asleep about 3 s after it lands.

## 3. Units and coordinates (DEC-036)

Decided by Zeynep Ertürk in PB-008.3:

| Item | Convention |
| --- | --- |
| World units | Meters, kilograms, seconds, radians. Box2D is tuned for moving objects of about **0.1 to 10 m**. |
| Pixels per meter | **32**. An 800 x 450 window shows 25 m x 14.06 m. |
| World axes | **y up**, angles counter-clockwise, as Box2D expects. Gravity is `(0, -10)`. |
| Screen axes | raylib's: pixels, origin top-left, **y down**, angles in degrees and clockwise. |
| Where conversion happens | **Only in the renderer.** Physics, ECS components and gameplay use meters and y up. |

Conversion (`sandbox/src/world_to_screen.hpp`, world origin at the bottom-left of the window):

```
xPixels = xMeters * 32
yPixels = screenHeightPixels - yMeters * 32
screenDegrees = -angleRadians * 180 / pi      // y flips, so the turn direction flips too
```

The camera (where the world origin sits on screen, zoom) is not decided; the spike's fixed bottom-left origin is only for the spike.

## 4. userData: from a body back to our entity

`b2BodyDef::userData` (and `b2ShapeDef::userData`) is a `void*` that Box2D stores and never reads; `b2Body_GetUserData(id)` returns it. The spike stores an entity id in it (`reinterpret_cast<void*>(std::uintptr_t{id})`) and reads it back in contact events. For the adapter:

- It is the cheap way to go body → entity in event handling. Entity → body still needs our own map.
- If it stores an entity id, that id must carry its generation, or a destroyed and reused entity is misreported (E-002).
- Do not store pointers into ECS storage there: storage can move when it grows.

## 5. Reading contact events

```cpp
b2World_Step(world, dt, 4);
b2ContactEvents events = b2World_GetContactEvents(world);
for (int i = 0; i < events.beginCount; ++i) {
    b2ShapeId a = events.beginEvents[i].shapeIdA;      // b2Shape_GetBody(a) -> userData -> entity
    b2ShapeId b = events.beginEvents[i].shapeIdB;
}
// also events.endEvents / endCount, events.hitEvents / hitCount
```

- Events are **pulled after the step**, not delivered by callbacks. The arrays are valid **only until the next step**, so copy what you need.
- Begin and end touch are reported only if **at least one** of the two shapes has `enableContactEvents = true`. **It is off by default in v3.1.1** (release notes: "Contact events are now disabled by default"); tutorials for v3.0 assume it is on. Hit events (impact speed) need `enableHitEvents`.
- On end events, check `b2Shape_IsValid`: the shape may have been destroyed.
- Sensors report through `b2World_GetSensorEvents` instead, and need `isSensor` plus `enableSensorEvents`.

## 6. Pitfalls found

| Pitfall | What happens | Fix |
| --- | --- | --- |
| Contact events off by default (v3.1.1) | No begin events at all. Turning the flag off in the spike makes `BoxLands_ContactEventNamesGroundAndBox` fail. | Set `enableContactEvents` on the dynamic shapes (E-026) |
| Defs not started from `b2Default*Def()` | A zero-initialised def fails Box2D's validity assert in debug builds; in release the assert is compiled out and the body silently gets zero friction, zero density and no gravity | Always `b2DefaultWorldDef()` / `BodyDef()` / `ShapeDef()`, then change fields (E-028) |
| Rotation sign | Box2D angles are counter-clockwise in y up; raylib's `rotation` turns clockwise on a y-down screen. Passing the angle unchanged mirrors every rotation | Negate when converting to degrees (E-027) |
| `DrawRectanglePro` origin | `rec.x, rec.y` is where `origin` is placed, and the rotation is around `origin`. With `origin = {0, 0}`, a body drawn at its centre position appears shifted and rotates around a corner | Use `origin = {w/2, h/2}` so `rec.x, rec.y` is the body centre (E-027) |
| Half extents | `b2MakeBox(halfWidth, halfHeight)`; passing full sizes doubles the body | Name variables `half...` or divide by 2 at the call |
| Pixels in Box2D | A 32-pixel box simulated as 32 m falls in slow motion | Meters everywhere except the renderer (E-005, DEC-036) |
| v2.4 tutorials | `b2World world(gravity)`, `body->GetPosition()` do not exist in v3 | Use the v3 id API (E-018) |

## 7. Running the spike

```
cmake --preset macos-debug          # or windows-debug
cmake --build --preset macos-debug --target box2d_raylib_spike
./build/macos-debug/sandbox/box2d_raylib_spike            # Windows: build\windows-debug\sandbox\Debug\box2d_raylib_spike.exe
ctest --preset macos-debug -R "FallingBoxScene|WorldToScreen"
```

The box is dropped from 12 m with a 0.3 rad tilt, lands on a corner, tips over and comes to rest flat with its centre at 1.50 m (ground top 1 m plus half the box), then sleeps.

## 8. What the adapter should take from this

- Wrap `b2WorldId` / `b2BodyId` in our own types; no Box2D header outside `engine/physics-box2d/` (R-3). The spike already hides Box2D behind a header that only uses our types.
- RAII: destroying the world destroys its bodies and shapes, so the adapter owns exactly one world and destroys it once.
- `Step(dt)` = `b2World_Step(world, dt, 4)` then copy contact events, translated to entity ids and sorted (`physics-adapter-design.md` section 5).
- Shapes that should report collisions need `enableContactEvents`; decide per collider type in the adapter design.

## Sources

- Box2D v3.1.1 docs shipped with the pinned source: `docs/hello.md` (Hello Box2D), `docs/simulation.md` (Simulation), `docs/release_notes_v310.md`; online at [box2d.org/documentation](https://box2d.org/documentation/pages.html) (newest version, not necessarily 3.1.1).
- Box2D v3.1.1 headers: `include/box2d/box2d.h`, `types.h`, `collision.h`, `math_functions.h`.
- raylib 6.2 `raylib.h` (`DrawRectanglePro`).

## Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-11-01 | Created for PB-008: API summary, units decision (DEC-036), findings from the spike | Zeynep Ertürk + claude (PB-008 session) |
