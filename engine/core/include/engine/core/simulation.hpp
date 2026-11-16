#pragma once

#include "engine/core/input_actions.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/tick_phase.hpp"

namespace engine {

// What one tick phase gets from the game loop.
struct TickContext {
    // The number of the running tick: 0 for the first tick of a run, then one more for each tick
    // (srs.fr.1.5). The ECS hands it to systems as ctx.tick(), and events and telemetry are
    // stamped with it.
    Tick tick = 0;
    // The tick's input action state, built in the input application phase before any phase runs
    // (srs.fr.1.14). It does not change during the tick. The InputApplication system copies it
    // into the World; other code reads the World, not this.
    const InputActionState& input;
};

// What the game loop advances: everything that runs per simulation tick (srs.fr.1.13): every ECS
// system, the Physics Adapter, event dispatch and telemetry recording. Keyboard input collection,
// camera follow, rendering, presentation and Editor panels run per render frame and are not part
// of it. An implementation must not depend on a window, on rendering or on wall-clock time, so
// that headless and normal mode give the same results (R-6).
//
// The game loop runs one tick as the seven tick phases of srs.fr.1.12: it calls runPhase() once
// for each phase of `tickPhases`, in that order, with the same context, in normal and headless
// mode alike. The ECS fills this seam: runPhase() becomes SystemScheduler::runPhase() (systems,
// then event dispatch, then command playback), and PhysicsStep steps the Physics Adapter by
// 1/60 s (DEC-041).
class ISimulation {
public:
    virtual ~ISimulation() = default;

    // Runs one tick phase of tick `context.tick`. A phase that throws engine::RunError ends the
    // run; the remaining phases of that tick do not run (srs.fr.1.11).
    virtual void runPhase(TickPhase phase, const TickContext& context) = 0;
};

} // namespace engine
