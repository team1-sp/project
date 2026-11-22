#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "aegis/assistant/grounded/answer_format.hpp"
#include "aegis/assistant/grounded/content_hash.hpp"
#include "aegis/assistant/grounded/grounded_assistant.hpp"
#include "aegis/assistant/grounded/scripts_folder_reader.hpp"
#include "demo_run.hpp"
#include "grounded_fakes.hpp"

namespace {

using namespace aegis::assistant;
using namespace std::chrono_literals;
using engine::bridge::AssistantChunk;
using engine::bridge::AssistantCitation;
using grounded_test::demoRunNumber;

AssistantCitation bridgeCitation(const GroundedCitation& citation) {
    const Citation encoded = encodeCitation(citation);
    return {encoded.kind, encoded.payload};
}

std::vector<GroundedCitation> decodedCitations(const ChatMessage& message) {
    std::vector<GroundedCitation> result;
    for (const Citation& citation : message.citations) {
        const DecodedCitation decoded = decodeCitation(citation);
        EXPECT_TRUE(decoded.succeeded()) << citation.kind << ": " << decoded.error;
        if (decoded.succeeded()) {
            result.push_back(*decoded.citation);
        }
    }
    return result;
}

class GroundedAssistantTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove_all(root);
        RunEvidence run = grounded_test::baseRun();
        run.events = {{41,
                       30,
                       "UnlockFailedEvent",
                       {engine::EntityId{2}, engine::EntityId{1}},
                       {{"requiredItem", "Key"}, {"inventory", "Coin"}}}};
        sources.telemetry.run = run;
        sources.scripts.files = {{"Scripts/door.cpp", "bool unlocked = false;\nvoid open();\n"},
                                 {"Scripts/other.cpp", "secret"}};
    }

    void TearDown() override { std::filesystem::remove_all(root); }

    GroundedAssistant makeAssistant(engine::bridge::AgentBridge* configured) {
        return {chat, store, configured, sources.view(), policy, "local-model", 5s};
    }

    static std::filesystem::path testRoot() {
        return std::filesystem::temp_directory_path() /
               ("aegis-us111-" +
                std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
    }

    // Declared before the store so the store sees the initialised path.
    std::filesystem::path root = testRoot();
    grounded_test::Sources sources;
    grounded_test::FakePolicy policy;
    grounded_test::FakeBridge bridge;
    Chat chat{chatFormatVersion, "grounded", {}};
    grounded_test::FixedUserProjectPaths rootPaths{root};
    ChatStore store{rootPaths};
};

