// Deferred component adds and removes (US-2.2; srs.fr.2.6, 2.7, 2.116;
// design/ecs-api-design.md §9).
#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/command_buffer.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::HealthComponent;
using ecs_test::RecordingLogSink;
using engine::CommandBuffer;
using engine::Entity;
using engine::RunError;
using engine::World;

TEST(CommandBufferComponents, Add_TakesEffectAtPlaybackWithTheRecordedValue) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    CommandBuffer commands{world};
    HealthComponent health{55};

    commands.add(e, health);
    health.points = 0; // the value was copied when recorded

    EXPECT_FALSE(world.has<HealthComponent>(e));
    commands.playback();
    EXPECT_EQ(world.get<HealthComponent>(e).points, 55);
    EXPECT_TRUE(world.isComponentEnabled<HealthComponent>(e));
}

TEST(CommandBufferComponents, Remove_TakesEffectAtPlayback) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<HealthComponent>(e);
    CommandBuffer commands{world};

    commands.remove<HealthComponent>(e);

    EXPECT_TRUE(world.has<HealthComponent>(e));
    commands.playback();
    EXPECT_FALSE(world.has<HealthComponent>(e));
}

// D-5: an entity created in the buffer can get components in the same buffer.
TEST(CommandBufferComponents, AddToACreatedEntity_AppearsWithItsBuiltIns) {
    World world{ecs_test::types()};
    CommandBuffer commands{world};

    const Entity e = commands.createEntity();
    commands.add(e, engine::PositionComponent{3.0f, 4.0f});
    commands.playback();

    EXPECT_TRUE(world.has<engine::NameComponent>(e));
    EXPECT_EQ(world.get<engine::PositionComponent>(e).yMeters, 4.0f);
}

// §9 rule 2: adding a type the entity already has, or removing one it lacks, is skipped.
TEST(CommandBufferComponents, DuplicateAddAndMissingRemove_AreSkippedWithWarnings) {
    World world{ecs_test::types()};
    RecordingLogSink log;
    world.setLogSink(&log);
    const Entity e = world.createEntity();
    CommandBuffer commands{world};

    commands.add(e, HealthComponent{1});
    commands.add(e, HealthComponent{2});
    commands.remove<engine::VelocityComponent>(e);
    commands.playback();

    EXPECT_EQ(world.get<HealthComponent>(e).points, 1);
    EXPECT_EQ(log.entries.size(), 2u);
}

TEST(CommandBufferComponents, AddToAnEntityDestroyedEarlierInThePlayback_IsSkipped) {
    World world{ecs_test::types()};
    RecordingLogSink log;
    world.setLogSink(&log);
    const Entity e = world.createEntity();
    CommandBuffer commands{world};

    commands.destroy(e);
    commands.add<HealthComponent>(e);
    commands.playback();

    EXPECT_FALSE(world.isAlive(e));
    EXPECT_EQ(log.entries.size(), 1u);
}

TEST(CommandBufferComponents, BuiltInComponents_CannotBeAddedOrRemovedThroughTheBuffer) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    CommandBuffer commands{world};

    EXPECT_THROW(commands.add<engine::NameComponent>(e), RunError);
    EXPECT_THROW(commands.remove<engine::HierarchyComponent>(e), RunError);
    EXPECT_EQ(commands.size(), 0u);
}

// A created-and-destroyed entity never appears, so its component commands do nothing either.
TEST(CommandBufferComponents, ComponentsOfAnUnplayedEntity_AreDroppedQuietly) {
    World world{ecs_test::types()};
    RecordingLogSink log;
    world.setLogSink(&log);
    CommandBuffer commands{world};

    const Entity e = commands.createEntity();
    commands.add<HealthComponent>(e);
    commands.destroy(e);
    commands.playback();

    EXPECT_FALSE(world.isAlive(e));
    EXPECT_EQ(world.pool<HealthComponent>().size(), 0u);
    EXPECT_TRUE(log.entries.empty());
}
