# Coding Standards

**Status: Accepted (2026-10-23).** Agreed by the team and approved by the advisor (DEC-031, PB-005). Changes go through a PR and a new decision entry. Amended 2026-11-05 for the ECS design (DEC-044).

Language is **C++** (DEC-013). Physics is Box2D v3.1.1 behind the Physics Adapter (DEC-014, DEC-020); graphics is Raylib behind the rendering abstraction (DEC-015); 2D only (DEC-016); ECS is custom (DEC-018). **Platforms (DEC-029):** the project is developed on Windows and macOS, Windows has priority; games are built for Windows only for now. **Standard: C++20 (DEC-019). Raylib 6.2 (DEC-033).** **Compilers (DEC-030):** whatever each platform needs, none pinned — no compiler-specific extensions. **Build system CMake and test framework GoogleTest (DEC-023).** **Formatter: clang-format 19** with the repo's [`.clang-format`](../../.clang-format) (DEC-031).

## 1. Universal rules

1. **Respect the invariants** in `agents/rules.md` section A (layers, ECS hub, adapter, fixed timestep, headless).
2. **Components are plain data.** No behaviour, no references to systems, adapters or framework types.
3. **Systems are small** and declare everything they touch in `describe()`: reads, writes, publishes, subscribes, entity creation and destruction, enable changes and random use. The declarations are checked in every build (`design/ecs-api-design.md` §10).
4. **No global mutable state** in the simulation, including static members of systems. Pass the `World` (or needed parts) explicitly; several Worlds may run in one process (D-23).
5. **No wall-clock time in simulation.** Only the fixed step, `ctx.fixedDeltaSeconds()`. Randomness only from the World's seeded gameplay generator, `ctx.random()` (PCG32, srs.fr.7.5), by systems that declare `usesRandom()` (DEC-042).
6. **Deterministic iteration**: queries iterate in ascending entity ID; never depend on hash-map order.
7. **No allocations in per-tick hot paths** where avoidable (allocation spikes show up in tick-time telemetry). Pre-size buffers.
8. **Library types stay behind seams**: physics types only in the adapter implementation; graphics-framework types only in the rendering implementation.
9. **Fail loudly in every build**: check invariants (entity↔body mapping, valid entity handles, component exists before access) with checks that stay on in release builds, not `assert()`. An unrecoverable violation throws `engine::RunError` (DEC-042).
10. **Comments explain why**, not what. Public interfaces get a short doc comment: purpose, units, ownership, threading assumptions.
11. **Units are explicit** in names or docs: `positionMeters`, `dtSeconds`, or documented per type. World units are meters with y up; pixels exist only in the renderer (DEC-036).
12. **Errors**: engine code does not swallow errors. Unrecoverable run errors throw `engine::RunError`, which only the game loop catches (log message and tick, Error Pause in the Editor, non-zero exit headless, srs.fr.1.11). Recoverable problems are logged warnings. Always log with context: tick, `EntityId`, system, type.

## 2. Naming (language-neutral)

| Thing | Convention | Example |
| --- | --- | --- |
| Types (classes, structs, enums) | `PascalCase` | `ComponentPool`, `TickPhase` |
| Component types | `<Name>Component`; registered `typeName` is `<Name>` | `PositionComponent`, `typeName = "Position"` |
| Systems | class `<Name>System`; registered name `<Name>` | `PlayerMovementSystem`, `d.name("PlayerMovement")` |
| Events | `<Name>Event` (past tense) | `ItemCollectedEvent` |
| Tags | `<Name>Tag`: an empty struct with no storage, used only as a filter (D-17). An empty type that needs its own enabled flag, like an SRS role, is a component | `PrefabTag`; `ObjectiveComponent` |
| Functions and local variables | `camelCase` | `createEntity`, `fixedDeltaSeconds` |
| Private data members | `camelCase` with a trailing underscore | `sparse_`, `nextRuntimeId_` |
| Component and event fields | `camelCase`, no underscore, unit in the name when it has one | `xMeters`, `speedMetersPerSecond` |
| Namespace | `engine` for engine code | `engine::World` |
| Interfaces | purpose-based, not `Manager` | `IPhysicsAdapter` |
| Tests | `<Unit>_<Condition>_<Expected>` | `PlayerMovementSystem_MoveRightHeld_SetsPositiveVelocity` |
| Branches | see git-workflow | `feat/pb-026-box2d-world-fixed-step` |

