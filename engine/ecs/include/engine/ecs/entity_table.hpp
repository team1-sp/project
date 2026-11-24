#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/ecs/entity.hpp"

namespace engine {

// The World's table of entities (design/ecs-api-design.md §4): which slots are alive, the
// generation, EntityId and enabled flag of each, and the lookups from EntityId to slot. The World
// checks every precondition before calling it and turns misuse into engine::RunError.
//
// EntityIds (srs.fr.2.104): 1 to 2^32 - 1 is the Editor range; the Editor allocates these IDs and
// the table only maps them. 2^32 and above is the runtime range, which the table allocates in
// increasing order (srs.fr.2.63) and never gives out twice until resetRuntimeIds() (srs.fr.2.2).
class EntityTable {
public:
    // Takes a free slot, or appends one, for an entity with ID `id`. The caller has checked that
    // `id` may be used (World::createEntity).
    Entity create(EntityId id);
    // Frees the slot of the live entity `e`. Its generation goes up, so every handle to it
    // becomes stale, and its index is reused by a later create(), the last freed first. A slot
    // whose generation would wrap around is retired instead, so a stale handle never matches it.
    void destroy(Entity e);

    [[nodiscard]] bool isAlive(Entity e) const;
    [[nodiscard]] std::optional<Entity> find(EntityId id) const;
    // `e` must be alive.
    [[nodiscard]] EntityId idOf(Entity e) const { return slots_[e.index].id; }
    [[nodiscard]] bool isEnabled(Entity e) const { return slots_[e.index].enabled; }
    void setEnabled(Entity e, bool enabled) { slots_[e.index].enabled = enabled; }
    // The enabled flag as queries see it: it follows the enabled flag when the next query
    // starts, never during one (srs.fr.2.116).
    [[nodiscard]] bool isQueryEnabled(Entity e) const { return slots_[e.index].queryEnabled; }
    void setQueryEnabled(Entity e, bool enabled) { slots_[e.index].queryEnabled = enabled; }
    [[nodiscard]] std::size_t aliveCount() const { return aliveCount_; }

    // Gives out the next runtime ID. It stays reserved until an entity is created with it.
    EntityId allocateRuntimeId();
    // Whether `id` came from allocateRuntimeId() and no entity has been created with it yet.
    [[nodiscard]] bool isReservedRuntimeId(EntityId id) const;
    [[nodiscard]] bool hasAliveRuntimeEntity() const { return aliveRuntimeCount_ > 0; }
    // Starts runtime IDs again at firstRuntimeEntityId. Only for a table with no live runtime
    // entity: a run start (srs.fr.2.63).
    void resetRuntimeIds();

private:
    struct Slot {
        std::uint32_t generation = 0;
        EntityId id = noEntity;
        bool alive = false;
        bool enabled = true;
        bool queryEnabled = true;
    };

    // Values of runtimeSlots_ that are not slot indices.
    static constexpr std::uint32_t reservedId = 0xFFFF'FFFE; // allocated, no entity yet
    static constexpr std::uint32_t releasedId = 0xFFFF'FFFF; // its entity was destroyed

    [[nodiscard]] static bool isRuntimeId(EntityId id) { return id >= firstRuntimeEntityId; }
    [[nodiscard]] static std::size_t runtimeOffset(EntityId id) {
        return static_cast<std::size_t>(static_cast<std::uint64_t>(id) -
                                        static_cast<std::uint64_t>(firstRuntimeEntityId));
    }

    std::vector<Slot> slots_;
    std::vector<std::uint32_t> freeSlots_; // destroyed indices, reused last in, first out
    // Runtime ID - firstRuntimeEntityId -> slot index, reservedId or releasedId. Runtime IDs are
    // dense, so a plain vector is the lookup; find() is on hot paths and uses no hash map.
    std::vector<std::uint32_t> runtimeSlots_;
    // Editor-range IDs and their slots, sorted by ID. They change only at scene load and Editor
    // edits, so a sorted vector with binary search is enough.
    std::vector<std::pair<EntityId, std::uint32_t>> editorSlots_;
    std::size_t aliveCount_ = 0;
    std::size_t aliveRuntimeCount_ = 0;
};

} // namespace engine
