// Creating and destroying entities in a World (US-2.1; srs.fr.2.1, 2.2, 2.5, 2.63, 2.104).
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/entity_id.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::editorId;
using ecs_test::RecordingObserver;
using ecs_test::runtimeId;
using engine::Entity;
using engine::EntityId;
using engine::EntityInfoComponent;
using engine::HierarchyComponent;
using engine::NameComponent;
using engine::RunError;
using engine::World;

// srs.fr.2.1, 2.63: runtime entities get increasing IDs from the start of the runtime range.
TEST(World, CreateEntity_GivesIncreasingRuntimeIds) {
    World world{ecs_test::types()};

    const Entity first = world.createEntity();
    const Entity second = world.createEntity();

    EXPECT_EQ(world.idOf(first), engine::firstRuntimeEntityId);
    EXPECT_EQ(world.idOf(second), runtimeId(1));
    EXPECT_EQ(world.find(runtimeId(1)), second);
    EXPECT_EQ(world.entityCount(), 2u);
}

// srs.fr.2.5: name, enabled flag, tag, layer and an optional parent, from the start.
TEST(World, CreateEntity_HasTheBuiltInComponentsWithDefaults) {
    World world{ecs_test::types()};

    const Entity e = world.createEntity();

    EXPECT_TRUE(world.isEnabled(e));
    EXPECT_TRUE(world.has<NameComponent>(e));
    EXPECT_TRUE(world.get<NameComponent>(e).name.empty());
    EXPECT_EQ(world.get<EntityInfoComponent>(e).tag, engine::untaggedTag);
    EXPECT_EQ(world.get<EntityInfoComponent>(e).layer, 0);
    EXPECT_EQ(world.get<EntityInfoComponent>(e).sourceScene, engine::noAsset);
    EXPECT_EQ(world.get<HierarchyComponent>(e).parent, engine::noEntity);
    EXPECT_EQ(world.get<HierarchyComponent>(e).childCount, 0u);
}

TEST(World, BuiltInComponents_CanBeChanged) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();

    EXPECT_TRUE(world.get<NameComponent>(e).name.assign("Player"));
    world.get<EntityInfoComponent>(e).layer = 3;

    EXPECT_EQ(world.get<NameComponent>(e).name.view(), "Player");
    EXPECT_EQ(world.tryGet<EntityInfoComponent>(e)->layer, 3);
}

// srs.fr.2.2: a destroyed entity's ID never goes to another entity in the same run.
TEST(World, DestroyedId_IsNotGivenToANewEntity) {
    World world{ecs_test::types()};
    const Entity first = world.createEntity();
    const EntityId firstId = world.idOf(first);

    world.destroyEntity(first);
    const Entity second = world.createEntity();

    EXPECT_NE(world.idOf(second), firstId);
    EXPECT_FALSE(world.find(firstId).has_value());
    EXPECT_THROW(world.createEntity(firstId), RunError);
}

TEST(World, DestroyEntity_MakesItsHandleStale) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();

    world.destroyEntity(e);

    EXPECT_FALSE(world.isAlive(e));
    EXPECT_EQ(world.entityCount(), 0u);
    EXPECT_THROW((void)world.idOf(e), RunError);
    EXPECT_THROW((void)world.get<NameComponent>(e), RunError);
    EXPECT_THROW(world.destroyEntity(e), RunError);
    EXPECT_THROW(world.setEnabled(e, false), RunError);
}

TEST(World, CreateEntityWithEditorId_IsFoundByThatId) {
    World world{ecs_test::types()};

    const Entity e = world.createEntity(editorId(42));

    EXPECT_EQ(world.idOf(e), editorId(42));
    EXPECT_EQ(world.find(editorId(42)), e);
}

// srs.fr.2.1: no two loaded entities share an ID.
TEST(World, CreateEntityWithAnIdInUse_IsRefused) {
    World world{ecs_test::types()};
    world.createEntity(editorId(42));

    EXPECT_THROW(world.createEntity(editorId(42)), RunError);
    EXPECT_THROW(world.createEntity(engine::noEntity), RunError);
    EXPECT_EQ(world.entityCount(), 1u);
}

// srs.fr.2.63, 2.104: a runtime-range ID must come from allocateRuntimeId().
TEST(World, CreateEntityWithAnUnreservedRuntimeId_IsRefused) {
    World world{ecs_test::types()};

    EXPECT_THROW(world.createEntity(engine::firstRuntimeEntityId), RunError);

    const EntityId reserved = world.allocateRuntimeId();
    const Entity e = world.createEntity(reserved);
    EXPECT_EQ(world.idOf(e), reserved);
    EXPECT_THROW(world.createEntity(reserved), RunError);
}

// srs.fr.2.63: runs of the same content give the same IDs.
TEST(World, BeginRun_StartsRuntimeIdsAgain) {
    World world{ecs_test::types()};
    world.createEntity(editorId(1));
    const Entity runtime = world.createEntity();
    world.destroyEntity(runtime);

    world.beginRun();

    EXPECT_EQ(world.idOf(world.createEntity()), engine::firstRuntimeEntityId);
}

// srs.fr.2.2: restarting runtime IDs while a runtime entity lives would reuse its ID.
TEST(World, BeginRunWithALiveRuntimeEntity_IsRefused) {
    World world{ecs_test::types()};
    world.createEntity();

    EXPECT_THROW(world.beginRun(), RunError);
}

TEST(World, TwoWorlds_GiveTheSameIdsForTheSameCalls) {
    World a{ecs_test::types()};
    World b{ecs_test::types()};
    std::vector<EntityId> idsA;
    std::vector<EntityId> idsB;

    for (World* world : {&a, &b}) {
        std::vector<EntityId>& ids = world == &a ? idsA : idsB;
        world->createEntity(editorId(5));
        const Entity first = world->createEntity();
        world->destroyEntity(first);
        ids.push_back(world->idOf(world->createEntity()));
        ids.push_back(world->idOf(world->createEntity()));
    }

    EXPECT_EQ(idsA, idsB);
}

// srs.fr.2.5, 2.116: the enabled flag changes at once and observers hear about it.
TEST(World, SetEnabled_ChangesTheFlagAndTellsObserversOnce) {
    World world{ecs_test::types()};
    RecordingObserver observer;
    world.addObserver(observer);
    const Entity e = world.createEntity();

    world.setEnabled(e, false);
    world.setEnabled(e, false);

    EXPECT_FALSE(world.isEnabled(e));
    ASSERT_EQ(observer.enabledChanges.size(), 1u);
    EXPECT_EQ(observer.enabledChanges[0].first, e);
    EXPECT_FALSE(observer.enabledChanges[0].second);
}

// The pools stay in ascending EntityId, whatever order entities are created in (D-3).
TEST(World, EntitiesCreatedOutOfIdOrder_AreKeptInIdOrderInThePools) {
    World world{ecs_test::types()};
    world.createEntity(editorId(30));
    world.createEntity(editorId(10));
    world.createEntity(editorId(20));

    const engine::ComponentPool<NameComponent>& names = world.pool<NameComponent>();
    std::vector<EntityId> ids;
    for (std::size_t pos = 0; pos < names.size(); ++pos) {
        ids.push_back(names.idAt(pos));
    }

    EXPECT_EQ(ids, (std::vector<EntityId>{editorId(10), editorId(20), editorId(30)}));
}
