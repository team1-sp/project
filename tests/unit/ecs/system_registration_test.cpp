// Registering systems in the World (US-2.3; srs.fr.2.13, 2.16, 2.38, 2.39, 2.48, 2.126).
#include <string>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/run_error.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::named;
using ecs_test::testSystem;
using engine::RunError;
using engine::SystemDescription;
using engine::TickPhase;
using engine::World;

namespace {

struct NotRegistered {
    static constexpr std::string_view typeName = "NotRegistered";
    int value = 0;
};

// The message of the RunError that `add` throws, or "" when it does not throw.
std::string refusal(World& world, const engine::SystemFactory& factory) {
    try {
        world.systems().add(factory);
    } catch (const RunError& error) {
        return error.what();
    }
    return "";
}

} // namespace

// srs.fr.2.13: systems are registered in the World and keep their declaration.
TEST(SystemRegistration, Add_RegistersTheSystemWithItsDeclaration) {
    World world{ecs_test::types()};

    const engine::SystemId id = world.systems().add(testSystem([](SystemDescription& d) {
        d.name("Mover")
            .purpose("Moves things.")
            .phase(TickPhase::GameplayRules)
            .reads<engine::VelocityComponent>()
            .writes<engine::PositionComponent>();
    }));

    EXPECT_EQ(world.systems().size(), 1u);
    EXPECT_EQ(world.systems().find("Mover"), id);
    EXPECT_EQ(world.systems().phaseOf(id), TickPhase::GameplayRules);
    EXPECT_EQ(world.systems().declaration(id).writes.size(), 1u);
    EXPECT_EQ(world.systems().declaration(id).writes[0].name, "Position");
}

// srs.fr.2.38: no purpose, no registration.
TEST(SystemRegistration, WithoutAPurpose_IsRefused) {
    World world{ecs_test::types()};

    const std::string problem = refusal(world, testSystem([](SystemDescription& d) {
                                            d.name("Silent").phase(TickPhase::GameplayRules);
                                        }));

    EXPECT_NE(problem.find("Silent needs a name and a purpose"), std::string::npos) << problem;
    EXPECT_EQ(world.systems().size(), 0u);
}

// srs.fr.2.39: names are unique.
TEST(SystemRegistration, ASecondSystemWithTheSameName_IsRefused) {
    World world{ecs_test::types()};
    world.systems().add(testSystem(named("Twin", TickPhase::GameplayInput)));

    EXPECT_NE(refusal(world, testSystem(named("Twin", TickPhase::GameplayRules))).find("already"),
              std::string::npos);
    EXPECT_EQ(world.systems().size(), 1u);
}

// srs.fr.2.126: a declared type that is not registered refuses the system, naming both.
TEST(SystemRegistration, DeclaringAnUnregisteredType_IsRefusedNamingSystemAndType) {
    World world{ecs_test::types()};

    const std::string problem = refusal(world, testSystem([](SystemDescription& d) {
                                            d.name("Curious")
                                                .purpose("Reads something unknown.")
                                                .phase(TickPhase::GameplayRules)
                                                .reads<NotRegistered>();
                                        }));

    EXPECT_NE(problem.find("Curious"), std::string::npos) << problem;
    EXPECT_NE(problem.find("NotRegistered"), std::string::npos) << problem;
}

// srs.fr.2.48: the physics step holds no systems.
TEST(SystemRegistration, InThePhysicsStepPhase_IsRefused) {
    World world{ecs_test::types()};

    EXPECT_THROW(world.systems().add(testSystem(named("Stepper", TickPhase::PhysicsStep))),
                 RunError);
    EXPECT_THROW(world.systems().defineGroup("Steps", TickPhase::PhysicsStep, "Physics."),
                 RunError);
}

// srs.fr.2.48: a system in a group runs in the group's phase.
TEST(SystemRegistration, InAGroup_TakesTheGroupsPhase) {
    World world{ecs_test::types()};
    world.systems().defineGroup("Rules", TickPhase::GameplayRules, "Gameplay rules.");

    const engine::SystemId id = world.systems().add(testSystem(
        [](SystemDescription& d) { d.name("Door").purpose("Opens doors.").group("Rules"); }));

    EXPECT_EQ(world.systems().phaseOf(id), TickPhase::GameplayRules);
}

TEST(SystemRegistration, GroupProblems_AreRefused) {
    World world{ecs_test::types()};
    world.systems().defineGroup("Rules", TickPhase::GameplayRules, "Gameplay rules.");

    EXPECT_NE(refusal(world, testSystem([](SystemDescription& d) {
                          d.name("Lost").purpose("No such group.").group("Nowhere");
                      }))
                  .find("not defined"),
              std::string::npos);
    EXPECT_NE(refusal(world, testSystem([](SystemDescription& d) {
                          d.name("Torn")
                              .purpose("Two phases.")
                              .group("Rules")
                              .phase(TickPhase::GameplayInput);
                      }))
                  .find("is not the phase"),
              std::string::npos);
    EXPECT_NE(refusal(world, testSystem([](SystemDescription& d) {
                          d.name("Drifting").purpose("No phase at all.");
                      }))
                  .find("names no tick phase"),
              std::string::npos);
    EXPECT_THROW(world.systems().defineGroup("Rules", TickPhase::GameplayInput, "Again."),
                 RunError);
}
