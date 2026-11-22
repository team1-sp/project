# PB-010.1 — ECS Storage Research: Sparse Set vs Archetype

## 1. Question

Which storage model should our custom C++20 ECS use: **sparse sets** (independent per-type pools, as in EnTT) or **archetypes** (tables per component combination, as in Unity Entities)?

The task asks us to compare **implementation complexity**, **add/remove cost** and **iteration speed for our scale (< 1,000 entities)**.

## 2. Our constraints (what "good" means for Aegis)

| Constraint | Source | Why it matters for storage |
| --- | --- | --- |
| Scenes up to **1,000 entities** in the Editor; **200 active entities at ≥ 60 FPS** | srs.nfr.2.2, srs.nfr.2.4 | Small scale. Raw iteration speed is unlikely to be the bottleneck. |
| Headless: **≥ 600 simulation ticks/s** | srs.nfr.2.5 | ~10× real time; per-tick ECS overhead must stay small, but 1/600 s = 1.67 ms per tick is a large budget for ~200 entities. |
| **Deterministic** replays: same inputs → identical positions/events at every tick | srs.nfr.3.1, R-5, R-6 | Iteration order must be reproducible. |
| **Introspection** for the Semantic Architecture Layer: list types, entities, their components, systems' read/write sets | `ecs-design.md` §1, ip.fr.10.*, DEC-018 rationale | The storage should make "all components of type T" and "all components of entity E" cheap and obvious. |
| Gameplay adds/removes components often (Collectible picked up, Unlockable unlocked, trigger/tag components, enabled/disabled state) | srs.fr.2, srs.fr.6, R-7 | Add/remove must be cheap and simple. |
| Single-threaded simulation | No requirement for jobs/multithreading | Multithreading advantages of a model are worth little to us. |
| Team of students, one semester to build engine + editor | Initial Plan | **Implementation and debugging simplicity outweigh peak performance.** |

## 3. Sources

- skypjack (EnTT author), *ECS back and forth*, parts 1–13 plus "insights" posts (2019–2021) — local copies in `SP/_posts_about_ecs/`.
- Unity **Entities 6.5** manual (Entities is a core package versioned with the Unity 6.5 editor) — pages listed in §11.
- Personal notes from the Unity ECS documentation (`SP/_posts_about_ecs/unity-ecs-notes.txt`).

---

## 4. The three models in one picture

Entities `e1: {Hp, Dmg}`, `e2: {Hp}`, `e3: {Hp, Dmg}`.

```
BIG MATRIX (entityx)          ARCHETYPES (Unity)                 SPARSE SETS (EnTT)
index = entity id             one table per component set        one independent pool per type

Hp : [hp1|hp2|hp3]            Archetype {Hp,Dmg}                 Pool<Hp>  sparse: e1→0 e2→1 e3→2
Dmg: [dm1| -- |dm3]  ← hole     entities: [e1|e3]                          dense : [e1|e2|e3]
mask: e1=11 e2=10 e3=11         Hp : [hp1|hp3]                             data  : [hp1|hp2|hp3]
                                Dmg: [dm1|dm3]                   Pool<Dmg> sparse: e1→0 e3→1
                              Archetype {Hp}                               dense : [e1|e3]
                                entities: [e2]                             data  : [dm1|dm3]
                                Hp : [hp2]
```

- **Big matrix:** entity = array index; holes where an entity lacks a component; every system scans every entity's bitmask. (skypjack part 1, part 11)
- **Archetype:** an entity lives in exactly one table, chosen by its exact component set; a type's data is spread over every table that contains it. (part 2; Unity 6.5 *Archetypes concepts*)
- **Sparse set:** each component type has its own packed array; a sparse array maps entity → position. Pools know nothing about each other. (parts 2, 9)

The big matrix is the simplest of all and would also cope with 1,000 entities, but it scans *all* entities for every system and leaves holes in every array (no packed per-type data to export). The sparse set gives the same independent-pool simplicity **plus** packed arrays, so the realistic choice is between the other two.

---

## 5. Archetypes — as Unity Entities 6.5 implements them

Facts from the 6.5 manual `[VERIFIED 2026-11-05 against docs.unity3d.com, Entities 6.5]`:

