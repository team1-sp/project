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

**Not designed here** (each needs its own document; the API leaves room for them): prefabs and variants (srs.fr.2.29–2.35, 2.69–2.128), the `.scene` file format and loading (2.19–2.21), data assets beyond their reflection hook (2.22–2.28), project-script DLL loading, reload and placeholders (2.36–2.41, 2.103–2.106, 2.121), Play-mode snapshot and Editor undo (2.108), Physics Adapter internals, the renderer.

## 2. Design decisions

| # | Topic | Decision | Main reason |
| --- | --- | --- | --- |
| D-1 | Storage | Sparse sets: one independent pool per component type, no groups | PB-010.1 research |
| D-2 | Identity | Two IDs: `Entity` handle `{index, generation}` for runtime storage; 64-bit `EntityId` (srs.fr.2.107) as identity. References, events, telemetry and scenes store `EntityId` | Task text + SRS |
| D-3 | Order | Pools stay **sorted by `EntityId`**; every query iterates in ascending `EntityId` | Coding standard rule 6 (DEC-031) |
| D-4 | Structural changes | Systems and event handlers record them in a `CommandBuffer`, played back **at the end of each tick phase, after event dispatch**. Direct `World` calls only outside system execution | srs.fr.2.119, E-003 |
| D-5 | Create during a tick | `commands.createEntity()` assigns handle and `EntityId` **now**; components appear at playback | Deterministic IDs, usable references |
| D-6 | Reflection | Registration builder with member pointers, no macros | Coding standard §4 "avoid macros" |
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
idToSlot_ : std::unordered_map<EntityId, std::uint32_t>
freeList_ : std::vector<std::uint32_t>     destroyed indices, reused last-in first-out
nextRuntimeId_ : EntityId                  reset to firstRuntimeEntityId at every run start
```

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

    // Type-erased access for telemetry, scene saving and the Inspector (§12).
    const TypeRegistry& types() const;
    const IComponentPool* pool(TypeId type) const;
    void forEachComponent(Entity e, const std::function<void(const ComponentTypeInfo&,
                                                             const void*)>& fn) const;
};
```

**Errors.** Precondition violations (stale handle, duplicate add, missing component) fail loudly (coding rule 9): they are reported with the `EntityId`, the type name and the tick and handled like an access violation (§10).

**Reference lifetime.** A `T&` from `get`/`each` stays valid until the next structural change to `T`. Systems never make structural changes directly, so inside a phase every reference stays valid until that phase's command playback. Systems keep entity handles or `EntityId`s, never component pointers.

**Built-in components** (D-12). Every entity gets these at creation and they cannot be removed:

```cpp
struct NameComponent {                 // managed (std::string)
    static constexpr std::string_view typeName = "Name";
    std::string name;
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

```
add(e, value):
    pos = lower_bound(ids_, idOf(e))             O(log n)
    insert at pos into ids_, entities_, components_, enabled_
    for i in pos … size − 1: sparse_[entities_[i].index] = i
remove(e):
    pos = sparse_[e.index]; erase pos from every array
    for i in pos … size − 1: sparse_[entities_[i].index] = i
    sparse_[e.index] = npos
contains(e):
    e.index < sparse_.size() && sparse_[e.index] != npos && entities_[sparse_[e.index]] == e