TEST_F(GroundedAssistantTest, Ask_SendsGroundedContextThroughAgentBridgeAndPersistsTypedCitations) {
    const StateCitation tick41{demoRunNumber, StatePoint::atTick(41)};
    const SourceCitation line2{"Scripts/door.cpp", 2,
                               contentHash("bool unlocked = false;\nvoid open();\n")};
    bridge.control->chunks = {
        {"The door stayed locked because the player had no key.",
         {bridgeCitation(tick41)},
         {"question", "telemetry"}},
        {" See open().", {bridgeCitation(line2)}, {}},
    };
    GroundedAssistant assistant = makeAssistant(&bridge);
    StartAnswerResult started = assistant.ask(
        {"Why did the unlock fail?", {LastRunContext{}, SourceCodeContext{{"Scripts/door.cpp"}}}});
    ASSERT_TRUE(started.succeeded()) << started.error;
    EXPECT_EQ(started.operation->run().state, MessageState::Complete);

    ASSERT_EQ(bridge.requests.size(), 1U);
    const engine::bridge::AssistantRequest& request = bridge.requests[0];
    EXPECT_EQ(request.question, "Why did the unlock fail?");
    EXPECT_EQ(request.modelId, "local-model");
    EXPECT_EQ(request.semanticGraphStructure.rfind(std::string(groundedContextMagic), 0), 0U);
    EXPECT_NE(request.semanticGraphStructure.find("tick 41 seq=30 event UnlockFailedEvent"),
              std::string::npos);
    EXPECT_NE(request.semanticGraphStructure.find("SOURCE Scripts/door.cpp"), std::string::npos);
    EXPECT_EQ(request.semanticGraphStructure.find("secret"), std::string::npos);
    EXPECT_EQ(request.dataCategories,
              (std::vector<std::string>{"telemetry", "source-code", "question"}));
    EXPECT_EQ(sources.scripts.readPaths, std::vector<std::string>{"Scripts/door.cpp"});

    ASSERT_EQ(chat.messages.size(), 2U);
    const ChatMessage& answer = chat.messages[1];
    EXPECT_EQ(answer.text.rfind("Findings from the attached evidence:", 0), 0U) << answer.text;
    EXPECT_NE(answer.text.find("the required item \"Key\" was not in the inventory [Coin]"),
              std::string::npos);
    EXPECT_NE(answer.text.find("The door stayed locked"), std::string::npos);
    const std::vector<GroundedCitation> citations = decodedCitations(answer);
    EXPECT_NE(std::find(citations.begin(), citations.end(), GroundedCitation{tick41}),
              citations.end());
    EXPECT_NE(std::find(citations.begin(), citations.end(), GroundedCitation{line2}),
              citations.end());

    // The persisted chat reloads with the same typed citations.
    const LoadResult loaded = store.load("grounded");
    ASSERT_TRUE(loaded.succeeded()) << loaded.error;
    EXPECT_EQ(decodedCitations(loaded.chat->messages[1]), citations);
}

TEST_F(GroundedAssistantTest, Ask_RemovesModelCitationsNotBackedByEvidence) {
    bridge.control->chunks = {
        {"Claims.",
         {bridgeCitation(StateCitation{demoRunNumber, StatePoint::atTick(99)}),
          bridgeCitation(SourceCitation{"Scripts/other.cpp", 1, "sha256:00"}),
          bridgeCitation(
              EntityCitation{{engine::EntityId{2}, "Gate", "Level1", false}, std::nullopt}),
          {"state", "run 7 tick 41"},
          {"guess", "anything"},
          bridgeCitation(StateCitation{demoRunNumber, StatePoint::start()})},
         {"source-code"}}};
    GroundedAssistant assistant = makeAssistant(&bridge);
    StartAnswerResult started = assistant.ask({"Why?", {LastRunContext{}}});
    ASSERT_TRUE(started.succeeded()) << started.error;
    ASSERT_EQ(started.operation->run().state, MessageState::Complete);

    const ChatMessage& answer = chat.messages.at(1);
    const std::vector<GroundedCitation> citations = decodedCitations(answer);
    const PrepareResult prepared = assistant.prepare({"Why?", {LastRunContext{}}});
    ASSERT_TRUE(prepared.succeeded());
    for (const GroundedCitation& citation : citations) {
        EXPECT_TRUE(checkCitation(prepared.turn->context.evidence, citation).supported)
            << describeCitation(citation);
    }
    EXPECT_NE(std::find(citations.begin(), citations.end(),
                        GroundedCitation{StateCitation{demoRunNumber, StatePoint::start()}}),
              citations.end());
    EXPECT_NE(answer.text.find("Note: 5 citation(s) from the AI service did not match"),
              std::string::npos)
        << answer.text;
    // The model cannot add a data category the request did not carry.
    EXPECT_EQ(std::find(answer.dataCategories.begin(), answer.dataCategories.end(), "source-code"),
              answer.dataCategories.end());
}

TEST_F(GroundedAssistantTest, Ask_WithoutEvidence_StreamsAbstentionBeforeModelText) {
    bridge.control->chunks = {{"Hypothesis: the key was never picked up.", {}, {}}};
    GroundedAssistant assistant = makeAssistant(&bridge);
    StartAnswerResult started = assistant.ask({"Why did the unlock fail?", {}});
    ASSERT_TRUE(started.succeeded()) << started.error;
    ASSERT_EQ(started.operation->run().state, MessageState::Complete);
    const ChatMessage& answer = chat.messages.at(1);
    EXPECT_EQ(answer.text.rfind("The attached context does not contain the information needed", 0),
              0U);
    EXPECT_NE(answer.text.find("Information not available:\n- Semantic Graph or telemetry data"),
              std::string::npos);
    EXPECT_TRUE(answer.citations.empty());
    EXPECT_EQ(sources.telemetry.calls, 0);
    EXPECT_EQ(sources.graph.calls, 0);
}

