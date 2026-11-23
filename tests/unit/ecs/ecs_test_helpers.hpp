#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/core/logging.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/world_observer.hpp"

namespace ecs_test {

inline engine::EntityId editorId(std::uint64_t value) {
    return engine::EntityId{value};
}

inline engine::EntityId runtimeId(std::uint64_t offset) {
    return engine::EntityId{static_cast<std::uint64_t>(engine::firstRuntimeEntityId) + offset};
}

// Remembers every observer call in order.
class RecordingObserver final : public engine::IWorldObserver {
public:
    void onEntityDestroyed(engine::Entity /*e*/, engine::EntityId id) override {
        destroyed.push_back(id);
    }
    void onEntityEnabledChanged(engine::Entity e, bool enabled) override {
        enabledChanges.emplace_back(e, enabled);
    }

    std::vector<engine::EntityId> destroyed;
    std::vector<std::pair<engine::Entity, bool>> enabledChanges;
};

class RecordingLogSink final : public engine::ILogSink {
public:
    void write(engine::LogEntry entry) override { entries.push_back(std::move(entry)); }

    std::vector<engine::LogEntry> entries;
};

} // namespace ecs_test