Avoid vague names: `Manager`, `Helper`, `Util`, `Data`, `Handler` unless truly generic.

## 3. File and module organisation

See [repository-structure.md](repository-structure.md). One concept per file; header/implementation pairing per language convention. Layer folders must not include each other's headers (Engine never includes Semantic/AI).

## 4. C++ rules

- Standard: **C++20** (DEC-019); do not use C++23 features; the project must build on Windows and macOS (Windows priority, DEC-029), so keep platform-specific APIs out of engine code and ask before adding any.
- Ownership: `std::unique_ptr` by default; no owning raw pointers; IDs or references for non-owning links. Entities are IDs, not pointers — this avoids dangling pointers.
- RAII for every resource (physics bodies/worlds, Raylib textures/shaders/fonts, windows). Raylib resources must be unloaded before `CloseWindow`.
- Components and events are aggregates with a `static constexpr std::string_view typeName`, trivially copyable where possible (unmanaged, D-14). No raw C arrays as members; use `FixedArray<T, N>` and `FixedString<N>` (the registry counts members, `design/ecs-api-design.md` §11).
- Prefer `enum class`, `std::span`/`string_view`, `constexpr`; avoid macros.
- Box2D and Raylib headers are included **only** in the adapter implementation and the rendering implementation respectively (R-3, R-4). Wrap C-style handles (Box2D v3 ids, Raylib structs) in our own types.
- Raylib is a C API: its functions need a window/GL context for textures, shaders and fonts. Never call them from simulation code or in headless mode.
- Exceptions: only `engine::RunError` for unrecoverable run errors, caught only by the game loop (DEC-042). No exceptions across the adapter boundary.
- Formatting: **clang-format 19** with the checked-in [`.clang-format`](../../.clang-format) (DEC-031): 4-space indent, 100 columns, braces on the same line, braces always on `if`/`for`/`while`, includes grouped standard library / third-party / ours. Install the same version everyone uses with `pip install clang-format==19.1.7` and turn on format-on-save in your editor. CI fails a PR whose C++ files are not formatted. Other files follow [`.editorconfig`](../../.editorconfig).
- Warnings on, treated as errors where practical; sanitisers (ASan/UBSan) on debug test builds if the toolchain supports them.

## 5. 2D only

No 3D types, cameras, models or physics (DEC-016). Use 2D vector/transform types of our own (`Vec2`, …) at the engine boundary so a future 3D extension can add types rather than rewrite existing ones.

## 6. Dependencies

- Every new dependency needs a decision entry (S-5) with: purpose, license, version pinned, maintenance status, size.
- Pin versions. Do not use "latest".
- Prefer fewer, well-documented dependencies (documentation quality is a selection criterion).

## 7. Security and hygiene

- No secrets in the repo; `.env`/API keys git-ignored (S-1).
- AI layer: no outbound calls without explicit configuration (ip.nfr.2.1).
- Validate any file loaded from disk (level, telemetry import) — fail with a clear message.

## 8. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Created | claude-code/sonnet-5.5/012r |
| 2026-10-13 | C++ decided (DEC-013); C# section removed; Box2D/Raylib rules added | claude-code/sonnet-5.5/012r |
| 2026-10-13 | C++20/platforms proposed, not decided (DEC-019) | claude-code/sonnet-5.5/012r |
| 2026-10-15 | C++20 (DEC-019), Raylib v6.0 (DEC-021) accepted; platforms/compilers decided | claude-code/sonnet-5.5/012r |
| 2026-10-23 | Accepted by the team and advisor; formatter clang-format 19 with `.clang-format` (DEC-031, PB-005) | claude-code (session 0171) |
| 2026-10-24 | Raylib 6.2 (DEC-033) | claude-code (PB-002 session) |
| 2026-11-01 | World units decided (DEC-036) | claude (PB-008 session) |
| 2026-11-05 | ECS amendments (DEC-044): describe() declarations, RunError instead of dev-only asserts, gameplay RNG and fixed step, naming rows for functions, members, fields, namespace and tags | claude (PR #28 review) |