TEST_F(GroundedAssistantTest, Ask_UnauthorizedContextSendsNothingAndLeavesChatUnchanged) {
    GroundedAssistant assistant = makeAssistant(&bridge);
    StartAnswerResult outside = assistant.ask({"q", {SourceCodeContext{{"../secrets.env"}}}});
    EXPECT_FALSE(outside.succeeded());
    policy.denied = {"telemetry"};
    StartAnswerResult denied = assistant.ask({"q", {LastRunContext{}}});
    EXPECT_FALSE(denied.succeeded());
    EXPECT_NE(denied.error.find("telemetry"), std::string::npos);
    EXPECT_TRUE(bridge.requests.empty());
    EXPECT_TRUE(chat.messages.empty());
    EXPECT_FALSE(std::filesystem::exists(store.chatFilePath("grounded")));
}

TEST_F(GroundedAssistantTest, Ask_EarlierMessagesFollowConsentRules) {
    chat.messages = {
        {MessageRole::Question,
         MessageState::Complete,
         MessageContent::Conversation,
         "Earlier run question",
         {},
         {"question", "telemetry"}},
        {MessageRole::Answer,
         MessageState::Complete,
         MessageContent::Conversation,
         "Earlier run answer",
         {},
         {"question", "telemetry"}},
        {MessageRole::Question,
         MessageState::Complete,
         MessageContent::Conversation,
         "Plain question",
         {},
         {"question"}},
    };
    policy.denied = {"telemetry"};
    GroundedAssistant assistant = makeAssistant(&bridge);
    StartAnswerResult started = assistant.ask({"Follow-up", {}});
    ASSERT_TRUE(started.succeeded()) << started.error;
    const std::string& document = bridge.requests.at(0).semanticGraphStructure;
    EXPECT_EQ(document.find("Earlier run question"), std::string::npos);
    EXPECT_EQ(document.find("Earlier run answer"), std::string::npos);
    EXPECT_NE(document.find("user: [earlier message left out: contains telemetry]"),
              std::string::npos);
    EXPECT_NE(document.find("user: Plain question"), std::string::npos);
    EXPECT_EQ(bridge.requests[0].dataCategories, std::vector<std::string>{"question"});
}

TEST_F(GroundedAssistantTest, Ask_NoConfiguredServiceReadsNothing) {
    GroundedAssistant assistant = makeAssistant(nullptr);
    const StartAnswerResult started = assistant.ask({"q", {LastRunContext{}}});
    EXPECT_FALSE(started.succeeded());
    EXPECT_EQ(sources.telemetry.calls, 0);
    EXPECT_TRUE(chat.messages.empty());
}

TEST_F(GroundedAssistantTest, Cancel_BeforeRun_StopsGroundedAndConfiguredStreams) {
    bridge.control->chunks = {{"partial", {}, {}}};
    GroundedAssistant assistant = makeAssistant(&bridge);
    StartAnswerResult started = assistant.ask({"q", {LastRunContext{}}});
    ASSERT_TRUE(started.succeeded());
    started.operation->cancel();
    EXPECT_EQ(started.operation->run().state, MessageState::Stopped);
    EXPECT_TRUE(bridge.control->cancelled.load());
    EXPECT_EQ(chat.messages.at(1).state, MessageState::Stopped);
}

TEST_F(GroundedAssistantTest, Prepare_HasNoSideEffectsOnChatOrBridge) {
    GroundedAssistant assistant = makeAssistant(&bridge);
    const PrepareResult prepared = assistant.prepare({"q", {LastRunContext{}}});
    ASSERT_TRUE(prepared.succeeded()) << prepared.error;
    EXPECT_TRUE(bridge.requests.empty());
    EXPECT_TRUE(chat.messages.empty());
    EXPECT_FALSE(std::filesystem::exists(root));
}

