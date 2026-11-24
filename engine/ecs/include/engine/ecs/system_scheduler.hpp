#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "engine/core/simulation.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/ecs/command_buffer.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/system_access.hpp"
#include "engine/ecs/system_context.hpp"

namespace engine {

class World;

using SystemFactory = std::function<std::unique_ptr<ISystem>()>;

// A named set of systems that runs in one tick phase and can be turned off as a whole
// (srs.fr.2.48, 2.52).
struct SystemGroup {
    std::string name;
    std::string purpose;
    TickPhase phase = TickPhase::GameplayRules;
    bool enabled = true;
};

// How long a system's update took in the last tick that ran it (srs.fr.2.17), in milliseconds.
// Reported only (the Systems panel, telemetry); it never feeds back into the simulation (R-6).
struct SystemTiming {
    SystemId system = 0;
    double milliseconds = 0.0;
};

// The result of a change to the run order: accepted, or refused with the reason (srs.fr.2.51).
struct OrderResult {
    bool accepted = true;
    std::string problem;
};

// The systems of one World and the order they run in (design/ecs-api-design.md §8, D-10).
//
// Order (srs.fr.2.50, 2.51): each tick runs the seven tick phases in order (srs.fr.1.12); inside
// a phase the systems follow their before/after rules, and where the rules leave a choice, the
// manual order, which starts as the registration order. A rule that names a group applies to
// each system in it. A rule between two phases is ignored when the phase order already keeps it
// and refused when it contradicts it. Rules that form a cycle are refused. A refused change
// keeps the last valid order and reports the systems or the rule and phases at fault. A rule
// that names no known system or group is ignored. PhysicsStep holds no systems (srs.fr.2.48).
class SystemScheduler {
public:
    explicit SystemScheduler(World& world);
    SystemScheduler(const SystemScheduler&) = delete;
    SystemScheduler& operator=(const SystemScheduler&) = delete;
    ~SystemScheduler();

    // Defines a group before its systems are added. Refused: an empty purpose (srs.fr.2.38), a
    // name a system or group already has (srs.fr.2.39), or the PhysicsStep phase.
    void defineGroup(std::string_view name, TickPhase phase, std::string_view purpose);

    // Registers a system in the World (srs.fr.2.13): creates this World's instance, checks its
    // declaration and places it in the order, then calls its setup(). Refused with a RunError,
    // and nothing is added, when it has no name or purpose (srs.fr.2.38), its name is taken
    // (srs.fr.2.39), it declares an unregistered component type (srs.fr.2.126), its phase is
    // missing, PhysicsStep or different from its group's, its group is not defined, or its rules
    // break the order.
    SystemId add(const SystemFactory& factory);
    template <typename S>
    SystemId add() {
        return add([] { return std::make_unique<S>(); });
    }

    // srs.fr.2.15, 2.52: a disabled system, or every system of a disabled group, keeps its place
    // in the order and is skipped when its phase runs.
    void setEnabled(SystemId system, bool enabled);
    [[nodiscard]] bool isEnabled(SystemId system) const;
    void setGroupEnabled(std::string_view group, bool enabled);
    [[nodiscard]] bool isGroupEnabled(std::string_view group) const;
    // Whether the system runs when its phase comes: it and its group are enabled.
    [[nodiscard]] bool runs(SystemId system) const;

    // The user's manual order of all systems (the Systems panel). Accepted only when it holds
    // every system once and the rules allow an order from it; otherwise the last valid order
    // stays.
    OrderResult setManualOrder(std::span<const SystemId> order);

    // Runs one tick phase (srs.fr.2.14): every system of the phase that runs, in order, once;
    // then the phase's command playback (srs.fr.2.116). Event dispatch joins with US-2.8.
    void runPhase(TickPhase phase, const TickContext& context);

    // --- Introspection: the Systems panel, the schema (srs.fr.8.29) ---
    [[nodiscard]] std::size_t size() const { return entries_.size(); }
    [[nodiscard]] std::optional<SystemId> find(std::string_view name) const;
    [[nodiscard]] const SystemDeclaration& declaration(SystemId system) const;
    [[nodiscard]] TickPhase phaseOf(SystemId system) const;
    // The systems of a phase in run order.
    [[nodiscard]] std::span<const SystemId> runOrder(TickPhase phase) const;
    // The system's position in the run order of the whole tick, from 0.
    [[nodiscard]] std::uint32_t orderIndex(SystemId system) const;
    [[nodiscard]] std::span<const SystemGroup> groups() const { return groups_; }
    // The systems that ran in the last tick, with their times (srs.fr.2.17).
    [[nodiscard]] std::vector<SystemTiming> lastTickTimings() const;
    // The command buffer systems record into; played back at the end of every phase.
    [[nodiscard]] CommandBuffer& commands() { return commands_; }

private:
    struct Entry {
        std::unique_ptr<ISystem> system;
        SystemDeclaration declaration;
        TickPhase phase = TickPhase::GameplayRules;
        std::unique_ptr<SystemAccess> access;
        std::unique_ptr<SystemContext> context;
        bool enabled = true;
        bool ranLastTick = false;
        double lastMilliseconds = 0.0;
    };

    using PhaseOrder = std::array<std::vector<SystemId>, tickPhaseCount>;

    [[nodiscard]] const SystemGroup* findGroup(std::string_view name) const;
    [[nodiscard]] const Entry& entry(SystemId system) const;
    // The order the rules give from `manual`, or the reason there is none.
    [[nodiscard]] std::optional<std::string> computeOrder(std::span<const SystemId> manual,
                                                          PhaseOrder& order) const;
    // The systems a rule names: the system, or every system of the group.
    [[nodiscard]] std::vector<SystemId> resolve(std::string_view name) const;

    World& world_;
    CommandBuffer commands_;
    std::vector<Entry> entries_;
    std::vector<SystemGroup> groups_;
    std::vector<SystemId> manualOrder_;
    PhaseOrder runOrder_;
    Tick timingTick_ = 0;
    bool timingStarted_ = false;
};

} // namespace engine
