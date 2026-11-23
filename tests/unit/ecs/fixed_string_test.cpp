// Inline strings for unmanaged components (US-2.1; the Name built-in component).
#include <type_traits>

#include <gtest/gtest.h>

#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/fixed_string.hpp"

TEST(FixedString, Assign_KeepsTextUpToTheCapacity) {
    engine::FixedString<8> text;

    EXPECT_TRUE(text.assign("12345678"));

    EXPECT_EQ(text.view(), "12345678");
    EXPECT_EQ(text.size(), 8u);
}

TEST(FixedString, AssignTooLong_IsRefusedAndKeepsTheOldText) {
    engine::FixedString<4> text;
    ASSERT_TRUE(text.assign("door"));

    EXPECT_FALSE(text.assign("doors"));

    EXPECT_EQ(text.view(), "door");
}

TEST(FixedString, ShorterText_LeavesNoOldBytesBehind) {
    engine::FixedString<8> reused;
    ASSERT_TRUE(reused.assign("longname"));
    ASSERT_TRUE(reused.assign("key"));
    engine::FixedString<8> fresh;
    ASSERT_TRUE(fresh.assign("key"));

    EXPECT_EQ(reused, fresh);
}

// D-14: the built-in components are unmanaged, so their pools can be copied as raw memory.
TEST(BuiltInComponents, AreTriviallyCopyableAggregates) {
    EXPECT_TRUE(std::is_trivially_copyable_v<engine::NameComponent>);
    EXPECT_TRUE(std::is_trivially_copyable_v<engine::EntityInfoComponent>);
    EXPECT_TRUE(std::is_trivially_copyable_v<engine::HierarchyComponent>);
    EXPECT_TRUE(std::is_aggregate_v<engine::NameComponent>);
    EXPECT_TRUE(std::is_aggregate_v<engine::EntityInfoComponent>);
    EXPECT_TRUE(std::is_aggregate_v<engine::HierarchyComponent>);
}
