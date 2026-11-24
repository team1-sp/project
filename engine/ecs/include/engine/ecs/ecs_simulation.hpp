#pragma once

#include <functional>
#include <utility>

#include "engine/core/simulation.hpp"
#include "engine/core/tick_phase.hpp"

namespace engine {

class World;

// The ECS behind the game loop's ISimulation seam (DEC-059): every tick phase runs the World's
// systems of that phase and then its command playback (srs.fr.1.12, 2.14). The PhysicsStep phase
// holds no systems (srs.fr.2.48); the game loop steps the Physics Adapter there through
// setPhysicsStep() (DEC-041), which the physics stories connect. The World is not owned.
class EcsSimulation final : public ISimulation {
public:
    explicit EcsSimulation(World& world)
        : world_(world) {}

    void setPhysicsStep(std::function<void(const TickContext&)> step) {
        physicsStep_ = std::move(step);
    }

    void runPhase(TickPhase phase, const TickContext& context) override;

private:
    World& world_;
    std::function<void(const TickContext&)> physicsStep_;
};

} // namespace engine