- An archetype is the set of all entities in a world with the **same exact combination** of component types. Queries match archetypes, not entities; the set of archetypes "tends to stabilize early", so queries can be cached.
- Data lives in **chunks of 16 KiB**; each chunk holds one array per component type plus an entity array. **At most 128 entities fit in a chunk.**
- **Adding or removing a component moves the entity to another archetype** (creating it if needed); the last entity of the old chunk is moved into the gap. The manual: *"Moving entities frequently is resource-intensive and reduces the performance of your application."*
- **Structural changes** = create/destroy entity, add/remove component, set a shared component value. They can only run on the main thread and create **sync points**; Unity recommends recording them in **entity command buffers** and playing them back later.
- **Enableable components** exist "to avoid structural changes" for state that changes "often and unpredictably" — i.e. a workaround for the cost of add/remove.
- **Shared components** split chunks by value: *"A large number of unique values fragments entities across many sparsely occupied chunks."*
- **Fragmentation warning:** "a high number of archetypes with a low chunk count each" means too many different component sets; a chunk that is not full still costs 16 KiB.

What skypjack adds (part 2, part 10):

- Archetypes are **automatic** (no tuning) and give the **best multi-component iteration** and **easy multithreading** (hand chunks to threads).
- Cost: every add/remove copies *all* of the entity's components; fragmentation grows with the number of component combinations; a type is split over many tables, so there is no single `(T*, size)` array per type; sorting a type is hard; pointer stability is practically impossible.
- Implementation is "not trivial at all": archetype graph (add/remove edges), chunk allocator, query cache, structural-change tracking.

## 6. Sparse sets — as EnTT implements them

From skypjack parts 2, 3, 5, 9, 12, 13:

- A pool = **sparse array** (entity index → dense position) + **dense entity array** + **component array** in the same order.
- **Add:** push back, write the position in the sparse array — O(1).
  **Remove:** swap with the last element, pop — O(1). Only the pool of *that* type is touched; the entity's other components never move.
- **Contains / get:** one sparse-array lookup (a tombstone/null value or a version check avoids touching the dense array).
- **Single-type iteration:** walk one tightly packed array — the best possible.
  **Multi-type query:** walk the **smallest** pool and check membership in the others (one lookup per extra type). Optional *groups* reorder pools to remove even that check — skypjack advises not to use them while prototyping.
- **Sorting a pool** is easy (part 5); per-type tweaks (pagination, pointer stability) are possible because pools are independent (parts 11–13).
- Adding/removing during iteration is possible (iterate backwards, part 9), or deferred with a command buffer.
- Production use: EnTT is used in Minecraft (Mojang), ArcGIS Runtime SDKs, etc.

---

## 7. Comparison

| Criterion | Archetype | Sparse set | Better for us |
| --- | --- | --- | --- |
| **Implementation complexity** | High: archetype graph, chunk allocator, entity moves, query cache, fragmentation handling | Low–medium: one generic `ComponentPool<T>` + entity manager + query helper | **Sparse set** |
| **Add/remove component** | Move entity + *all* its components to another table; may allocate a chunk; needs deferral/sync points | O(1) in one pool; other components untouched | **Sparse set** |
| **Create/destroy entity** | Allocate slot in chunk; destroy moves the last entity into the gap | Recycle an id; remove from each pool the entity is in | ≈ |
| **Single-type iteration** | Over all chunks that contain the type (several arrays) | One packed array | Sparse set |
| **Multi-type iteration** | Packed arrays per matching archetype, no checks | Walk smallest pool + membership lookups | Archetype (difference negligible at < 1,000 entities, see §8) |
| **Random access `Get<T>(e)`** | entity → chunk + index → column | entity → pool sparse array → dense | ≈ |
| **Memory** | 16 KiB per chunk, ≤ 128 entities; mostly empty chunks with many archetypes | Sparse array per pool sized by max entity index (≈ 4 KB per pool for 1,000 entities) | Sparse set |
| **Frequent tag/state changes** | Each one is a structural change → Unity added enableable components to avoid them | Cheap | **Sparse set** |
| **Introspection / export** | Data for a type is scattered across tables | "All instances of T" = one array; "components of e" = ask each pool | **Sparse set** |
| **Deterministic order** | Deterministic for the same operation sequence; order depends on archetype/chunk layout | Deterministic for the same operation sequence; swap-and-pop changes order after removals | ≈ (both need a defined export order) |
| **Sorting / custom order** | Hard | Easy | Sparse set |
| **Multithreading** | Easy (chunks) | Possible, less trivial | Archetype (not needed) |
| **Learning value / debuggability** | Many moving parts | Small, inspectable data structures | **Sparse set** |