```

| Operation | Cost | Note |
| --- | --- | --- |
| Add to an entity created during the run | O(1) amortised | Runtime IDs only grow, so it appends |
| Add to an older entity, remove | O(n) | Shifts at most ~1,000 entries (microseconds); price of rule 6 |
| `has`, `get`, `tryGet` | O(1) | One sparse lookup plus a handle compare |
| Iterate one type | O(n) | Packed array, ascending `EntityId` |

- **Tags** (`std::is_empty_v<T>`) store no `components_`; only `has`, `add`, `remove` and query filters apply.
- **Unmanaged** components (`std::is_trivially_copyable_v<T>`) may contain only fixed-size fields. Their pools can be copied with `memcpy` (snapshots, telemetry fast path).
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
5. Disabling an entity sets its flag at once (handlers see it), but **queries see it from the next query on** (srs.fr.2.119). The table keeps a separate `queryEnabled` flag that is synchronised when no query is running.

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
    SystemId add(std::unique_ptr<ISystem> system);       // validates, then recomputes the order
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

**Playback rules:**

1. Commands are applied **in recording order**.
2. An `add` for a type the entity already has, a `remove` for a type it lacks, and any command on an entity destroyed earlier in the same playback are skipped with a warning that names the system, the entity and the type.
3. **`destroy`:** collect the entity and its descendants (depth-first, in child order). For each one: notify the observers (the Physics Adapter disables the body now and destroys it in the next sync-in, srs.fr.2.119), unlink it from the hierarchy, remove it from every pool, and free its slot.
4. **Reference clearing** (srs.fr.2.44, D-18): after the destructions, every reflected `EntityId` field (including elements of `std::vector<EntityId>`) that holds a destroyed ID is set to `noEntity`. This is one pass over the reference fields, and only in phases that destroyed something.

**Disable** is not a command. `ctx.setEnabled(e, false)` sets the flag at once, queries see it from the next query, and the body follows at the end of the phase (srs.fr.2.119, 2.120).

```cpp
class IWorldObserver {                     // engine-internal: Physics Adapter, telemetry
public:
    virtual ~IWorldObserver() = default;
    virtual void onEntityDestroyed(Entity e, EntityId id) {}
    virtual void onEntityEnabledChanged(Entity e, bool enabled) {}
    virtual void onComponentAdded(Entity e, TypeId type) {}
    virtual void onComponentRemoved(Entity e, TypeId type) {}
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
| `commands().createEntity` / `destroy`, `setEnabled` | nothing (recorded in telemetry with the system's name) |
| `publish<E>` / `subscribe<E>` | `E` in `publishes` / `subscribes` |

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
    std::function<FieldValue(const void* component)> read;
    std::function<void(void* component, const FieldValue& value)> write;
};

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

`FieldValue` is a `std::variant` over the field types (lists hold `FieldValue`s). The Inspector, scene saving, telemetry and reference clearing (§9) all go through `read`/`write`; none of them is a hot path.

```cpp
struct PositionComponent {
    static constexpr std::string_view typeName = "Position";
    float xMeters = 0.0f;
    float yMeters = 0.0f;
};

struct InventoryComponent {                // managed: owns a std::vector
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

**Known limit:** C++20 cannot list a struct's members, so a member that is never registered with `.field()` goes unnoticed. Mitigation: one generated round-trip test per component (set every registered field to a non-default value, copy through `FieldValue`s, compare with `==`).

## 12. Metadata export (D-8, D-9)

```cpp
struct SystemInfo {
    std::string name, purpose, group;
    TickPhase phase;
    std::vector<std::string> runsBefore, runsAfter;
    std::vector<std::string> reads, writes, publishes, subscribes;   // type names
    std::uint32_t orderIndex;              // position in the computed run order
    bool enabled;
};

struct Schema {                            // plain data, no accessors
    std::uint32_t schemaVersion = 1;
    std::vector<ComponentSchema> components;     // ComponentTypeInfo without read/write
    std::vector<EventSchema> events;
    std::vector<EnumSchema> enums;
    std::vector<GroupInfo> groups;
    std::vector<SystemInfo> systems;
    // Derived from the systems (D-8), keyed by component name:
    std::map<std::string, std::vector<std::string>> readBy, writtenBy;
};

Schema buildSchema(const TypeRegistry& types, const SystemScheduler& systems);
```

**Flow:** the ECS builds the `Schema` (no file I/O in `engine/ecs`). The writer in `engine/telemetry` writes it as the **first record after the run header** of every telemetry file. The Semantic Architecture Layer reads only that file (R-1). Project scripts never reload during a run (srs.fr.2.40), so one schema per file is always correct. The Editor's type cache (srs.fr.2.121) can store the same structure.

Example (one line in the file, wrapped here):

```json
{"type":"schema","schemaVersion":1,
 "phases":["InputApplication","GameplayInput","PhysicsSyncIn","PhysicsStep","PhysicsSyncOut","GameplayRules","TelemetryRecording"],
 "components":[{"name":"Position","kind":"unmanaged","tag":false,"alwaysEnabled":true,
   "description":"Where the entity is in the world (meters, y up, DEC-036).",
   "fields":[{"name":"x","type":"float","unit":"m","default":0.0,"description":"Horizontal position"},
             {"name":"y","type":"float","unit":"m","default":0.0,"description":"Vertical position"}],
   "readBy":["CameraFollow"],"writtenBy":["PhysicsWriteBack"]}],
 "systems":[{"name":"Movement","purpose":"Turns the held move actions into the player's horizontal velocity.",
   "phase":"GameplayInput","group":"Player","order":2,"runsAfter":["InputMapping"],
   "reads":["InputState","PlayerControlled"],"writes":["Velocity"],"publishes":[],"subscribes":[]}]}
```

The JSON-lines format follows `telemetry-and-events.md`, which is still `[PROPOSED]`; the final record shape is settled with the telemetry PB item. Runtime values (entity states per tick) use the same reflection (`forEachComponent` + `FieldInfo::read`), keyed by `EntityId`.

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

- **Decision log:** ADR-001 (D-1), plus the D-2…D-20 choices once the team accepts them in PB-010.3.
- **`coding-standards.md`:** add "functions camelCase, members with trailing underscore" (D-19); fix the tag rule to `<Name>Tag` (D-17).
- **`ecs-design.md`:** §2 component list vs the SRS roles (Collectible/Unlockable/Objective); §3 RenderSystem is per render frame (srs.fr.1.13); §8 open questions answered (storage, metadata form).
- **`telemetry-and-events.md`:** add the schema record (§12).
- **Q-013** stays open: Q-013 extras (valid ranges, examples, notes) can be added to `FieldInfo` and the builder later without changing the API.

## 15. Implementation test checklist

Entity table: create, destroy, stale handle, slot reuse with a new generation, `find`/`idOf`, runtime IDs restart at 2³² every run · Pool: sorted insert and remove, sparse fix-up, tags without storage · Query: ascending `EntityId`, disabled entity and disabled component skipped, `with`/`without`/tag/layer filters · CommandBuffer: recording order, recursive destroy, reference clearing in plain and list fields, created entity invisible until playback · Access: undeclared read, write to a read-only type, undeclared publish, each stops the run · Scheduler: stable order, group rules, cycle refused with the last valid order kept, disabled group skipped, timings recorded · Reflection: defaults read from `T{}`, missing description refused, duplicate name refused, round-trip test per component · Schema: matches a golden JSON file.
