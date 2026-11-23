// The sorted sparse-set pool (US-2.1; design/ecs-api-design.md §6, D-1, D-3).
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "ecs_test_helpers.hpp"
#include "engine/core/entity_id.hpp"
#include "engine/ecs/component_pool.hpp"
#include "engine/ecs/entity.hpp"

namespace {

using ecs_test::editorId;
using engine::Entity;
using engine::EntityId;

struct ValueComponent {
    static constexpr std::string_view typeName = "Value";
    int value = 0;
};

std::vector<EntityId> idsOf(const engine::ComponentPool<ValueComponent>& pool) {
    std::vector<EntityId> ids;
    for (std::size_t pos = 0; pos < pool.size(); ++pos) {
        ids.push_back(pool.idAt(pos));
    }
    return ids;
}

} // namespace

TEST(ComponentPool, Add_IsFoundByItsHandleOnly) {
    engine::ComponentPool<ValueComponent> pool;
    pool.add(Entity{3, 0}, editorId(7), ValueComponent{42});

    ASSERT_NE(pool.tryGet(Entity{3, 0}), nullptr);
    EXPECT_EQ(pool.tryGet(Entity{3, 0})->value, 42);
    EXPECT_TRUE(pool.contains(Entity{3, 0}));
    EXPECT_FALSE(pool.contains(Entity{3, 1})); // same slot, later generation: a stale match
    EXPECT_FALSE(pool.contains(Entity{4, 0}));
    EXPECT_EQ(pool.typeName(), "Value");
}

TEST(ComponentPool, AddOutOfIdOrder_IsSortedByEntityIdAfterSort) {
    engine::ComponentPool<ValueComponent> pool;
    pool.add(Entity{0, 0}, editorId(30), ValueComponent{3});
    pool.add(Entity{1, 0}, editorId(10), ValueComponent{1});
    pool.add(Entity{2, 0}, editorId(20), ValueComponent{2});

    pool.sortIfDirty();

    EXPECT_EQ(idsOf(pool), (std::vector<EntityId>{editorId(10), editorId(20), editorId(30)}));
    // The sparse array follows the moved entries.
    EXPECT_EQ(pool.tryGet(Entity{0, 0})->value, 3);
    EXPECT_EQ(pool.tryGet(Entity{1, 0})->value, 1);
    EXPECT_EQ(pool.tryGet(Entity{2, 0})->value, 2);
}

TEST(ComponentPool, Remove_SwapsTheLastEntryInAndSortsBack) {
    engine::ComponentPool<ValueComponent> pool;
    for (std::uint32_t i = 0; i < 5; ++i) {
        pool.add(Entity{i, 0}, editorId(i + 1), ValueComponent{static_cast<int>(i)});
    }

    EXPECT_TRUE(pool.removeIfPresent(Entity{1, 0}));
    EXPECT_FALSE(pool.removeIfPresent(Entity{1, 0}));
    pool.sortIfDirty();

    EXPECT_EQ(idsOf(pool),
              (std::vector<EntityId>{editorId(1), editorId(3), editorId(4), editorId(5)}));
    EXPECT_FALSE(pool.contains(Entity{1, 0}));
    EXPECT_EQ(pool.tryGet(Entity{4, 0})->value, 4);
    EXPECT_EQ(pool.tryGet(Entity{2, 0})->value, 2);
}

TEST(ComponentPool, LongPermutation_EndsSortedWithEveryValueOnItsEntity) {
    engine::ComponentPool<ValueComponent> pool;
    const std::vector<std::uint64_t> ids = {9, 2, 7, 1, 8, 3, 6, 4, 5};
    for (std::uint32_t i = 0; i < ids.size(); ++i) {
        pool.add(Entity{i, 0}, editorId(ids[i]), ValueComponent{static_cast<int>(ids[i])});
    }

    pool.sortIfDirty();

    for (std::size_t pos = 0; pos + 1 < pool.size(); ++pos) {
        EXPECT_LT(pool.idAt(pos), pool.idAt(pos + 1));
    }
    for (std::uint32_t i = 0; i < ids.size(); ++i) {
        EXPECT_EQ(pool.tryGet(Entity{i, 0})->value, static_cast<int>(ids[i]));
    }
}