> Note added after PB-010.2: the API design iterates every query in **ascending entity ID** (decision D-3, coding standard rule 6). Add and remove stay O(1) swap-and-pop; each pool touched during command playback is sorted once at the end of playback (O(n log n), once per phase), so pools are sorted whenever a system runs. A sorted insert per command was considered and rejected, because scene entities have lower IDs than runtime ones and most gameplay changes would shift the pool. The comparison still holds: only one pool is touched, and the entity's other components never move.

## 8. What "our scale" really means `[ASSUMPTION — estimate, to be measured]`

Rough budget for the worst listed case (200 active entities, headless at 600 ticks/s):

- Suppose ~15 systems each run one 2–3 component query per tick over ≤ 200 entities: ≈ 15 × 200 × 2 = **6,000 membership lookups per tick**.
- At a pessimistic ~5 ns per lookup (cache misses included) that is ≈ **30 µs per tick**, about **2 %** of the 1.67 ms tick budget at 600 ticks/s.
- The data fits in the CPU cache: 1,000 entities × ~10 component types × ~32 bytes ≈ 320 KB (inside L3 on every reference CPU, inside L2 on many). The ≤ 200 entities a tick works on need ≈ 64 KB, well inside L2.

With archetypes the same queries would save some of those lookups, but the gain is a fraction of a percent of the tick, while every add/remove (which our gameplay does often) costs a full entity move. Physics (Box2D), rendering and the semantic export will dominate; skypjack makes the same point: at this level "bottlenecks will be elsewhere" (part 2), and the main reason to use an ECS is code organisation, not performance (part 1).

**Conclusion of the estimate:** iteration speed does not decide the choice at our scale; implementation cost, add/remove cost and introspection do.

---

## 9. Recommendation

**Use sparse-set storage with independent pools** — one `ComponentPool<T>` per component type, no groups.

Reasons (input for ADR-001):

1. **Simplest correct implementation** for a student team; each part (entity manager, pool, query) is small and unit-testable.
2. **Cheap add/remove**, which our gameplay roles and tag/state components rely on; no need for workarounds like enableable components or shared-component tuning.
3. **Full control of introspection for the Semantic Graph**: every component type is one packed, enumerable array with a registered name — exactly what the export needs (DEC-018 rationale, Q-013).
4. **Learning goal**: the data structures are understandable end-to-end.
5. Performance is more than sufficient for ≤ 1,000 entities (§8); the archetype advantages (multi-component iteration, multithreading) are ones we do not need.

**Deliberately not in the first version** (can be added later without changing the API): groups, pool pagination, pointer stability, multithreading, archetype-style chunking.

**What we still borrow from Unity's design** (model-independent ideas):

| Unity concept | Aegis use |
| --- | --- |
| System groups + `UpdateInGroup` / `UpdateBefore/After` | `ISystem::Stage` = the fixed tick phases (srs.fr.1.12, R-9); explicit order inside a stage |
| Entity command buffer + playback point | `CommandBuffer` for structural changes during iteration, played back at defined points in the tick |
| Tag components | Empty components for filtering (`Prefab`, `PlayerControlled`, …) |
| Prefab tag excluded from queries | Prefab entities are skipped by normal queries (also skypjack part 7) |
| Enableable components | "Enabled" state for SRS enable/disable requirements — with sparse sets this can simply be a tag component or a per-pool flag |
| Cleanup components | On-destroy hooks so the Physics Adapter releases the Box2D body |
| Singleton components | World-level data (input state, tick counter) |
| Systems never call each other | Already rule R-2: communicate through components and events |

## 10. Inputs for PB-010.2 (API design)

These came out of the reading and must be handled in `ecs.md`:

