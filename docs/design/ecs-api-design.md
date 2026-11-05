# PB-010.2 — ECS API Design
---

## 1. Scope

| Task item (PB-010.2) | Section |
| --- | --- |
| `Entity {uint32 index, uint32 generation}` | §4 |
| `World::createEntity / destroyEntity / isAlive` | §5 |
| `add<T> / remove<T> / get<T> / tryGet<T> / has<T>` | §5, §6 |
| `each<Ts...>(fn)` query | §7 |
| `ISystem {name, stage, reads, writes, update}` | §8 |
| `CommandBuffer` | §9 |
| Reflection for components | §11 |
| How metadata is exported | §12 |
| Event bus (added on request) | companion file |
| Agent I/O for the AI bridge (review of PR #28) | §16 |
| What is built this semester | §17 |

**Not designed here** (each needs its own document; the API leaves room for them): prefabs and variants (srs.fr.2.29–2.35, 2.69–2.128), the `.scene` file format and loading (2.19–2.21), data assets beyond their reflection hook (2.22–2.28), project-script DLL loading, reload and placeholders (2.36–2.41, 2.103–2.106, 2.121), Play-mode snapshot and Editor undo (2.108), Physics Adapter internals, the renderer.

## 2. Design decisions

| # | Topic | Decision | Main reason |
| --- | --- | --- | --- |
| D-1 | Storage | Sparse sets: one independent pool per component type, no groups | PB-010.1 research |
| D-2 | Identity | Two IDs: `Entity` handle `{index, generation}` for runtime storage; 64-bit `EntityId` (srs.fr.2.107) as identity. References, events, telemetry and scenes store `EntityId` | Task text + SRS |
| D-3 | Order | Every query iterates in ascending `EntityId`. Pools use swap-and-pop during command playback and each pool touched is **sorted once at the end of playback**, so pools are always sorted when systems run | Coding standard rule 6 (DEC-031); a sorted insert per command would shift the pool on most gameplay changes (§6) |
| D-4 | Structural changes | Systems and event handlers record them in a `CommandBuffer`, played back **at the end of each tick phase, after event dispatch**. Direct `World` calls only outside system execution | srs.fr.2.119, E-003 |
| D-5 | Create during a tick | `commands.createEntity()` assigns handle and `EntityId` **now**; components appear at playback | Deterministic IDs, usable references |
| D-6 | Reflection | Registration builder with member pointers, no macros. Each field is stored as `{offset, FieldType, count}`; only managed fields get type-erased accessors | Coding standard §4 "avoid macros"; per-tick telemetry, snapshots and the AI bridge read fields without indirect calls (§11) |
| D-7 | Type key | `static constexpr std::string_view typeName` in every ECS type, hashed at compile time | Stable across runs and DLLs (skypjack part 8) |
| D-8 | Metadata | SRS minimum (srs.fr.2.12); "read/written by" is **derived** from system declarations; Q-013 extras later | No duplicated declarations |
| D-9 | Export | In-memory introspection → `Schema`; the telemetry writer puts it as the first record of every telemetry file | R-1: the semantic layer only reads files |
| D-10 | Ordering | Full SRS model: phase, named groups, before/after rules, stable topological sort, cycles refused | srs.fr.2.48–2.52 |
| D-11 | Access check | Declared reads/writes are checked **in every build**; a violation **stops the run** | Semantic Graph trusts declarations |
| D-12 | Entity data | `World` keeps `EntityId`, generation, enabled flag; name, tag, layer, hierarchy, source scene are built-in components | Same storage, reflection, saving |
| D-13 | Component enabled flag | Parallel flag array in each pool; Position and Velocity have none | srs.fr.2.11 |
| D-14 | Component kinds | **Unmanaged** (trivially copyable) and **managed** (owns heap memory), auto-detected | Unity-like split |
| D-15 | Query API | `each<Ts...>(fn)` plus `query<Ts...>()` builder with `with / without / withTag / withLayer` | srs.fr.2.57 |
| D-16 | Singletons | Singleton components on engine-created, runtime-only entities; `single<T>()` | Unity style |
| D-17 | Tags | `<Name>Tag`, empty structs, no storage | Coding standard §2 |
| D-18 | Dead references | Cleared **eagerly** through reflection when destructions are played back | srs.fr.2.44 |
| D-19 | Naming | camelCase functions, trailing-underscore members, namespace `engine` | Matches merged code |
| D-20 | Event bus | Full API, separate document | Size guideline |
| D-21 | Structural power | Systems declare `createsEntities`, `destroysEntities` and `togglesEnabled`; checked like reads/writes and exported | The Semantic Graph can answer "which system can destroy this?" (§10) |
| D-22 | Handler phase | The schema records, per subscription, the phase in which the handler actually runs (the publisher's) | A handler's writes happen there, not in the subscriber's own phase (§12) |
| D-23 | Several Worlds | Many independent `World`s per process; systems are registered as factories and hold no static mutable state | Vectorized RL and simultaneous games (ADR-002, §16) |
| D-24 | Snapshot | `World::snapshot()` / `restore()` is a requirement of the ECS, even though its design is a separate document | RL episode `reset()` and Play mode (§16) |
| D-25 | Agent I/O | Actions enter as input in `InputApplication`; observations are built by a declared system; the bridge runs only between ticks | Keeps access checks, determinism and replay valid for agent runs (§16) |
| D-26 | Build phases | This semester builds the Phase 1 slice only (§17); everything else keeps its API slot | Human decision 2026-11-05: about 20 % of the project is built this semester |

## 3. Overview

```
TypeRegistry ── describes ──► component, tag, event, enum, system types (names, fields, units)
     │
     ▼
World ─┬─ EntityTable        index → {generation, EntityId, enabled};  EntityId → index
       ├─ ComponentPool<T>   one sorted sparse set per registered component type
       ├─ SystemScheduler    ISystem instances, run order per tick phase, timings
       ├─ CommandBuffer      structural changes recorded during a phase
       └─ EventBus           (companion document)
                │ each system runs with
                ▼
          SystemContext ──── access-checked view: queries, get, commands, publish/subscribe

buildSchema(TypeRegistry, SystemScheduler) ──► Schema ──► telemetry writer ──► file ──► Semantic layer
```

---

## 4. Entity identity (D-2)

```cpp
namespace engine {

// Runtime handle: an index into the World's entity table plus the generation of that slot.
// Valid only in the World that created it. Never saved, never written to telemetry.
struct Entity {
    static constexpr std::uint32_t invalidIndex = 0xFFFF'FFFF;
    std::uint32_t index = invalidIndex;
    std::uint32_t generation = 0;
    friend bool operator==(Entity, Entity) = default;
};
inline constexpr Entity nullEntity{};

// The entity's identity (srs.fr.2.107). Never reused in a run or editing session (srs.fr.2.2).
// 0 = no entity (empty reference); 1 … 2^32 − 1 Editor range; 2^32 and above runtime range.
enum class EntityId : std::uint64_t {};
inline constexpr EntityId noEntity{0};
inline constexpr EntityId firstRuntimeEntityId{std::uint64_t{1} << 32};

} // namespace engine
```

`EntityId` is an `enum class` so it cannot be mixed up with other integers, and reflection can recognise reference fields by their type.

**Entity table** (inside `World`):

```
slots_    : index → { generation, EntityId id, bool enabled, bool queryEnabled, bool alive }
runtimeIdToSlot_ : std::vector<std::uint32_t>   runtime EntityId − firstRuntimeEntityId → index (dense)
editorIdToSlot_  : sorted std::vector<{EntityId, std::uint32_t}>   Editor-range IDs, binary search
freeList_ : std::vector<std::uint32_t>     destroyed indices, reused last-in first-out
nextRuntimeId_ : EntityId                  reset to firstRuntimeEntityId at every run start
```

`find(id)` is on hot paths (hierarchy walks every tick, event handlers), so it uses no hash map. Runtime IDs are dense from `firstRuntimeEntityId`, so they index a plain vector. Editor IDs change only at scene load and Editor edits, so a sorted vector is enough.

- **Create:** take an index from the free list or append one; record the `EntityId` (given by the caller or the next runtime ID).
- **Destroy:** increment the slot's generation, so every handle to it becomes stale; put the index on the free list. A slot whose generation would wrap past 2³² − 1 is retired instead of reused.
- **isAlive(e):** `e.index < slots_.size() && slots_[e.index].alive && slots_[e.index].generation == e.generation`.
- **Runtime IDs** (srs.fr.2.65): increasing from `firstRuntimeEntityId` in every run, so runs of the same content give the same IDs. **Editor IDs** (srs.fr.2.64, the watermark) are allocated by the Editor and passed in; the ECS only checks they are unused. ID collisions on scene load (srs.fr.2.66) are the scene loader's job; it uses `find()` and `allocateRuntimeId()`.
- Handle indices are reused in any order without affecting determinism: nothing observable depends on them, because pools are ordered by `EntityId` (D-3).

## 5. World API

```cpp
class World {
public:
    explicit World(const TypeRegistry& types);

    // Entities. Direct calls are for loading, the Editor and tests; systems use the
    // CommandBuffer (§9). Calling these while a system runs is an error (§10).
    Entity createEntity();                        // next runtime EntityId (srs.fr.2.65)
    Entity createEntity(EntityId id);             // explicit ID (scene load, Editor)
    void destroyEntity(Entity e);                 // with all descendants (srs.fr.2.4)
    bool isAlive(Entity e) const;
    std::optional<Entity> find(EntityId id) const;
    EntityId idOf(Entity e) const;
    EntityId allocateRuntimeId();
    void setEnabled(Entity e, bool enabled);      // not structural (srs.fr.2.119)
    bool isEnabled(Entity e) const;
    void setParent(Entity child, Entity parent);  // nullEntity makes it a root

    // Components. At most one of each type per entity (srs.fr.2.10).
    template <ComponentType T> T& add(Entity e, T value = T{});  // error if present
    template <ComponentType T> void remove(Entity e);             // error if absent
    template <ComponentType T> bool has(Entity e) const;
    template <ComponentType T> T& get(Entity e);                  // error if absent
    template <ComponentType T> const T& get(Entity e) const;
    template <ComponentType T> T* tryGet(Entity e);               // nullptr if absent
    template <ComponentType T> const T* tryGet(Entity e) const;
    template <ComponentType T> void setComponentEnabled(Entity e, bool enabled);
    template <ComponentType T> bool isComponentEnabled(Entity e) const;

    // Singletons (D-16).
    template <ComponentType T> Entity createSingleton(T value = T{});
    template <ComponentType T> T& single();       // error unless exactly one entity has T

    // Queries (§7), systems (§8), events (companion), observers (§9).
    template <typename... Ts, typename Fn> void each(Fn&& fn);
    template <typename... Ts> Query<Ts...> query();
    SystemScheduler& systems();
    EventBus& events();
    void addObserver(IWorldObserver& observer);

    // Runs.
    void beginRun();                              // resets the runtime ID counter
    void endRun();                                // drops queued events (srs.fr.2.102)

    // Snapshots (D-24). Design of the format, Play mode and Editor undo: separate document.
    WorldSnapshot snapshot() const;               // entity table, pools, ID counters, event queues
    void restore(const WorldSnapshot& s);         // observers get onRestored() (Physics Adapter rebuilds bodies)

    // Type-erased access for telemetry, scene saving and the Inspector (§12).
    const TypeRegistry& types() const;
    const IComponentPool* pool(TypeId type) const;
    void forEachComponent(Entity e, const std::function<void(const ComponentTypeInfo&,
                                                             const void*)>& fn) const;
};
```

**Errors.** Precondition violations (stale handle, duplicate add, missing component) fail loudly (coding rule 9): they are reported with the `EntityId`, the type name and the tick and handled like an access violation (§10).

**Several Worlds** (D-23). A process may hold any number of `World`s that share one finalized `const TypeRegistry&`. Nothing in the ECS is global or static and mutable. Each World creates its own system instances from the registered factories (§8), so a system's member state belongs to one World.

**Reference lifetime.** A `T&` from `get`/`each` stays valid until the next structural change to `T`. Systems never make structural changes directly, so inside a phase every reference stays valid until that phase's command playback. Systems keep entity handles or `EntityId`s, never component pointers.

**Built-in components** (D-12). Every entity gets these at creation and they cannot be removed:

```cpp
struct NameComponent {                 // unmanaged, so every built-in pool can be copied and exported raw
    static constexpr std::string_view typeName = "Name";
    FixedString<64> name;              // UTF-8, longer names are refused by the Editor
};

struct EntityInfoComponent {           // unmanaged
    static constexpr std::string_view typeName = "EntityInfo";
    TagId tag = untaggedTag;           // one value from the project's tag list (srs.fr.2.53)
    std::uint8_t layer = 0;            // 0 … 31, 0 = Default (srs.fr.2.54, 2.82)
    AssetId sourceScene{};             // srs.fr.2.85
};

struct HierarchyComponent {            // unmanaged; links are EntityIds (D-2)
    static constexpr std::string_view typeName = "Hierarchy";
    EntityId parent = noEntity;
    EntityId firstChild = noEntity;
    EntityId nextSibling = noEntity;   // sibling order is kept (srs.fr.2.32)
    EntityId prevSibling = noEntity;
    std::uint32_t childCount = 0;
};
```

`HierarchyComponent` is changed only through `setParent`, which keeps both sides of the links consistent (skypjack part 4, "unconstrained model"). `TagId` and `AssetId` come from the project-settings and asset designs `[TBD]`.

## 6. Storage: `ComponentPool<T>` (D-1, D-3, D-13, D-14)

```cpp
template <ComponentType T>
class ComponentPool final : public IComponentPool {
    std::vector<std::uint32_t> sparse_;    // entity index → position in the arrays below
    std::vector<Entity> entities_;         // sorted by EntityId
    std::vector<EntityId> ids_;            // same order; used for binary search and export
    std::vector<T> components_;            // same order; not present for tags
    std::vector<std::uint8_t> enabled_;    // same order; not present for always-enabled types
};
```

Pools only change during command playback (§9) and direct `World` calls outside system execution, never while a system or query runs. So a pool does not need to stay sorted after every single add or remove; it only needs to be sorted again before the next system runs.

```
add(e, value):                                   (playback or direct World call)
    append to ids_, entities_, components_, enabled_; sparse_[e.index] = size − 1
    if size > 1 and ids_[size − 2] > ids_[size − 1]: dirty_ = true
remove(e):
    pos = sparse_[e.index]; move the last entry into pos; pop_back every array
    sparse_[moved.index] = pos; sparse_[e.index] = npos
    if pos != size: dirty_ = true
sortIfDirty():                                   end of playback, end of each direct call batch
    if dirty_: sort all arrays together by ids_ (std::sort on a permutation), rebuild sparse_
    dirty_ = false
contains(e):
    e.index < sparse_.size() && sparse_[e.index] != npos && entities_[sparse_[e.index]] == e
```

`World` keeps a list of pools touched since the last sort and calls `sortIfDirty()` on them at the end of `CommandBuffer::playback()` and at the end of every public structural `World` call (scene loading can batch these through `World::beginBatch()` / `endBatch()`).

| Operation | Cost | Note |
| --- | --- | --- |
| Add, remove | O(1) | Swap-and-pop; only this pool is touched |
| Re-sort at playback end | O(n log n) per touched pool, once per phase | Nothing when a phase only appended runtime-created entities in ID order |
| `has`, `get`, `tryGet` | O(1) | One sparse lookup plus a handle compare |
| Iterate one type | O(n) | Packed array, ascending `EntityId` |

A sorted insert per command was rejected: scene entities have Editor IDs (below 2³²), so adding or removing a component on a placed door, key or pickup, the most common gameplay change, would shift every later entry and rewrite its `sparse_` slot.

- **Tags** (`std::is_empty_v<T>`) store no `components_`; only `has`, `add`, `remove` and query filters apply.
- **Unmanaged** components (`std::is_trivially_copyable_v<T>`) may contain only fixed-size fields. Their pools can be copied with `memcpy` (snapshots, telemetry fast path) and exposed as one contiguous buffer with a field layout (§11, §16).
- **Managed** components may own heap memory (`std::string`, `std::vector`). They are copied with their copy constructors. Keep them out of hot paths where possible (coding rule 7).
- `IComponentPool` is the type-erased interface: `type()`, `size()`, `contains(e)`, `removeIfPresent(e)`, `idAt(pos)`, `enabledAt(pos)` and `rawAt(pos)` (used with the reflection accessors of §11).

## 7. Queries (D-15)

```cpp
// The callback takes the components in template order, optionally preceded by the Entity.
// A const type is a read; a non-const type is a write (checked, §10).
ctx.each<const VelocityComponent, PositionComponent>(
    [](const VelocityComponent& vel, PositionComponent& pos) { /* ... */ });

ctx.query<HealthComponent>()
    .with<PlayerControlledTag>()           // must also have it; no parameter
    .without<InvulnerableTag>()            // must not have it
    .withTag(playerTag)                    // srs.fr.2.57
    .withLayer(2)
    .each([](Entity e, HealthComponent& health) { /* ... */ });
```

```cpp
template <typename... Ts>
class Query {
public:
    template <typename... Required> Query& with();
    template <typename... Excluded> Query& without();
    Query& withTag(TagId tag);
    Query& withLayer(std::uint8_t layer);
    Query& includeDisabled();              // World only (Editor, scene saving); not in systems
    template <typename Fn> void each(Fn&& fn);
    std::size_t count();
};
```

**Semantics**

1. Visits every alive, **enabled** entity that has all `Ts` and the `with` types, none of the `without` types, and whose `Ts` components are enabled (srs.fr.2.11, 2.15, 2.18). Position and Velocity have no component flag; a disabled entity skips them too.
2. Iterates in **ascending `EntityId`** (rule 6). The smallest of the `Ts` pools drives the loop. Every pool is sorted the same way, so the output is ascending.
3. Results are never cached; each call reads the pools as they are (ecs-design.md §5).
4. No structural change can happen during the loop: systems only record commands. A direct `World` structural call during iteration is an error.
5. Disabling an entity sets its flag at once (`isEnabled` and handlers see it), but **queries see it from the next system on** (srs.fr.2.119). The table keeps a separate `queryEnabled` flag. `setEnabled` changes `enabled` and appends the entity to a short pending list; the scheduler applies that list to `queryEnabled` at one fixed point, before each system's `update()` and before each event handler call. A query that is already running never sees a change, and no check of "is a query running" is needed.

```
each<A, B, C>(fn):
    driver = smallest of pool<A>, pool<B>, pool<C>
    for pos in 0 … driver.size − 1:
        e = driver.entities_[pos]
        if not table.queryEnabled(e): continue
        if not (pool<A>.containsEnabled(e) and pool<B>… and pool<C>…): continue
        if not filters(e): continue
        fn([e,] pool<A>.at(e), pool<B>.at(e), pool<C>.at(e))
```

## 8. Systems (D-10)

```cpp
enum class TickPhase : std::uint8_t {   // srs.fr.1.12, in execution order
    InputApplication,
    GameplayInput,
    PhysicsSyncIn,
    PhysicsStep,                        // the Box2D step; holds no systems (srs.fr.2.48)
    PhysicsSyncOut,                     // write-back and collision event collection
    GameplayRules,
    TelemetryRecording,
};

class ISystem {
public:
    virtual ~ISystem() = default;
    // Static facts: name, purpose, phase, group, ordering rules, access. Called once when
    // the system is registered; the result does not change afterwards.
    virtual void describe(SystemDescription& d) const = 0;
    // Optional one-time setup when added to a World (e.g. event subscriptions).
    virtual void setup(SystemContext& ctx) {}
    // Called exactly once per simulation tick in the system's phase (srs.fr.2.14).
    virtual void update(SystemContext& ctx) = 0;
};
```

The task's fields map as follows: **Name** → `d.name()`, **Stage** → `d.phase()` (the SRS tick phase), **Reads** → `d.reads<…>()`, **Writes** → `d.writes<…>()`, **Update** → `update()`. One `describe()` builder replaces five getters so that registration can validate everything in one place and the same data becomes the exported metadata.

```cpp
class SystemDescription {
public:
    SystemDescription& name(std::string_view name);              // unique (srs.fr.2.39)
    SystemDescription& purpose(std::string_view text);           // required (srs.fr.2.38)
    SystemDescription& phase(TickPhase phase);                   // or taken from the group
    SystemDescription& group(std::string_view group);            // srs.fr.2.48
    SystemDescription& runsBefore(std::string_view systemOrGroup);  // srs.fr.2.49
    SystemDescription& runsAfter(std::string_view systemOrGroup);
    template <typename... Ts> SystemDescription& reads();        // srs.fr.2.16
    template <typename... Ts> SystemDescription& writes();       // a write allows reading
    template <typename... Es> SystemDescription& publishes();    // event types
    template <typename... Es> SystemDescription& subscribes();
    SystemDescription& createsEntities();                        // D-21
    SystemDescription& destroysEntities();
    SystemDescription& togglesEnabled();                         // setEnabled on entities
};
```

```cpp
class MovementSystem final : public ISystem {
public:
    void describe(SystemDescription& d) const override {
        d.name("Movement")
            .purpose("Turns the held move actions into the player's horizontal velocity.")
            .group("Player")                                     // group "Player" is in GameplayInput
            .runsAfter("InputMapping")
            .reads<InputStateComponent, PlayerControlledTag>()
            .writes<VelocityComponent>();
    }

    void update(SystemContext& ctx) override {
        const auto& input = ctx.single<const InputStateComponent>();
        ctx.query<VelocityComponent>().with<PlayerControlledTag>().each(
            [&](VelocityComponent& vel) { vel.xMetersPerSecond = input.moveX * speed; });
    }

private:
    static constexpr float speed = 4.0f;   // meters per second
};
```

```cpp
class SystemScheduler {
public:
    SystemId add(SystemFactory factory);                 // creates this World's instance, validates, recomputes the order
    void remove(SystemId id);
    void defineGroup(std::string_view name, TickPhase phase, std::string_view purpose);
    void setEnabled(SystemId id, bool enabled);          // srs.fr.2.15
    void setGroupEnabled(std::string_view group, bool enabled);  // srs.fr.2.52
    void setManualOrder(std::span<const SystemId> order);  // Systems panel order
    OrderResult computeOrder();                          // see below
    void runPhase(TickPhase phase, World& world);        // called by the game loop
    std::span<const SystemTiming> lastTickTimings() const;  // srs.fr.2.17
};
```

`SystemFactory` is `std::function<std::unique_ptr<ISystem>()>`; `types.system<S>()` registers `[] { return std::make_unique<S>(); }` by default. Each World calls the factory once, so a system's members are per-World state (D-23). Systems must not have static mutable members; review checks this.

**Run order** (srs.fr.2.50, 2.51), computed per phase:

1. A system in a group takes the group's phase. A system that names a different phase is refused. `PhysicsStep` accepts no systems.
2. A rule that names a group applies to every member of the group. A rule against a system in another phase is ignored if the phase order already satisfies it, and refused if it contradicts it. A rule that names an unknown system is ignored with a warning (srs.fr.2.105).
3. **Stable topological sort** (Kahn's algorithm): among the systems whose predecessors have all been placed, take the one that comes first in the manual order (initially registration order). This keeps the user's order wherever the rules allow.
4. If systems are left over, the rules form a cycle: the new order is refused, the last valid order is kept, and the systems in the cycle are reported.
5. Disabled systems and groups keep their place in the order; they are skipped when the phase runs.

**Timings.** The scheduler measures each system's `update()` plus its event handlers in that tick, in milliseconds, with `std::chrono::steady_clock`. The values are only reported (Systems panel, telemetry) and never feed back into the simulation (rule 5, R-6).

**Rendering is not a system.** Rendering runs per render frame, not per tick (srs.fr.1.13). The renderer reads the `World` after the ticks of a frame and is not scheduled here. This changes `ecs-design.md` §3, which lists a RenderSystem (§14).

## 9. CommandBuffer (D-4, D-5, D-18)

```cpp
class CommandBuffer {
public:
    Entity createEntity();                                   // handle + EntityId now (D-5)
    template <ComponentType T> void add(Entity e, T value = T{});
    template <ComponentType T> void remove(Entity e);
    void destroy(Entity e);                                  // with descendants (srs.fr.2.4)
    void setParent(Entity child, Entity parent);
    std::size_t size() const;
};
```

Each system reaches it through `ctx.commands()`. One buffer serves a whole phase; each command records the system that issued it, for error messages and telemetry.

**Where it fits in a tick phase:**

```
runPhase(phase):
    for each system in the phase's run order:
        if enabled: system.update(ctx)       commands are only recorded
    events.dispatch(phase)                    handlers run; their commands join the same buffer
    commands.playback()                       structural changes become visible
    observers: bodies of destroyed/disabled entities are disabled (srs.fr.2.119)
```

**`createEntity()`** allocates the handle and the next runtime `EntityId` at once. It touches no pool, so it is safe during iteration, and the new `EntityId` can be stored in other components straight away. The entity has no components (not even the built-ins) until playback, so no query sees it before then. Since systems run in a fixed order, IDs are allocated in a deterministic order.

**A created entity before playback** ("pending"):

| Call on the pending handle or its `EntityId` | Result |
| --- | --- |
| `isAlive(e)`, `find(id)`, `idOf(e)` | true / the handle / the ID, so references to it can be stored at once |
| `has<T>(e)` | false |
| `get<T>(e)`, `tryGet<T>(e)` | error / `nullptr` |
| `setEnabled(e, …)` | applied to the slot; takes effect at playback |
| `commands().add/remove/setParent(e, …)` | recorded as usual |
| `commands().destroy(e)` in the same phase | create and destroy cancel out: the slot is freed at playback, the `EntityId` is never reused, no observer is notified, and reference clearing (rule 4) still clears stored copies of the ID |

**Playback rules:**

1. Commands are applied **in recording order**.
2. An `add` for a type the entity already has, a `remove` for a type it lacks, and any command on an entity destroyed earlier in the same playback are skipped with a warning that names the system, the entity and the type.
3. **`destroy`:** collect the entity and its descendants (depth-first, in child order). For each one: notify the observers (the Physics Adapter disables the body now and destroys it in the next sync-in, srs.fr.2.119), unlink it from the hierarchy, remove it from every pool, and free its slot.
4. **Reference clearing** (srs.fr.2.44, D-18): after the destructions, every reflected `EntityId` field (including elements of `std::vector<EntityId>`) that holds a destroyed ID is set to `noEntity`. At `finalize()` the registry builds the list of `(type, field offset)` pairs that hold `EntityRef` fields; playback walks only those pools, reads the field at its offset, and checks it against the sorted vector of IDs destroyed in this playback (binary search). Only phases that destroyed something do this pass.
5. **Sort:** every pool touched by this playback is re-sorted by `EntityId` (§6), so the next phase sees sorted pools.

**Disable** is not a command. `ctx.setEnabled(e, false)` sets the flag at once, queries see it from the next query, and the body follows at the end of the phase (srs.fr.2.119, 2.120).

```cpp
class IWorldObserver {                     // engine-internal: Physics Adapter, telemetry
public:
    virtual ~IWorldObserver() = default;
    virtual void onEntityDestroyed(Entity e, EntityId id) {}
    virtual void onEntityEnabledChanged(Entity e, bool enabled) {}
    virtual void onComponentAdded(Entity e, TypeId type) {}
    virtual void onComponentRemoved(Entity e, TypeId type) {}
    virtual void onRestored() {}           // after World::restore (D-24): rebuild outside resources
};
```

Observers fill the role of Unity's cleanup components: the owner of an outside resource (a Box2D body) is told before the entity disappears.

## 10. SystemContext and access checking (D-11)

```cpp
class SystemContext {
public:
    template <typename... Ts, typename Fn> void each(Fn&& fn);
    template <typename... Ts> Query<Ts...> query();
    template <typename T> T& get(Entity e);           // T may be const
    template <typename T> T* tryGet(Entity e);
    template <typename T> bool has(Entity e) const;
    template <typename T> T& single();
    bool isAlive(Entity e) const;
    std::optional<Entity> find(EntityId id) const;
    EntityId idOf(Entity e) const;
    void setEnabled(Entity e, bool enabled);
    template <typename T> void setComponentEnabled(Entity e, bool enabled);
    CommandBuffer& commands();
    template <typename E> EventSeq publish(E event);  // companion document
    template <typename E, typename S>
    SubscriptionId subscribe(void (S::*handler)(const EventInfo&, const E&, SystemContext&));
    void unsubscribe(SubscriptionId id);
    Tick tick() const;
    std::string_view systemName() const;
};
```

| Operation | Must be declared |
| --- | --- |
| `each`, `query`, `get`, `tryGet`, `single` with `const T` | `T` in `reads` or `writes` |
| The same with non-const `T` | `T` in `writes` |
| `has<T>`, `with<T>`, `without<T>` | `T` in `reads` or `writes` |
| `commands().add<T>` / `remove<T>`, `setComponentEnabled<T>` | `T` in `writes` |
| `commands().setParent` | `HierarchyComponent` in `writes` |
| `commands().createEntity` | `createsEntities()` |
| `commands().destroy` | `destroysEntities()` |
| `setEnabled` | `togglesEnabled()` |
| `publish<E>` / `subscribe<E>` | `E` in `publishes` / `subscribes` |

Destroying or disabling an entity removes or hides every one of its components, so these are declared like writes (D-21). The calls are also recorded in telemetry with the system's name, and the schema exports the three flags, so the Semantic Graph can answer "which systems can destroy or disable entities?" and telemetry answers "which one did".

- **Cost.** Each system has a read bitset and a write bitset indexed by the registry's dense type index. A query checks once per call; `get` checks one bit.
- **Violation** (every build): the error is logged with the system, the type, the operation and the tick, and it is treated as an **unrecoverable run error** (srs.fr.1.11). The Editor goes to Error Pause and a headless run exits with a non-zero code. Systems cannot reach `World` directly, so the check cannot be bypassed.
- **Registration checks:** declaring a type that is not registered refuses the system.

## 11. Reflection (D-6, D-7, D-8, D-14)

```cpp
template <typename T>
concept EcsType = requires {
    { T::typeName } -> std::convertible_to<std::string_view>;
} && std::is_default_constructible_v<T> && std::is_move_constructible_v<T>;

template <typename T> concept ComponentType = EcsType<T>;
template <typename T> concept TagType = ComponentType<T> && std::is_empty_v<T>;

struct TypeId {
    std::uint64_t value = 0;
    friend bool operator==(TypeId, TypeId) = default;
};

// 64-bit FNV-1a of the type name: the same value in every run, in the engine and in script DLLs.
constexpr TypeId typeIdFromName(std::string_view name) {
    std::uint64_t hash = 14695981039346656037ull;
    for (char c : name) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ull;
    }
    return TypeId{hash};
}

template <EcsType T> constexpr TypeId typeIdOf() { return typeIdFromName(T::typeName); }
```

Files and exports always use the **name**; `TypeId` is the in-memory key. A hash collision between two different names is refused like a duplicate (srs.fr.2.39). The registry also gives each type a dense **type index** (registration order) for pool arrays and access bitsets. The type index is never saved.

**Field types.** Each member's C++ type is mapped at compile time; an unsupported type is a compile error.

| `FieldType` | C++ type | Kind |
| --- | --- | --- |
| `Bool`, `Int32`, `Int64`, `Float`, `Double` | `bool`, `std::int32_t`, `std::int64_t`, `float`, `double` | unmanaged |
| `Vec2` | `engine::Vec2` | unmanaged |
| `Enum` | a registered `enum class` | unmanaged |
| `EntityRef`, `AssetRef` | `EntityId`, `AssetId` | unmanaged |
| `FixedString`, `FixedArray` | `FixedString<N>`, `FixedArray<T, N>` | unmanaged |
| `String`, `List` | `std::string`, `std::vector<T>` | **managed** |

Nested structs are not supported in this version, so components stay flat.

```cpp
enum class Unit : std::uint8_t {          // components.md, DEC-036
    None, Meters, MetersPerSecond, Radians, RadiansPerSecond, Kilograms, Seconds, Ticks,
};

struct FieldInfo {
    std::string name;
    FieldType type;
    Unit unit;
    std::string description;
    FieldValue defaultValue;              // read from T{} at registration: never typed twice
    std::string enumType;                 // for Enum fields
    // Layout (D-6): computed from the member pointer at registration.
    std::uint32_t offset;                 // byte offset inside T
    std::uint32_t size;                   // sizeof the member
    std::uint32_t count;                  // 1, or N for FixedArray<T, N> / FixedString<N>
    // Managed fields only (String, List); empty for unmanaged fields.
    std::function<FieldValue(const void* component)> readManaged;
    std::function<void(void* component, const FieldValue& value)> writeManaged;
};

FieldValue readField(const FieldInfo& f, const void* component);    // offset path or readManaged
void writeField(const FieldInfo& f, void* component, const FieldValue& value);

struct ComponentTypeInfo {
    TypeId id;
    std::string name;                     // T::typeName
    std::string description;              // meaning (srs.fr.2.12)
    ComponentKind kind;                   // Unmanaged or Managed, auto-detected (D-14)
    bool isTag;
    bool alwaysEnabled;                   // Position and Velocity only (srs.fr.2.11)
    bool builtIn;
    std::vector<FieldInfo> fields;
};
```

`FieldValue` is a `std::variant` over the field types (lists hold `FieldValue`s). The Inspector and scene saving use `readField`/`writeField`; they are not hot paths. **Per-tick paths do not build `FieldValue`s:** telemetry state records, snapshots, reference clearing and the agent bridge read unmanaged fields straight from the pool at `offset`, and copy whole unmanaged pools with `memcpy`. The layout (`offset`, `size`, `count`, `FieldType`) is exported in the schema, so a reader can decode a raw pool buffer without C++ (§16).

```cpp
struct PositionComponent {
    static constexpr std::string_view typeName = "Position";
    float xMeters = 0.0f;
    float yMeters = 0.0f;
};

struct InventoryComponent {                // managed: owns a std::vector (Phase 1 uses FixedArray<AssetId, 8>, §17)
    static constexpr std::string_view typeName = "Inventory";
    std::vector<AssetId> items;
};

struct PlayerControlledTag {
    static constexpr std::string_view typeName = "PlayerControlled";
};

void registerCoreComponents(TypeRegistry& types) {
    types.component<PositionComponent>()
        .describe("Where the entity is in the world (meters, y up, DEC-036).")
        .alwaysEnabled()
        .field("x", &PositionComponent::xMeters, Unit::Meters, "Horizontal position")
        .field("y", &PositionComponent::yMeters, Unit::Meters, "Vertical position");

    types.component<InventoryComponent>()
        .describe("Items the entity has collected and still holds.")
        .field("items", &InventoryComponent::items, Unit::None, "Asset IDs of held items");

    types.component<PlayerControlledTag>()
        .describe("Marks the entity that the user or the Playtest Agent controls.");
}
```

`TypeRegistry` offers the same builder for `event<E>()` (companion document), `enumeration<E>(name)` with `.value(E::Static, "Static", "description")`, `dataAsset<T>()` (srs.fr.2.22, 2.23) and `system<S>()` (srs.fr.2.37). `types.finalize()` validates and freezes the registry: every type and field has a description (srs.fr.2.38), names are unique (srs.fr.2.39), and an empty type has no fields.

**Unregistered members are refused.** C++20 cannot list a struct's member names, but it can count the members of an aggregate: try brace-initialising `T` with 0, 1, 2, … values convertible to anything and keep the largest count that compiles (the technique Boost.PFR uses; we implement the counter in about 20 lines, no dependency). Every component, tag and event must be an aggregate (`static_assert(std::is_aggregate_v<T>)`), and `finalize()` refuses a type whose member count differs from the number of `.field()` calls. A round-trip test cannot do this: a forgotten member stays at its default on both sides and the test passes.

A static `typeName` member does not count, because brace initialisation only sees non-static data members. Raw C arrays as members are not allowed (use `FixedArray<T, N>`): brace elision would make the counter overcount them.

## 12. Metadata export (D-8, D-9)

```cpp
struct SystemInfo {
    std::string name, purpose, group;
    TickPhase phase;
    std::vector<std::string> runsBefore, runsAfter;
    std::vector<std::string> reads, writes, publishes;                // type names
    std::vector<SubscriptionInfo> subscribes;
    bool createsEntities, destroysEntities, togglesEnabled;           // D-21
    std::uint32_t orderIndex;              // position in the computed run order
    bool enabled;
};

struct SubscriptionInfo {                  // D-22
    std::string event;                     // event type name
    std::vector<TickPhase> runsIn;         // phases where the handler can actually run
};

struct PublisherInfo {                     // engine modules that publish (event bus E-2)
    std::string name;                      // e.g. "PhysicsAdapter"
    TickPhase phase;                       // e.g. PhysicsSyncOut
    std::vector<std::string> publishes;
};

struct Schema {                            // plain data, no accessors
    std::uint32_t schemaVersion = 1;
    std::vector<ComponentSchema> components;     // ComponentTypeInfo without read/write
    std::vector<EventSchema> events;
    std::vector<EnumSchema> enums;
    std::vector<GroupInfo> groups;
    std::vector<SystemInfo> systems;
    std::vector<PublisherInfo> enginePublishers;
    // Derived from the systems (D-8), keyed by component name:
    std::map<std::string, std::vector<std::string>> readBy, writtenBy;
};

Schema buildSchema(const TypeRegistry& types, const SystemScheduler& systems);
```

**Where handlers run** (D-22). A handler runs at the dispatch point of the phase where its event was published, not in its system's own phase (event bus §4). Its writes therefore happen there. `runsIn` lists, for each subscription, the phases of every declared publisher of that event: systems through `publishes<E>()`, engine modules through `types.enginePublisher("PhysicsAdapter", TickPhase::PhysicsSyncOut).publishes<CollisionEvent>()`. When a publisher is itself a handler, its events wait for the next dispatch point (srs.fr.2.78), so the next phase is listed too. In the example of the event bus document, `Unlock` has phase `GameplayRules` but its `Collision` subscription has `runsIn: ["PhysicsSyncOut"]`, which is where its `Unlockable` writes happen.

**Flow:** the ECS builds the `Schema` (no file I/O in `engine/ecs`). The writer in `engine/telemetry` writes it as the **first record after the run header** of every telemetry file. The Semantic Architecture Layer reads only that file (R-1). Project scripts never reload during a run (srs.fr.2.40), so one schema per file is always correct. The Editor's type cache (srs.fr.2.121) can store the same structure.

Example (one line in the file, wrapped here):

```json
{"type":"schema","schemaVersion":1,
 "phases":["InputApplication","GameplayInput","PhysicsSyncIn","PhysicsStep","PhysicsSyncOut","GameplayRules","TelemetryRecording"],
 "components":[{"name":"Position","kind":"unmanaged","tag":false,"alwaysEnabled":true,
   "description":"Where the entity is in the world (meters, y up, DEC-036).",
   "size":8,
   "fields":[{"name":"x","type":"float","unit":"m","default":0.0,"offset":0,"size":4,"count":1,"description":"Horizontal position"},
             {"name":"y","type":"float","unit":"m","default":0.0,"offset":4,"size":4,"count":1,"description":"Vertical position"}],
   "readBy":["CameraFollow"],"writtenBy":["PhysicsWriteBack"]}],
 "systems":[{"name":"Movement","purpose":"Turns the held move actions into the player's horizontal velocity.",
   "phase":"GameplayInput","group":"Player","order":2,"runsAfter":["InputMapping"],
   "reads":["InputState","PlayerControlled"],"writes":["Velocity"],"publishes":[],"subscribes":[],
   "createsEntities":false,"destroysEntities":false,"togglesEnabled":false}],
 "enginePublishers":[{"name":"PhysicsAdapter","phase":"PhysicsSyncOut","publishes":["Collision"]}]}
```

The JSON-lines format follows `telemetry-and-events.md`, which is still `[PROPOSED]`; the final record shape is settled with the telemetry PB item. Runtime values (entity states per tick) use the same field layout, read at each field's `offset` without building `FieldValue`s (§11), keyed by `EntityId`.

---

## 13. Traceability

| SRS | Where |
| --- | --- |
| 2.1, 2.2, 2.61, 2.64–2.66, 2.107 | §4 |
| 2.3, 2.4, 2.44, 2.119, 2.120 | §9 |
| 2.5, 2.53, 2.54, 2.85 | §5 built-in components |
| 2.6–2.10 | §5, §6 |
| 2.11, 2.15, 2.18, 2.57 | §6, §7 |
| 2.12, 2.16, 2.36–2.39 | §8, §11, §12 |
| 2.13, 2.14, 2.17, 2.48–2.52 | §8 |
| 2.45–2.47, 2.77–2.84, 2.102, 2.116–2.118 | companion file |

## 14. Follow-ups this design creates

- **Decision log:** ADR-001 (D-1), plus the D-2…D-25 choices once the team accepts them in PB-010.3. D-26 restates the human's scope decision of 2026-11-05 and should be logged with it.
- **Agent bridge task (PB-065 follow-up):** the `Observation`/`Action` proto, `AgentObservationSystem` and the input path of §16.
- **Snapshot design:** a separate document for `WorldSnapshot` (D-24), Play mode and Editor undo.
- **`coding-standards.md`:** add "functions camelCase, members with trailing underscore" (D-19); fix the tag rule to `<Name>Tag` (D-17).
- **`ecs-design.md`:** §2 component list vs the SRS roles (Collectible/Unlockable/Objective); §3 RenderSystem is per render frame (srs.fr.1.13); §8 open questions answered (storage, metadata form).
- **`telemetry-and-events.md`:** add the schema record (§12).
- **Q-013** stays open: Q-013 extras (valid ranges, examples, notes) can be added to `FieldInfo` and the builder later without changing the API.

## 15. Implementation test checklist

Phase 1 items (§17) are marked **[P1]**; the rest are written when their feature is built.

Entity table: create, destroy, stale handle, slot reuse with a new generation, `find`/`idOf` for runtime and Editor IDs, runtime IDs restart at 2³² every run **[P1]** · Pool: swap-and-pop add and remove, sparse fix-up, sorted again after playback, tags without storage **[P1]** · Query: ascending `EntityId` after removals and out-of-order adds **[P1]**; disabled entity and disabled component skipped, disable seen from the next system only, `with`/`without`/tag/layer filters · CommandBuffer: recording order, created entity invisible to queries until playback, pending-entity table of §9 **[P1]**; create and destroy in one phase cancel out, recursive destroy, reference clearing in plain and list fields · Access: undeclared read, write to a read-only type, undeclared publish, undeclared create/destroy, each stops the run **[P1]**; undeclared `setEnabled` · Scheduler: registration order inside a phase, timings recorded **[P1]**; stable order with rules, group rules, cycle refused with the last valid order kept, disabled group skipped · Reflection: defaults read from `T{}`, missing description refused, duplicate name refused, unregistered member refused, field offsets match `offsetof` **[P1]** · Schema: matches a golden JSON file, handler `runsIn` phases **[P1]** · Worlds: two Worlds from one registry run the same input to identical state, no shared system state **[P1]** · Snapshot: restore then rerun gives the same telemetry as the first run.

## 16. Agent I/O: the C++ to Python bridge (D-25)

ADR-002 puts an `AgentBridge` between the engine and the Python agent: gRPC + protobuf first, an optional in-process pybind11 adapter later if profiling justifies it. The ECS does not know about the bridge (R-1); it gives the bridge three things.

1. **Actions are input.** The bridge turns an `Action` into the same input stream headless mode already accepts (DEC-005). That input is applied in `InputApplication` into `InputStateComponent`, exactly like keyboard input. The bridge never writes to the `World`, so the access check, determinism and telemetry replay hold for agent runs too, and an agent run replays like a human one.
2. **Observations come from a declared system.** `AgentObservationSystem` runs in `TelemetryRecording`, declares the components it reads, and fills an `ObservationBuffer` owned by the bridge seam (an engine-side interface with no transport types). The Semantic Graph therefore shows exactly what the agent can see.
3. **Exchange happens between ticks only.** The game loop calls the bridge after a tick ends and before the next one starts; nothing in a tick waits on Python.

**Wire contract** (for the bridge task, not the ECS):

- A hand-written, narrow `Observation` / `Action` proto, as ADR-002 says. One message per component type is **not** generated: that would tie code generation to every component change.
- One generic `EntityState { uint64 id; repeated ComponentState components; }` with `ComponentState { string type; repeated FieldState fields; }`, built from reflection, for the assistant's read-only questions ("where is the key?").
- The `Schema` (§12) is sent once per session, so Python decodes component data by name and layout without generated code.
- `EntityId` fields are `uint64` or `fixed64`, never `int32`: runtime IDs start at 2³². (They are also safe in JSON and Python, below 2⁵³.)

**pybind11 fast path** (only if profiling asks for it): every unmanaged pool is a contiguous `components_` array with a known layout (§11) and a parallel `ids_` array in ascending `EntityId`. The adapter can expose both as buffer views; Python maps them onto a NumPy structured dtype built from the schema's `offset`/`size`/`type`. This is the zero-copy buffer-view mode that measured fastest in PB-065. It only works for unmanaged components, which is why the built-in components are unmanaged (§5).

**Vectorized and parallel runs:** one process may step many `World`s (D-23), and `World::snapshot()`/`restore()` (D-24) gives a fast episode `reset()`. `beginRun()` resets runtime IDs, so every episode from the same snapshot and inputs is identical.

## 17. Build phases (D-26)

The team builds about 20 % of the whole project this semester (human decision, 2026-11-05). For the ECS that means Phase 1 below: enough for the small test scene (player, wall, door, key), the semantic layer's schema and telemetry, and the seams the bridge needs. Every later item already has its place in the API above, so adding it does not change Phase 1 code that uses the ECS.

| Area | Phase 1 (this semester) | Later |
| --- | --- | --- |
| Entities | Entity table, runtime and Editor IDs, `find`/`idOf`, dense ID lookup | Hierarchy (`setParent`, recursive destroy), entity and component enabled flags |
| Storage | `ComponentPool<T>` with sort-at-playback, tags, unmanaged components | Managed components (`String`, `List`); in Phase 1 a list is a `FixedArray<T, N>`, e.g. the inventory |
| Queries | `each<Ts...>` in ascending `EntityId` | `query<Ts...>()` builder (`with`, `without`, tag, layer), `includeDisabled` |
| Systems | `ISystem`, `describe()` with phase, reads, writes, create/destroy flags; access check; registration order inside a phase; timings; factories | Groups, `runsBefore`/`runsAfter`, manual order, cycle reporting, enable/disable of systems and groups |
| Structural changes | `CommandBuffer` with create, destroy, add, remove; pending-entity rules | Eager reference clearing (D-18), `setParent` commands |
| Events | Typed publish/subscribe, sequence numbers, dispatch at the end of the publishing phase, handler events wait for the next dispatch point | The 10,000-per-tick limit, drop statuses, run-end drops, subscribe/unsubscribe during dispatch |
| Reflection and export | `TypeRegistry` with descriptions, units, field layout, member-count check; `Schema` written to telemetry | Data assets, enums beyond the ones the test scene needs, Q-013 extras |
| Singletons | `single<T>()` for `InputStateComponent` | General singleton support |
| Agent I/O and snapshot | Rules only: input is the only way in, no static state in systems, unmanaged built-ins | `AgentObservationSystem`, `snapshot()`/`restore()`, the bridge itself |

Phase 1 is the critical path for the semantic layer (it needs the schema and real telemetry) and for the Increment 1 demo.
