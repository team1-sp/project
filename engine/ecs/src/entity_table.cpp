#include "engine/ecs/entity_table.hpp"

#include <algorithm>
#include <limits>

#include "engine/core/run_error.hpp"

namespace engine {

namespace {

constexpr std::uint32_t lastGeneration = std::numeric_limits<std::uint32_t>::max();

bool idLess(const std::pair<EntityId, std::uint32_t>& entry, EntityId id) {
    return entry.first < id;
}

} // namespace

Entity EntityTable::create(EntityId id) {
    std::uint32_t index = 0;
    if (!freeSlots_.empty()) {
        index = freeSlots_.back();
        freeSlots_.pop_back();
    } else {
        // Slot indices must stay below the two marker values of runtimeSlots_.
        if (slots_.size() >= reservedId) {
            throw RunError("EntityTable: no free entity slot left");
        }
        index = static_cast<std::uint32_t>(slots_.size());
        slots_.emplace_back();
    }
    Slot& slot = slots_[index];
    slot.id = id;
    slot.alive = true;
    slot.enabled = true;
    slot.queryEnabled = true;

    if (isRuntimeId(id)) {
        runtimeSlots_[runtimeOffset(id)] = index;
        ++aliveRuntimeCount_;
    } else {
        const auto at = std::lower_bound(editorSlots_.begin(), editorSlots_.end(), id, idLess);
        editorSlots_.insert(at, {id, index});
    }
    ++aliveCount_;
    return Entity{index, slot.generation};
}

void EntityTable::destroy(Entity e) {
    Slot& slot = slots_[e.index];
    if (isRuntimeId(slot.id)) {
        runtimeSlots_[runtimeOffset(slot.id)] = releasedId;
        --aliveRuntimeCount_;
    } else {
        const auto at = std::lower_bound(editorSlots_.begin(), editorSlots_.end(), slot.id, idLess);
        editorSlots_.erase(at);
    }
    slot.alive = false;
    slot.id = noEntity;
    --aliveCount_;
    if (slot.generation == lastGeneration) {
        return; // retired: never reused
    }
    ++slot.generation;
    freeSlots_.push_back(e.index);
}

bool EntityTable::isAlive(Entity e) const {
    return e.index < slots_.size() && slots_[e.index].alive &&
           slots_[e.index].generation == e.generation;
}

std::optional<Entity> EntityTable::find(EntityId id) const {
    std::uint32_t index = releasedId;
    if (isRuntimeId(id)) {
        const std::size_t offset = runtimeOffset(id);
        if (offset < runtimeSlots_.size()) {
            index = runtimeSlots_[offset];
        }
    } else {
        const auto at = std::lower_bound(editorSlots_.begin(), editorSlots_.end(), id, idLess);
        if (at != editorSlots_.end() && at->first == id) {
            index = at->second;
        }
    }
    if (index == reservedId || index == releasedId) {
        return std::nullopt;
    }
    return Entity{index, slots_[index].generation};
}

EntityId EntityTable::allocateRuntimeId() {
    const std::uint64_t next = static_cast<std::uint64_t>(firstRuntimeEntityId) +
                               static_cast<std::uint64_t>(runtimeSlots_.size());
    runtimeSlots_.push_back(reservedId);
    return EntityId{next};
}

bool EntityTable::isReservedRuntimeId(EntityId id) const {
    if (!isRuntimeId(id)) {
        return false;
    }
    const std::size_t offset = runtimeOffset(id);
    return offset < runtimeSlots_.size() && runtimeSlots_[offset] == reservedId;
}

void EntityTable::resetRuntimeIds() {
    runtimeSlots_.clear();
}

} // namespace engine
