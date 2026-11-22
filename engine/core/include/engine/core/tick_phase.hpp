#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace engine {

// The tick phases of srs.fr.1.12, in the order every simulation tick runs them, in normal and
// in headless mode alike. The game loop owns this order: Application::runOneTick() calls
// ISimulation::runPhase() once for each phase, in this order, and nothing else runs a phase.
// The ECS places systems in these phases (design/ecs-api-design.md §8).
enum class TickPhase : std::uint8_t {
    // (1) The tick's input action state from its input source (srs.fr.1.14).
    InputApplication,
    // (2) Gameplay input processing, for example player movement.
    GameplayInput,
    // (3) The PhysicsSyncIn system pushes component changes to the physics bodies (srs.fr.3.63).
    PhysicsSyncIn,
    // (4) The physics world steps by 1/60 s; this phase holds no systems (srs.fr.2.48).
    PhysicsStep,
    // (5) The PhysicsSyncOut system writes bodies back and publishes collision events.
    PhysicsSyncOut,
    // (6) Gameplay rules.
    GameplayRules,
    // (7) Telemetry recording; the tick's sample is taken at its end (srs.fr.8.28).
    TelemetryRecording,
};

inline constexpr std::size_t tickPhaseCount = 7;

// Every tick phase in execution order.
inline constexpr std::array<TickPhase, tickPhaseCount> tickPhases = {
    TickPhase::InputApplication,   TickPhase::GameplayInput,  TickPhase::PhysicsSyncIn,
    TickPhase::PhysicsStep,        TickPhase::PhysicsSyncOut, TickPhase::GameplayRules,
    TickPhase::TelemetryRecording,
};

// The phase's name as the schema, telemetry and logs write it: "InputApplication",
// "GameplayInput", "PhysicsSyncIn", "PhysicsStep", "PhysicsSyncOut", "GameplayRules",
// "TelemetryRecording".
[[nodiscard]] const char* tickPhaseName(TickPhase phase);

} // namespace engine
