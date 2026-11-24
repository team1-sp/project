// Reading and writing component fields through their description (US-2.2; srs.fr.2.8, 2.9;
// design/ecs-api-design.md §11).
#include <cstdint>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/vec2.hpp"
#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/core_components.hpp"
#include "engine/ecs/field.hpp"

namespace {

using engine::FieldInfo;
using engine::FieldValue;

const FieldInfo& fieldOf(std::string_view type, std::string_view name) {
    for (const FieldInfo& field : ecs_test::types().find(type)->fields) {
        if (field.name == name) {
            return field;
        }
    }
    ADD_FAILURE() << "no field " << name;
    static const FieldInfo none;
    return none;
}

} // namespace

TEST(Field, ReadAndWriteAFloat_GoThroughTheMember) {
    engine::PositionComponent position;
    position.yMeters = 2.5f;
    const FieldInfo& y = fieldOf("Position", "y");

    EXPECT_EQ(std::get<float>(engine::readField(y, &position)), 2.5f);
    EXPECT_FALSE(engine::writeField(y, &position, FieldValue{7.0f}).has_value());

    EXPECT_EQ(position.yMeters, 7.0f);
}

TEST(Field, WrongKindOfValue_IsRefusedAndChangesNothing) {
    engine::PositionComponent position;

    const auto problem =
        engine::writeField(fieldOf("Position", "x"), &position, FieldValue{std::int32_t{3}});

    ASSERT_TRUE(problem.has_value());
    EXPECT_EQ(position.xMeters, 0.0f);
}

TEST(Field, FixedString_RoundTripsAndRefusesTooLongText) {
    ecs_test::MoodComponent mood;
    const FieldInfo& label = fieldOf("Mood", "label");

    EXPECT_FALSE(engine::writeField(label, &mood, FieldValue{std::string("grumpy")}).has_value());
    EXPECT_EQ(mood.label.view(), "grumpy");
    EXPECT_EQ(std::get<std::string>(engine::readField(label, &mood)), "grumpy");

    EXPECT_TRUE(engine::writeField(label, &mood, FieldValue{std::string(17, 'x')}).has_value());
    EXPECT_EQ(mood.label.view(), "grumpy");
}

TEST(Field, Enum_IsReadAsItsValueAndRangeChecked) {
    ecs_test::MoodComponent mood;
    const FieldInfo& field = fieldOf("Mood", "mood");

    EXPECT_FALSE(engine::writeField(field, &mood, FieldValue{std::int64_t{1}}).has_value());
    EXPECT_EQ(mood.mood, ecs_test::MoodKind::Angry);
    EXPECT_EQ(std::get<std::int64_t>(engine::readField(field, &mood)), 1);
    EXPECT_TRUE(engine::writeField(field, &mood, FieldValue{std::int64_t{-1}}).has_value());
    EXPECT_TRUE(engine::writeField(field, &mood, FieldValue{std::int64_t{256}}).has_value());
}

TEST(Field, Layer_IsAnUnsignedByte) {
    engine::EntityInfoComponent info;
    const FieldInfo& layer = fieldOf("EntityInfo", "layer");

    EXPECT_FALSE(engine::writeField(layer, &info, FieldValue{std::uint32_t{31}}).has_value());
    EXPECT_EQ(info.layer, 31);
    EXPECT_TRUE(engine::writeField(layer, &info, FieldValue{std::uint32_t{256}}).has_value());
    EXPECT_EQ(std::get<std::uint32_t>(engine::readField(layer, &info)), 31u);
}

TEST(Field, EntityReference_HoldsAnEntityId) {
    engine::HierarchyComponent links;
    const FieldInfo& parent = fieldOf("Hierarchy", "parent");

    EXPECT_FALSE(
        engine::writeField(parent, &links, FieldValue{ecs_test::editorId(12)}).has_value());

    EXPECT_EQ(links.parent, ecs_test::editorId(12));
    EXPECT_EQ(std::get<engine::EntityId>(engine::readField(parent, &links)),
              ecs_test::editorId(12));
}
