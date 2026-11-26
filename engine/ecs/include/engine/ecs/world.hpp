#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/core/logging.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/component_pool.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/entity_table.hpp"
#include "engine/ecs/gameplay_random.hpp"
#include "engine/ecs/type_id.hpp"
#include "engine/ecs/type_registry.hpp"
#include "engine/ecs/world_observer.hpp"

namespace engine {

class CommandBuffer;
class SystemContext;
class SystemScheduler;

// Everything one simulation holds: its entities and their components (design/ecs-api-design.md
// §3, §5). A process may hold several Worlds that share one finalized TypeRegistry; nothing in a
// World is global or shared (D-23).
//
// The calls below change the World at once. They are for scene loading, the Editor and tests;
// systems record structural changes in a CommandBuffer instead, which applies them at the end of
// the tick phase (§9, srs.fr.2.116). Every failed precondition (a stale handle, an ID in use, an
// unregistered type, a missing or duplicate component, a parent that would make a cycle) throws
// engine::RunError (D-29, srs.fr.1.11), in every build.
class World {
public:
    // `types` must be finalized and must outlive the World. The World makes one pool per type.
    explicit World(const TypeRegistry& types);
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = delete;
    World& operator=(World&&) = delete;
    ~World();

    [[nodiscard]] const TypeRegistry& types() const { return types_; }

    // --- Entities (srs.fr.2.1 to 2.5) ---

    // Creates an entity with the next runtime ID (srs.fr.2.63) and its built-in components:
    // an empty name, enabled, tag Untagged, layer Default, no parent.
    Entity createEntity();
    // Creates an entity with a given ID: an Editor-range ID that no loaded entity uses (scene
    // loading, the Editor), or a runtime ID from allocateRuntimeId() that no entity has used.
    Entity createEntity(EntityId id);
    // Destroys the entity and all of its descendants (srs.fr.2.4). Observers hear about each of
    // them, in depth-first order, before any component is removed.
    void destroyEntity(Entity e);
    [[nodiscard]] bool isAlive(Entity e) const { return entities_.isAlive(e); }
    [[nodiscard]] std::optional<Entity> find(EntityId id) const { return entities_.find(id); }
    [[nodiscard]] EntityId idOf(Entity e) const;
    // Reserves the next runtime ID, for a loader that must know an entity's ID before creating
    // it, such as on an ID collision (srs.fr.2.64). It is never given out again in this run.
    EntityId allocateRuntimeId() { return entities_.allocateRuntimeId(); }
    // Live entities, including ones a CommandBuffer has created but not played back yet.
    [[nodiscard]] std::size_t entityCount() const { return entities_.aliveCount(); }

    // The entity enabled flag (srs.fr.2.5). Changing it is not a structural change: the flag
    // changes at once and observers are told, and queries see the change from the next query
    // on, never in the middle of one (srs.fr.2.116). It affects only this entity, not its
    // descendants (DEC-48.1). Queries skip disabled entities (srs.fr.2.15).
    void setEnabled(Entity e, bool enabled);
    [[nodiscard]] bool isEnabled(Entity e) const;

    // Makes `child` the last child of `parent`, or a root when `parent` is nullEntity (srs.fr.2.5).
    // Refused when `parent` is `child` itself or one of its descendants.
    void setParent(Entity child, Entity parent);

    // --- Components (srs.fr.2.6 to 2.12) ---

    // Adds a component of a registered type (srs.fr.2.6); an entity has at most one of each type
    // (srs.fr.2.10), so adding a second is refused. A new component is enabled. Every entity
    // already has the built-in components.
    template <ComponentType T>
    T& add(Entity e, T value = T{});
    // Removes a component (srs.fr.2.7). The built-in components cannot be removed.
    template <ComponentType T>
    void remove(Entity e);

    template <ComponentType T>
    [[nodiscard]] bool has(Entity e) const {
        checkAlive(e, "World::has");
        return typedPool<T>("World::has").contains(e);
    }

    // Read access (srs.fr.2.8). Null when the entity has no such component.
    template <ComponentType T>
    [[nodiscard]] const T* tryGet(Entity e) const {
        checkAlive(e, "World::tryGet");
        return typedPool<T>("World::tryGet").tryGet(e);
    }

