// Component types and their semantic descriptions (US-2.2; srs.fr.2.12, 2.38, 2.39;
// design/ecs-api-design.md §11).
#include <cstdint>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/run_error.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/field.hpp"
#include "engine/ecs/type_registry.hpp"
#include "engine/ecs/world.hpp"

namespace {

using engine::ComponentTypeInfo;
using engine::FieldInfo;
using engine::FieldType;
using engine::RunError;
using engine::TypeRegistry;
using engine::Unit;

const FieldInfo& fieldNamed(const ComponentTypeInfo& info, std::string_view name) {
    for (const FieldInfo& field : info.fields) {
        if (field.name == name) {
            return field;
        }
    }
    ADD_FAILURE() << "no field " << name;
    static const FieldInfo none;
    return none;
}

struct Undescribed {
    static constexpr std::string_view typeName = "Undescribed";
    std::int32_t value = 0;
};

struct HalfRegistered {
    static constexpr std::string_view typeName = "HalfRegistered";
    std::int32_t first = 0;
    std::int32_t second = 0;
};

struct SameName {
    static constexpr std::string_view typeName = "Health";
    float value = 0.0f;
};

std::string finalizeProblem(TypeRegistry& types) {
    try {
        types.finalize();
    } catch (const RunError& error) {
        return error.what();
    }
    return "";
}

} // namespace

// srs.fr.2.12: meaning, and each field's name, type, unit, default value and description.
TEST(TypeRegistry, CoreComponent_CarriesItsSemanticDescription) {
    const ComponentTypeInfo* position = ecs_test::types().find<engine::PositionComponent>();
    ASSERT_NE(position, nullptr);

    EXPECT_EQ(position->name, "Position");
    EXPECT_FALSE(position->description.empty());
    EXPECT_TRUE(position->alwaysEnabled);
    EXPECT_FALSE(position->builtIn);
    ASSERT_EQ(position->fields.size(), 5u);
    const FieldInfo& x = fieldNamed(*position, "x");
    EXPECT_EQ(x.type, FieldType::Float);
    EXPECT_EQ(x.unit, Unit::Meters);
    EXPECT_EQ(x.offset, 0u);
    EXPECT_EQ(std::get<float>(x.defaultValue), 0.0f);
    EXPECT_FALSE(x.description.empty());
    EXPECT_EQ(std::get<float>(fieldNamed(*position, "scaleX").defaultValue), 1.0f);
    EXPECT_EQ(fieldNamed(*position, "rotation").unit, Unit::Radians);
    EXPECT_STREQ(engine::unitName(Unit::MetersPerSecond), "m/s");
}

TEST(TypeRegistry, BuiltInComponents_AreRegisteredFirstAndHaveNoEnabledFlag) {
    const TypeRegistry& types = ecs_test::types();

    ASSERT_GE(types.components().size(), 3u);
    EXPECT_EQ(types.components()[0].name, "Name");
    EXPECT_EQ(types.components()[1].name, "EntityInfo");
    EXPECT_EQ(types.components()[2].name, "Hierarchy");
    for (std::size_t i = 0; i < 3; ++i) {
        EXPECT_TRUE(types.components()[i].builtIn);
        EXPECT_TRUE(types.components()[i].alwaysEnabled);
    }
    EXPECT_EQ(fieldNamed(types.components()[1], "layer").type, FieldType::UInt8);
    EXPECT_EQ(fieldNamed(types.components()[1], "tag").type, FieldType::Tag);
    EXPECT_EQ(fieldNamed(types.components()[2], "parent").type, FieldType::EntityRef);
}

TEST(TypeRegistry, EnumAndFixedStringFields_KnowTheirEnumAndCapacity) {
    const ComponentTypeInfo* mood = ecs_test::types().find("Mood");
    ASSERT_NE(mood, nullptr);

    EXPECT_EQ(fieldNamed(*mood, "mood").type, FieldType::Enum);
    EXPECT_EQ(fieldNamed(*mood, "mood").enumType, "MoodKind");
    EXPECT_EQ(fieldNamed(*mood, "label").type, FieldType::FixedString);
    EXPECT_EQ(fieldNamed(*mood, "label").count, 16u);
    ASSERT_NE(ecs_test::types().findEnum("MoodKind"), nullptr);
    EXPECT_EQ(ecs_test::types().findEnum("MoodKind")->values.size(), 2u);
}

TEST(TypeRegistry, Find_ByNameAndById) {
    const TypeRegistry& types = ecs_test::types();

    const ComponentTypeInfo* byName = types.find("Health");
    ASSERT_NE(byName, nullptr);
    EXPECT_EQ(types.find(engine::typeIdOf<ecs_test::HealthComponent>()), byName);
    EXPECT_EQ(types.find("Nothing"), nullptr);
    EXPECT_TRUE(types.find("Marker")->isTag);
}

// srs.fr.2.38: a type without a description is refused, and the type is named.
TEST(TypeRegistry, FinalizeWithoutDescription_IsRefusedNamingTheType) {
    TypeRegistry types;
    types.component<Undescribed>().field("value", &Undescribed::value, Unit::None, "A value");

    const std::string problem = finalizeProblem(types);

    EXPECT_NE(problem.find("Undescribed has no description"), std::string::npos) << problem;
    EXPECT_FALSE(types.isFinalized());
}

// srs.fr.2.39: two types with the same name are refused.
TEST(TypeRegistry, FinalizeWithTwoTypesOfOneName_IsRefused) {
    TypeRegistry types;
    types.component<ecs_test::HealthComponent>()
        .describe("Hit points.")
        .field("points", &ecs_test::HealthComponent::points, Unit::None, "Points");
    types.component<SameName>()
        .describe("Another Health.")
        .field("value", &SameName::value, Unit::None, "Value");

    EXPECT_NE(finalizeProblem(types).find("two types are named Health"), std::string::npos);
}

// A data member that is not registered would be lost by scenes and the Inspector.
TEST(TypeRegistry, FinalizeWithAnUnregisteredMember_IsRefused) {
    TypeRegistry types;
    types.component<HalfRegistered>()
        .describe("Only one field registered.")
        .field("first", &HalfRegistered::first, Unit::None, "First");

    EXPECT_NE(finalizeProblem(types).find("2 data members but 1 registered fields"),
              std::string::npos);
}

TEST(TypeRegistry, AfterFinalize_RegistrationIsRefused) {
    TypeRegistry types;
    types.finalize();

    EXPECT_THROW(types.component<ecs_test::HealthComponent>(), RunError);
}

TEST(TypeRegistry, WorldWithAnUnfinalizedRegistry_IsRefused) {
    TypeRegistry types;

    EXPECT_THROW(engine::World{types}, RunError);
}
