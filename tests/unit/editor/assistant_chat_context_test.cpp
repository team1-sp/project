// US-33.2: typed context chips (srs.fr.33.9, srs.fr.33.10).
#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "assistant_chat_fakes.hpp"
#include "editor/assistant_chat_context.hpp"
#include "editor/assistant_chat_services.hpp"
#include "editor/console_model.hpp"
#include "editor/entity_navigation.hpp"
#include "editor/status_bar.hpp"

namespace {

using namespace chat_test;
using editor::ContextChipSet;
using editor::ContextChipType;

struct ExpectedChip {
    ContextChipType type;
    std::string label;
    std::string detail;
    std::string size;
};

TEST(AssistantContextChips, EveryContextTypeBecomesATypedChipWithItsSize) {
    const std::vector<ExpectedChip> expected = {
        {ContextChipType::Selection, "Selection", "2 entities", "100 B"},
        {ContextChipType::LastRun, "Last run", "last 600 ticks", "200 B"},
        {ContextChipType::Graph, "Graph", "whole graph", "300 B"},
        {ContextChipType::GraphNodes, "Graph nodes", "3 nodes", "400 B"},
        {ContextChipType::SourceCode, "Source code", "Scripts/door.lua", "500 B"},
        {ContextChipType::Asset, "Asset", "Sprites/door.png", "600 B"},
        {ContextChipType::LogEntry, "Log entry", "Door is locked", "700 B"},
        {ContextChipType::ComponentType, "Component type", "Door", "800 B"},
    };
    const FakeSizes sizes;
    ContextChipSet chips;
    for (aa::ContextItem item : oneItemOfEveryType()) {
        chips = chips.with(std::move(item), sizes);
    }

    ASSERT_EQ(chips.chips().size(), expected.size());
    std::set<editor::ContextChipId> ids;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        const editor::ContextChip& chip = chips.chips()[i];
        SCOPED_TRACE(expected[i].label);
        EXPECT_EQ(chip.type, expected[i].type);
        EXPECT_EQ(chip.label, expected[i].label);
        EXPECT_EQ(chip.detail, expected[i].detail);
        EXPECT_EQ(editor::formatRequestSize(chip.requestBytes), expected[i].size);
        EXPECT_EQ(editor::contextChipTypeOf(chip.item), expected[i].type);
        ids.insert(chip.id);
    }
    EXPECT_EQ(ids.size(), expected.size());
    EXPECT_TRUE(chips.unattachedTypes().empty());
}

TEST(AssistantContextChips, RequestSizesAreFormattedForReading) {
    EXPECT_EQ(editor::formatRequestSize(std::nullopt), "\xE2\x80\x94");
    EXPECT_EQ(editor::formatRequestSize(0), "0 B");
    EXPECT_EQ(editor::formatRequestSize(1023), "1023 B");
    EXPECT_EQ(editor::formatRequestSize(1536), "1.5 KB");
    EXPECT_EQ(editor::formatRequestSize(2U * 1024U * 1024U), "2.0 MB");
}

TEST(AssistantContextChips, DetailsDescribeRangesFocusAndCounts) {
    EXPECT_EQ(editor::contextChipDetail(aa::LastRunContext{aa::TickRange{10, 20}}), "ticks 10-20");
    EXPECT_EQ(editor::contextChipDetail(aa::GraphContext{{engine::EntityId{1}}}),
              "around 1 entity");
    EXPECT_EQ(editor::contextChipDetail(aa::SourceCodeContext{{"Scripts/a.lua", "Scripts/b.lua"}}),
              "2 files");
}

TEST(AssistantContextChips, RemovingAChipKeepsTheOthersAndTheirIds) {
    const FakeSizes sizes;
    const ContextChipSet chips =
        ContextChipSet{}.with(aa::GraphContext{}, sizes).with(aa::AssetContext{"a.png"}, sizes);
    const editor::ContextChipId graphId = chips.chips()[0].id;
    const editor::ContextChipId assetId = chips.chips()[1].id;

    const ContextChipSet remaining = chips.without(graphId);

    EXPECT_EQ(chips.chips().size(), 2U) << "the original set is unchanged";
    ASSERT_EQ(remaining.chips().size(), 1U);
    EXPECT_EQ(remaining.chips()[0].id, assetId);
    EXPECT_EQ(remaining.items().size(), 1U);
    EXPECT_EQ(remaining.without(999).chips().size(), 1U);
}