    // Write access (srs.fr.2.9). The Hierarchy component is read-only here: setParent() keeps its
    // links consistent.
    template <ComponentType T>
        requires(!std::same_as<T, HierarchyComponent>)
    [[nodiscard]] T* tryGet(Entity e) {
        return const_cast<T*>(std::as_const(*this).tryGet<T>(e));
    }

    template <ComponentType T>
    [[nodiscard]] const T& get(Entity e) const {
        const T* component = tryGet<T>(e);
        if (component == nullptr) {
            throw RunError("World::get: " + text(e) + " has no " + std::string(T::typeName) +
                           " component");
        }
        return *component;
    }

    template <ComponentType T>
        requires(!std::same_as<T, HierarchyComponent>)
    [[nodiscard]] T& get(Entity e) {
        return const_cast<T&>(std::as_const(*this).get<T>(e));
    }

    // The component enabled flag (srs.fr.2.11). Position, Velocity and the built-in components
    // have none, and setting it on them is refused. Queries skip a disabled component at once.
    template <ComponentType T>
    void setComponentEnabled(Entity e, bool enabled);
    template <ComponentType T>
    [[nodiscard]] bool isComponentEnabled(Entity e) const;

    // Pending-write marks of Position and Velocity (D-27, srs.fr.3.21): a system's non-const
    // access sets one (SystemContext does it); PhysicsSyncIn applies the write and clears it;
    // PhysicsSyncOut skips a marked component (srs.fr.3.48). Direct World::get() calls (spawn
    // code, tests) do not mark; call markWritten() when such a change must reach the body.
    // For a type that does not track writes these do nothing and hasPendingWrite() is false.
    template <ComponentType T>
    void markWritten(Entity e) {
        checkAlive(e, "World::markWritten");
        typedPool<T>("World::markWritten").markWritten(e);
    }
    template <ComponentType T>
    [[nodiscard]] bool hasPendingWrite(Entity e) const {
        checkAlive(e, "World::hasPendingWrite");
        return typedPool<T>("World::hasPendingWrite").hasPendingWrite(e);
    }
    template <ComponentType T>
    void clearPendingWrite(Entity e) {
        checkAlive(e, "World::clearPendingWrite");
        typedPool<T>("World::clearPendingWrite").clearPendingWrite(e);
    }

    // Read-only view of a type's storage, in ascending EntityId, for code that reads every
    // component of a type, such as telemetry and scene saving (§12).
    template <ComponentType T>
    [[nodiscard]] const ComponentPool<T>& pool() const {
        return typedPool<T>("World::pool");
    }
    // The same, by type ID; null for a type that is not registered.
    [[nodiscard]] const IComponentPool* pool(TypeId type) const;
    // Calls `fn` for each component of `e`, in registration order of the types, with the type's
    // description and the component's raw memory (the Inspector, scene saving).
    void
    forEachComponent(Entity e,
                     const std::function<void(const ComponentTypeInfo&, const void*)>& fn) const;

    // --- Queries (srs.fr.2.18, design/ecs-api-design.md §7) ---

    // Calls `fn` for every enabled entity that has all of `Ts`, each of them enabled, in ascending
    // EntityId (coding standard rule 6). `fn` takes the components in the order of `Ts`, optionally
    // preceded by the Entity; a `const T` is passed as `const T&`. Disabled entities are skipped,
    // their Position and Velocity too (srs.fr.2.15). No structural change can happen while the
    // query runs: a direct structural World call from inside it is refused. This form checks no
    // system declaration; systems query through SystemContext::each().
    template <typename... Ts, typename Fn>
    void each(Fn&& fn);

    // Singletons (D-16): the component of the one entity that has a T. Refused unless exactly one
    // entity has one.
    template <ComponentType T>
    [[nodiscard]] const T& single() const;
    template <ComponentType T>
        requires(!std::same_as<T, HierarchyComponent>)
    [[nodiscard]] T& single() {
        return const_cast<T&>(std::as_const(*this).single<T>());
    }
    // Creates a runtime entity that holds `value`, for world-level data such as the input state.
    template <ComponentType T>
    Entity createSingleton(T value = T{});

    // --- Systems (srs.fr.2.13 to 2.17) ---

