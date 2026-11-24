// Running systems in the tick phases (US-2.3; srs.fr.2.14, 2.15, 2.17, 2.52, 2.116).
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/input_actions.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/system_context.hpp"
#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::named;
using ecs_test::runTick;
using ecs_test::testSystem;
using engine::Entity;
using engine::SystemContext;
using engine::SystemDescription;
using engine::TickPhase;
using engine::World;

namespace {

// A system that writes its name into `log` each time it runs.
engine::SystemFactory logging(std::string name, TickPhase phase, std::vector<std::string>& log) {
    return testSystem(named(name, phase), [name, &log](SystemContext&) { log.push_back(name); });
}

} // namespace

// srs.fr.2.14: every enabled system runs once per tick, in its phase, phases in order.
TEST(SystemRun, EverySystemRunsOncePerTickInPhaseOrder) {
    World world{ecs_test::types()};
    std::vector<std::string> log;
    world.systems().add(logging("Rules", TickPhase::GameplayRules, log));
    world.systems().add(logging("Telemetry", TickPhase::TelemetryRecording, log));
    world.systems().add(logging("Input", TickPhase::InputApplication, log));

    runTick(world, 0);
    runTick(world, 1);

    EXPECT_EQ(log, (std::vector<std::string>{"Input", "Rules", "Telemetry", "Input", "Rules",
                                             "Telemetry"}));
}

TEST(SystemRun, RunPhase_RunsOnlyThatPhase) {
    World world{ecs_test::types()};
    std::vector<std::string> log;
    world.systems().add(logging("Rules", TickPhase::GameplayRules, log));
    world.systems().add(logging("Input", TickPhase::GameplayInput, log));
    const engine::InputActionState input;

    world.systems().runPhase(TickPhase::GameplayInput, engine::TickContext{3, input});

    EXPECT_EQ(log, (std::vector<std::string>{"Input"}));
}

// srs.fr.2.15, 2.52: a disabled system and the systems of a disabled group are skipped.
TEST(SystemRun, DisabledSystemsAndGroups_AreSkipped) {
    World world{ecs_test::types()};
    std::vector<std::string> log;
    world.systems().defineGroup("Doors", TickPhase::GameplayRules, "Door rules.");
    const engine::SystemId alone =
        world.systems().add(logging("Alone", TickPhase::GameplayRules, log));
    world.systems().add(testSystem(
        [](SystemDescription& d) { d.name("InGroup").purpose("A grouped system.").group("Doors"); },
        [&log](SystemContext&) { log.push_back("InGroup"); }));

    world.systems().setEnabled(alone, false);
    world.systems().setGroupEnabled("Doors", false);
    runTick(world, 0);
    EXPECT_TRUE(log.empty());
    EXPECT_FALSE(world.systems().runs(alone));

    world.systems().setEnabled(alone, true);
    world.systems().setGroupEnabled("Doors", true);
    runTick(world, 1);
    EXPECT_EQ(log, (std::vector<std::string>{"Alone", "InGroup"}));
}

// srs.fr.2.17: each system's time in the last tick, for the systems that ran.
TEST(SystemRun, Timings_AreRecordedForTheSystemsThatRan) {
    World world{ecs_test::types()};
    std::vector<std::string> log;
    const engine::SystemId ran = world.systems().add(logging("Ran", TickPhase::GameplayRules, log));
    const engine::SystemId off = world.systems().add(logging("Off", TickPhase::GameplayRules, log));
    world.systems().setEnabled(off, false);

    runTick(world, 0);

    const std::vector<engine::SystemTiming> timings = world.systems().lastTickTimings();
    ASSERT_EQ(timings.size(), 1u);
    EXPECT_EQ(timings[0].system, ran);
    EXPECT_GE(timings[0].milliseconds, 0.0);
}

TEST(SystemRun, Context_GivesTheTickAndTheFixedStep) {
    World world{ecs_test::types()};
    std::vector<engine::Tick> ticks;
    world.systems().add(
        testSystem(named("Clock", TickPhase::GameplayRules), [&ticks](SystemContext& ctx) {
            ticks.push_back(ctx.tick());
            EXPECT_DOUBLE_EQ(ctx.fixedDeltaSeconds(), 1.0 / 60.0);
            EXPECT_EQ(ctx.systemName(), "Clock");
        }));

    runTick(world, 41);
    runTick(world, 42);

    EXPECT_EQ(ticks, (std::vector<engine::Tick>{41, 42}));
}

// srs.fr.2.116: a destruction requested in a phase applies at its end: later systems of the same
// phase still find the entity, the next phase does not.
TEST(SystemRun, DestroyRequest_AppliesAtTheEndOfThePhase) {
    World world{ecs_test::types()};
    const Entity target = world.createEntity();
    world.add<ecs_test::HealthComponent>(target);
    int seenLaterInPhase = 0;
    int seenNextPhase = 0;
    world.systems().add(testSystem(
        [](SystemDescription& d) {
            d.name("Killer")
                .purpose("Destroys what it sees.")
                .phase(TickPhase::GameplayInput)
                .reads<ecs_test::HealthComponent>()
                .destroysEntities();
        },
        [](SystemContext& ctx) {
            ctx.each<const ecs_test::HealthComponent>(
                [&ctx](Entity e, const ecs_test::HealthComponent&) { ctx.commands().destroy(e); });
        }));
    world.systems().add(testSystem(
        [](SystemDescription& d) {
            d.name("Witness")
                .purpose("Counts in the same phase.")
                .phase(TickPhase::GameplayInput)
                .reads<ecs_test::HealthComponent>();
        },
        [&seenLaterInPhase](SystemContext& ctx) {
            ctx.each<const ecs_test::HealthComponent>(
                [&](const ecs_test::HealthComponent&) { ++seenLaterInPhase; });
        }));
    world.systems().add(testSystem(
        [](SystemDescription& d) {
            d.name("Later")
                .purpose("Counts in the next phase.")
                .phase(TickPhase::GameplayRules)
                .reads<ecs_test::HealthComponent>();
        },
        [&seenNextPhase](SystemContext& ctx) {
            ctx.each<const ecs_test::HealthComponent>(
                [&](const ecs_test::HealthComponent&) { ++seenNextPhase; });
        }));

    runTick(world, 0);

    EXPECT_EQ(seenLaterInPhase, 1);
    EXPECT_EQ(seenNextPhase, 0);
    EXPECT_FALSE(world.isAlive(target));
}
