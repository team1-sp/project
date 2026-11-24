// Deferred creation and destruction (US-2.1; srs.fr.2.3, 2.4, 2.115, 2.116;
// design/ecs-api-design.md §9).
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/entity_id.hpp"
#include "engine/core/logging.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/command_buffer.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::editorId;
using ecs_test::RecordingLogSink;
using ecs_test::RecordingObserver;
using ecs_test::runtimeId;
using engine::CommandBuffer;
using engine::Entity;
using engine::EntityId;
using engine::HierarchyComponent;
using engine::NameComponent;
using engine::RunError;
using engine::World;

// D-5: the handle and the ID exist at once; the components appear at playback.
TEST(CommandBuffer, CreateEntity_GivesHandleAndIdNowAndComponentsAtPlayback) {
    World world{ecs_test::types()};
    CommandBuffer commands{world};

    const Entity e = commands.createEntity();

    EXPECT_TRUE(world.isAlive(e));
    EXPECT_EQ(world.idOf(e), engine::firstRuntimeEntityId);
    EXPECT_EQ(world.find(engine::firstRuntimeEntityId), e);
    EXPECT_FALSE(world.has<NameComponent>(e));
    EXPECT_THROW((void)world.get<NameComponent>(e), RunError);

    commands.playback();

    EXPECT_TRUE(world.has<NameComponent>(e));
    EXPECT_TRUE(world.has<HierarchyComponent>(e));
    EXPECT_EQ(commands.size(), 0u);
}

// srs.fr.2.3, 2.116: a destruction request takes effect at playback, not before; until then
// the entity is still found (srs.fr.2.115).
TEST(CommandBuffer, Destroy_TakesEffectAtPlayback) {
    World world{ecs_test::types()};
    RecordingObserver observer;
    world.addObserver(observer);
    const Entity e = world.createEntity(editorId(7));
    CommandBuffer commands{world};

    commands.destroy(e);

    EXPECT_TRUE(world.isAlive(e));
    EXPECT_EQ(world.find(editorId(7)), e);
    EXPECT_TRUE(observer.destroyed.empty());

    commands.playback();

    EXPECT_FALSE(world.isAlive(e));
    EXPECT_FALSE(world.find(editorId(7)).has_value());
    EXPECT_EQ(observer.destroyed, (std::vector<EntityId>{editorId(7)}));
}

// srs.fr.2.4 through a request.
TEST(CommandBuffer, Destroy_DestroysTheDescendantsToo) {
    World world{ecs_test::types()};
    const Entity parent = world.createEntity(editorId(1));
    const Entity child = world.createEntity(editorId(2));
    world.setParent(child, parent);
    CommandBuffer commands{world};

    commands.destroy(parent);
    commands.playback();

    EXPECT_FALSE(world.isAlive(parent));
    EXPECT_FALSE(world.isAlive(child));
}

// §9: create and destroy in one buffer cancel out; the ID is still never used again.
TEST(CommandBuffer, CreateAndDestroyInOneBuffer_NeverAppears) {
    World world{ecs_test::types()};
    RecordingObserver observer;
    world.addObserver(observer);
    CommandBuffer commands{world};

    const Entity e = commands.createEntity();
    const EntityId id = world.idOf(e);
    commands.destroy(e);
    commands.playback();

    EXPECT_FALSE(world.isAlive(e));
    EXPECT_EQ(world.entityCount(), 0u);
    EXPECT_TRUE(observer.destroyed.empty());
    EXPECT_NE(world.idOf(world.createEntity()), id);
}

// srs.fr.2.4: a created parent that gets a child in the same buffer takes the child with it.
TEST(CommandBuffer, CreatedParentWithAChildDestroyedInOneBuffer_TakesTheChild) {
    World world{ecs_test::types()};
    const Entity child = world.createEntity(editorId(1));
    CommandBuffer commands{world};

    const Entity parent = commands.createEntity();
    commands.setParent(child, parent);
    commands.destroy(parent);
    commands.playback();

    EXPECT_FALSE(world.isAlive(parent));
    EXPECT_FALSE(world.isAlive(child));
}

TEST(CommandBuffer, SetParent_LinksAtPlaybackInRecordingOrder) {
    World world{ecs_test::types()};
    const Entity parent = world.createEntity(editorId(1));
    CommandBuffer commands{world};

    const Entity first = commands.createEntity();
    const Entity second = commands.createEntity();
    commands.setParent(second, parent);
    commands.setParent(first, parent);
    commands.playback();

    const HierarchyComponent& links = world.get<HierarchyComponent>(parent);
    EXPECT_EQ(links.firstChild, world.idOf(second));
    EXPECT_EQ(world.get<HierarchyComponent>(second).nextSibling, world.idOf(first));
    EXPECT_EQ(links.childCount, 2u);
}

// §9 rule 2: a command on an entity an earlier command destroyed is skipped with a warning.
TEST(CommandBuffer, DestroyTwice_SkipsTheSecondWithAWarning) {
    World world{ecs_test::types()};
    RecordingLogSink log;
    world.setLogSink(&log);
    const Entity e = world.createEntity(editorId(9));
    CommandBuffer commands{world};

    commands.destroy(e);
    commands.destroy(e);
    commands.playback();

    EXPECT_FALSE(world.isAlive(e));
    ASSERT_EQ(log.entries.size(), 1u);
    EXPECT_EQ(log.entries[0].severity, engine::LogSeverity::Warning);
    EXPECT_EQ(log.entries[0].entity, editorId(9));
}

TEST(CommandBuffer, SetParentThatWouldMakeACycle_IsSkippedWithAWarning) {
    World world{ecs_test::types()};
    RecordingLogSink log;
    world.setLogSink(&log);
    const Entity parent = world.createEntity(editorId(1));
    const Entity child = world.createEntity(editorId(2));
    world.setParent(child, parent);
    CommandBuffer commands{world};

    commands.setParent(parent, child);
    commands.playback();

    EXPECT_EQ(world.get<HierarchyComponent>(parent).parent, engine::noEntity);
    EXPECT_EQ(log.entries.size(), 1u);
}

// A stale handle is a programming error at once, not at playback (coding standard rule 9).
TEST(CommandBuffer, DestroyWithAStaleHandle_IsRefused) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.destroyEntity(e);
    CommandBuffer commands{world};

    EXPECT_THROW(commands.destroy(e), RunError);
    EXPECT_EQ(commands.size(), 0u);
}

// Entities created through the buffer get increasing runtime IDs in recording order (D-5).
TEST(CommandBuffer, CreatedEntities_GetRuntimeIdsInRecordingOrder) {
    World world{ecs_test::types()};
    CommandBuffer commands{world};

    const Entity a = commands.createEntity();
    const Entity b = commands.createEntity();
    commands.playback();

    EXPECT_EQ(world.idOf(a), engine::firstRuntimeEntityId);
    EXPECT_EQ(world.idOf(b), runtimeId(1));
}
