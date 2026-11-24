#pragma once

#include "engine/core/entity_id.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/type_id.hpp"

namespace engine {

// Engine-internal code that must hear about entity changes it does not make itself: the owner of
// something outside the World, such as the Physics Adapter's bodies, or a recorder such as
// telemetry (design/ecs-api-design.md §9). Observers take the role of Unity's cleanup components:
// they are told before an entity disappears. They must not change the World from these calls.
class IWorldObserver {
public:
    virtual ~IWorldObserver() = default;

    // Called for every destroyed entity, descendants included, before any of their components
    // are removed, so an observer can still read them. The Physics Adapter disables the body now
    // and destroys it in the next synchronization-in (srs.fr.2.116, 3.58).
    virtual void onEntityDestroyed(Entity /*e*/, EntityId /*id*/) {}

    // Called when an entity's enabled flag changes (srs.fr.2.116: at once).
    virtual void onEntityEnabledChanged(Entity /*e*/, bool /*enabled*/) {}

    // Called after a component is added to or removed from a live entity, directly or at command
    // playback. Not called for the built-in components of a new entity, nor for the components a
    // destroyed entity loses (onEntityDestroyed covers those).
    virtual void onComponentAdded(Entity /*e*/, TypeId /*type*/) {}
    virtual void onComponentRemoved(Entity /*e*/, TypeId /*type*/) {}
};

} // namespace engine