    [[nodiscard]] SystemScheduler& systems() { return *systems_; }
    [[nodiscard]] const SystemScheduler& systems() const { return *systems_; }

    // --- Runs ---

    // Starts a run: runtime IDs start again at their first value, so runs of the same content
    // give the same IDs (srs.fr.2.63), and the gameplay random number generator starts the
    // sequence of the run's seed (srs.fr.7.5). Refused while an entity with a runtime ID is alive,
    // because its ID would be given out again (srs.fr.2.2).
    void beginRun(std::uint64_t seed = 0);

    // --- Observers and warnings ---

    // Observers are told in the order they were added. They are not owned and must outlive the
    // World.
    void addObserver(IWorldObserver& observer) { observers_.push_back(&observer); }
    // Where warnings go, such as a skipped command at playback. Without a sink they are written
    // to the standard error output. The sink is not owned.
    void setLogSink(ILogSink* sink) { logSink_ = sink; }

private:
    friend class CommandBuffer;
    friend class SystemContext;
    friend class SystemScheduler;

    template <ComponentType T>
    [[nodiscard]] const ComponentTypeInfo& infoOf(std::string_view operation) const {
        const ComponentTypeInfo* info = types_.find<T>();
        if (info == nullptr) {
            throw RunError(std::string(operation) + ": component type " + std::string(T::typeName) +
                           " is not registered");
        }
        return *info;
    }

    template <ComponentType T>
    [[nodiscard]] const ComponentPool<T>& typedPool(std::string_view operation) const {
        return static_cast<const ComponentPool<T>&>(*pools_[infoOf<T>(operation).index]);
    }

    template <ComponentType T>
    [[nodiscard]] ComponentPool<T>& typedPool(std::string_view operation) {
        return const_cast<ComponentPool<T>&>(std::as_const(*this).typedPool<T>(operation));
    }

    void checkAlive(Entity e, std::string_view operation) const;
    void checkNewId(EntityId id, std::string_view operation) const;
    // Structural changes are refused while a query runs (§7 rule 4).
    void checkNotQuerying(std::string_view operation) const;
    // Makes enabled-flag changes visible to queries; called when an outermost query starts.
    void applyPendingEnables();
    // "entity <id>" for messages; `e` must be alive.
    [[nodiscard]] std::string text(Entity e) const;
    void notifyAdded(Entity e, TypeId type);
    void notifyRemoved(Entity e, TypeId type);

    // An entity created by a CommandBuffer: handle and ID now, components at playback (D-5).
    Entity reserveEntity();
    void addBuiltIns(Entity e, EntityId id);
    // Destroys `root` and its descendants without sorting the pools (the caller sorts).
    void destroyTree(Entity root);
    // Frees the slot of an entity that was created and destroyed in one playback and never got
    // components; nobody is told about it (§9).
    void releaseUnplayed(Entity e) { entities_.destroy(e); }
    // Why `child` cannot get `parent`, or nothing when it can.
    [[nodiscard]] std::optional<std::string> parentProblem(Entity child, Entity parent) const;
    // Moves `child` under `parent` (or to the roots); parentProblem() must have found nothing.
    void link(Entity child, Entity parent);
    void unlink(Entity child);
    [[nodiscard]] Entity linked(EntityId id) const;
    [[nodiscard]] HierarchyComponent& hierarchyOf(Entity e);
    void collectSubtree(Entity root);
    void sortPools();
    void warn(std::string message, EntityId id);

