#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "editor/console_model.hpp"
#include "editor/entity_navigation.hpp"
#include "engine/core/entity_id.hpp"
#include "engine/core/logging.hpp"

namespace {

engine::LogEntry entry(engine::LogSeverity severity, std::string message,
                       std::optional<engine::Tick> tick = std::nullopt) {
    engine::LogEntry result;
    result.severity = severity;
    result.message = std::move(message);
    result.tick = tick;
    return result;
}

class FakeExternalCodeEditor final : public editor::IExternalCodeEditor {
public:
    bool open(const engine::SourceLocation& source) override {
        requests.push_back(source);
        return true;
    }

    std::vector<engine::SourceLocation> requests;
};

class FakeEntityNavigator final : public editor::IEntityNavigator {
public:
    void navigateTo(engine::EntityId entity) override { requests.push_back(entity); }

    std::vector<engine::EntityId> requests;
};

} // namespace

TEST(ConsoleModel, AddingSeveritiesAndOptionalTicks_PreservesEntriesAndCounts) {
    editor::EditorStatus status;
    editor::ConsoleModel console(status);

    console.write(entry(engine::LogSeverity::Info, "loaded", 4));
    console.write(entry(engine::LogSeverity::Warning, "missing sprite"));
    console.write(entry(engine::LogSeverity::Error, "invalid entity", 9));

    ASSERT_EQ(console.entries().size(), 3u);
    EXPECT_EQ(console.entries()[0].tick, 4u);
    EXPECT_FALSE(console.entries()[1].tick);
    EXPECT_EQ(console.entries()[2].tick, 9u);
    EXPECT_EQ(console.counts(), (editor::ConsoleCounts{1, 1, 1}));
    EXPECT_EQ(console.tabLabel(), "Console  I 1  W 1  E 1###Console");
    EXPECT_EQ(status.lastConsoleMessage, "invalid entity");
}

TEST(ConsoleModel, Clear_RemovesEntriesResetsCountsSelectionAndStatus) {
    editor::EditorStatus status;
    editor::ConsoleModel console(status);
    console.write(entry(engine::LogSeverity::Warning, "one"));
    console.select(0);

    console.clear();

    EXPECT_TRUE(console.entries().empty());
    EXPECT_EQ(console.counts(), editor::ConsoleCounts{});
    EXPECT_FALSE(console.selectedEntry());
    EXPECT_TRUE(status.lastConsoleMessage.empty());
}

TEST(ConsoleModel, PlayStarted_ClearOnPlayOnClearsAndOffPreservesEntries) {
    editor::EditorStatus status;
    editor::ConsoleModel console(status);
    console.write(entry(engine::LogSeverity::Info, "before first play"));

    console.onPlayStarted();
    EXPECT_TRUE(console.entries().empty());

    console.write(entry(engine::LogSeverity::Info, "before second play"));
    console.setClearOnPlay(false);
    console.onPlayStarted();
    ASSERT_EQ(console.entries().size(), 1u);
    EXPECT_EQ(console.entries().front().message, "before second play");
}

TEST(ConsoleModel, CollapseOn_GroupsOnlyEntriesWhoseEveryStructuredFieldIsIdentical) {
    editor::EditorStatus status;
    editor::ConsoleModel console(status);
    engine::LogEntry base = entry(engine::LogSeverity::Error, "failure", 3);
    base.stackTrace = "stack";
    base.source = engine::SourceLocation{"game.cpp", 17};
    base.entity = engine::EntityId{42};
    console.write(base);
    console.write(base);

    engine::LogEntry differentTick = base;
    differentTick.tick = 4;
    console.write(differentTick);
    engine::LogEntry differentStack = base;
    differentStack.stackTrace = "other stack";
    console.write(differentStack);
    engine::LogEntry differentSource = base;
    differentSource.source = engine::SourceLocation{"other.cpp", 17};
    console.write(differentSource);
    engine::LogEntry differentEntity = base;
    differentEntity.entity = engine::EntityId{43};
    console.write(differentEntity);
    engine::LogEntry differentSeverity = base;
    differentSeverity.severity = engine::LogSeverity::Warning;
    console.write(differentSeverity);

    console.setCollapse(true);
    const std::vector<editor::ConsoleRow> rows = console.visibleRows();

    ASSERT_EQ(rows.size(), 6u);
    EXPECT_EQ(rows.front(), (editor::ConsoleRow{0, 2}));
    for (std::size_t i = 1; i < rows.size(); ++i) {
        EXPECT_EQ(rows[i].count, 1u);
    }
    EXPECT_EQ(console.entries().size(), 7u);
}

