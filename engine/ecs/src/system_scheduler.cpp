#include "engine/ecs/system_scheduler.hpp"

#include <algorithm>
#include <chrono>
#include <utility>

#include "engine/core/run_error.hpp"
#include "engine/ecs/world.hpp"

namespace engine {

namespace {

std::size_t indexOf(TickPhase phase) {
    return static_cast<std::size_t>(phase);
}

} // namespace

SystemScheduler::SystemScheduler(World& world)
    : world_(world)
    , commands_(world) {
}

SystemScheduler::~SystemScheduler() = default;

void SystemScheduler::defineGroup(std::string_view name, TickPhase phase,
                                  std::string_view purpose) {
    const std::string where = "SystemScheduler::defineGroup: group " + std::string(name);
    if (name.empty() || purpose.empty()) {
        throw RunError(where + " needs a name and a purpose");
    }
    if (findGroup(name) != nullptr || find(name).has_value()) {
        throw RunError(where + ": a system or group already has this name");
    }
    if (phase == TickPhase::PhysicsStep) {
        throw RunError(where + ": the PhysicsStep phase holds no systems");
    }
    groups_.push_back(SystemGroup{std::string(name), std::string(purpose), phase, true});
}

SystemId SystemScheduler::add(const SystemFactory& factory) {
    std::unique_ptr<ISystem> system = factory();
    if (system == nullptr) {
        throw RunError("SystemScheduler::add: the factory made no system");
    }
    SystemDescription description;
    system->describe(description);
    const SystemDeclaration& declared = description.declaration();
    const std::string where = "SystemScheduler::add: system " +
                              (declared.name.empty() ? std::string("(unnamed)") : declared.name);
    if (declared.name.empty() || declared.purpose.empty()) {
        throw RunError(where + " needs a name and a purpose");
    }
    if (find(declared.name).has_value() || findGroup(declared.name) != nullptr) {
        throw RunError(where + ": a system or group already has this name");
    }
    std::optional<TickPhase> phase = declared.phase;
    if (!declared.group.empty()) {
        const SystemGroup* group = findGroup(declared.group);
        if (group == nullptr) {
            throw RunError(where + ": group " + declared.group + " is not defined");
        }
        if (phase.has_value() && *phase != group->phase) {
            throw RunError(where + ": its phase " + tickPhaseName(*phase) + " is not the phase " +
                           tickPhaseName(group->phase) + " of its group " + group->name);
        }
        phase = group->phase;
    }
    if (!phase.has_value()) {
        throw RunError(where + " names no tick phase and no group");
    }
    if (*phase == TickPhase::PhysicsStep) {
        throw RunError(where + ": the PhysicsStep phase holds no systems");
    }

    const TypeRegistry& types = world_.types();
    auto access = std::make_unique<SystemAccess>();
    access->name = declared.name;
    access->reads.assign(types.components().size(), 0);
    access->writes.assign(types.components().size(), 0);
    const auto mark = [&](const std::vector<DeclaredType>& list, bool write) {
        for (const DeclaredType& type : list) {
            const ComponentTypeInfo* info = types.find(type.id);
            if (info == nullptr || info->name != type.name) {
                throw RunError(where + " declares the component type " + type.name +
                               ", which is not registered");
            }
            access->reads[info->index] = 1;
            if (write) {
                access->writes[info->index] = 1;
            }
        }
    };
    mark(declared.reads, false);
    mark(declared.writes, true);
    access->createsEntities = declared.createsEntities;
    access->destroysEntities = declared.destroysEntities;
    access->togglesEnabled = declared.togglesEnabled;
    access->usesRandom = declared.usesRandom;

    // Place it: the new system goes last in the manual order, then the rules decide.
    const auto id = static_cast<SystemId>(entries_.size());
    Entry added;
    added.system = std::move(system);
    added.declaration = declared;
    added.phase = *phase;
    added.access = std::move(access);
    added.context = std::make_unique<SystemContext>(world_, *added.access, commands_);
    entries_.push_back(std::move(added));
    std::vector<SystemId> manual = manualOrder_;
    manual.push_back(id);
    PhaseOrder order;
    if (const std::optional<std::string> problem = computeOrder(manual, order)) {
        entries_.pop_back();
        throw RunError(where + ": " + *problem);
    }
    manualOrder_ = std::move(manual);
    runOrder_ = std::move(order);
    Entry& placed = entries_.back();
    placed.system->setup(*placed.context);
    return id;
}

void SystemScheduler::setEnabled(SystemId system, bool enabled) {
    (void)entry(system);
    entries_[system].enabled = enabled;
}

bool SystemScheduler::isEnabled(SystemId system) const {
    return entry(system).enabled;
}

void SystemScheduler::setGroupEnabled(std::string_view group, bool enabled) {
    for (SystemGroup& candidate : groups_) {
        if (candidate.name == group) {
            candidate.enabled = enabled;
            return;
        }
    }
    throw RunError("SystemScheduler::setGroupEnabled: group " + std::string(group) +
                   " is not defined");
}

bool SystemScheduler::isGroupEnabled(std::string_view group) const {
    const SystemGroup* found = findGroup(group);
    if (found == nullptr) {
        throw RunError("SystemScheduler::isGroupEnabled: group " + std::string(group) +
                       " is not defined");
    }
    return found->enabled;
}

bool SystemScheduler::runs(SystemId system) const {
    const Entry& candidate = entry(system);
    if (!candidate.enabled) {
        return false;
    }
    const SystemGroup* group = findGroup(candidate.declaration.group);
    return group == nullptr || group->enabled;
}

OrderResult SystemScheduler::setManualOrder(std::span<const SystemId> order) {
    std::vector<std::uint8_t> seen(entries_.size(), 0);
    for (const SystemId system : order) {
        if (system >= entries_.size() || seen[system] != 0) {
            return {false, "the manual order must name every system exactly once"};
        }
        seen[system] = 1;
    }
    if (order.size() != entries_.size()) {
        return {false, "the manual order must name every system exactly once"};
    }
    PhaseOrder computed;
    if (const std::optional<std::string> problem = computeOrder(order, computed)) {
        return {false, *problem};
    }
    manualOrder_.assign(order.begin(), order.end());
    runOrder_ = std::move(computed);
    return {};
}

void SystemScheduler::runPhase(TickPhase phase, const TickContext& context) {
    if (!timingStarted_ || context.tick != timingTick_) {
        for (Entry& candidate : entries_) {
            candidate.ranLastTick = false;
            candidate.lastMilliseconds = 0.0;
        }
        timingTick_ = context.tick;
        timingStarted_ = true;
    }
    struct IssuerGuard {
        CommandBuffer& commands;
        ~IssuerGuard() { commands.setIssuer(nullptr); }
    } guard{commands_};
    for (const SystemId system : runOrder_[indexOf(phase)]) {
        if (!runs(system)) {
            continue;
        }
        Entry& running = entries_[system];
        running.context->tick_ = context.tick;
        commands_.setIssuer(running.access.get());
        const auto start = std::chrono::steady_clock::now();
        running.system->update(*running.context);
        const auto end = std::chrono::steady_clock::now();
        commands_.setIssuer(nullptr);
        running.lastMilliseconds = std::chrono::duration<double, std::milli>(end - start).count();
        running.ranLastTick = true;
    }
    commands_.playback();
}

std::optional<SystemId> SystemScheduler::find(std::string_view name) const {
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        if (entries_[i].declaration.name == name) {
            return static_cast<SystemId>(i);
        }
    }
    return std::nullopt;
}

