#include "engine/ecs/ecs_simulation.hpp"

#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/world.hpp"

namespace engine {

void EcsSimulation::runPhase(TickPhase phase, const TickContext& context) {
    if (phase == TickPhase::PhysicsStep && physicsStep_) {
        physicsStep_(context);
    }
    world_.systems().runPhase(phase, context);
}

} // namespace engine
