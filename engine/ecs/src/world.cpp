#include "engine/ecs/world.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>

#include "engine/ecs/system_scheduler.hpp"

namespace engine {

namespace {

std::string text(EntityId id) {
    return "entity " + std::to_string(static_cast<std::uint64_t>(id));
}

} // namespace

World::World(const TypeRegistry& types)
    : types_(types) {
    if (!types.isFinalized()) {
        throw RunError("World: the type registry must be finalized first");
    }
    for (const ComponentTypeInfo& info : types.components()) {
        pools_.push_back(types.makePool(info));
    }
    hierarchies_ = &typedPool<HierarchyComponent>("World");
    systems_ = std::make_unique<SystemScheduler>(*this);
}

World::~World() = default;

Entity World::createEntity() {
    return createEntity(allocateRuntimeId());
}

Entity World::createEntity(EntityId id) {
    checkNotQuerying("World::createEntity");
    checkNewId(id, "World::createEntity");
    const Entity e = entities_.create(id);
    addBuiltIns(e, id);
    sortPools();
    return e;
}

void World::destroyEntity(Entity e) {
    checkNotQuerying("World::destroyEntity");
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
    pendingEnables_.push_back(e);
    for (IWorldObserver* observer : observers_) {
        observer->onEntityEnabledChanged(e, enabled);
    }
}

bool World::isEnabled(Entity e) const {
    checkAlive(e, "World::isEnabled");
    return entities_.isEnabled(e);
}

void World::setParent(Entity child, Entity parent) {
    checkNotQuerying("World::setParent");
    if (const std::optional<std::string> problem = parentProblem(child, parent)) {
        throw RunError("World::setParent: " + *problem);
    }
    link(child, parent);
}

const IComponentPool* World::pool(TypeId type) const {
    const ComponentTypeInfo* info = types_.find(type);
    return info == nullptr ? nullptr : pools_[info->index].get();
}

void World::forEachComponent(
    Entity e, const std::function<void(const ComponentTypeInfo&, const void*)>& fn) const {
    checkAlive(e, "World::forEachComponent");
    for (const ComponentTypeInfo& info : types_.components()) {
        if (const void* component = pools_[info.index]->rawOf(e)) {
            fn(info, component);
        }
    }
}

void World::beginRun(std::uint64_t seed) {
    if (entities_.hasAliveRuntimeEntity()) {
        throw RunError("World::beginRun: entities with runtime IDs from an earlier run are still "
                       "alive, and their IDs would be given out again");
    }
    entities_.resetRuntimeIds();
    random_.reseed(seed);
}

void World::checkNotQuerying(std::string_view operation) const {
    if (queryDepth_ > 0) {
        throw RunError(std::string(operation) +
                       ": structural changes are not allowed while a query runs; record them in a "
                       "CommandBuffer");
    }
}

void World::applyPendingEnables() {
    for (const Entity e : pendingEnables_) {
        if (entities_.isAlive(e)) {
            entities_.setQueryEnabled(e, entities_.isEnabled(e));
        }
    }
    pendingEnables_.clear();
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
            throw RunError(std::string(operation) + ": " + engine::text(id) +
                           " is in the runtime range but was not reserved with "
                           "allocateRuntimeId(), or an entity already used it");
        }
    } else if (entities_.find(id)) {
        throw RunError(std::string(operation) + ": " + engine::text(id) +
                       " is already used by a loaded entity");
    }
}

std::string World::text(Entity e) const {
    return engine::text(entities_.idOf(e));
}

void World::notifyAdded(Entity e, TypeId type) {
    for (IWorldObserver* observer : observers_) {
        observer->onComponentAdded(e, type);
    }
}

void World::notifyRemoved(Entity e, TypeId type) {
    for (IWorldObserver* observer : observers_) {
        observer->onComponentRemoved(e, type);
    }
}

Entity World::reserveEntity() {
    return entities_.create(allocateRuntimeId());
}

void World::addBuiltIns(Entity e, EntityId id) {
    typedPool<NameComponent>("World").add(e, id, NameComponent{});
    typedPool<EntityInfoComponent>("World").add(e, id, EntityInfoComponent{});
    hierarchies_->add(e, id, HierarchyComponent{});
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
        for (const std::unique_ptr<IComponentPool>& components : pools_) {
            components->removeIfPresent(e);
        }
        entities_.destroy(e);
    }
}

std::optional<std::string> World::parentProblem(Entity child, Entity parent) const {
    if (!entities_.isAlive(child)) {
        return "the child handle is not alive";
    }
    if (!hierarchies_->contains(child)) {
        return text(child) + " has no Hierarchy component yet";
    }
    if (parent == nullEntity) {
        return std::nullopt;
    }
    if (!entities_.isAlive(parent)) {
        return "the parent handle is not alive";
    }
    if (!hierarchies_->contains(parent)) {
        return text(parent) + " has no Hierarchy component yet";
    }
    // `parent` must not be `child` or one of its descendants: walk up from `parent`.
    const EntityId childId = entities_.idOf(child);
    for (EntityId ancestor = entities_.idOf(parent); ancestor != noEntity;
         ancestor = hierarchies_->tryGet(linked(ancestor))->parent) {
        if (ancestor == childId) {
            return text(parent) + " cannot become the parent of " + engine::text(childId) +
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
    HierarchyComponent* childLinks = hierarchies_->tryGet(child);
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
        throw RunError("World: a hierarchy link points at " + engine::text(id) +
                       ", which is not loaded");
    }
    return *e;
}

HierarchyComponent& World::hierarchyOf(Entity e) {
    HierarchyComponent* links = hierarchies_->tryGet(e);
    if (links == nullptr) {
        throw RunError("World: " + text(e) + " has no Hierarchy component");
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
        const HierarchyComponent* links = hierarchies_->tryGet(e);
        if (links == nullptr) {
            continue; // created by a CommandBuffer and not played back: no children
        }
        const std::size_t firstPushed = pending_.size();
        for (EntityId child = links->firstChild; child != noEntity;
             child = hierarchies_->tryGet(linked(child))->nextSibling) {
            pending_.push_back(linked(child));
        }
        std::reverse(pending_.begin() + static_cast<std::ptrdiff_t>(firstPushed), pending_.end());
    }
}

void World::sortPools() {
    for (const std::unique_ptr<IComponentPool>& components : pools_) {
        components->sortIfDirty();
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
