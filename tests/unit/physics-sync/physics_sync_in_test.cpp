// PhysicsSyncIn keeps bodies and shapes in step with the components (US-3.1; srs.fr.3.2, 3.8).
#include <memory>
#include <optional>

#include <gtest/gtest.h>

#include "engine/core/input_actions.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/physics_components.hpp"
#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/type_registry.hpp"
#include "engine/ecs/world.hpp"
#include "engine/physics_box2d/box2d_physics_adapter.hpp"
#include "engine/physics_sync/physics_sync.hpp"

namespace {

using engine::BoxColliderComponent;
using engine::CircleColliderComponent;
using engine::Entity;
using engine::PositionComponent;
using engine::RigidBodyComponent;
using engine::RigidBodyType;
using engine::World;
using engine::physics::BodyId;
using engine::physics_sync::PhysicsSync;
using engine::physics_sync::PhysicsSyncInSystem;

const engine::TypeRegistry& types() {
    static const engine::TypeRegistry registry = [] {
        engine::TypeRegistry r;
        engine::registerCoreComponents(r);
        engine::registerPhysicsComponents(r);
        r.finalize();
        return r;
    }();
    return registry;
}

// One World with its Box2D adapter and PhysicsSyncIn, as a game would set them up.
struct Scene {
    Scene() {
        world.addObserver(sync);
        world.systems().add([this] { return std::make_unique<PhysicsSyncInSystem>(sync); });
    }

    // One tick: PhysicsSyncIn, then the step the game loop runs in PhysicsStep.
    void tick() {
        const engine::InputActionState input;
        world.systems().runPhase(engine::TickPhase::PhysicsSyncIn,
                                 engine::TickContext{tickNumber++, input});
        physics.step(1.0f / 60.0f);
    }

    std::optional<BodyId> bodyOf(Entity e) const { return physics.findBody(world.idOf(e)); }
    float yOf(Entity e) const { return physics.bodyState(*bodyOf(e)).positionMeters.y; }

    engine::physics_box2d::Box2DPhysicsAdapter physics;
    PhysicsSync sync{physics};
    World world{types()};
    engine::Tick tickNumber = 0;
};

Entity crateAt(World& world, float yMeters) {
    const Entity e = world.createEntity();
    world.add(e, PositionComponent{0.0f, yMeters, 0.0f, 1.0f, 1.0f});
    world.add(e, BoxColliderComponent{});
    return e;
}

} // namespace

TEST(PhysicsSyncIn, ColliderWithoutRigidBody_GetsAStaticBodyAtItsPosition) {
    Scene scene;
    const Entity wall = crateAt(scene.world, 3.0f);
    scene.tick();
    scene.tick();

    ASSERT_TRUE(scene.bodyOf(wall).has_value());
    EXPECT_EQ(scene.sync.bodyOf(scene.world.idOf(wall)), scene.bodyOf(wall));
    EXPECT_FLOAT_EQ(scene.yOf(wall), 3.0f); // static: gravity does not move it
}

TEST(PhysicsSyncIn, DynamicRigidBody_FallsWithItsMass) {
    Scene scene;
    const Entity crate = crateAt(scene.world, 3.0f);
    scene.world.add(crate, RigidBodyComponent{RigidBodyType::Dynamic, 2.5f, 1.0f, 0.0f, false});
    scene.tick();
    scene.tick();

    EXPECT_LT(scene.yOf(crate), 3.0f);
    EXPECT_FLOAT_EQ(scene.physics.bodyMassKilograms(*scene.bodyOf(crate)), 2.5f);
}

TEST(PhysicsSyncIn, RigidBodyWithoutCollider_GetsNoBody) {
    Scene scene;
    const Entity e = scene.world.createEntity();
    scene.world.add(e, PositionComponent{});
    scene.world.add(e, RigidBodyComponent{});
    scene.tick();

    EXPECT_EQ(scene.bodyOf(e), std::nullopt);
}

TEST(PhysicsSyncIn, RemovingOneOfTwoColliders_KeepsTheBodyAndTheLastOneDestroysIt) {
    Scene scene;
    const Entity e = crateAt(scene.world, 0.0f);
    scene.world.add(e, CircleColliderComponent{});
    scene.tick();
    const std::optional<BodyId> body = scene.bodyOf(e);
    ASSERT_TRUE(body.has_value());

    scene.world.remove<BoxColliderComponent>(e);
    scene.tick();
    EXPECT_EQ(scene.bodyOf(e), body);

    scene.world.remove<CircleColliderComponent>(e);
    scene.tick();
    EXPECT_EQ(scene.bodyOf(e), std::nullopt);
    EXPECT_EQ(scene.sync.bodyOf(scene.world.idOf(e)), std::nullopt);
}

TEST(PhysicsSyncIn, ColliderAddedLater_GivesTheEntityABody) {
    Scene scene;
    const Entity e = scene.world.createEntity();
    scene.world.add(e, PositionComponent{});
    scene.tick();
    EXPECT_EQ(scene.bodyOf(e), std::nullopt);

    scene.world.add(e, CircleColliderComponent{});
    scene.tick();
    EXPECT_TRUE(scene.bodyOf(e).has_value());
}

TEST(PhysicsSyncIn, RemovingTheRigidBody_MakesTheBodyStatic) {
    Scene scene;
    const Entity crate = crateAt(scene.world, 3.0f);
    scene.world.add(crate, RigidBodyComponent{});
    scene.tick();
    scene.tick();
    ASSERT_LT(scene.yOf(crate), 3.0f); // falling

    scene.world.remove<RigidBodyComponent>(crate);
    scene.tick();
    const float stoppedAt = scene.yOf(crate);
    scene.tick();
    scene.tick();
    EXPECT_FLOAT_EQ(scene.yOf(crate), stoppedAt);
}

TEST(PhysicsSyncIn, DestroyingTheEntity_DestroysItsBody) {
    Scene scene;
    const Entity crate = crateAt(scene.world, 0.0f);
    const engine::EntityId id = scene.world.idOf(crate);
    scene.tick();
    ASSERT_TRUE(scene.physics.findBody(id).has_value());

    scene.world.destroyEntity(crate);
    scene.tick();
    EXPECT_EQ(scene.physics.findBody(id), std::nullopt);
}

TEST(PhysicsSyncIn, Declaration_ReadsTheComponentsItUsesInItsPhase) {
    Scene scene;
    const auto id = scene.world.systems().find("PhysicsSyncIn");
    ASSERT_TRUE(id.has_value());

    EXPECT_EQ(scene.world.systems().phaseOf(*id), engine::TickPhase::PhysicsSyncIn);
    EXPECT_EQ(scene.world.systems().declaration(*id).reads.size(), 5u);
    EXPECT_TRUE(scene.world.systems().declaration(*id).writes.empty());
}
