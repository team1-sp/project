// The entity table: slots, generations and ID lookups (US-2.1; design/ecs-api-design.md §4).
#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/entity_id.hpp"
#include "engine/ecs/entity.hpp"
#include "engine/ecs/entity_table.hpp"

using ecs_test::editorId;
using ecs_test::runtimeId;
using engine::Entity;

TEST(EntityTable, Destroy_MakesTheHandleStaleAndReusesTheSlotWithANewGeneration) {
    engine::EntityTable table;
    const Entity first = table.create(editorId(1));

    table.destroy(first);
    const Entity second = table.create(editorId(2));

    EXPECT_FALSE(table.isAlive(first));
    EXPECT_TRUE(table.isAlive(second));
    EXPECT_EQ(second.index, first.index);
    EXPECT_EQ(second.generation, first.generation + 1);
    EXPECT_EQ(table.aliveCount(), 1u);
}

TEST(EntityTable, FreedSlots_AreReusedLastFreedFirst) {
    engine::EntityTable table;
    const Entity a = table.create(editorId(1));
    const Entity b = table.create(editorId(2));

    table.destroy(a);
    table.destroy(b);

    EXPECT_EQ(table.create(editorId(3)).index, b.index);
    EXPECT_EQ(table.create(editorId(4)).index, a.index);
}

TEST(EntityTable, Find_GivesTheLiveHandleForEditorAndRuntimeIds) {
    engine::EntityTable table;
    const Entity editor = table.create(editorId(500));
    const engine::EntityId reserved = table.allocateRuntimeId();
    const Entity runtime = table.create(reserved);

    EXPECT_EQ(table.find(editorId(500)), editor);
    EXPECT_EQ(table.find(reserved), runtime);
    EXPECT_EQ(table.idOf(runtime), reserved);
    EXPECT_FALSE(table.find(editorId(501)).has_value());
    EXPECT_FALSE(table.find(runtimeId(1)).has_value());

    table.destroy(editor);
    table.destroy(runtime);

    EXPECT_FALSE(table.find(editorId(500)).has_value());
    EXPECT_FALSE(table.find(reserved).has_value());
}

// srs.fr.2.63, 2.2: runtime IDs increase from 2^32 and a used one is never reserved again.
TEST(EntityTable, RuntimeIds_IncreaseFromTheFirstRuntimeIdAndAreGivenOnce) {
    engine::EntityTable table;

    const engine::EntityId first = table.allocateRuntimeId();
    const engine::EntityId second = table.allocateRuntimeId();

    EXPECT_EQ(first, engine::firstRuntimeEntityId);
    EXPECT_EQ(second, runtimeId(1));
    EXPECT_TRUE(table.isReservedRuntimeId(first));

    const Entity e = table.create(first);
    EXPECT_FALSE(table.isReservedRuntimeId(first));
    table.destroy(e);
    EXPECT_FALSE(table.isReservedRuntimeId(first));
    EXPECT_FALSE(table.isReservedRuntimeId(runtimeId(2))); // not allocated yet
    EXPECT_FALSE(table.isReservedRuntimeId(editorId(5)));
}

TEST(EntityTable, ResetRuntimeIds_StartsAgainAtTheFirstRuntimeId) {
    engine::EntityTable table;
    table.allocateRuntimeId();
    table.allocateRuntimeId();

    table.resetRuntimeIds();

    EXPECT_EQ(table.allocateRuntimeId(), engine::firstRuntimeEntityId);
}

TEST(EntityTable, EnabledFlag_StartsOnAndIsResetForAReusedSlot) {
    engine::EntityTable table;
    const Entity e = table.create(editorId(1));
    EXPECT_TRUE(table.isEnabled(e));

    table.setEnabled(e, false);
    EXPECT_FALSE(table.isEnabled(e));
    table.destroy(e);
    const Entity reused = table.create(editorId(2));

    EXPECT_EQ(reused.index, e.index);
    EXPECT_TRUE(table.isEnabled(reused));
}
