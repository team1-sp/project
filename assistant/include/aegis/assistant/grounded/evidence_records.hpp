#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/vec2.hpp"

// Read-only evidence values the grounded assistant (US-11.1) receives from the Semantic layer and
// the telemetry recording. They are plain copies: the assistant never holds a reference into
// engine or recording state, so it cannot change it.
namespace aegis::assistant {

// A point in a recorded run (srs.fr.8.26): either the run's initial state, shown as "Start", or
// the state after tick N completed. Start is never tick 0.
class StatePoint {
public:
    [[nodiscard]] static constexpr StatePoint start() noexcept { return StatePoint(true, 0); }
    [[nodiscard]] static constexpr StatePoint atTick(engine::Tick tick) noexcept {
        return StatePoint(false, tick);
    }

    [[nodiscard]] constexpr bool isStart() const noexcept { return start_; }
    [[nodiscard]] constexpr std::optional<engine::Tick> tick() const noexcept {
        return start_ ? std::nullopt : std::optional<engine::Tick>(tick_);
    }

    friend constexpr bool operator==(StatePoint, StatePoint) = default;

private:
    constexpr StatePoint(bool isStartPoint, engine::Tick tick) noexcept
        : start_(isStartPoint)
        , tick_(tick) {}

    bool start_ = true;
    engine::Tick tick_ = 0;
};

// Identity facts of an entity, as an entity citation repeats them (srs.fr.11.4).
struct EntityFacts {
    engine::EntityId id = engine::noEntity;
    std::string name;
    std::string sourceScene;
    bool createdAtRuntime = false;

    bool operator==(const EntityFacts&) const = default;
};

enum class GraphNodeKind { Entity, System, ComponentType, Event, Asset };

struct GraphNode {
    std::string id;
    GraphNodeKind kind = GraphNodeKind::Entity;
    std::string label;
    std::optional<EntityFacts> entity;
    // A source file named by the node; it makes the node count as source code (srs.fr.33.36).
    std::string sourceFile;

    bool operator==(const GraphNode&) const = default;
};

struct GraphEdge {
    std::string from;
    std::string to;
    // For example "requires", "reads", "writes", "collided with", "triggered".
    std::string relation;

    bool operator==(const GraphEdge&) const = default;
};

// A bounded part of the Semantic Graph (srs.fr.33.36, srs.fr.33.53).
struct GraphSlice {
    std::vector<GraphNode> nodes;
    std::vector<GraphEdge> edges;
    std::size_t omittedNodes = 0;

    bool operator==(const GraphSlice&) const = default;
};

// Component type context (srs.fr.33.52). The reader and writer lists are declarations only: they
// are never evidence that a system made a particular change (srs.fr.11.3).
struct ComponentTypeFacts {
    std::string name;
    std::string semanticDescription;
    std::vector<std::string> fields;
    std::vector<std::string> readingSystems;
    std::vector<std::string> writingSystems;

    bool operator==(const ComponentTypeFacts&) const = default;
};

// Asset context (srs.fr.33.50): metadata only, never the file's contents.
struct AssetFacts {
    std::string name;
    std::string path;
    std::string type;
    std::uint64_t sizeBytes = 0;
    std::vector<std::pair<std::string, std::string>> importSettings;
    bool isScript = false;

    bool operator==(const AssetFacts&) const = default;
};

// Log entry context (srs.fr.33.51, srs.fr.33.55).
struct LogEntryFacts {
    std::string level;
    std::string message;
    std::optional<engine::Tick> tick;
    std::string stackTrace;
    bool loggedDuringRun = false;

    bool operator==(const LogEntryFacts&) const = default;
};

// The four debug flags of srs.fr.12.9 to srs.fr.12.12.
enum class DebugFlag { NoCollider, UnlockWithoutItem, UnreachableObjective, OutOfBounds };

enum class ColliderKind { Box, Circle, Polygon };

// A collider as the debug-injection record stores it (srs.fr.12.16): kind, size or vertices and
// offset, in meters relative to the entity's position.
struct ColliderRecord {
    ColliderKind kind = ColliderKind::Box;
    engine::Vec2 offsetMeters;
    engine::Vec2 sizeMeters;
    float radiusMeters = 0.0f;
    std::vector<engine::Vec2> verticesMeters;