    const TypeRegistry& types_;
    EntityTable entities_;
    std::vector<std::unique_ptr<IComponentPool>> pools_; // by type index
    ComponentPool<HierarchyComponent>* hierarchies_ = nullptr;
    std::vector<IWorldObserver*> observers_;
    ILogSink* logSink_ = nullptr;
    // Scratch buffers of destroyTree(), kept so that destroying allocates nothing once warm.
    std::vector<Entity> subtree_;
    std::vector<Entity> pending_;
    // Entities whose enabled flag changed since the last query started (srs.fr.2.116).
    std::vector<Entity> pendingEnables_;
    std::size_t queryDepth_ = 0;
    GameplayRandom random_;
    std::unique_ptr<SystemScheduler> systems_;
};

// --- templates ---

template <typename... Ts, typename Fn>
void World::each(Fn&& fn) {
    static_assert(sizeof...(Ts) > 0, "a query names at least one component type");
    if (queryDepth_ == 0) {
        applyPendingEnables();
    }
    const std::tuple<ComponentPool<std::remove_const_t<Ts>>*...> pools{
        &typedPool<std::remove_const_t<Ts>>("World::each")...};
    // The smallest pool drives the loop; every pool is in ascending EntityId (D-3).
    const IComponentPool* driver = nullptr;
    std::apply(
        [&driver](const auto*... pool) {
            ((driver = (driver == nullptr || pool->size() < driver->size()) ? pool : driver), ...);
        },
        pools);
    struct DepthGuard {
        std::size_t& depth;
        ~DepthGuard() { --depth; }
    } guard{++queryDepth_};
    const std::size_t count = driver->size();
    for (std::size_t pos = 0; pos < count; ++pos) {
        const Entity e = driver->entityAt(pos);
        if (!entities_.isQueryEnabled(e)) {
            continue;
        }
        const bool matches = std::apply(
            [e](const auto*... pool) { return (pool->containsEnabled(e) && ...); }, pools);
        if (!matches) {
            continue;
        }
        std::apply(
            [&fn, e](auto*... pool) {
                if constexpr (std::is_invocable_v<Fn&, Entity, Ts&...>) {
                    fn(e, *pool->tryGet(e)...);
                } else {
                    fn(*pool->tryGet(e)...);
                }
            },
            pools);
    }
}

template <ComponentType T>
const T& World::single() const {
    const ComponentPool<T>& components = typedPool<T>("World::single");
    if (components.size() != 1) {
        throw RunError("World::single: " + std::to_string(components.size()) + " entities have a " +
                       std::string(T::typeName) + " component, not exactly one");
    }
    return components.componentAt(0);
}

template <ComponentType T>
Entity World::createSingleton(T value) {
    const Entity e = createEntity();
    add(e, std::move(value));
    return e;
}

template <ComponentType T>
T& World::add(Entity e, T value) {
    checkNotQuerying("World::add");
    checkAlive(e, "World::add");
    const ComponentTypeInfo& info = infoOf<T>("World::add");
    ComponentPool<T>& components = typedPool<T>("World::add");
    if (components.contains(e)) {
        throw RunError("World::add: " + text(e) + " already has a " + info.name + " component");
    }
    components.add(e, entities_.idOf(e), std::move(value));
    components.sortIfDirty();
    notifyAdded(e, info.id);
    return *components.tryGet(e);
}

template <ComponentType T>
void World::remove(Entity e) {
    checkNotQuerying("World::remove");
    checkAlive(e, "World::remove");
    const ComponentTypeInfo& info = infoOf<T>("World::remove");
    if (info.builtIn) {
        throw RunError("World::remove: " + info.name + " is a built-in component");
    }
    ComponentPool<T>& components = typedPool<T>("World::remove");
    if (!components.removeIfPresent(e)) {
        throw RunError("World::remove: " + text(e) + " has no " + info.name + " component");
    }
    components.sortIfDirty();
    notifyRemoved(e, info.id);
}

template <ComponentType T>
void World::setComponentEnabled(Entity e, bool enabled) {
    checkAlive(e, "World::setComponentEnabled");
    const ComponentTypeInfo& info = infoOf<T>("World::setComponentEnabled");
    if (info.alwaysEnabled) {
        throw RunError("World::setComponentEnabled: " + info.name +
                       " has no enabled flag of its own");
    }
    ComponentPool<T>& components = typedPool<T>("World::setComponentEnabled");
    if (!components.contains(e)) {
        throw RunError("World::setComponentEnabled: " + text(e) + " has no " + info.name +
                       " component");
    }
    components.setEnabled(e, enabled);
}

template <ComponentType T>
bool World::isComponentEnabled(Entity e) const {
    checkAlive(e, "World::isComponentEnabled");
    const ComponentPool<T>& components = typedPool<T>("World::isComponentEnabled");
    if (!components.contains(e)) {
        throw RunError("World::isComponentEnabled: " + text(e) + " has no " +
                       std::string(T::typeName) + " component");
    }
    return components.containsEnabled(e);
}

} // namespace engine
