// Pending-write marks of Position and Velocity (D-27, srs.fr.3.21, 3.48).
#include <memory>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/system_context.hpp"
#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/world.hpp"

using engine::Entity;
using engine::PositionComponent;
using engine::SystemContext;
using engine::SystemDescription;
using engine::TickPhase;
using engine::VelocityComponent;
using engine::World;

namespace {

struct Fixture {
    Fixture() {
        e = world.createEntity();
        world.add(e, PositionComponent{});
        world.add(e, VelocityComponent{});
    }
    void run(void (*body)(SystemContext&, Entity), void (*declare)(SystemDescription&)) {
        world.systems().add(ecs_test::testSystem(
            [declare](SystemDescription& d) {
                d.name("Writer").purpose("Test.").phase(TickPhase::GameplayRules);
                declare(d);
            },
            [this, body](SystemContext& ctx) { body(ctx, e); }));
        ecs_test::runTick(world, 0);
    }
    World world{ecs_test::types()};
    Entity e{};
};

} // namespace

TEST(WriteTracking, ConstAccess_MarksNothing) {
    Fixture f;
    f.run([](SystemContext& ctx, Entity e) { (void)ctx.get<const PositionComponent>(e); },
          [](SystemDescription& d) { d.writes<PositionComponent>(); });

    EXPECT_FALSE(f.world.hasPendingWrite<PositionComponent>(f.e));
}

TEST(WriteTracking, NonConstGet_MarksAPendingWrite) {
    Fixture f;
    f.run([](SystemContext& ctx, Entity e) { ctx.get<PositionComponent>(e).xMeters = 1.0f; },
          [](SystemDescription& d) { d.writes<PositionComponent>(); });

    EXPECT_TRUE(f.world.hasPendingWrite<PositionComponent>(f.e));
    EXPECT_FALSE(f.world.hasPendingWrite<VelocityComponent>(f.e));
}

TEST(WriteTracking, NonConstEach_MarksEveryEntityVisited) {
    Fixture f;
    f.run(
        [](SystemContext& ctx, Entity) {
            ctx.each<VelocityComponent>([](VelocityComponent& v) { v.xMetersPerSecond = 2.0f; });
        },
        [](SystemDescription& d) { d.writes<VelocityComponent>(); });

    EXPECT_TRUE(f.world.hasPendingWrite<VelocityComponent>(f.e));
    EXPECT_EQ(f.world.get<VelocityComponent>(f.e).xMetersPerSecond, 2.0f);
}

TEST(WriteTracking, WriteUntracked_SetsTheValueWithoutAMark) {
    Fixture f;
    f.run([](SystemContext& ctx,
             Entity e) { ctx.writeUntracked(e, PositionComponent{4, 0, 0, 1, 1}); },
          [](SystemDescription& d) { d.writesUntracked<PositionComponent>(); });

    EXPECT_EQ(f.world.get<PositionComponent>(f.e).xMeters, 4.0f);
    EXPECT_FALSE(f.world.hasPendingWrite<PositionComponent>(f.e));
}

TEST(WriteTracking, WriteUntrackedWithoutDeclaringIt_IsRefused) {
    Fixture f;
    EXPECT_THROW(
        f.run([](SystemContext& ctx, Entity e) { ctx.writeUntracked(e, PositionComponent{}); },
              [](SystemDescription& d) { d.writes<PositionComponent>(); }),
        engine::RunError);
}

TEST(WriteTracking, ClearPendingWrite_RemovesTheMark) {
    Fixture f;
    f.world.markWritten<PositionComponent>(f.e);
    ASSERT_TRUE(f.world.hasPendingWrite<PositionComponent>(f.e));
    f.world.clearPendingWrite<PositionComponent>(f.e);

    EXPECT_FALSE(f.world.hasPendingWrite<PositionComponent>(f.e));
}