1. **Entity = `{uint32 index, uint32 generation}`.** Generation increments on destroy; recycle indices through an implicit free list inside the entity array (part 3). A stale handle fails `IsAlive`.
2. **Runtime handle ≠ persistent ID.** SRS srs.fr.2.64–69 requires a stable entity ID across save/load and scenes; index/generation is runtime-only. Plan a separate persistent ID (e.g. a component) used by scenes, prefabs, telemetry and the Semantic Graph.
3. **Stable component type identity.** Counter-based type ids (`family<T>`) change between runs and can break across DLL boundaries on Windows (part 8). Use explicit registration with a stable name and field metadata (`RegisterComponent<T>("Position", …)`); this is also the reflection the Semantic layer needs.
4. **Defined iteration/export order.** Swap-and-pop reorders pools after removals. Simulation stays deterministic for the same operation sequence, but telemetry and Semantic export should use a defined order (e.g. sorted by persistent ID). PB-010.2 sorts touched pools at the end of each command playback (D-3).
5. **Reference invalidation rule.** A `T&` from `Get<T>` is invalid after any add/remove of the same type `T`. Systems and the Physics Adapter store entity handles, never component pointers.
6. **Structural changes during iteration go through `CommandBuffer`.** (Backward-iteration tricks are optional and not needed.)
7. **Hierarchy** (SRS transform composition, srs.fr.3): a parent/children component with first/next/prev links (part 4); parent-before-child updates via a dirty tag + sort (part 4 insights).
8. **Prefabs and shared data** (part 7): prefab = entity + `Prefab` tag; assets referenced by handle to an asset manager outside the ECS (flyweight / "weak reference").

### Open points for the team (not decided here)

- **Where ADR-001 lives:** `docs/adr/` as the task says, or `DEC-038` in `docs/history/decision-log.md` (the repo's current convention).
- **Q-013** (what "LLM-instructive" component metadata contains) is still deferred; it determines the reflection/metadata part of PB-010.2.
- **Final location of this research:** `docs/design/ecs.md` (per task) or `docs/research/ecs-storage.md` next to `research/box2d.md`.

---

## 11. References

skypjack, *ECS back and forth* (local copies in `SP/_posts_about_ecs/`):

- Part 1 — Introduction (big matrix, why ECS): https://skypjack.github.io/2019-02-14-ecs-baf-part-1/
- Part 2 — Archetypes vs sparse sets: https://skypjack.github.io/2019-03-07-ecs-baf-part-2/
- Part 2 insights — Groups: https://skypjack.github.io/2019-03-21-ecs-baf-part-2-insights/
- Part 3 — Entity recycling and versions: https://skypjack.github.io/2019-05-06-ecs-baf-part-3/
- Part 4 + insights — Hierarchies: https://skypjack.github.io/2019-06-25-ecs-baf-part-4/ , https://skypjack.github.io/2019-08-20-ecs-baf-part-4-insights/
- Part 5 — Sorting sparse sets: https://skypjack.github.io/2019-09-25-ecs-baf-part-5/
- Part 6 — Nested groups: https://skypjack.github.io/2019-11-19-ecs-baf-part-6/
- Part 7 — Shared data, prefabs: https://skypjack.github.io/2020-02-02-ecs-baf-part-7/
- Part 8 — Type ids: https://skypjack.github.io/2020-03-14-ecs-baf-part-8/
- Part 9 — Sparse sets in EnTT: https://skypjack.github.io/2020-08-02-ecs-baf-part-9/
- Part 10 — Hybrid storage: https://skypjack.github.io/2021-02-27-ecs-baf-part-10/
- Part 11 — Big matrix revised: https://skypjack.github.io/2021-06-12-ecs-baf-part-11/
- Parts 12–13 — Pointer stability: https://skypjack.github.io/2021-08-29-ecs-baf-part-12/ , https://skypjack.github.io/2021-10-09-ecs-baf-part-13/
- EnTT repository: https://github.com/skypjack/entt

Unity Entities 6.5 manual:

- Archetypes concepts: https://docs.unity3d.com/Packages/com.unity.entities@6.5/manual/concepts-archetypes.html
- Managing chunk allocations: https://docs.unity3d.com/Packages/com.unity.entities@6.5/manual/performance-chunk-allocations.html
- Structural changes concepts: https://docs.unity3d.com/Packages/com.unity.entities@6.5/manual/concepts-structural-changes.html
- Enableable components: https://docs.unity3d.com/Packages/com.unity.entities@6.5/manual/components-enableable-intro.html
- Shared components: https://docs.unity3d.com/Packages/com.unity.entities@6.5/manual/components-shared-introducing.html
- System update order: https://docs.unity3d.com/Packages/com.unity.entities@6.5/manual/systems-update-order.html
- ArchetypeChunk API: https://docs.unity3d.com/Packages/com.unity.entities@6.5/api/Unity.Entities.ArchetypeChunk.html
