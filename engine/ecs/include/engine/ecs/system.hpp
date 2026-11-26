#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "engine/core/tick_phase.hpp"
#include "engine/ecs/type_id.hpp"

namespace engine {

class SystemContext;

// A system's position in its World's scheduler, in registration order. Never saved.
using SystemId = std::uint32_t;

// A type a system names in its declaration: the registry key and the name the schema shows.
struct DeclaredType {
    TypeId id;
    std::string name;

    friend bool operator==(const DeclaredType&, const DeclaredType&) = default;
};

// Everything a system declares about itself (design/ecs-api-design.md §8, srs.fr.2.16): the
// access check enforces it (srs.fr.2.126) and the schema exports it (srs.fr.8.29).
struct SystemDeclaration {
    std::string name;                    // unique (srs.fr.2.39)
    std::string purpose;                 // required (srs.fr.2.38)
    std::string group;                   // empty: no group
    std::optional<TickPhase> phase;      // may be left out when the group gives it
    std::vector<std::string> runsBefore; // systems or groups (srs.fr.2.49)
    std::vector<std::string> runsAfter;
    std::vector<DeclaredType> reads;           // component types
    std::vector<DeclaredType> writes;          // a write allows reading too
    std::vector<DeclaredType> writesUntracked; // engine systems only: writes without a mark (D-27)
    std::vector<DeclaredType> publishes;       // event types (the event bus, US-2.8)
    std::vector<DeclaredType> subscribes;
    bool createsEntities = false;
    bool destroysEntities = false;
    bool togglesEnabled = false;
    bool usesRandom = false;
};

// The builder a system fills in describe() (D-10, D-21, D-28). One builder instead of many
// getters, so registration validates everything in one place and the same data is exported.
class SystemDescription {
public:
    SystemDescription& name(std::string_view text);
    SystemDescription& purpose(std::string_view text);
    SystemDescription& phase(TickPhase tickPhase);
    SystemDescription& group(std::string_view groupName);
    SystemDescription& runsBefore(std::string_view systemOrGroup);
    SystemDescription& runsAfter(std::string_view systemOrGroup);

    template <EcsType... Ts>
    SystemDescription& reads() {
        (add<Ts>(declaration_.reads), ...);
        return *this;
    }
    template <EcsType... Ts>
    SystemDescription& writes() {
        (add<Ts>(declaration_.writes), ...);
        return *this;
    }
    // Writes that set no pending-write mark: PhysicsSyncOut writing bodies back (D-27).
    // Allows reading too. Engine systems only.
    template <EcsType... Ts>
    SystemDescription& writesUntracked() {
        (add<Ts>(declaration_.writesUntracked), ...);
        return *this;
    }
    template <EcsType... Es>
    SystemDescription& publishes() {
        (add<Es>(declaration_.publishes), ...);
        return *this;
    }
    template <EcsType... Es>
    SystemDescription& subscribes() {
        (add<Es>(declaration_.subscribes), ...);
        return *this;
    }

    SystemDescription& createsEntities();
    SystemDescription& destroysEntities();
    SystemDescription& togglesEnabled();
    SystemDescription& usesRandom();

    [[nodiscard]] const SystemDeclaration& declaration() const { return declaration_; }

private:
    template <typename T>
    static void add(std::vector<DeclaredType>& list) {
        list.push_back(DeclaredType{typeIdOf<std::remove_const_t<T>>(), std::string(T::typeName)});
    }

    SystemDeclaration declaration_;
};

// Game logic that runs once per simulation tick in its tick phase (srs.fr.2.13, 2.14). A system
// never calls another system; it reads and writes components and records structural changes
// through its SystemContext (R-2). It holds no static mutable state: each World creates its own
// instance (D-23).
class ISystem {
public:
    virtual ~ISystem() = default;

    // Static facts: name, purpose, phase or group, ordering rules and access. Called once, when
    // the system is added; the result does not change afterwards.
    virtual void describe(SystemDescription& d) const = 0;
    // One-time setup when the system is added to a World (event subscriptions, US-2.8).
    virtual void setup(SystemContext& /*ctx*/) {}
    // Called exactly once per simulation tick in the system's phase, while it is enabled.
    virtual void update(SystemContext& ctx) = 0;
};

} // namespace engine
