#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "aegis/assistant/grounded/content_hash.hpp"
#include "aegis/assistant/grounded/context_assembly.hpp"
#include "demo_run.hpp"
#include "grounded_fakes.hpp"

namespace {

using namespace aegis::assistant;
using grounded_test::FakePolicy;
using grounded_test::Sources;

class ContextAssemblyTest : public ::testing::Test {
protected:
    AssemblyResult assemble(GroundedQuestion question) {
        return assembleContext(question, chat, sources.view(), policy);
    }

    Sources sources;
    FakePolicy policy;
    Chat chat{chatFormatVersion, "chat", {}};
};

TEST_F(ContextAssemblyTest, QuestionOnly_ReadsNoProjectSource) {
    const AssemblyResult result = assemble({"Why did the unlock fail?", {}});
    ASSERT_TRUE(result.succeeded()) << result.error;
    EXPECT_EQ(result.context->question, "Why did the unlock fail?");
    EXPECT_TRUE(result.context->dataCategories.empty());
    EXPECT_EQ(sources.graph.calls, 0);
    EXPECT_EQ(sources.telemetry.calls, 0);
    EXPECT_EQ(sources.assets.calls, 0);
    EXPECT_TRUE(sources.scripts.readPaths.empty());
}

TEST_F(ContextAssemblyTest, EmptyQuestion_IsRejected) {
    EXPECT_FALSE(assemble({"", {}}).succeeded());
}

TEST_F(ContextAssemblyTest, SourceCode_ReadsOnlyListedFilesWithHashes) {
    sources.scripts.files = {{"Scripts/door.cpp", "line one\nline two\n"},
                             {"Scripts/secret.cpp", "do not read"}};
    const AssemblyResult result = assemble({"q", {SourceCodeContext{{"Scripts/door.cpp"}}}});
    ASSERT_TRUE(result.succeeded()) << result.error;
    EXPECT_EQ(sources.scripts.readPaths, std::vector<std::string>{"Scripts/door.cpp"});
    ASSERT_EQ(result.context->evidence.sources.size(), 1U);
    const AttachedSourceFile& file = result.context->evidence.sources[0];
    EXPECT_EQ(file.contentHash, contentHash("line one\nline two\n"));
    EXPECT_EQ(file.lines, (std::vector<std::string>{"line one", "line two"}));
    EXPECT_EQ(result.context->dataCategories, std::vector<std::string>{"source-code"});
}

TEST_F(ContextAssemblyTest, SourceCode_PathsOutsideScriptsAreRefusedBeforeReading) {
    for (const std::string path :
         {"Assets/door.cpp", "Scripts/../ProjectSettings/secrets.txt", "/etc/passwd",
          "Scripts\\..\\x.cpp", "C:/Scripts/x.cpp", "Scripts/", "Scripts//x.cpp", "Scripts/./x.cpp",
          "scripts/x.cpp"}) {
        const AssemblyResult result = assemble({"q", {SourceCodeContext{{path}}}});
        EXPECT_FALSE(result.succeeded()) << path;
        EXPECT_NE(result.error.find("Source code"), std::string::npos) << result.error;
    }
    EXPECT_TRUE(sources.scripts.readPaths.empty());
}

TEST_F(ContextAssemblyTest, SourceCode_DeniedCategoryIsRefusedWithoutReading) {
    sources.scripts.files = {{"Scripts/door.cpp", "x"}};
    policy.denied = {"source-code"};
    const AssemblyResult result = assemble({"q", {SourceCodeContext{{"Scripts/door.cpp"}}}});
    EXPECT_FALSE(result.succeeded());
    EXPECT_NE(result.error.find("source-code"), std::string::npos);
    EXPECT_TRUE(sources.scripts.readPaths.empty());
}

TEST_F(ContextAssemblyTest, SourceCode_UnreadableOrOversizedFilesAreRefused) {
    EXPECT_FALSE(assemble({"q", {SourceCodeContext{{"Scripts/missing.cpp"}}}}).succeeded());
    sources.scripts.files = {{"Scripts/big.cpp", std::string(maximumSourceBytes + 1U, 'x')}};
    EXPECT_FALSE(assemble({"q", {SourceCodeContext{{"Scripts/big.cpp"}}}}).succeeded());
}

TEST_F(ContextAssemblyTest, LastRun_DeniedTelemetryIsRefusedWithoutQuerying) {
    sources.telemetry.run = grounded_test::baseRun();
    policy.denied = {"telemetry"};
    const AssemblyResult result = assemble({"q", {LastRunContext{}}});
    EXPECT_FALSE(result.succeeded());
    EXPECT_EQ(sources.telemetry.calls, 0);
}

TEST_F(ContextAssemblyTest, LastRun_DefaultRangeIsLast600TicksAndKeepsStartRecords) {
    RunEvidence run = grounded_test::baseRun();
    run.lastCompletedTick = 1000;
    run.events = {{100, 10, "Old", {}, {}}, {401, 11, "Recent", {}, {}}};
    run.writes = {{StatePoint::start(), 5, "Injection", engine::EntityId{3}, "Collider.count", "0"},
                  {StatePoint::atTick(399), 12, "Sys", engine::EntityId{1}, "A.b", "1"}};
    sources.telemetry.run = run;

    const AssemblyResult result = assemble({"q", {LastRunContext{}}});
    ASSERT_TRUE(result.succeeded()) << result.error;
    const RunEvidence& limited = *result.context->evidence.run;
    EXPECT_EQ(limited.includedTicks, (TickRange{401, 1000}));
    ASSERT_EQ(limited.events.size(), 1U);
    EXPECT_EQ(limited.events[0].type, "Recent");
    ASSERT_EQ(limited.writes.size(), 1U);
    EXPECT_TRUE(limited.writes[0].at.isStart());
    EXPECT_EQ(limited.creations.size(), 4U);
    EXPECT_EQ(result.context->dataCategories, std::vector<std::string>{"telemetry"});
}

TEST_F(ContextAssemblyTest, LastRun_ExplicitRangeLimitsTickedRecords) {
    RunEvidence run = grounded_test::baseRun();
    run.events = {{5, 1, "A", {}, {}}, {50, 2, "B", {}, {}}};
    const RunEvidence limited = limitRunToTicks(run, TickRange{0, 10});
    ASSERT_EQ(limited.events.size(), 1U);
    EXPECT_EQ(limited.events[0].type, "A");
}

TEST_F(ContextAssemblyTest, LastRun_NoRunIsReportedUnavailable) {
    const AssemblyResult result = assemble({"q", {LastRunContext{}}});
    ASSERT_TRUE(result.succeeded());
    EXPECT_FALSE(result.context->evidence.run.has_value());
    EXPECT_EQ(result.context->unavailable, std::vector<std::string>{"no recorded run exists"});
}

TEST_F(ContextAssemblyTest, Graph_UsesTwoRelationsAnd500NodesAndClassifiesCategories) {
    sources.graph.slice.nodes = {
        {"e1", GraphNodeKind::Entity, "Player", grounded_test::playerFacts(), {}},
        {"ev", GraphNodeKind::Event, "UnlockFailedEvent", std::nullopt, {}},
        {"s1", GraphNodeKind::System, "DoorSystem", std::nullopt, "Scripts/door.cpp"}};
    const AssemblyResult result = assemble({"q", {GraphContext{{engine::EntityId{1}}}}});
    ASSERT_TRUE(result.succeeded()) << result.error;
    EXPECT_EQ(sources.graph.lastMaxRelations, 2U);
    EXPECT_EQ(sources.graph.lastMaxNodes, 500U);
    EXPECT_EQ(result.context->dataCategories,
              (std::vector<std::string>{"semantic-graph", "telemetry", "source-code"}));

    policy.denied = {"telemetry"};
    EXPECT_FALSE(assemble({"q", {GraphContext{}}}).succeeded());
    policy.denied = {"source-code"};
    EXPECT_FALSE(assemble({"q", {GraphNodesContext{{"s1"}}}}).succeeded());
    EXPECT_TRUE(assemble({"q", {GraphNodesContext{{"e1"}}}}).succeeded());
}

TEST_F(ContextAssemblyTest, SelectionComponentTypeAssetAndLogEntry_RetrieveOnlyTheirContent) {
    sources.graph.entities = {grounded_test::doorFacts()};
    sources.graph.types = {{"Unlockable", "Can be unlocked", {"unlocked"}, {}, {"DoorSystem"}}};
    sources.assets.assets = {{"door.png", "Assets/door.png", "texture", 1024, {}, false},
                             {"door.cpp", "Scripts/door.cpp", "script", 10, {}, true}};
    const AssemblyResult result =
        assemble({"q",
                  {SelectionContext{{engine::EntityId{2}, engine::EntityId{99}}},
                   ComponentTypeContext{"Unlockable"}, AssetContext{"Assets/door.png"},
                   LogEntryContext{{"Error", "Door stuck", 41, {}, true}}}});
    ASSERT_TRUE(result.succeeded()) << result.error;
    EXPECT_EQ(result.context->evidence.entities,
              std::vector<EntityFacts>{grounded_test::doorFacts()});
    EXPECT_EQ(result.context->evidence.componentTypes.size(), 1U);
    EXPECT_EQ(result.context->evidence.assets.size(), 1U);
    EXPECT_EQ(result.context->evidence.logEntries.size(), 1U);
    EXPECT_EQ(result.context->unavailable,
              std::vector<std::string>{"entity 99 is not in the Semantic Graph"});
    EXPECT_TRUE(sources.scripts.readPaths.empty());

    policy.denied = {"source-code"};
    EXPECT_FALSE(assemble({"q", {AssetContext{"Scripts/door.cpp"}}}).succeeded());
    EXPECT_FALSE(
        assemble({"q", {LogEntryContext{{"Error", "x", std::nullopt, "door.cpp:3", false}}}})
            .succeeded());
}

TEST_F(ContextAssemblyTest, History_LeavesOutMessagesWithNowDeniedCategories) {
    chat.messages = {
        {MessageRole::Question,
         MessageState::Complete,
         MessageContent::Conversation,
         "What does the door do?",
         {},
         {"question", "semantic-graph"}},
        {MessageRole::Answer,
         MessageState::Complete,
         MessageContent::Conversation,
         "It unlocks.",
         {},
         {"question", "semantic-graph"}},
        {MessageRole::Question,
         MessageState::Complete,
         MessageContent::Conversation,
         "Why did run 7 fail?",
         {},
         {"question", "telemetry"}},
        {MessageRole::Answer,
         MessageState::Complete,
         MessageContent::Conversation,
         "Tick 41 shows ...",
         {},
         {"question", "telemetry"}},
    };
    policy.denied = {"telemetry"};
    const AssemblyResult result = assemble({"And now?", {}});
    ASSERT_TRUE(result.succeeded()) << result.error;
    const std::vector<HistoryMessage> expected = {
        {MessageRole::Question, "What does the door do?", false},
        {MessageRole::Answer, "It unlocks.", false},
        {MessageRole::Question, "[earlier message left out: contains telemetry]", true},
        {MessageRole::Answer, "[earlier message left out: contains telemetry]", true},
    };
    EXPECT_EQ(result.context->history, expected);
    // Categories pass on only from included messages (srs.fr.33.59).
    EXPECT_EQ(result.context->dataCategories, std::vector<std::string>{"semantic-graph"});
}

TEST_F(ContextAssemblyTest, History_AllowedMessagesAreKeptWholeAndCarryTheirCategories) {
    chat.messages = {{MessageRole::Question,
                      MessageState::Complete,
                      MessageContent::Conversation,
                      "Why did run 7 fail?",
                      {},
                      {"question", "telemetry"}}};
    const AssemblyResult result = assemble({"Thanks", {}});
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.context->history.size(), 1U);
    EXPECT_FALSE(result.context->history[0].leftOut);
    EXPECT_EQ(result.context->dataCategories, std::vector<std::string>{"telemetry"});
}

} // namespace
