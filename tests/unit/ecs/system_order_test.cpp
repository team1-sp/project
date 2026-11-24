// The run order of systems (US-2.3; srs.fr.2.14, 2.49 to 2.51).
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/run_error.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::named;
using ecs_test::testSystem;
using engine::RunError;
using engine::SystemDescription;
using engine::SystemId;
using engine::TickPhase;
using engine::World;

namespace {

constexpr TickPhase rules = TickPhase::GameplayRules;

// A system in GameplayRules with optional before/after rules.
engine::SystemFactory ruled(std::string name, std::vector<std::string> before,
                            std::vector<std::string> after, std::string group = "") {
    return testSystem([=](SystemDescription& d) {
        d.name(name).purpose("Ordered system " + name + ".");
        if (group.empty()) {
            d.phase(rules);
        } else {
            d.group(group);
        }
        for (const std::string& target : before) {
            d.runsBefore(target);
        }
        for (const std::string& target : after) {
            d.runsAfter(target);
        }
    });
}

std::vector<std::string> namesInOrder(const World& world, TickPhase phase) {
    std::vector<std::string> names;
    for (const SystemId id : world.systems().runOrder(phase)) {
        names.push_back(world.systems().declaration(id).name);
    }
    return names;
}

} // namespace

TEST(SystemOrder, WithoutRules_IsTheRegistrationOrderPerPhase) {
    World world{ecs_test::types()};
    world.systems().add(testSystem(named("A", rules)));
    world.systems().add(testSystem(named("Early", TickPhase::GameplayInput)));
    world.systems().add(testSystem(named("B", rules)));

    EXPECT_EQ(namesInOrder(world, rules), (std::vector<std::string>{"A", "B"}));
    EXPECT_EQ(namesInOrder(world, TickPhase::GameplayInput), (std::vector<std::string>{"Early"}));
    EXPECT_EQ(world.systems().orderIndex(world.systems().find("Early").value()), 0u);
}

// srs.fr.2.49, 2.50: before/after rules reorder, the rest keeps registration order.
TEST(SystemOrder, RunsAfter_PlacesTheSystemAfterTheOther) {
    World world{ecs_test::types()};
    world.systems().add(ruled("Collect", {}, {"Move"}));
    world.systems().add(ruled("Score", {}, {}));
    world.systems().add(ruled("Move", {}, {}));

    EXPECT_EQ(namesInOrder(world, rules), (std::vector<std::string>{"Score", "Move", "Collect"}));
}

// A rule naming a group applies to each system in it.
TEST(SystemOrder, RuleNamingAGroup_AppliesToItsMembers) {
    World world{ecs_test::types()};
    world.systems().defineGroup("Physicsy", rules, "Systems that run first.");
    world.systems().add(ruled("Last", {}, {"Physicsy"}));
    world.systems().add(ruled("G1", {}, {}, "Physicsy"));
    world.systems().add(ruled("G2", {}, {}, "Physicsy"));

    EXPECT_EQ(namesInOrder(world, rules), (std::vector<std::string>{"G1", "G2", "Last"}));
}

// srs.fr.2.50: the manual order is kept where the rules allow it.
TEST(SystemOrder, ManualOrder_IsKeptWhereTheRulesAllow) {
    World world{ecs_test::types()};
    const SystemId a = world.systems().add(ruled("A", {}, {}));
    const SystemId b = world.systems().add(ruled("B", {}, {}));
    const SystemId c = world.systems().add(ruled("C", {"A"}, {})); // C before A

    const std::vector<SystemId> manual = {a, b, c};
    EXPECT_TRUE(world.systems().setManualOrder(manual).accepted);
    EXPECT_EQ(namesInOrder(world, rules), (std::vector<std::string>{"B", "C", "A"}));

    const std::vector<SystemId> incomplete = {a, b};
    EXPECT_FALSE(world.systems().setManualOrder(incomplete).accepted);
}

// srs.fr.2.51: a cycle is refused, the last valid order stays and the cycle is named.
TEST(SystemOrder, Cycle_IsRefusedAndTheLastValidOrderStays) {
    World world{ecs_test::types()};
    world.systems().add(ruled("A", {}, {}));
    world.systems().add(ruled("B", {}, {"A"}));
    world.systems().add(ruled("Bystander", {}, {}));

    try {
        world.systems().add(ruled("C", {"A"}, {"B"})); // A < B < C < A
        FAIL() << "the cycle was accepted";
    } catch (const RunError& error) {
        const std::string message = error.what();
        EXPECT_NE(message.find("cycle"), std::string::npos) << message;
        EXPECT_NE(message.find("A, B, C"), std::string::npos) << message;
        EXPECT_EQ(message.find("Bystander"), std::string::npos) << message;
    }
    EXPECT_EQ(namesInOrder(world, rules), (std::vector<std::string>{"A", "B", "Bystander"}));
    EXPECT_FALSE(world.systems().find("C").has_value());
}

// srs.fr.2.51: a rule across phases is fine when the phase order keeps it, refused otherwise.
TEST(SystemOrder, RulesAcrossPhases_AreIgnoredOrRefusedByThePhaseOrder) {
    World world{ecs_test::types()};
    world.systems().add(testSystem(named("Input", TickPhase::GameplayInput)));
    world.systems().add(ruled("Fine", {}, {"Input"})); // GameplayRules after GameplayInput: kept

    try {
        world.systems().add(ruled("Wrong", {"Input"}, {}));
        FAIL() << "the contradicting rule was accepted";
    } catch (const RunError& error) {
        const std::string message = error.what();
        EXPECT_NE(message.find("GameplayRules"), std::string::npos) << message;
        EXPECT_NE(message.find("GameplayInput"), std::string::npos) << message;
    }
    EXPECT_EQ(namesInOrder(world, rules), (std::vector<std::string>{"Fine"}));
}

TEST(SystemOrder, RuleNamingAnUnknownSystem_IsIgnored) {
    World world{ecs_test::types()};
    world.systems().add(ruled("A", {"Ghost"}, {"Phantom"}));

    EXPECT_EQ(namesInOrder(world, rules), (std::vector<std::string>{"A"}));
}
