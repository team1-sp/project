#include "engine/ecs/world.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>

namespace engine {

namespace {

std::string text(EntityId id) {
    return "entity " + std::to_string(static_cast<std::uint64_t>(id));
}

} // namespace

Entity World::createEntity() {
    return createEntity(allocateRuntimeId());
}

Entity World::createEntity(EntityId id) {
    checkNewId(id, "World::createEntity");
    const Entity e = entities_.create(id);
    addBuiltIns(e, id);
    sortPools();
    return e;
}

void World::destroyEntity(Entity e) {
    checkAlive(e, "World::destroyEntity");
    destroyTree(e);
    sortPools();
}

EntityId World::idOf(Entity e) const {
    checkAlive(e, "World::idOf");
    return entities_.idOf(e);
}

void World::setEnabled(Entity e, bool enabled) {
    checkAlive(e, "World::setEnabled");
    if (entities_.isEnabled(e) == enabled) {
        return;
    }
    entities_.setEnabled(e, enabled);
    for (IWorldObserver* observer : observers_) {
        observer->onEntityEnabledChanged(e, enabled);
    }
}

bool World::isEnabled(Entity e) const {
    checkAlive(e, "World::isEnabled");
    return entities_.isEnabled(e);
}

void World::setParent(Entity child, Entity parent) {
    if (const std::optional<std::string> problem = parentProblem(child, parent)) {
        throw RunError("World::setParent: " + *problem);
    }
    link(child, parent);
}

void World::beginRun() {
    if (entities_.hasAliveRuntimeEntity()) {
        throw RunError("World::beginRun: entities with runtime IDs from an earlier run are still "
                       "alive, and their IDs would be given out again");
    }
    entities_.resetRuntimeIds();
}

void World::checkAlive(Entity e, std::string_view operation) const {
    if (!entities_.isAlive(e)) {
        throw RunError(std::string(operation) + ": the entity handle (index " +
                       std::to_string(e.index) + ", generation " + std::to_string(e.generation) +
                       ") is not alive");
    }
}

void World::checkNewId(EntityId id, std::string_view operation) const {
    if (id == noEntity) {
        throw RunError(std::string(operation) + ": 0 is not an entity ID");
    }
    if (id >= firstRuntimeEntityId) {
        if (!entities_.isReservedRuntimeId(id)) {
            throw RunError(std::string(operation) + ": " + text(id) +
                           " is in the runtime range but was not reserved with "
                           "allocateRuntimeId(), or an entity already used it");
        }
    } else if (entities_.find(id)) {
        throw RunError(std::string(operation) + ": " + text(id) +
                       " is already used by a loaded entity");
    }
}

std::string World::missingComponentMessage(Entity e, std::string_view type) const {
    return "World::get: " + text(entities_.idOf(e)) + " has no " + std::string(type) + " component";
}

Entity World::reserveEntity() {
    return entities_.create(allocateRuntimeId());
}

void World::addBuiltIns(Entity e, EntityId id) {
    names_.add(e, id, NameComponent{});
    infos_.add(e, id, EntityInfoComponent{});
    hierarchies_.add(e, id, HierarchyComponent{});
}

void World::destroyTree(Entity root) {
    collectSubtree(root);
    unlink(root);
    for (const Entity e : subtree_) {
        const EntityId id = entities_.idOf(e);
        for (IWorldObserver* observer : observers_) {
            observer->onEntityDestroyed(e, id);
        }
    }
    for (const Entity e : subtree_) {
        for (IComponentPool* pool : pools_) {
            pool->removeIfPresent(e);
        }
        entities_.destroy(e);
    }
}

std::optional<std::string> World::parentProblem(Entity child, Entity parent) const {
    if (!entities_.isAlive(child)) {
        return "the child handle is not alive";
    }
    const HierarchyComponent* childLinks = hierarchies_.tryGet(child);
    if (childLinks == nullptr) {
        return text(entities_.idOf(child)) + " has no Hierarchy component yet";
    }
    if (parent == nullEntity) {
        return std::nullopt;
    }
    if (!entities_.isAlive(parent)) {
        return "the parent handle is not alive";
    }
    if (!hierarchies_.contains(parent)) {
        return text(entities_.idOf(parent)) + " has no Hierarchy component yet";
    }
    // `parent` must not be `child` or one of its descendants: walk up from `parent`.
    const EntityId childId = entities_.idOf(child);
    for (EntityId ancestor = entities_.idOf(parent); ancestor != noEntity;
         ancestor = hierarchies_.tryGet(linked(ancestor))->parent) {
        if (ancestor == childId) {
            return text(entities_.idOf(parent)) + " cannot become the parent of " + text(childId) +
                   ", because it is that entity or one of its descendants";
        }
    }
    return std::nullopt;
}

void World::link(Entity child, Entity parent) {
    const EntityId parentId = parent == nullEntity ? noEntity : entities_.idOf(parent);
    if (hierarchyOf(child).parent == parentId) {
        return; // already there; its place among the siblings stays
    }
    unlink(child);
    if (parent == nullEntity) {
        return;
    }
    const EntityId childId = entities_.idOf(child);
    HierarchyComponent& parentLinks = hierarchyOf(parent);
    EntityId last = noEntity;
    for (EntityId sibling = parentLinks.firstChild; sibling != noEntity;
         sibling = hierarchyOf(linked(sibling)).nextSibling) {
        last = sibling;
    }
    if (last == noEntity) {
        parentLinks.firstChild = childId;
    } else {
        hierarchyOf(linked(last)).nextSibling = childId;
    }
    ++parentLinks.childCount;
    HierarchyComponent& childLinks = hierarchyOf(child);
    childLinks.parent = parentId;
    childLinks.prevSibling = last;
    childLinks.nextSibling = noEntity;
}

void World::unlink(Entity child) {
    HierarchyComponent* childLinks = hierarchies_.tryGet(child);
    if (childLinks == nullptr || childLinks->parent == noEntity) {
        return;
    }
    HierarchyComponent& parentLinks = hierarchyOf(linked(childLinks->parent));
    if (childLinks->prevSibling == noEntity) {
        parentLinks.firstChild = childLinks->nextSibling;
    } else {
        hierarchyOf(linked(childLinks->prevSibling)).nextSibling = childLinks->nextSibling;
    }
    if (childLinks->nextSibling != noEntity) {
        hierarchyOf(linked(childLinks->nextSibling)).prevSibling = childLinks->prevSibling;
    }
    --parentLinks.childCount;
    childLinks->parent = noEntity;
    childLinks->prevSibling = noEntity;
    childLinks->nextSibling = noEntity;
}

Entity World::linked(EntityId id) const {
    const std::optional<Entity> e = entities_.find(id);
    if (!e) {
        // setParent() and destroyTree() keep every link pointing at a live entity.
        throw RunError("World: a hierarchy link points at " + text(id) + ", which is not loaded");
    }
    return *e;
}

HierarchyComponent& World::hierarchyOf(Entity e) {
    HierarchyComponent* links = hierarchies_.tryGet(e);
    if (links == nullptr) {
        throw RunError("World: " + text(entities_.idOf(e)) + " has no Hierarchy component");
    }
    return *links;
}

void World::collectSubtree(Entity root) {
    // Depth first, each parent before its children and children in sibling order.
    subtree_.clear();
    pending_.clear();
    pending_.push_back(root);
    while (!pending_.empty()) {
        const Entity e = pending_.back();
        pending_.pop_back();
        subtree_.push_back(e);
        const HierarchyComponent* links = hierarchies_.tryGet(e);
        if (links == nullptr) {
            continue; // created by a CommandBuffer and not played back: no children
        }
        const std::size_t firstPushed = pending_.size();
        for (EntityId child = links->firstChild; child != noEntity;
             child = hierarchies_.tryGet(linked(child))->nextSibling) {
            pending_.push_back(linked(child));
        }
        std::reverse(pending_.begin() + static_cast<std::ptrdiff_t>(firstPushed), pending_.end());
    }
}

void World::sortPools() {
    for (IComponentPool* pool : pools_) {
        pool->sortIfDirty();
    }
}

void World::warn(std::string message, EntityId id) {
    if (logSink_ == nullptr) {
        std::cerr << "warning: " << message << '\n';
        return;
    }
    LogEntry entry;
    entry.severity = LogSeverity::Warning;
    entry.message = std::move(message);
    entry.entity = id;
    logSink_->write(std::move(entry));
}

} // namespace engine
