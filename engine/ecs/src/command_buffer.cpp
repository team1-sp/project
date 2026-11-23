#include "engine/ecs/command_buffer.hpp"

#include <algorithm>
#include <cstring>
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
    commands_.push_back({Kind::Create, e, world_.entities_.idOf(e), nullEntity, {}, 0});
    return e;
}

void CommandBuffer::destroy(Entity e) {
    world_.checkAlive(e, "CommandBuffer::destroy");
    commands_.push_back({Kind::Destroy, e, world_.entities_.idOf(e), nullEntity, {}, 0});
}

void CommandBuffer::setParent(Entity child, Entity parent) {
    world_.checkAlive(child, "CommandBuffer::setParent");
    if (parent != nullEntity) {
        world_.checkAlive(parent, "CommandBuffer::setParent");
    }
    commands_.push_back({Kind::SetParent, child, world_.entities_.idOf(child), parent, {}, 0});
}

void CommandBuffer::recordAdd(Entity e, TypeId type, std::string_view typeName, const void* value,
                              std::size_t size) {
    world_.checkAlive(e, "CommandBuffer::add");
    const ComponentTypeInfo* info = world_.types_.find(type);
    if (info == nullptr || info->name != typeName || info->size != size) {
        throw RunError("CommandBuffer::add: component type " + std::string(typeName) +
                       " is not registered");
    }
    if (info->builtIn) {
        throw RunError("CommandBuffer::add: " + info->name + " is a built-in component");
    }
    const auto at = static_cast<std::uint32_t>(bytes_.size());
    bytes_.resize(bytes_.size() + size);
    std::memcpy(bytes_.data() + at, value, size);
    commands_.push_back({Kind::Add, e, world_.entities_.idOf(e), nullEntity, type, at});
}

void CommandBuffer::recordRemove(Entity e, TypeId type, std::string_view typeName) {
    world_.checkAlive(e, "CommandBuffer::remove");
    const ComponentTypeInfo* info = world_.types_.find(type);
    if (info == nullptr || info->name != typeName) {
        throw RunError("CommandBuffer::remove: component type " + std::string(typeName) +
                       " is not registered");
    }
    if (info->builtIn) {
        throw RunError("CommandBuffer::remove: " + info->name + " is a built-in component");
    }
    commands_.push_back({Kind::Remove, e, world_.entities_.idOf(e), nullEntity, type, 0});
}

void CommandBuffer::playback() {
    findUnplayed();
    for (const Command& command : commands_) {
        if (isUnplayed(command.entity) && command.kind != Kind::Destroy) {
            continue; // the entity never appears
        }
        if (!world_.isAlive(command.entity)) {
            world_.warn("CommandBuffer: skipped a command for " + text(command.id) +
                            ": it was destroyed before this command",
                        command.id);
            continue;
        }
        switch (command.kind) {
        case Kind::Create:
            world_.addBuiltIns(command.entity, command.id);
            break;
        case Kind::Destroy:
            if (isUnplayed(command.entity)) {
                world_.releaseUnplayed(command.entity);
            } else {
                world_.destroyTree(command.entity);
            }
            break;
        case Kind::SetParent:
            if (const std::optional<std::string> problem =
                    world_.parentProblem(command.entity, command.parent)) {
                world_.warn("CommandBuffer: skipped setParent of " + text(command.id) + ": " +
                                *problem,
                            command.id);
            } else {
                world_.link(command.entity, command.parent);
            }
            break;
        case Kind::Add: {
            const ComponentTypeInfo& info = *world_.types_.find(command.type);
            IComponentPool& components = *world_.pools_[info.index];
            if (components.contains(command.entity)) {
                world_.warn("CommandBuffer: skipped adding " + info.name + " to " +
                                text(command.id) + ": it already has one",
                            command.id);
                break;
            }
            components.addRaw(command.entity, command.id, bytes_.data() + command.bytesAt);
            world_.notifyAdded(command.entity, command.type);
            break;
        }
        case Kind::Remove: {
            const ComponentTypeInfo& info = *world_.types_.find(command.type);
            if (!world_.pools_[info.index]->removeIfPresent(command.entity)) {
                world_.warn("CommandBuffer: skipped removing " + info.name + " from " +
                                text(command.id) + ": it has none",
                            command.id);
                break;
            }
            world_.notifyRemoved(command.entity, command.type);
            break;
        }
        }
    }
    commands_.clear();
    bytes_.clear();
    unplayed_.clear();
    world_.sortPools();
}

void CommandBuffer::findUnplayed() {
    // An entity is unplayed when this buffer creates and destroys it and parents nothing to it.
    // An entity that got a child here is created and destroyed for real, so the child is
    // destroyed with it (srs.fr.2.4, DEC-46.4).
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
