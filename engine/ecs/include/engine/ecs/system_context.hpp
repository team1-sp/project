#pragma once

#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "engine/core/entity_id.hpp"
#include "engine/core/tick.hpp"
#include "engine/ecs/command_buffer.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/gameplay_random.hpp"
#include "engine/ecs/system_access.hpp"
#include "engine/ecs/world.hpp"

namespace engine {

// A system's view of its World (design/ecs-api-design.md §10). Every call is checked against
// the system's declaration in every build (srs.fr.2.16, 2.126): a `const T` needs T in reads or
// writes, a plain `T` needs it in writes, and entity creation, destruction, enabled-flag changes
// and random draws need their flags. Systems cannot reach the World any other way, so the check
// cannot be bypassed. One context per system per World; the scheduler sets the tick.
class SystemContext {
public:
    SystemContext(World& world, const SystemAccess& access, CommandBuffer& commands)
        : world_(world)
        , access_(access)
        , commands_(commands) {}

    // Queries (srs.fr.2.18): as World::each(), after the access check of every type.
    // A non-const T of a type that tracks writes marks a pending write for each entity visited
    // (D-27), so a system declares `const T` unless it writes.
    template <typename... Ts, typename Fn>
    void each(Fn&& fn) {
        (check<Ts>("each"), ...);
        if constexpr ((... || !std::is_const_v<Ts>)) {
            world_.each<Ts...>([this, &fn](Entity e, Ts&... components) {
                (markIfWrite<Ts>(e), ...);
                if constexpr (std::is_invocable_v<Fn&, Entity, Ts&...>) {
                    fn(e, components...);
                } else {
                    fn(components...);
                }
            });
        } else {
            world_.each<Ts...>(std::forward<Fn>(fn));
        }
    }

    template <typename T>
    [[nodiscard]] T& single() {
        check<T>("single");
        if constexpr (std::is_const_v<T>) {
            return std::as_const(world_).single<std::remove_const_t<T>>();
        } else {
            return world_.single<T>();
        }
    }

    template <typename T>
    [[nodiscard]] T& get(Entity e) {
        check<T>("get");
        if constexpr (std::is_const_v<T>) {
            return std::as_const(world_).get<std::remove_const_t<T>>(e);
        } else {
            T& component = world_.get<T>(e);
            markIfWrite<T>(e);
            return component;
        }
    }

    template <typename T>
    [[nodiscard]] T* tryGet(Entity e) {
        check<T>("tryGet");
        if constexpr (std::is_const_v<T>) {
            return std::as_const(world_).tryGet<std::remove_const_t<T>>(e);
        } else {
            T* component = world_.tryGet<T>(e);
            if (component != nullptr) {
                markIfWrite<T>(e);
            }
            return component;
        }
    }

    // D-27, engine systems only: sets the component without a pending-write mark (PhysicsSyncOut
    // writing a body back, srs.fr.3.12, 3.13). Needs T in writesUntracked().
    template <typename T>
    void writeUntracked(Entity e, const T& value) {
        access_.checkWriteUntracked(world_.infoOf<T>("writeUntracked"), "writeUntracked");
        world_.get<T>(e) = value;
    }
    // Pending-write marks (D-27). Reading a mark needs T declared in any form; clearing it is
    // for PhysicsSyncIn, after it applied the write (srs.fr.3.11).
    template <typename T>
    [[nodiscard]] bool hasPendingWrite(Entity e) const {
        check<const T>("hasPendingWrite");
        return world_.hasPendingWrite<T>(e);
    }
    template <typename T>
    void clearPendingWrite(Entity e) {
        check<const T>("clearPendingWrite");
        world_.clearPendingWrite<T>(e);
    }

    template <typename T>
    [[nodiscard]] bool has(Entity e) const {
        check<const T>("has");
        return world_.has<std::remove_const_t<T>>(e);
    }

    template <typename T>
    void setComponentEnabled(Entity e, bool enabled) {
        check<T>("setComponentEnabled");
        world_.setComponentEnabled<T>(e, enabled);
    }

    template <typename T>
    [[nodiscard]] bool isComponentEnabled(Entity e) const {
        check<const T>("isComponentEnabled");
        return world_.isComponentEnabled<std::remove_const_t<T>>(e);
    }

    // Entity identity and state need no declaration.
    [[nodiscard]] bool isAlive(Entity e) const { return world_.isAlive(e); }
    [[nodiscard]] std::optional<Entity> find(EntityId id) const { return world_.find(id); }
    [[nodiscard]] EntityId idOf(Entity e) const { return world_.idOf(e); }
    [[nodiscard]] bool isEnabled(Entity e) const { return world_.isEnabled(e); }

    // Needs togglesEnabled(). Takes effect for queries from the next query (srs.fr.2.116).
    void setEnabled(Entity e, bool enabled) {
        access_.checkFlag(access_.togglesEnabled, "togglesEnabled()", "setEnabled");
        world_.setEnabled(e, enabled);
    }

    // Structural changes, applied at the end of the phase (§9); each command is checked.
    [[nodiscard]] CommandBuffer& commands() { return commands_; }

    // The running tick (srs.fr.1.5) and the fixed step (R-5). Systems never read real time.
    [[nodiscard]] Tick tick() const { return tick_; }
    [[nodiscard]] static double fixedDeltaSeconds() { return engine::fixedDeltaSeconds; }

    // The gameplay random number generator (srs.fr.7.5). Needs usesRandom().
    [[nodiscard]] GameplayRandom& random() {
        access_.checkFlag(access_.usesRandom, "usesRandom()", "random");
        return world_.random_;
    }

    [[nodiscard]] std::string_view systemName() const { return access_.name; }

private:
    friend class SystemScheduler;

    template <typename T>
    void markIfWrite(Entity e) {
        if constexpr (!std::is_const_v<T>) {
            world_.markWritten<T>(e);
        }
    }

    template <typename T>
    void check(std::string_view operation) const {
        const ComponentTypeInfo& info = world_.infoOf<std::remove_const_t<T>>(operation);
        if constexpr (std::is_const_v<T>) {
            access_.checkRead(info, operation);
        } else {
            access_.checkWrite(info, operation);
        }
    }

    World& world_;
    const SystemAccess& access_;
    CommandBuffer& commands_;
    Tick tick_ = 0;
};

} // namespace engine
