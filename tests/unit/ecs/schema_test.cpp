// The schema of a run (US-2.3; srs.fr.2.12, 2.16, 8.29; design/ecs-api-design.md §12).
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/schema.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::testSystem;
using engine::SystemDescription;
using engine::TickPhase;
using engine::World;

namespace {

void addSystems(World& world) {
    world.systems().defineGroup("Motion", TickPhase::GameplayInput, "Moves the player.");
    world.systems().add(testSystem([](SystemDescription& d) {
        d.name("Sync")
            .purpose("Copies positions out.")
            .phase(TickPhase::PhysicsSyncIn)
            .reads<engine::PositionComponent, engine::VelocityComponent>();
    }));
    world.systems().add(testSystem([](SystemDescription& d) {
        d.name("Walk")
            .purpose("Sets the velocity.")
            .group("Motion")
            .writes<engine::VelocityComponent>()
            .usesRandom();
    }));
}

} // namespace

// srs.fr.2.12, 8.29: the systems that read and write each component type, derived from the
// declarations.
TEST(Schema, ReadByAndWrittenBy_AreDerivedFromTheDeclarations) {
    World world{ecs_test::types()};
    addSystems(world);

    const engine::Schema schema = engine::buildSchema(world);

    EXPECT_EQ(schema.readBy.at("Position"), (std::vector<std::string>{"Sync"}));
    EXPECT_EQ(schema.readBy.at("Velocity"), (std::vector<std::string>{"Sync"}));
    EXPECT_EQ(schema.writtenBy.at("Velocity"), (std::vector<std::string>{"Walk"}));
    EXPECT_TRUE(schema.writtenBy.at("Position").empty());
    EXPECT_TRUE(schema.readBy.at("Health").empty());
}

// srs.fr.8.29: each system with its purpose, phase, group, run order, enabled state and
// declarations; the tick phases; the component descriptions.
TEST(Schema, Systems_AreListedInRunOrderWithTheirDeclarations) {
    World world{ecs_test::types()};
    addSystems(world);
    world.systems().setGroupEnabled("Motion", false);

    const engine::Schema schema = engine::buildSchema(world);

    ASSERT_EQ(schema.phases.size(), 7u);
    EXPECT_EQ(schema.phases[1], "GameplayInput");
    ASSERT_EQ(schema.systems.size(), 2u);
    const engine::SystemSchema& walk = schema.systems[0];
    EXPECT_EQ(walk.declaration.name, "Walk");
    EXPECT_EQ(walk.declaration.group, "Motion");
    EXPECT_EQ(walk.declaration.phase, TickPhase::GameplayInput);
    EXPECT_EQ(walk.orderIndex, 0u);
    EXPECT_FALSE(walk.enabled);
    EXPECT_TRUE(walk.declaration.usesRandom);
    EXPECT_EQ(schema.systems[1].declaration.name, "Sync");
    EXPECT_TRUE(schema.systems[1].enabled);
    ASSERT_EQ(schema.groups.size(), 1u);
    EXPECT_FALSE(schema.groups[0].enabled);
    EXPECT_EQ(schema.components.size(), ecs_test::types().components().size());
    EXPECT_EQ(schema.components[0].name, "Name");
}
