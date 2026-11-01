# Component Conventions

Status: **units and axes `Accepted` (DEC-036, 2026-11-01)**; everything else about components is designed in [design/ecs-design.md](design/ecs-design.md), which lists the components themselves. This page holds the conventions every component must follow, so they live in one place. Created for ClickUp PB-008.3.

## 1. Units and axes (DEC-036)

| Quantity | Unit in components | Notes |
| --- | --- | --- |
| Position, size, distance | meters | Box2D is tuned for moving objects of about 0.1 to 10 m |
| Velocity | meters per second | |
| Angle | radians, counter-clockwise | as Box2D reports it |
| Angular velocity | radians per second | |
| Mass | kilograms | |
| Time | seconds | the simulation step is a fixed 1/60 s (R-5) |

- The world is **y up**: gravity is `(0, -10)` m/s².
- Components never store pixels or screen coordinates. **Only the renderer converts** to raylib's screen (pixels, y down, degrees clockwise), with **32 pixels per meter**. Physics, gameplay and telemetry use the component values unchanged.
- Name fields or document them with their unit (`coding-standards.md` section 1, rule 11).

Conversion used by the renderer (world origin at the bottom-left of the window in the PB-008 spike; the camera is not decided yet):

```
xPixels       = xMeters * 32
yPixels       = screenHeightPixels - yMeters * 32
screenDegrees = -angleRadians * 180 / pi
```

## 2. Related

- Why and how this was chosen, with the Box2D findings: [research/box2d.md](research/box2d.md).
- The components, who writes and reads them: [design/ecs-design.md](design/ecs-design.md) section 2.
- Where the physics adapter converts nothing: [design/physics-adapter-design.md](design/physics-adapter-design.md) section 4.
- Pitfalls: [troubleshooting/common-errors.md](troubleshooting/common-errors.md) E-004, E-005, E-025, E-027.

## Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-11-01 | Created with the units and axes decision (DEC-036, PB-008.3) | Zeynep Ertürk + claude (PB-008 session) |