const SystemDeclaration& SystemScheduler::declaration(SystemId system) const {
    return entry(system).declaration;
}

TickPhase SystemScheduler::phaseOf(SystemId system) const {
    return entry(system).phase;
}

std::span<const SystemId> SystemScheduler::runOrder(TickPhase phase) const {
    return runOrder_[indexOf(phase)];
}

std::uint32_t SystemScheduler::orderIndex(SystemId system) const {
    (void)entry(system);
    std::uint32_t index = 0;
    for (const TickPhase phase : tickPhases) {
        for (const SystemId candidate : runOrder_[indexOf(phase)]) {
            if (candidate == system) {
                return index;
            }
            ++index;
        }
    }
    return index;
}

std::vector<SystemTiming> SystemScheduler::lastTickTimings() const {
    std::vector<SystemTiming> timings;
    for (const TickPhase phase : tickPhases) {
        for (const SystemId system : runOrder_[indexOf(phase)]) {
            if (entries_[system].ranLastTick) {
                timings.push_back(SystemTiming{system, entries_[system].lastMilliseconds});
            }
        }
    }
    return timings;
}

const SystemGroup* SystemScheduler::findGroup(std::string_view name) const {
    if (name.empty()) {
        return nullptr;
    }
    for (const SystemGroup& group : groups_) {
        if (group.name == name) {
            return &group;
        }
    }
    return nullptr;
}

const SystemScheduler::Entry& SystemScheduler::entry(SystemId system) const {
    if (system >= entries_.size()) {
        throw RunError("SystemScheduler: no system has the id " + std::to_string(system));
    }
    return entries_[system];
}