    bool operator==(const ColliderRecord&) const = default;
};

// A line of the run's test report about a debug flag (srs.fr.12.15, srs.fr.12.16, srs.fr.11.3).
struct DebugFlagReportLine {
    DebugFlag flag = DebugFlag::NoCollider;
    bool applied = false;
    // The report's reason when the flag was not applied, e.g. "no eligible obstacle".
    std::string reason;

    bool operator==(const DebugFlagReportLine&) const = default;
};

// The debug-injection record in the run's initial state (srs.fr.12.16). It exists only when
// telemetry recording was on.
struct DebugInjectionRecord {
    DebugFlag flag = DebugFlag::NoCollider;
    std::uint64_t sequence = 0;
    // The obstacle (srs.fr.12.9, 12.12) or Unlockable (srs.fr.12.10) entity; for srs.fr.12.11 the
    // objective whose cells are enclosed.
    engine::EntityId target = engine::noEntity;
    std::vector<ColliderRecord> removedColliders;
    std::vector<engine::EntityId> addedObstacles;

    bool operator==(const DebugInjectionRecord&) const = default;
};

// A lifecycle creation record (srs.fr.8.18) with the entity's identity facts.
struct CreationRecord {
    EntityFacts entity;
    StatePoint at = StatePoint::start();
    std::uint64_t sequence = 0;
    engine::Vec2 positionMeters;

    bool operator==(const CreationRecord&) const = default;
};

// An event published while a tick ran (srs.fr.8.26). Events never belong to Start.
struct EventRecord {
    engine::Tick tick = 0;
    std::uint64_t sequence = 0;
    std::string type;
    std::vector<engine::EntityId> entities;
    std::vector<std::pair<std::string, std::string>> payload;

    bool operator==(const EventRecord&) const = default;
};

// A recorded field write by a system or event handler (srs.fr.8.27, srs.fr.8.28).
struct FieldWriteRecord {
    StatePoint at = StatePoint::start();
    std::uint64_t sequence = 0;
    std::string writer;
    engine::EntityId entity = engine::noEntity;
    // "<Component>.<field>", for example "Unlockable.unlocked".
    std::string field;
    std::string value;

    bool operator==(const FieldWriteRecord&) const = default;
};

// A sampled field value at the end of a tick (srs.fr.8.26).
struct FieldSample {
    engine::Tick tick = 0;
    engine::EntityId entity = engine::noEntity;
    std::string field;
    std::string value;

    bool operator==(const FieldSample&) const = default;
};

// A sampled world position at the end of a tick, in meters.
struct PositionSample {
    engine::Tick tick = 0;
    engine::EntityId entity = engine::noEntity;
    engine::Vec2 positionMeters;

    bool operator==(const PositionSample&) const = default;
};

// Inclusive tick range.
struct TickRange {
    engine::Tick first = 0;
    engine::Tick last = 0;

    [[nodiscard]] constexpr bool contains(engine::Tick tick) const noexcept {
        return tick >= first && tick <= last;
    }

    bool operator==(const TickRange&) const = default;
};

// The Last run context of srs.fr.33.35 as the assistant sees it. `recorded` is false when
// telemetry recording was off, so only the test report exists.
struct RunEvidence {
    std::uint32_t runNumber = 0;
    bool recorded = false;
    std::optional<engine::Tick> lastCompletedTick;
    std::vector<DebugFlagReportLine> report;
    std::vector<DebugInjectionRecord> injections;
    std::vector<CreationRecord> creations;
    std::vector<EventRecord> events;
    std::vector<FieldWriteRecord> writes;
    std::vector<FieldSample> samples;
    std::vector<PositionSample> positions;
    // The tick range the events, ticked writes and samples were limited to; Start records are
    // always included (srs.fr.33.35).
    std::optional<TickRange> includedTicks;

    bool operator==(const RunEvidence&) const = default;
};

} // namespace aegis::assistant
