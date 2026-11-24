#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/core/logging.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/component_pool.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/entity_table.hpp"
#include "engine/ecs/world_observer.hpp"

namespace engine {

class CommandBuffer;

// The built-in component types (D-12). Until the type registry and the general component API
// arrive with US-2.2, they are the only components a World stores.
template <typename T>
concept BuiltInComponent = std::same_as<T, NameComponent> || std::same_as<T, EntityInfoComponent> ||
                           std::same_as<T, HierarchyComponent>;

// Everything one simulation holds: its entities and their components (design/ecs-api-design.md
// §3, §5). A process may hold several Worlds; nothing in a World is global or shared (D-23).
//
// The calls below change the World at once. They are for scene loading, the Editor and tests;
// systems record structural changes in a CommandBuffer instead, which applies them at the end of
// the tick phase (§9, srs.fr.2.116). Every failed precondition (a stale handle, an ID in use, a
// missing component, a parent that would make a cycle) throws engine::RunError (D-29,
// srs.fr.1.11), in every build.
class World {
public:
    World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = delete;
    World& operator=(World&&) = delete;
    ~World() = default;

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
    // changes at once (srs.fr.2.116) and observers are told. Systems skip disabled entities once
    // systems and queries exist (US-2.3, srs.fr.2.15).
    void setEnabled(Entity e, bool enabled);
    [[nodiscard]] bool isEnabled(Entity e) const;

    // Makes `child` the last child of `parent`, or a root when `parent` is nullEntity (srs.fr.2.5).
    // Refused when `parent` is `child` itself or one of its descendants.
    void setParent(Entity child, Entity parent);

    // --- Built-in components (D-12) ---

    // Every entity has all three from its creation until it is destroyed, except an entity a
    // CommandBuffer created that has not been played back yet. The Hierarchy component is
    // read-only here: setParent() keeps its links consistent.
    template <BuiltInComponent T>
    [[nodiscard]] bool has(Entity e) const {
        checkAlive(e, "World::has");
        return pool<T>().contains(e);
    }

    template <BuiltInComponent T>
    [[nodiscard]] const T* tryGet(Entity e) const {
        checkAlive(e, "World::tryGet");
        return pool<T>().tryGet(e);
    }

    template <BuiltInComponent T>
        requires(!std::same_as<T, HierarchyComponent>)
    [[nodiscard]] T* tryGet(Entity e) {
        return const_cast<T*>(std::as_const(*this).tryGet<T>(e));
    }

    template <BuiltInComponent T>
    [[nodiscard]] const T& get(Entity e) const {
        const T* component = tryGet<T>(e);
        if (component == nullptr) {
            throw RunError(missingComponentMessage(e, T::typeName));
        }
        return *component;
    }

    template <BuiltInComponent T>
        requires(!std::same_as<T, HierarchyComponent>)
    [[nodiscard]] T& get(Entity e) {
        return const_cast<T&>(std::as_const(*this).get<T>(e));
    }

    // Read-only view of a component type's storage, in ascending EntityId, for code that reads
    // every component of a type, such as telemetry and scene saving (§12).
    template <BuiltInComponent T>
    [[nodiscard]] const ComponentPool<T>& pool() const {
        if constexpr (std::same_as<T, NameComponent>) {
            return names_;
        } else if constexpr (std::same_as<T, EntityInfoComponent>) {
            return infos_;
        } else {
            return hierarchies_;
        }
    }

    // --- Runs ---

    // Starts runtime IDs again at their first value, so runs of the same content give the same
    // IDs (srs.fr.2.63). Refused while an entity with a runtime ID is alive, because its ID would
    // be given out again (srs.fr.2.2).
    void beginRun();

    // --- Observers and warnings ---

    // Observers are told in the order they were added. They are not owned and must outlive the
    // World.
    void addObserver(IWorldObserver& observer) { observers_.push_back(&observer); }
    // Where warnings go, such as a skipped command at playback. Without a sink they are written
    // to the standard error output. The sink is not owned.
    void setLogSink(ILogSink* sink) { logSink_ = sink; }

private:
    friend class CommandBuffer;

    void checkAlive(Entity e, std::string_view operation) const;
    void checkNewId(EntityId id, std::string_view operation) const;
    [[nodiscard]] std::string missingComponentMessage(Entity e, std::string_view type) const;

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

    EntityTable entities_;
    ComponentPool<NameComponent> names_;
    ComponentPool<EntityInfoComponent> infos_;
    ComponentPool<HierarchyComponent> hierarchies_;
    std::array<IComponentPool*, 3> pools_{&names_, &infos_, &hierarchies_};
    std::vector<IWorldObserver*> observers_;
    ILogSink* logSink_ = nullptr;
    // Scratch buffers of destroyTree(), kept so that destroying allocates nothing once warm.
    std::vector<Entity> subtree_;
    std::vector<Entity> pending_;
};

} // namespace engine
