// Parents, children and destroying a subtree (US-2.1; srs.fr.2.4, 2.5).
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/entity_id.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/world.hpp"

namespace {

using ecs_test::editorId;
using ecs_test::RecordingObserver;
using engine::Entity;
using engine::EntityId;
using engine::HierarchyComponent;
using engine::RunError;
using engine::World;

const HierarchyComponent& links(const World& world, Entity e) {
    return world.get<HierarchyComponent>(e);
}

// The parent's children, walked through the sibling links.
std::vector<EntityId> childrenOf(const World& world, Entity parent) {
    std::vector<EntityId> children;
    for (EntityId child = links(world, parent).firstChild; child != engine::noEntity;
         child = links(world, *world.find(child)).nextSibling) {
        children.push_back(child);
    }
    return children;
}

} // namespace

// srs.fr.2.5: an entity's optional parent; children keep the order they were added in.
TEST(WorldHierarchy, SetParent_AppendsTheChildAndLinksBothSides) {
    World world;
    const Entity parent = world.createEntity(editorId(1));
    const Entity a = world.createEntity(editorId(2));
    const Entity b = world.createEntity(editorId(3));

    world.setParent(b, parent);
    world.setParent(a, parent);

    EXPECT_EQ(childrenOf(world, parent), (std::vector<EntityId>{editorId(3), editorId(2)}));
    EXPECT_EQ(links(world, parent).childCount, 2u);
    EXPECT_EQ(links(world, a).parent, editorId(1));
    EXPECT_EQ(links(world, a).prevSibling, editorId(3));
    EXPECT_EQ(links(world, b).nextSibling, editorId(2));
}

TEST(WorldHierarchy, SetParentToNull_MakesTheEntityARootAndRelinksItsSiblings) {
    World world;
    const Entity parent = world.createEntity(editorId(1));
    const Entity a = world.createEntity(editorId(2));
    const Entity b = world.createEntity(editorId(3));
    const Entity c = world.createEntity(editorId(4));
    for (const Entity child : {a, b, c}) {
        world.setParent(child, parent);
    }

    world.setParent(b, engine::nullEntity);

    EXPECT_EQ(childrenOf(world, parent), (std::vector<EntityId>{editorId(2), editorId(4)}));
    EXPECT_EQ(links(world, parent).childCount, 2u);
    EXPECT_EQ(links(world, b).parent, engine::noEntity);
    EXPECT_EQ(links(world, b).prevSibling, engine::noEntity);
    EXPECT_EQ(links(world, b).nextSibling, engine::noEntity);
    EXPECT_EQ(links(world, c).prevSibling, editorId(2));
}

TEST(WorldHierarchy, SetParentToAnotherParent_MovesTheChild) {
    World world;
    const Entity first = world.createEntity(editorId(1));
    const Entity second = world.createEntity(editorId(2));
    const Entity child = world.createEntity(editorId(3));
    world.setParent(child, first);

    world.setParent(child, second);

    EXPECT_TRUE(childrenOf(world, first).empty());
    EXPECT_EQ(childrenOf(world, second), (std::vector<EntityId>{editorId(3)}));
}

TEST(WorldHierarchy, SetParentToItselfOrADescendant_IsRefused) {
    World world;
    const Entity root = world.createEntity(editorId(1));
    const Entity child = world.createEntity(editorId(2));
    const Entity grandchild = world.createEntity(editorId(3));
    world.setParent(child, root);
    world.setParent(grandchild, child);

    EXPECT_THROW(world.setParent(root, root), RunError);
    EXPECT_THROW(world.setParent(root, grandchild), RunError);
    EXPECT_EQ(links(world, root).parent, engine::noEntity);
}

// srs.fr.2.4: destroying an entity destroys all of its descendants, and nothing else.
TEST(WorldHierarchy, DestroyEntity_DestroysAllDescendants) {
    World world;
    const Entity root = world.createEntity(editorId(1));
    const Entity child = world.createEntity(editorId(2));
    const Entity grandchild = world.createEntity(editorId(3));
    const Entity other = world.createEntity(editorId(4));
    world.setParent(child, root);
    world.setParent(grandchild, child);

    world.destroyEntity(root);

    EXPECT_FALSE(world.isAlive(root));
    EXPECT_FALSE(world.isAlive(child));
    EXPECT_FALSE(world.isAlive(grandchild));
    EXPECT_TRUE(world.isAlive(other));
    EXPECT_EQ(world.entityCount(), 1u);
}

// Observers hear about every destroyed entity, parents before children, children in order.
TEST(WorldHierarchy, DestroyEntity_TellsObserversDepthFirst) {
    World world;
    RecordingObserver observer;
    world.addObserver(observer);
    const Entity root = world.createEntity(editorId(1));
    const Entity a = world.createEntity(editorId(2));
    const Entity a1 = world.createEntity(editorId(3));
    const Entity b = world.createEntity(editorId(4));
    world.setParent(a, root);
    world.setParent(b, root);
    world.setParent(a1, a);

    world.destroyEntity(root);

    EXPECT_EQ(observer.destroyed,
              (std::vector<EntityId>{editorId(1), editorId(2), editorId(3), editorId(4)}));
}

TEST(WorldHierarchy, DestroyAChild_KeepsTheParentAndTheOtherChildrenLinked) {
    World world;
    const Entity parent = world.createEntity(editorId(1));
    const Entity a = world.createEntity(editorId(2));
    const Entity b = world.createEntity(editorId(3));
    const Entity c = world.createEntity(editorId(4));
    for (const Entity child : {a, b, c}) {
        world.setParent(child, parent);
    }

    world.destroyEntity(b);

    EXPECT_EQ(childrenOf(world, parent), (std::vector<EntityId>{editorId(2), editorId(4)}));
    EXPECT_EQ(links(world, parent).childCount, 2u);
    EXPECT_EQ(links(world, c).prevSibling, editorId(2));
}
