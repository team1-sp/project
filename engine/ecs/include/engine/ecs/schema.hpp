#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "engine/core/tick_phase.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/type_registry.hpp"

namespace engine {

class World;

// One system as the schema exports it (srs.fr.8.29): purpose, phase, group, run order, enabled
// state and declarations (srs.fr.2.16).
struct SystemSchema {
    SystemDeclaration declaration; // as described; `phase` is filled from the group when needed
    std::uint32_t orderIndex = 0;  // position in the run order of the whole tick
    bool enabled = true;           // runs when its phase comes: it and its group are enabled
};

// The schema of a run (design/ecs-api-design.md §12, D-8, D-9; srs.fr.8.29): plain data the
// telemetry writer stores right after the run header, and the only description of the World the
// Semantic Architecture Layer reads (R-1). The ECS builds it in memory; it writes no file.
// Events and the phases their handlers run in join with the event bus (US-2.8).
struct Schema {
    std::uint32_t schemaVersion = 1;
    std::vector<std::string> phases;           // the tick phases in order
    std::vector<ComponentTypeInfo> components; // descriptions, fields, units, layout
    std::vector<EnumInfo> enums;
    std::vector<SystemGroup> groups;
    std::vector<SystemSchema> systems; // in run order
    // Derived from the declarations, never typed twice (D-8, srs.fr.2.12): component name ->
    // the systems that declare it in reads, and in writes.
    std::map<std::string, std::vector<std::string>> readBy;
    std::map<std::string, std::vector<std::string>> writtenBy;
};

[[nodiscard]] Schema buildSchema(const World& world);

} // namespace engine