TEST(AssistantContextChips, ReattachingATypeReplacesItsChipWithANewId) {
    const FakeSizes sizes;
    const ContextChipSet first = ContextChipSet{}.with(aa::AssetContext{"a.png"}, sizes);

    const ContextChipSet second = first.with(aa::AssetContext{"b.png"}, sizes);

    ASSERT_EQ(second.chips().size(), 1U);
    EXPECT_EQ(second.chips()[0].detail, "b.png");
    EXPECT_NE(second.chips()[0].id, first.chips()[0].id);
}

TEST(AssistantContextChips, UnattachedTypesExcludeAttachedOnes) {
    const FakeSizes sizes;
    const ContextChipSet chips = ContextChipSet{}
                                     .with(aa::SelectionContext{}, sizes)
                                     .with(aa::ComponentTypeContext{"Door"}, sizes);

    const std::vector<ContextChipType> unattached = chips.unattachedTypes();

    EXPECT_EQ(unattached.size(), editor::contextChipTypeCount - 2);
    for (const ContextChipType type : unattached) {
        EXPECT_NE(type, ContextChipType::Selection);
        EXPECT_NE(type, ContextChipType::ComponentType);
    }
}

TEST(AssistantContextChips, EntryPointContextsConvertToUs111Items) {
    const auto selection = editor::toContextItem(
        editor::SelectionContext{editor::EntitySelection{engine::EntityId{5}}});
    ASSERT_TRUE(selection.has_value());
    EXPECT_EQ(std::get<aa::SelectionContext>(*selection).entities,
              std::vector<engine::EntityId>{engine::EntityId{5}});

    const auto lastRun = editor::toContextItem(
        editor::LastRunContext{editor::LatestRun{3, 900}, editor::TickRange{301, 900}});
    ASSERT_TRUE(lastRun.has_value());
    EXPECT_EQ(std::get<aa::LastRunContext>(*lastRun).ticks, (aa::TickRange{301, 900}));

    EXPECT_FALSE(editor::toContextItem(editor::SelectionContext{editor::FindingSelection{"f1"}})
                     .has_value());
}

} // namespace

namespace {

class OneRun final : public editor::ILatestRunSource {
public:
    [[nodiscard]] std::optional<editor::LatestRun> latestRun() const override { return run; }
    std::optional<editor::LatestRun> run;
};

TEST(AssistantContextProvider, BuildsItemsFromTheEditorsSelectionRunAndConsole) {
    editor::EntityNavigation selection;
    OneRun runs;
    editor::EditorStatus status;
    editor::ConsoleModel console(status);
    const editor::EditorContextItemProvider provider(selection, runs, console);

    EXPECT_FALSE(provider.itemFor(ContextChipType::Selection).has_value());
    EXPECT_FALSE(provider.itemFor(ContextChipType::LastRun).has_value());
    EXPECT_FALSE(provider.itemFor(ContextChipType::LogEntry).has_value());
    EXPECT_TRUE(
        std::get<aa::GraphContext>(*provider.itemFor(ContextChipType::Graph)).focus.empty());
    for (const ContextChipType type : {ContextChipType::GraphNodes, ContextChipType::SourceCode,
                                       ContextChipType::Asset, ContextChipType::ComponentType}) {
        EXPECT_FALSE(provider.itemFor(type).has_value());
    }

    selection.navigateTo(engine::EntityId{4});
    runs.run = editor::LatestRun{2, 120};
    console.write({engine::LogSeverity::Error, "Door is locked", 41, std::nullopt, std::nullopt,
                   std::nullopt});
    console.select(0);

    EXPECT_EQ(
        std::get<aa::SelectionContext>(*provider.itemFor(ContextChipType::Selection)).entities,
        std::vector<engine::EntityId>{engine::EntityId{4}});
    EXPECT_EQ(std::get<aa::GraphContext>(*provider.itemFor(ContextChipType::Graph)).focus,
              std::vector<engine::EntityId>{engine::EntityId{4}});
    EXPECT_TRUE(provider.itemFor(ContextChipType::LastRun).has_value());
    const auto logEntry =
        std::get<aa::LogEntryContext>(*provider.itemFor(ContextChipType::LogEntry));
    EXPECT_EQ(logEntry.entry.message, "Door is locked");
    EXPECT_EQ(logEntry.entry.tick, std::optional<engine::Tick>(41));
    EXPECT_TRUE(logEntry.entry.loggedDuringRun);
}

} // namespace
