// Adding, removing, reading and changing components (US-2.2; srs.fr.2.6 to 2.11).
#include <cstddef>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/entity_id.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/type_id.hpp"
#include "engine/ecs/type_registry.hpp"
#include "engine/ecs/world.hpp"

namespace {

using ecs_test::editorId;
using ecs_test::HealthComponent;
using ecs_test::RecordingObserver;
using engine::Entity;
using engine::EntityId;
using engine::PositionComponent;
using engine::RunError;
using engine::VelocityComponent;
using engine::World;

struct UnregisteredComponent {
    static constexpr std::string_view typeName = "Unregistered";
    int value = 0;
};

} // namespace

// srs.fr.2.6, 2.8: add a component at runtime and read its fields.
TEST(WorldComponents, Add_StoresTheValueForThatEntity) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();

    world.add(e, PositionComponent{1.0f, 2.0f});

    ASSERT_TRUE(world.has<PositionComponent>(e));
    EXPECT_EQ(world.get<PositionComponent>(e).xMeters, 1.0f);
    EXPECT_EQ(world.get<PositionComponent>(e).yMeters, 2.0f);
    EXPECT_EQ(world.get<PositionComponent>(e).scaleX, 1.0f);
    EXPECT_FALSE(world.has<VelocityComponent>(e));
    EXPECT_EQ(world.tryGet<VelocityComponent>(e), nullptr);
}

// srs.fr.2.9: change a field at runtime.
TEST(WorldComponents, Get_GivesWriteAccess) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<HealthComponent>(e);

    world.get<HealthComponent>(e).points -= 30;

    EXPECT_EQ(world.get<HealthComponent>(e).points, 70);
}

// srs.fr.2.7: remove a component at runtime.
TEST(WorldComponents, Remove_TakesTheComponentAway) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<HealthComponent>(e);

    world.remove<HealthComponent>(e);

    EXPECT_FALSE(world.has<HealthComponent>(e));
    EXPECT_THROW(world.remove<HealthComponent>(e), RunError);
}

// srs.fr.2.10: at most one component of each type per entity.
TEST(WorldComponents, AddASecondOfTheSameType_IsRefused) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add(e, HealthComponent{40});

    EXPECT_THROW(world.add(e, HealthComponent{90}), RunError);
    EXPECT_EQ(world.get<HealthComponent>(e).points, 40);
    EXPECT_THROW(world.add<engine::NameComponent>(e), RunError);
}

TEST(WorldComponents, BuiltInComponents_CannotBeRemoved) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();

    EXPECT_THROW(world.remove<engine::NameComponent>(e), RunError);
    EXPECT_TRUE(world.has<engine::NameComponent>(e));
}

TEST(WorldComponents, UnregisteredType_IsRefused) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();

    EXPECT_THROW(world.add<UnregisteredComponent>(e), RunError);
    EXPECT_THROW((void)world.has<UnregisteredComponent>(e), RunError);
}

// srs.fr.2.11: components have an enabled flag, on when added.
TEST(WorldComponents, EnabledFlag_StartsOnAndCanBeTurnedOff) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<HealthComponent>(e);
    EXPECT_TRUE(world.isComponentEnabled<HealthComponent>(e));

    world.setComponentEnabled<HealthComponent>(e, false);

    EXPECT_FALSE(world.isComponentEnabled<HealthComponent>(e));
    EXPECT_TRUE(world.has<HealthComponent>(e)); // disabled, not removed
    EXPECT_EQ(world.get<HealthComponent>(e).points, 100);
}

// srs.fr.2.11: Position and Velocity have no enabled flag of their own; nor do the built-ins.
TEST(WorldComponents, PositionVelocityAndBuiltIns_HaveNoEnabledFlag) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<PositionComponent>(e);
    world.add<VelocityComponent>(e);

    EXPECT_THROW(world.setComponentEnabled<PositionComponent>(e, false), RunError);
    EXPECT_THROW(world.setComponentEnabled<VelocityComponent>(e, false), RunError);
    EXPECT_THROW(world.setComponentEnabled<engine::NameComponent>(e, false), RunError);
    EXPECT_TRUE(world.isComponentEnabled<PositionComponent>(e));
}

// The enabled flag stays with its component when the pool reorders (D-3, D-13).
TEST(WorldComponents, EnabledFlags_FollowTheirComponentsThroughRemovalsAndSorting) {
    World world{ecs_test::types()};
    const Entity c = world.createEntity(editorId(30));
    const Entity a = world.createEntity(editorId(10));
    const Entity b = world.createEntity(editorId(20));
    for (const Entity e : {c, a, b}) {
        world.add<HealthComponent>(e);
    }
    world.setComponentEnabled<HealthComponent>(c, false);

    world.remove<HealthComponent>(a);

    EXPECT_FALSE(world.isComponentEnabled<HealthComponent>(c));
    EXPECT_TRUE(world.isComponentEnabled<HealthComponent>(b));
    const engine::ComponentPool<HealthComponent>& pool = world.pool<HealthComponent>();
    ASSERT_EQ(pool.size(), 2u);
    EXPECT_EQ(pool.idAt(0), editorId(20));
    EXPECT_TRUE(pool.enabledAt(0));
    EXPECT_EQ(pool.idAt(1), editorId(30));
    EXPECT_FALSE(pool.enabledAt(1));
}

TEST(WorldComponents, AddAndRemove_TellObservers) {
    World world{ecs_test::types()};
    RecordingObserver observer;
    world.addObserver(observer);
    const Entity e = world.createEntity();

    world.add<HealthComponent>(e);
    world.remove<HealthComponent>(e);

    const engine::TypeId health = engine::typeIdOf<HealthComponent>();
    EXPECT_EQ(observer.added, (std::vector<engine::TypeId>{health}));
    EXPECT_EQ(observer.removed, (std::vector<engine::TypeId>{health}));
}

TEST(WorldComponents, DestroyEntity_RemovesAllItsComponents) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<HealthComponent>(e);
    world.add<PositionComponent>(e);

    world.destroyEntity(e);

    EXPECT_EQ(world.pool<HealthComponent>().size(), 0u);
    EXPECT_EQ(world.pool<PositionComponent>().size(), 0u);
    EXPECT_EQ(world.pool<engine::NameComponent>().size(), 0u);
}

TEST(WorldComponents, ForEachComponent_GivesEveryComponentInRegistrationOrder) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<HealthComponent>(e);
    world.add<PositionComponent>(e);
    std::vector<std::string> names;

    world.forEachComponent(e, [&names](const engine::ComponentTypeInfo& info, const void*) {
        names.push_back(info.name);
    });

    EXPECT_EQ(names,
              (std::vector<std::string>{"Name", "EntityInfo", "Hierarchy", "Position", "Health"}));
}

TEST(WorldComponents, PoolByTypeId_IsTheSameStorage) {
    World world{ecs_test::types()};
    const Entity e = world.createEntity();
    world.add<HealthComponent>(e);

    const engine::IComponentPool* pool = world.pool(engine::typeIdOf<HealthComponent>());

    ASSERT_NE(pool, nullptr);
    EXPECT_EQ(pool->typeName(), "Health");
    EXPECT_TRUE(pool->contains(e));
    EXPECT_EQ(world.pool(engine::typeIdFromName("Nothing")), nullptr);
}
