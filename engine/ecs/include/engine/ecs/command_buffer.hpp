#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/ecs/entity.hpp"

namespace engine {

class World;

// Structural changes recorded while systems and event handlers run, and applied together when
// the tick phase ends, after its event dispatch (design/ecs-api-design.md §9, D-4, srs.fr.2.116).
// Until then every query and handler of the phase still sees the World as it was. One buffer
// serves one World; the scheduler (US-2.3) plays it back at the end of every phase. This first
// version records entity creation, destruction and parenting; component adds and removes come
// with US-2.2.
class CommandBuffer {
public:
    explicit CommandBuffer(World& world);

    // Gives the new entity its handle and next runtime ID at once (D-5), so the ID can be stored
    // in other components straight away. Its components, the built-in ones included, appear at
    // playback, so until then it has none and no query can see it.
    Entity createEntity();
    // Requests that `e` and all its descendants be destroyed at playback (srs.fr.2.3, 2.4). Until
    // then it stays alive and can be found (srs.fr.2.115).
    void destroy(Entity e);
    // Requests World::setParent(child, parent) at playback.
    void setParent(Entity child, Entity parent);

    [[nodiscard]] std::size_t size() const { return commands_.size(); }

    // Applies the commands in the order they were recorded, sorts the pools they changed and
    // empties the buffer (§9). A command that no longer applies, such as destroying an entity a
    // command before it destroyed, is skipped with a warning naming the entity. An entity
    // created and destroyed in the same buffer, with no child parented to it there, never
    // appears: its slot is freed, its ID is never used again and no observer is told.
    void playback();

private:
    enum class Kind : std::uint8_t { Create, Destroy, SetParent };

    struct Command {
        Kind kind = Kind::Create;
        Entity entity;          // the created, destroyed or parented entity
        EntityId id = noEntity; // its ID when the command was recorded
        Entity parent;          // SetParent only; nullEntity makes `entity` a root
    };

    void findUnplayed();
    [[nodiscard]] bool isUnplayed(Entity e) const;

    World& world_;
    std::vector<Command> commands_;
    // Scratch of playback(), kept so that a playback allocates nothing once warm.
    std::vector<Entity> unplayed_; // created and destroyed in this buffer
    std::vector<Entity> created_;
    std::vector<Entity> parents_;
};

} // namespace engine