std::vector<SystemId> SystemScheduler::resolve(std::string_view name) const {
    if (const std::optional<SystemId> system = find(name)) {
        return {*system};
    }
    std::vector<SystemId> members;
    if (findGroup(name) != nullptr) {
        for (std::size_t i = 0; i < entries_.size(); ++i) {
            if (entries_[i].declaration.group == name) {
                members.push_back(static_cast<SystemId>(i));
            }
        }
    }
    return members;
}

std::optional<std::string> SystemScheduler::computeOrder(std::span<const SystemId> manual,
                                                         PhaseOrder& order) const {
    const std::size_t count = entries_.size();
    std::vector<std::size_t> manualPosition(count, 0);
    for (std::size_t position = 0; position < manual.size(); ++position) {
        manualPosition[manual[position]] = position;
    }
    std::vector<std::vector<SystemId>> successors(count);
    std::vector<std::size_t> predecessors(count, 0);
    // `first` must run before `second`.
    const auto rule = [&](SystemId first, SystemId second) -> std::optional<std::string> {
        if (first == second) {
            return std::nullopt;
        }
        const TickPhase firstPhase = entries_[first].phase;
        const TickPhase secondPhase = entries_[second].phase;
        if (firstPhase == secondPhase) {
            successors[first].push_back(second);
            ++predecessors[second];
            return std::nullopt;
        }
        if (indexOf(firstPhase) < indexOf(secondPhase)) {
            return std::nullopt; // the phase order already keeps it
        }
        return "the rule that " + entries_[first].declaration.name + " runs before " +
               entries_[second].declaration.name + " contradicts the tick phases " +
               tickPhaseName(firstPhase) + " and " + tickPhaseName(secondPhase);
    };
    for (std::size_t i = 0; i < count; ++i) {
        const auto system = static_cast<SystemId>(i);
        for (const std::string& target : entries_[i].declaration.runsBefore) {
            for (const SystemId other : resolve(target)) {
                if (std::optional<std::string> problem = rule(system, other)) {
                    return problem;
                }
            }
        }
        for (const std::string& target : entries_[i].declaration.runsAfter) {
            for (const SystemId other : resolve(target)) {
                if (std::optional<std::string> problem = rule(other, system)) {
                    return problem;
                }
            }
        }
    }
    // Stable topological sort (Kahn): of the systems whose predecessors are all placed, take
    // the one that comes first in the manual order.
    std::vector<SystemId> ready;
    for (std::size_t i = 0; i < count; ++i) {
        if (predecessors[i] == 0) {
            ready.push_back(static_cast<SystemId>(i));
        }
    }
    const auto later = [&manualPosition](SystemId a, SystemId b) {
        return manualPosition[a] > manualPosition[b];
    };
    std::vector<SystemId> placed;
    while (!ready.empty()) {
        const auto next = std::min_element(ready.begin(), ready.end(),
                                           [&](SystemId a, SystemId b) { return later(b, a); });
        const SystemId system = *next;
        ready.erase(next);
        placed.push_back(system);
        for (const SystemId successor : successors[system]) {
            if (--predecessors[successor] == 0) {
                ready.push_back(successor);
            }
        }
    }
    if (placed.size() != count) {
        // Left over: the systems on a cycle and the ones after it. Peel off those with no
        // successor left over, until only the cycle remains (srs.fr.2.51 names its systems).
        std::vector<std::uint8_t> left(count, 0);
        for (std::size_t i = 0; i < count; ++i) {
            left[i] = predecessors[i] > 0 ? 1 : 0;
        }
        bool peeled = true;
        while (peeled) {
            peeled = false;
            for (std::size_t i = 0; i < count; ++i) {
                const bool feedsLeftOver =
                    std::any_of(successors[i].begin(), successors[i].end(),
                                [&left](SystemId successor) { return left[successor] != 0; });
                if (left[i] != 0 && !feedsLeftOver) {
                    left[i] = 0;
                    peeled = true;
                }
            }
        }
        std::string cycle;
        for (std::size_t i = 0; i < count; ++i) {
            if (left[i] != 0) {
                cycle += (cycle.empty() ? "" : ", ") + entries_[i].declaration.name;
            }
        }
        return "the before/after rules form a cycle between " + cycle;
    }
    for (std::vector<SystemId>& phaseOrder : order) {
        phaseOrder.clear();
    }
    for (const SystemId system : placed) {
        order[indexOf(entries_[system].phase)].push_back(system);
    }
    return std::nullopt;
}

} // namespace engine