TEST(ConsoleModel, SeverityFilters_AreIndependentAndDoNotDeleteEntries) {
    editor::EditorStatus status;
    editor::ConsoleModel console(status);
    console.write(entry(engine::LogSeverity::Info, "info"));
    console.write(entry(engine::LogSeverity::Warning, "warning"));
    console.write(entry(engine::LogSeverity::Error, "error"));

    console.setSeverityVisible(engine::LogSeverity::Warning, false);
    console.setSeverityVisible(engine::LogSeverity::Error, false);
    ASSERT_EQ(console.visibleRows(), (std::vector<editor::ConsoleRow>{{0, 1}}));
    console.setSeverityVisible(engine::LogSeverity::Info, false);
    console.setSeverityVisible(engine::LogSeverity::Error, true);
    EXPECT_EQ(console.visibleRows(), (std::vector<editor::ConsoleRow>{{2, 1}}));
    EXPECT_EQ(console.entries().size(), 3u);
    EXPECT_EQ(console.counts(), (editor::ConsoleCounts{1, 1, 1}));
}

TEST(ConsoleModel, Search_IsCaseInsensitiveSubstringAndEmptySearchShowsAll) {
    editor::EditorStatus status;
    editor::ConsoleModel console(status);
    console.write(entry(engine::LogSeverity::Info, "PlayerMovement started"));
    console.write(entry(engine::LogSeverity::Info, "Physics step"));

    console.setSearch("movement");
    EXPECT_EQ(console.visibleRows(), (std::vector<editor::ConsoleRow>{{0, 1}}));
    console.setSearch("PLAYER");
    EXPECT_EQ(console.visibleRows(), (std::vector<editor::ConsoleRow>{{0, 1}}));
    console.setSearch("");
    EXPECT_EQ(console.visibleRows().size(), 2u);
}

TEST(ConsoleModel, Selection_ExposesCompleteMessageTickAndStackTrace) {
    editor::EditorStatus status;
    editor::ConsoleModel console(status);
    engine::LogEntry full =
        entry(engine::LogSeverity::Error, "complete message that is not truncated", 27);
    full.stackTrace = "first frame\nsecond frame";
    console.write(std::move(full));

    console.select(0);

    ASSERT_NE(console.selectedEntry(), nullptr);
    EXPECT_EQ(console.selectedEntry()->message, "complete message that is not truncated");
    EXPECT_EQ(console.selectedEntry()->tick, 27u);
    EXPECT_EQ(console.selectedEntry()->stackTrace, "first frame\nsecond frame");
}

TEST(ConsoleModel, Activation_RequestsExactSourceAndEntityThroughInjectedInterfaces) {
    editor::EditorStatus status;
    editor::ConsoleModel console(status);
    engine::LogEntry linked = entry(engine::LogSeverity::Error, "entity failed", 8);
    linked.source = engine::SourceLocation{"src/player.cpp", 91};
    linked.entity = engine::EntityId{1234};
    console.write(std::move(linked));
    FakeExternalCodeEditor editor;
    FakeEntityNavigator navigator;

    const editor::ConsoleActivation activated = console.activate(0, editor, navigator);

    EXPECT_TRUE(activated.externalEditorRequested);
    EXPECT_TRUE(activated.entityNavigationRequested);
    EXPECT_EQ(editor.requests, (std::vector<engine::SourceLocation>{{"src/player.cpp", 91}}));
    EXPECT_EQ(navigator.requests, (std::vector<engine::EntityId>{engine::EntityId{1234}}));
}

TEST(ConsoleModel, ErrorPause_OnlyFirstTickedErrorOfEachPlayRunRequestsPause) {
    editor::EditorStatus status;
    int pauses = 0;
    editor::ConsoleModel console(status, [&] { ++pauses; });
    console.setErrorPause(true);
    console.onPlayStarted();

    console.write(entry(engine::LogSeverity::Error, "outside tick"));
    console.write(entry(engine::LogSeverity::Warning, "warning", 1));
    console.write(entry(engine::LogSeverity::Error, "first", 2));
    console.write(entry(engine::LogSeverity::Error, "second", 3));
    EXPECT_EQ(pauses, 1);

    console.onPlayStopped();
    console.write(entry(engine::LogSeverity::Error, "not playing", 4));
    EXPECT_EQ(pauses, 1);
    console.onPlayStarted();
    console.write(entry(engine::LogSeverity::Error, "next run", 0));
    EXPECT_EQ(pauses, 2);
}

TEST(EntityNavigation, NavigateTo_SelectsTheEntityForOtherEditorPanels) {
    editor::EntityNavigation navigation;

    navigation.navigateTo(engine::EntityId{77});

    EXPECT_EQ(navigation.selectedEntity(), engine::EntityId{77});
}