TEST_F(GroundedAssistantTest, GroundedBridge_RefusesRequestsForOtherQuestions) {
    GroundedAssistant assistant = makeAssistant(&bridge);
    const PrepareResult prepared = assistant.prepare({"Why?", {}});
    ASSERT_TRUE(prepared.succeeded());
    GroundedAgentBridge grounded(bridge, prepared.turn);
    engine::bridge::AssistantRequest request;
    request.question = "Something else";
    EXPECT_EQ(grounded.startAssistantResponse(request, 1s), nullptr);
    EXPECT_TRUE(bridge.requests.empty());
}

// Read-only (#72): answering uses real files under Scripts and leaves them byte-identical; the only
// file written is the user-local chat.
TEST_F(GroundedAssistantTest, Ask_WithScriptsFolderReader_IsReadOnlyOutsideUserLocalChat) {
    const std::filesystem::path project = root / "project";
    std::filesystem::create_directories(project / "Scripts");
    {
        std::ofstream(project / "Scripts" / "door.cpp", std::ios::binary) << "int door = 1;\n";
        std::ofstream(project / "secrets.env", std::ios::binary) << "TOKEN=x\n";
    }
    const auto snapshot = [&project] {
        std::vector<std::pair<std::string, std::string>> files;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(project)) {
            if (entry.is_regular_file()) {
                std::ifstream input(entry.path(), std::ios::binary);
                files.emplace_back(entry.path().string(),
                                   std::string((std::istreambuf_iterator<char>(input)),
                                               std::istreambuf_iterator<char>()));
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    };
    const auto before = snapshot();

    const ScriptsFolderReader reader(project);
    const GroundedSources view{sources.graph, sources.telemetry, sources.assets, reader};
    GroundedAssistant assistant(chat, store, &bridge, view, policy, "local-model", 5s);
    StartAnswerResult started =
        assistant.ask({"What is door?", {SourceCodeContext{{"Scripts/door.cpp"}}}});
    ASSERT_TRUE(started.succeeded()) << started.error;
    ASSERT_EQ(started.operation->run().state, MessageState::Complete);

    EXPECT_EQ(snapshot(), before);
    EXPECT_TRUE(std::filesystem::exists(store.chatFilePath("grounded")));
    EXPECT_NE(bridge.requests.at(0).semanticGraphStructure.find("1| int door = 1;"),
              std::string::npos);
    EXPECT_EQ(bridge.requests.at(0).semanticGraphStructure.find("TOKEN"), std::string::npos);
}

TEST(ScriptsFolderReader, ReadsOnlyRegularFilesInsideScripts) {
    const std::filesystem::path project =
        std::filesystem::temp_directory_path() / "aegis-us111-scripts-reader";
    std::filesystem::remove_all(project);
    std::filesystem::create_directories(project / "Scripts" / "sub");
    std::ofstream(project / "Scripts" / "sub" / "a.cpp") << "a";
    std::ofstream(project / "outside.cpp") << "outside";

    const ScriptsFolderReader reader(project);
    EXPECT_EQ(reader.readScript("Scripts/sub/a.cpp"), std::optional<std::string>("a"));
    EXPECT_FALSE(reader.readScript("Scripts/../outside.cpp").has_value());
    EXPECT_FALSE(reader.readScript("outside.cpp").has_value());
    EXPECT_FALSE(reader.readScript("Scripts/sub").has_value());
    EXPECT_FALSE(reader.readScript("Scripts/missing.cpp").has_value());

    std::error_code error;
    std::filesystem::create_symlink(project / "outside.cpp", project / "Scripts" / "link.cpp",
                                    error);
    if (!error) {
        EXPECT_FALSE(reader.readScript("Scripts/link.cpp").has_value());
    }
    std::filesystem::remove_all(project);
}

} // namespace
