// Queries by component and singletons (US-2.3; srs.fr.2.15, 2.18, 2.116;
// design/ecs-api-design.md §7).
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/entity_id.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/world.hpp"

using ecs_test::editorId;
using ecs_test::HealthComponent;
using engine::Entity;
using engine::EntityId;
using engine::PositionComponent;
using engine::RunError;
using engine::VelocityComponent;
using engine::World;

namespace {

std::vector<EntityId> idsWithPositionAndVelocity(World& world) {
    std::vector<EntityId> ids;
    world.each<const PositionComponent, VelocityComponent>(
        [&](Entity e, const PositionComponent&, VelocityComponent&) {
            ids.push_back(world.idOf(e));
        });
    return ids;
}

} // namespace

// srs.fr.2.18: every entity with all the types, and only those; ascending EntityId (rule 6).
TEST(Query, Each_VisitsEntitiesWithAllTypesInAscendingId) {
    World world{ecs_test::types()};
    for (const std::uint64_t id : {30u, 10u, 20u, 40u}) {
        const Entity e = world.createEntity(editorId(id));
        world.add<PositionComponent>(e);
        if (id != 40) {
            world.add<VelocityComponent>(e);
        }
    }

    EXPECT_EQ(idsWithPositionAndVelocity(world),
              (std::vector<EntityId>{editorId(10), editorId(20), editorId(30)}));
}

TEST(Query, Each_GivesWriteAccessToNonConstTypes) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add(e, VelocityComponent{1.0f, 0.0f});

    world.each<VelocityComponent>(
        [](VelocityComponent& velocity) { velocity.xMetersPerSecond = 5.0f; });

    EXPECT_EQ(world.get<VelocityComponent>(e).xMetersPerSecond, 5.0f);
}

// srs.fr.2.15: a disabled entity is skipped, its Position and Velocity too.
TEST(Query, DisabledEntity_IsSkipped) {
    World world{ecs_test::types()};
    const Entity a = world.createEntity(editorId(1));
    const Entity b = world.createEntity(editorId(2));
    for (const Entity e : {a, b}) {
        world.add<PositionComponent>(e);
        world.add<VelocityComponent>(e);
    }

    world.setEnabled(a, false);

    EXPECT_EQ(idsWithPositionAndVelocity(world), (std::vector<EntityId>{editorId(2)}));
}

// srs.fr.2.11, 2.15: an entity whose queried component is disabled is skipped.
TEST(Query, DisabledComponent_IsSkipped) {
    World world{ecs_test::types()};
    const Entity a = world.createEntity(editorId(1));
    const Entity b = world.createEntity(editorId(2));
    world.add<HealthComponent>(a);
    world.add<HealthComponent>(b);
    world.setComponentEnabled<HealthComponent>(b, false);
    std::vector<EntityId> ids;

    world.each<const HealthComponent>(
        [&](Entity e, const HealthComponent&) { ids.push_back(world.idOf(e)); });

    EXPECT_EQ(ids, (std::vector<EntityId>{editorId(1)}));
}

// srs.fr.2.116: a disable takes effect for queries from the next query, not inside the current.
TEST(Query, DisableDuringAQuery_IsSeenFromTheNextQuery) {
    World world{ecs_test::types()};
    const Entity first = world.createEntity(editorId(1));
    const Entity second = world.createEntity(editorId(2));
    world.add<HealthComponent>(first);
    world.add<HealthComponent>(second);
    int visited = 0;

    world.each<HealthComponent>([&](Entity e, HealthComponent&) {
        ++visited;
        if (e == first) {
            world.setEnabled(second, false);
            EXPECT_FALSE(world.isEnabled(second)); // the flag itself changes at once
        }
    });
    EXPECT_EQ(visited, 2);

    visited = 0;
    world.each<HealthComponent>([&](HealthComponent&) { ++visited; });
    EXPECT_EQ(visited, 1);
}

// §7 rule 4: structural changes wait for the CommandBuffer; a direct one inside a query fails.
TEST(Query, StructuralChangeInsideAQuery_IsRefused) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<HealthComponent>(e);

    EXPECT_THROW(world.each<HealthComponent>([&](HealthComponent&) { world.createEntity(); }),
                 RunError);
    EXPECT_THROW(world.each<HealthComponent>([&](HealthComponent&) { world.destroyEntity(e); }),
                 RunError);
    EXPECT_TRUE(world.isAlive(e));
    world.createEntity(); // allowed again once the query has ended
}

// D-16: a singleton is the component of the one entity that has it.
TEST(Query, Single_GivesTheOneComponentAndRefusesNoneOrTwo) {
    World world{ecs_test::types()};
    EXPECT_THROW((void)world.single<HealthComponent>(), RunError);

    world.createSingleton(HealthComponent{7});
    EXPECT_EQ(world.single<HealthComponent>().points, 7);

    world.createSingleton(HealthComponent{8});
    EXPECT_THROW((void)world.single<HealthComponent>(), RunError);
}
