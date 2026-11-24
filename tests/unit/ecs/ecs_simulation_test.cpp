// The ECS behind the game loop (US-2.3; srs.fr.1.11, 1.12, 2.14; DEC-059).
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/application.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/ecs/ecs_simulation.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/system_context.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::named;
using ecs_test::testSystem;
using engine::SystemContext;
using engine::TickPhase;

namespace {

engine::ApplicationConfig headless(engine::Tick maxTicks) {
    engine::ApplicationConfig config;
    config.headless = true;
    config.maxTicks = maxTicks;
    return config;
}

} // namespace

// srs.fr.2.14: a headless run calls each system once per tick, with the tick numbers.
TEST(EcsSimulation, HeadlessRun_RunsTheSystemsEveryTick) {
    engine::World world{ecs_test::types()};
    std::vector<engine::Tick> ticks;
    world.systems().add(testSystem(named("Counter", TickPhase::GameplayRules),
                                   [&ticks](SystemContext& ctx) { ticks.push_back(ctx.tick()); }));
    engine::EcsSimulation simulation(world);
    int physicsSteps = 0;
    simulation.setPhysicsStep([&physicsSteps](const engine::TickContext&) { ++physicsSteps; });
    engine::Application app(simulation);
    ASSERT_TRUE(app.init(headless(5)));

    EXPECT_EQ(app.run(), 0);

    EXPECT_EQ(ticks, (std::vector<engine::Tick>{0, 1, 2, 3, 4}));
    EXPECT_EQ(physicsSteps, 5);
    EXPECT_EQ(app.outcome(), engine::RunOutcome::MaxTicks);
}

// srs.fr.1.11, 2.126: an undeclared access ends the run with the outcome Error.
TEST(EcsSimulation, UndeclaredAccess_EndsTheRunWithAnError) {
    engine::World world{ecs_test::types()};
    world.systems().add(
        testSystem(named("Sneaky", TickPhase::GameplayRules), [](SystemContext& ctx) {
            ctx.each<ecs_test::HealthComponent>([](ecs_test::HealthComponent&) {});
        }));
    engine::EcsSimulation simulation(world);
    engine::Application app(simulation);
    ASSERT_TRUE(app.init(headless(5)));

    EXPECT_NE(app.run(), 0);

    EXPECT_EQ(app.outcome(), engine::RunOutcome::Error);
}
