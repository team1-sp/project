#pragma once

#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/core/logging.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/field.hpp"
#include "engine/ecs/fixed_string.hpp"
#include "engine/ecs/type_id.hpp"
#include "engine/ecs/type_registry.hpp"
#include "engine/ecs/world_observer.hpp"

namespace ecs_test {

inline engine::EntityId editorId(std::uint64_t value) {
    return engine::EntityId{value};
}

inline engine::EntityId runtimeId(std::uint64_t offset) {
    return engine::EntityId{static_cast<std::uint64_t>(engine::firstRuntimeEntityId) + offset};
}

// Components only the tests use.
struct HealthComponent {
    static constexpr std::string_view typeName = "Health";
    std::int32_t points = 100;
};

enum class MoodKind : std::uint8_t { Calm, Angry };

struct MoodComponent {
    static constexpr std::string_view typeName = "Mood";
    MoodKind mood = MoodKind::Calm;
    engine::FixedString<16> label;
};

struct MarkerTag {
    static constexpr std::string_view typeName = "Marker";
};

// One finalized registry for all ECS tests: the built-ins, Position, Velocity and the test types.
inline const engine::TypeRegistry& types() {
    static const engine::TypeRegistry registry = [] {
        engine::TypeRegistry r;
        engine::registerCoreComponents(r);
        r.enumeration<MoodKind>("MoodKind", "How an entity feels")
            .value(MoodKind::Calm, "Calm", "Not angry")
            .value(MoodKind::Angry, "Angry", "Angry");
        r.component<HealthComponent>()
            .describe("Hit points of a test entity.")
            .field("points", &HealthComponent::points, engine::Unit::None, "Remaining hit points");
        r.component<MoodComponent>()
            .describe("Mood of a test entity.")
            .field("mood", &MoodComponent::mood, engine::Unit::None, "Current mood")
            .field("label", &MoodComponent::label, engine::Unit::None, "Short label");
        r.component<MarkerTag>().describe("Marks a test entity.");
        r.finalize();
        return r;
    }();
    return registry;
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
    void onComponentAdded(engine::Entity /*e*/, engine::TypeId type) override {
        added.push_back(type);
    }
    void onComponentRemoved(engine::Entity /*e*/, engine::TypeId type) override {
        removed.push_back(type);
    }

    std::vector<engine::EntityId> destroyed;
    std::vector<std::pair<engine::Entity, bool>> enabledChanges;
    std::vector<engine::TypeId> added;
    std::vector<engine::TypeId> removed;
};

class RecordingLogSink final : public engine::ILogSink {
public:
    void write(engine::LogEntry entry) override { entries.push_back(std::move(entry)); }

    std::vector<engine::LogEntry> entries;
};

} // namespace ecs_test
