#include "engine/core/tick_phase.hpp"

namespace engine {

const char* tickPhaseName(TickPhase phase) {
    switch (phase) {
    case TickPhase::InputApplication:
        return "InputApplication";
    case TickPhase::GameplayInput:
        return "GameplayInput";
    case TickPhase::PhysicsSyncIn:
        return "PhysicsSyncIn";
    case TickPhase::PhysicsStep:
        return "PhysicsStep";
    case TickPhase::PhysicsSyncOut:
        return "PhysicsSyncOut";
    case TickPhase::GameplayRules:
        return "GameplayRules";
    case TickPhase::TelemetryRecording:
        return "TelemetryRecording";
    }
    return "Unknown";
}

} // namespace engine
