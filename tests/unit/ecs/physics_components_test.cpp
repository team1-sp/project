#include <string_view>
#include <variant>

#include <gtest/gtest.h>

#include "engine/ecs/core_components.hpp"
#include "engine/ecs/physics_components.hpp"
#include "engine/ecs/type_registry.hpp"

namespace {

using engine::ComponentTypeInfo;
using engine::FieldInfo;
using engine::FieldType;
using engine::TypeRegistry;
using engine::Unit;

const TypeRegistry& physicsTypes() {
    static const TypeRegistry registry = [] {
        TypeRegistry r;
        engine::registerCoreComponents(r);
        engine::registerPhysicsComponents(r);
        r.finalize(); // refuses missing descriptions and unregistered members
        return r;
    }();
    return registry;
}

const FieldInfo& field(const ComponentTypeInfo& type, std::string_view name) {
    for (const FieldInfo& info : type.fields) {
        if (info.name == name) {
            return info;
        }
    }
    ADD_FAILURE() << "no field " << name;
    return type.fields.front();
}

} // namespace

TEST(PhysicsComponents, RigidBody_IsDynamicOneKilogramByDefault) {
    const ComponentTypeInfo* rigidBody = physicsTypes().find<engine::RigidBodyComponent>();
    ASSERT_NE(rigidBody, nullptr);

    EXPECT_FALSE(rigidBody->alwaysEnabled); // has an enabled flag (srs.fr.3.31)
    EXPECT_EQ(field(*rigidBody, "bodyType").type, FieldType::Enum);
    EXPECT_EQ(field(*rigidBody, "bodyType").enumType, "RigidBodyType");
    EXPECT_EQ(field(*rigidBody, "mass").unit, Unit::Kilograms);
    EXPECT_EQ(std::get<float>(field(*rigidBody, "mass").defaultValue), 1.0f);
    EXPECT_EQ(engine::RigidBodyComponent{}.bodyType, engine::RigidBodyType::Dynamic);
}

TEST(PhysicsComponents, Colliders_ShareOffsetFrictionRestitutionAndSensor) {
    for (const std::string_view name : {"BoxCollider", "CircleCollider", "PolygonCollider"}) {
        const ComponentTypeInfo* collider = physicsTypes().find(name);
        ASSERT_NE(collider, nullptr) << name;
        EXPECT_FALSE(collider->alwaysEnabled) << name; // srs.fr.3.30
        EXPECT_EQ(field(*collider, "offsetX").unit, Unit::Meters) << name;
        EXPECT_EQ(std::get<float>(field(*collider, "friction").defaultValue), 0.6f) << name;
        EXPECT_EQ(field(*collider, "isSensor").type, FieldType::Bool) << name;
    }
}

TEST(PhysicsComponents, PolygonCollider_IsAOneMeterSquareByDefault) {
    const engine::PolygonColliderComponent polygon;
    EXPECT_EQ(polygon.vertexCount, 4);
    EXPECT_EQ(polygon.vertex0, (engine::Vec2{-0.5f, -0.5f}));
    EXPECT_EQ(polygon.vertex2, (engine::Vec2{0.5f, 0.5f}));
    EXPECT_EQ(physicsTypes().find("PolygonCollider")->fields.size(), 14u);
}
