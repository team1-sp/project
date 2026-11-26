// The ECS and the physics world in step (US-3.2; srs.fr.3.11 to 3.13, 3.18, 3.22, 3.37, 3.48).
#include <cmath>
#include <functional>
#include <memory>
#include <numbers>
#include <optional>
#include <utility>

#include <gtest/gtest.h>

#include "engine/core/input_actions.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/ecs_simulation.hpp"
#include "engine/ecs/physics_components.hpp"
#include "engine/ecs/system.hpp"
#include "engine/ecs/system_context.hpp"
#include "engine/ecs/system_scheduler.hpp"
#include "engine/ecs/transform.hpp"
#include "engine/ecs/type_registry.hpp"
#include "engine/ecs/world.hpp"
#include "engine/physics_box2d/box2d_physics_adapter.hpp"
#include "engine/physics_sync/physics_sync.hpp"

namespace {

using engine::BoxColliderComponent;
using engine::Entity;
using engine::PositionComponent;
using engine::RigidBodyComponent;
using engine::RigidBodyType;
using engine::SystemContext;
using engine::TickPhase;
using engine::Vec2;
using engine::VelocityComponent;
using engine::World;

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

// A gameplay system for tests: runs `body` in GameplayInput, before PhysicsSyncIn.
class GameplayInput final : public engine::ISystem {
public:
    explicit GameplayInput(std::function<void(SystemContext&)> body)
        : body_(std::move(body)) {}
    void describe(engine::SystemDescription& d) const override {
        d.name("TestInput")
            .purpose("Writes components like a gameplay system.")
            .phase(TickPhase::GameplayInput)
            .writes<PositionComponent, VelocityComponent>();
    }
    void update(SystemContext& ctx) override { body_(ctx); }

private:
    std::function<void(SystemContext&)> body_;
};

// A World on the game loop's phases, connected to Box2D the way a game does it.
struct Game {
    Game() { engine::physics_sync::connectPhysics(world, simulation, sync, physics); }

    void tick(int count = 1) {
        const engine::InputActionState input;
        for (int i = 0; i < count; ++i) {
            for (const TickPhase phase : engine::tickPhases) {
                simulation.runPhase(phase, engine::TickContext{tickNumber, input});
            }
            ++tickNumber;
        }
    }

    Entity body(Vec2 at, std::optional<RigidBodyType> type) {
        const Entity e = world.createEntity();
        world.add(e, PositionComponent{at.x, at.y, 0.0f, 1.0f, 1.0f});
        world.add(e, VelocityComponent{});
        world.add(e, BoxColliderComponent{});
        if (type) {
            world.add(e, RigidBodyComponent{*type, 1.0f, 1.0f, 0.0f, false});
        }
        return e;
    }

    const PositionComponent& position(Entity e) const { return world.get<PositionComponent>(e); }
    const VelocityComponent& velocity(Entity e) const { return world.get<VelocityComponent>(e); }
    engine::physics::BodyState bodyState(Entity e) const {
        return physics.bodyState(*physics.findBody(world.idOf(e)));
    }

    engine::physics_box2d::Box2DPhysicsAdapter physics;
    engine::physics_sync::PhysicsSync sync{physics};
    World world{types()};
    engine::EcsSimulation simulation{world};
    engine::Tick tickNumber = 0;
};

} // namespace

TEST(PhysicsSyncOut, DynamicBody_PositionAndVelocityComeBackToTheEcs) {
    Game game;
    const Entity crate = game.body({0, 10}, RigidBodyType::Dynamic);
    game.tick(30);

    EXPECT_LT(game.position(crate).yMeters, 10.0f);
    EXPECT_FLOAT_EQ(game.position(crate).yMeters, game.bodyState(crate).positionMeters.y);
    EXPECT_LT(game.velocity(crate).yMetersPerSecond, -4.0f);            // about -5 m/s after 0.5 s
    EXPECT_FALSE(game.world.hasPendingWrite<PositionComponent>(crate)); // written untracked
}

TEST(PhysicsSyncOut, StaticBody_IsNeverMovedAndItsVelocityIgnored) {
    Game game;
    const Entity wall = game.body({3, 3}, std::nullopt);
    game.world.get<VelocityComponent>(wall) = VelocityComponent{5, 0};
    game.world.markWritten<VelocityComponent>(wall);
    game.tick(30);

    EXPECT_EQ(game.bodyState(wall).positionMeters, (Vec2{3, 3}));
    EXPECT_EQ(game.position(wall).xMeters, 3.0f);
}

TEST(PhysicsSyncOut, KinematicBody_MovesAtItsVelocityComponent) {
    Game game;
    const Entity platform = game.body({0, 0}, RigidBodyType::Kinematic);
    game.world.get<VelocityComponent>(platform) = VelocityComponent{2, 0};
    game.tick(60);

    EXPECT_NEAR(game.position(platform).xMeters, 2.0f, 1e-3f);
    EXPECT_EQ(game.velocity(platform).xMetersPerSecond, 2.0f); // physics leaves it alone
}

