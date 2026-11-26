#include "engine/ecs/schema.hpp"

#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/world.hpp"

namespace engine {

Schema buildSchema(const World& world) {
    Schema schema;
    for (const TickPhase phase : tickPhases) {
        schema.phases.emplace_back(tickPhaseName(phase));
    }
    const TypeRegistry& types = world.types();
    schema.components.assign(types.components().begin(), types.components().end());
    schema.enums.assign(types.enumerations().begin(), types.enumerations().end());
    for (const ComponentTypeInfo& component : schema.components) {
        schema.readBy[component.name];
        schema.writtenBy[component.name];
    }

    const SystemScheduler& systems = world.systems();
    schema.groups.assign(systems.groups().begin(), systems.groups().end());
    for (const TickPhase phase : tickPhases) {
        for (const SystemId system : systems.runOrder(phase)) {
            SystemSchema entry;
            entry.declaration = systems.declaration(system);
            entry.declaration.phase = systems.phaseOf(system);
            entry.orderIndex = systems.orderIndex(system);
            entry.enabled = systems.runs(system);
            for (const DeclaredType& type : entry.declaration.reads) {
                schema.readBy[type.name].push_back(entry.declaration.name);
            }
            for (const DeclaredType& type : entry.declaration.writes) {
                schema.writtenBy[type.name].push_back(entry.declaration.name);
            }
            for (const DeclaredType& type : entry.declaration.writesUntracked) {
                schema.writtenBy[type.name].push_back(entry.declaration.name);
            }
            schema.systems.push_back(std::move(entry));
        }
    }
    return schema;
}

} // namespace engine
