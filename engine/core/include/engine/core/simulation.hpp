#pragma once

namespace engine {

// What the game loop advances: everything that runs per simulation tick (srs.fr.1.13). One call
// is one tick of exactly 1/60 s of simulated time (R-5). An implementation must not depend on a
// window, on rendering or on wall-clock time, so that headless and normal mode give the same
// results (R-6).
//
// This is the seam the ECS fills once engine/ecs exists: runTick() becomes the seven tick
// phases of srs.fr.1.12.
class ISimulation {
public:
    virtual ~ISimulation() = default;

    virtual void runTick() = 0;
};

} // namespace engine
