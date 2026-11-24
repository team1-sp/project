#include "engine/ecs/command_buffer.hpp"

#include <algorithm>
#include <optional>
#include <string>

#include "engine/ecs/world.hpp"

namespace engine {

namespace {

std::string text(EntityId id) {
    return "entity " + std::to_string(static_cast<std::uint64_t>(id));
}

bool byHandle(Entity a, Entity b) {
    return a.index != b.index ? a.index < b.index : a.generation < b.generation;
}

} // namespace

CommandBuffer::CommandBuffer(World& world)
    : world_(world) {
}

Entity CommandBuffer::createEntity() {
    const Entity e = world_.reserveEntity();
    commands_.push_back({Kind::Create, e, world_.entities_.idOf(e), nullEntity});
    return e;
}

void CommandBuffer::destroy(Entity e) {
    world_.checkAlive(e, "CommandBuffer::destroy");
    commands_.push_back({Kind::Destroy, e, world_.entities_.idOf(e), nullEntity});
}

void CommandBuffer::setParent(Entity child, Entity parent) {
    world_.checkAlive(child, "CommandBuffer::setParent");
    if (parent != nullEntity) {
        world_.checkAlive(parent, "CommandBuffer::setParent");
    }
    commands_.push_back({Kind::SetParent, child, world_.entities_.idOf(child), parent});
}

void CommandBuffer::playback() {
    findUnplayed();
    for (const Command& command : commands_) {
        switch (command.kind) {
        case Kind::Create:
            if (isUnplayed(command.entity)) {
                break;
            }
            if (!world_.isAlive(command.entity)) {
                world_.warn("CommandBuffer: skipped creating " + text(command.id) +
                                ": it was destroyed before playback",
                            command.id);
                break;
            }
            world_.addBuiltIns(command.entity, command.id);
            break;
        case Kind::Destroy:
            if (!world_.isAlive(command.entity)) {
                world_.warn("CommandBuffer: skipped destroying " + text(command.id) +
                                ": it was already destroyed",
                            command.id);
                break;
            }
            if (isUnplayed(command.entity)) {
                world_.releaseUnplayed(command.entity);
                break;
            }
            world_.destroyTree(command.entity);
            break;
        case Kind::SetParent:
            if (isUnplayed(command.entity)) {
                break; // the child never appears
            }
            if (const std::optional<std::string> problem =
                    world_.parentProblem(command.entity, command.parent)) {
                world_.warn("CommandBuffer: skipped setParent of " + text(command.id) + ": " +
                                *problem,
                            command.id);
                break;
            }
            world_.link(command.entity, command.parent);
            break;
        }
    }
    commands_.clear();
    unplayed_.clear();
    world_.sortPools();
}

void CommandBuffer::findUnplayed() {
    // An entity is unplayed when this buffer creates and destroys it and parents nothing to it.
    // An entity that got a child here is created and destroyed for real, so the child is
    // destroyed with it (srs.fr.2.4).
    created_.clear();
    parents_.clear();
    for (const Command& command : commands_) {
        if (command.kind == Kind::Create) {
            created_.push_back(command.entity);
        } else if (command.kind == Kind::SetParent && command.parent != nullEntity) {
            parents_.push_back(command.parent);
        }
    }
    std::sort(created_.begin(), created_.end(), byHandle);
    std::sort(parents_.begin(), parents_.end(), byHandle);
    unplayed_.clear();
    for (const Command& command : commands_) {
        if (command.kind == Kind::Destroy &&
            std::binary_search(created_.begin(), created_.end(), command.entity, byHandle) &&
            !std::binary_search(parents_.begin(), parents_.end(), command.entity, byHandle)) {
            unplayed_.push_back(command.entity);
        }
    }
    std::sort(unplayed_.begin(), unplayed_.end(), byHandle);
}

bool CommandBuffer::isUnplayed(Entity e) const {
    return std::binary_search(unplayed_.begin(), unplayed_.end(), e, byHandle);
}

} // namespace engine
