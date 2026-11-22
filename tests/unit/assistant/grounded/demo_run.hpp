#pragma once

// A small recorded demo run shared by the diagnosis and integration tests: a player (1), a locked
// door (2) that needs a key, an obstacle (3), an objective (4) and two ring obstacles created by
// debug injection (runtime IDs).

#include "aegis/assistant/grounded/evidence_records.hpp"

namespace grounded_test {

using namespace aegis::assistant;

inline constexpr std::uint32_t demoRunNumber = 7;

inline EntityFacts playerFacts() {
    return {engine::EntityId{1}, "Player", "Level1", false};
}
inline EntityFacts doorFacts() {
    return {engine::EntityId{2}, "Door", "Level1", false};
}
inline EntityFacts obstacleFacts() {
    return {engine::EntityId{3}, "Crate", "Level1", false};
}
inline EntityFacts objectiveFacts() {
    return {engine::EntityId{4}, "Goal", "Level1", false};
}
inline EntityFacts runtimeRingFacts(std::uint64_t offset) {
    return {engine::EntityId{(std::uint64_t{1} << 32) + offset}, "Ring " + std::to_string(offset),
            "Level1", true};
}

// A recorded run with creation records at Start and one sample at tick 0, no debug flags.
inline RunEvidence baseRun() {
    RunEvidence run;
    run.runNumber = demoRunNumber;
    run.recorded = true;
    run.lastCompletedTick = 120;
    run.creations = {
        {playerFacts(), StatePoint::start(), 1, {0.0f, 0.0f}},
        {doorFacts(), StatePoint::start(), 2, {10.0f, 0.0f}},
        {obstacleFacts(), StatePoint::start(), 3, {5.0f, 0.0f}},
        {objectiveFacts(), StatePoint::start(), 4, {20.0f, 0.0f}},
    };
    run.positions = {{0, engine::EntityId{1}, {0.0f, 0.0f}}};
    return run;
}

inline ColliderRecord crateCollider() {
    ColliderRecord collider;
    collider.kind = ColliderKind::Box;
    collider.sizeMeters = {2.0f, 2.0f};
    return collider;
}

} // namespace grounded_test
