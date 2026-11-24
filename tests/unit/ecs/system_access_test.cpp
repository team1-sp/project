// Checking every system access against its declaration (US-2.3; srs.fr.2.16, 2.126, 7.5).
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/run_error.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/system_context.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::HealthComponent;
using ecs_test::runTick;
using ecs_test::testSystem;
using engine::Entity;
using engine::RunError;
using engine::SystemContext;
using engine::SystemDescription;
using engine::TickPhase;
using engine::World;

namespace {

// Adds a system with the given declaration and update to a world with one entity that has a
// Health and a Position, runs one tick and returns the RunError message, or "".
std::string violation(const std::function<void(SystemDescription&)>& declare,
                      const std::function<void(SystemContext&, Entity)>& act) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<HealthComponent>(e);
    world.add<engine::PositionComponent>(e);
    world.systems().add(testSystem(
        [declare](SystemDescription& d) {
            d.name("Probe").purpose("Touches the World.").phase(TickPhase::GameplayRules);
            declare(d);
        },
        [act, e](SystemContext& ctx) { act(ctx, e); }));
    try {
        runTick(world, 0);
    } catch (const RunError& error) {
        return error.what();
    }
    return "";
}

void none(SystemDescription&) {
}

} // namespace

// srs.fr.2.126: an undeclared read stops the run, naming the system, the type and the operation.
TEST(SystemAccessCheck, UndeclaredRead_IsARunErrorNamingSystemTypeAndOperation) {
    const std::string message = violation(none, [](SystemContext& ctx, Entity) {
        ctx.each<const HealthComponent>([](const HealthComponent&) {});
    });

    EXPECT_NE(message.find("Probe"), std::string::npos) << message;
    EXPECT_NE(message.find("read of Health"), std::string::npos) << message;
    EXPECT_NE(message.find("each"), std::string::npos) << message;
}

TEST(SystemAccessCheck, WriteNeedsAWriteDeclaration) {
    EXPECT_NE(
        violation([](SystemDescription& d) { d.reads<HealthComponent>(); },
                  [](SystemContext& ctx, Entity e) { ctx.get<HealthComponent>(e).points = 1; })
            .find("write of Health"),
        std::string::npos);
    EXPECT_EQ(violation([](SystemDescription& d) { d.reads<HealthComponent>(); },
                        [](SystemContext& ctx, Entity e) {
                            EXPECT_EQ(ctx.get<const HealthComponent>(e).points, 100);
                        }),
              "");
}

TEST(SystemAccessCheck, ADeclaredWrite_AllowsReadingToo) {
    EXPECT_EQ(violation([](SystemDescription& d) { d.writes<HealthComponent>(); },
                        [](SystemContext& ctx, Entity e) {
                            ctx.get<HealthComponent>(e).points -= 1;
                            EXPECT_TRUE(ctx.has<HealthComponent>(e));
                            ctx.each<const HealthComponent>([](const HealthComponent&) {});
                        }),
              "");
}

TEST(SystemAccessCheck, StructuralCommands_NeedTheirDeclarations) {
    EXPECT_NE(
        violation(none, [](SystemContext& ctx, Entity) { (void)ctx.commands().createEntity(); })
            .find("createsEntities()"),
        std::string::npos);
    EXPECT_NE(violation(none, [](SystemContext& ctx, Entity e) { ctx.commands().destroy(e); })
                  .find("destroysEntities()"),
              std::string::npos);
    EXPECT_NE(violation(none, [](SystemContext& ctx,
                                 Entity e) { ctx.commands().add<engine::VelocityComponent>(e); })
                  .find("write of Velocity"),
              std::string::npos);
    EXPECT_NE(violation(none, [](SystemContext& ctx,
                                 Entity e) { ctx.commands().setParent(e, engine::nullEntity); })
                  .find("write of Hierarchy"),
              std::string::npos);
    EXPECT_EQ(
        violation(
            [](SystemDescription& d) { d.createsEntities().writes<engine::VelocityComponent>(); },
            [](SystemContext& ctx, Entity) {
                const Entity made = ctx.commands().createEntity();
                ctx.commands().add<engine::VelocityComponent>(made);
            }),
        "");
}

TEST(SystemAccessCheck, EnabledFlagsAndRandom_NeedTheirDeclarations) {
    EXPECT_NE(violation(none, [](SystemContext& ctx, Entity e) { ctx.setEnabled(e, false); })
                  .find("togglesEnabled()"),
              std::string::npos);
    EXPECT_NE(violation(none, [](SystemContext& ctx,
                                 Entity e) { ctx.setComponentEnabled<HealthComponent>(e, false); })
                  .find("write of Health"),
              std::string::npos);
    EXPECT_NE(violation(none, [](SystemContext& ctx, Entity) { (void)ctx.random().next(); })
                  .find("usesRandom()"),
              std::string::npos);
}

// srs.fr.7.5: with the declaration, systems draw from the run's seeded generator.
TEST(SystemAccessCheck, Random_FollowsTheRunsSeed) {
    const auto draws = [](std::uint64_t seed) {
        World world{ecs_test::types()};
        world.beginRun(seed);
        std::vector<std::uint32_t> values;
        world.systems().add(testSystem(
            [](SystemDescription& d) {
                d.name("Dice").purpose("Rolls dice.").phase(TickPhase::GameplayRules).usesRandom();
            },
            [&values](SystemContext& ctx) { values.push_back(ctx.random().nextBelow(1000)); }));
        for (engine::Tick tick = 0; tick < 5; ++tick) {
            runTick(world, tick);
        }
        return values;
    };

    EXPECT_EQ(draws(42), draws(42));
    EXPECT_NE(draws(42), draws(43));
}