TEST(PhysicsSyncIn, GameplayPositionWrite_TeleportsTheBodyAndClearsTheMark) {
    Game game;
    const Entity crate = game.body({0, 10}, RigidBodyType::Dynamic);
    bool teleport = false;
    game.world.systems().add([&] {
        return std::make_unique<GameplayInput>([&](SystemContext& ctx) {
            if (teleport) {
                ctx.get<PositionComponent>(crate).xMeters = 7.0f; // marks a pending write
                teleport = false;
            }
        });
    });
    game.tick(10);
    teleport = true;
    game.tick();

    EXPECT_NEAR(game.bodyState(crate).positionMeters.x, 7.0f, 1e-4f);
    EXPECT_NEAR(game.position(crate).xMeters, 7.0f, 1e-4f);
    EXPECT_FALSE(game.world.hasPendingWrite<PositionComponent>(crate));
}

TEST(PhysicsSyncIn, GameplayVelocityWrite_SetsTheBodyVelocity) {
    Game game;
    const Entity crate = game.body({0, 10}, RigidBodyType::Dynamic);
    bool kick = true;
    game.world.systems().add([&] {
        return std::make_unique<GameplayInput>([&](SystemContext& ctx) {
            if (kick) {
                ctx.get<VelocityComponent>(crate).xMetersPerSecond = 3.0f;
                kick = false;
            }
        });
    });
    game.tick(30);

    EXPECT_NEAR(game.velocity(crate).xMetersPerSecond, 3.0f, 1e-3f);
    EXPECT_GT(game.position(crate).xMeters, 1.0f);
}

TEST(PhysicsSyncOut, ComponentWrittenAfterSyncIn_KeepsTheWrittenValue) {
    Game game;
    const Entity crate = game.body({0, 10}, RigidBodyType::Dynamic);
    game.tick(5);
    // A write after this tick's synchronization-in stays pending (srs.fr.3.48).
    game.world.get<PositionComponent>(crate).yMeters = 50.0f;
    game.world.markWritten<PositionComponent>(crate);
    const engine::InputActionState input;
    for (const TickPhase phase : {TickPhase::PhysicsStep, TickPhase::PhysicsSyncOut}) {
        game.simulation.runPhase(phase, engine::TickContext{game.tickNumber, input});
    }

    EXPECT_EQ(game.position(crate).yMeters, 50.0f);
    EXPECT_TRUE(game.world.hasPendingWrite<PositionComponent>(crate));
}

TEST(PhysicsSyncOut, DynamicChild_PositionIsWrittenRelativeToItsParent) {
    Game game;
    const Entity parent = game.world.createEntity();
    game.world.add(parent, PositionComponent{10, 0, std::numbers::pi_v<float> / 2, 2, 2});
    const Entity crate = game.body({0, 0}, RigidBodyType::Dynamic);
    game.world.get<PositionComponent>(crate) = PositionComponent{1, 0, 0, 1, 1};
    game.world.setParent(crate, parent);
    game.tick();

    // Local (1, 0) under a parent at (10, 0), turned a quarter and scaled 2: world (10, 2).
    EXPECT_NEAR(game.bodyState(crate).positionMeters.x, 10.0f, 0.01f);
    const engine::WorldTransform parentWorld =
        engine::compose(engine::worldOrigin, game.position(parent));
    const engine::WorldTransform world = engine::compose(parentWorld, game.position(crate));
    EXPECT_NEAR(world.position.x, game.bodyState(crate).positionMeters.x, 1e-4f);
    EXPECT_NEAR(world.position.y, game.bodyState(crate).positionMeters.y, 1e-4f);
}

TEST(PhysicsSyncIn, StaticChild_FollowsItsParent) {
    Game game;
    const Entity parent = game.world.createEntity();
    game.world.add(parent, PositionComponent{});
    const Entity sensor = game.body({2, 0}, std::nullopt);
    game.world.setParent(sensor, parent);
    game.tick();
    game.world.get<PositionComponent>(parent).xMeters = 5.0f; // the parent moves
    game.tick();

    EXPECT_NEAR(game.bodyState(sensor).positionMeters.x, 7.0f, 1e-4f);
}

TEST(PhysicsSyncOut, Declaration_WritesPositionAndVelocityUntracked) {
    Game game;
    const auto id = game.world.systems().find("PhysicsSyncOut");
    ASSERT_TRUE(id.has_value());

    EXPECT_EQ(game.world.systems().phaseOf(*id), TickPhase::PhysicsSyncOut);
    EXPECT_EQ(game.world.systems().declaration(*id).writesUntracked.size(), 2u);
    EXPECT_TRUE(game.world.systems().declaration(*id).writes.empty());
}
