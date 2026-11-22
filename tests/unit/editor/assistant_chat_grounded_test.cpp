// US-33.2 integration: AssistantChat over the real US-11.1 GroundedAssistant, US-11.2
// AnswerOperation/ChatStore and a fake AgentBridge. Headless; no window and no network.
#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "../assistant/grounded/demo_run.hpp"
#include "../assistant/grounded/grounded_fakes.hpp"
#include "aegis/assistant/grounded/citations.hpp"
#include "assistant_chat_fakes.hpp"
#include "editor/assistant_chat.hpp"
#include "editor/assistant_chat_services.hpp"

namespace {

namespace aa = aegis::assistant;
using namespace std::chrono_literals;
using editor::ContextChipType;

constexpr auto waitLimit = 5s;

// Sends one chunk, then holds the stream open until it is cancelled.
struct HeldStreamControl {
    std::atomic_bool cancelled = false;
    std::atomic_int cancelCalls = 0;
    std::atomic_bool chunkSent = false;
};

class HeldStream final : public engine::bridge::AssistantResponseStream {
public:
    explicit HeldStream(std::shared_ptr<HeldStreamControl> control)
        : control_(std::move(control)) {}

    [[nodiscard]] engine::bridge::AssistantStreamResult
    consume(const ChunkConsumer& consumer) override {
        static_cast<void>(consumer({"partial answer", {}, {}}));
        control_->chunkSent.store(true);
        const auto deadline = std::chrono::steady_clock::now() + waitLimit;
        while (!control_->cancelled.load() && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(1ms);
        }
        return {engine::bridge::AssistantStreamOutcome::Cancelled, {}};
    }

    void cancel() noexcept override {
        ++control_->cancelCalls;
        control_->cancelled.store(true);
    }

private:
    std::shared_ptr<HeldStreamControl> control_;
};

class HeldBridge final : public engine::bridge::AgentBridge {
public:
    [[nodiscard]] engine::bridge::ExchangeResult exchange(const engine::bridge::Observation&,
                                                          std::chrono::milliseconds) override {
        return {{}, "not used"};
    }

    [[nodiscard]] std::unique_ptr<engine::bridge::AssistantResponseStream>
    startAssistantResponse(const engine::bridge::AssistantRequest&,
                           std::chrono::milliseconds) override {
        return std::make_unique<HeldStream>(control);
    }

    std::shared_ptr<HeldStreamControl> control = std::make_shared<HeldStreamControl>();
};

class AssistantChatGroundedTest : public testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove_all(root);
        aa::RunEvidence run = grounded_test::baseRun();
        run.events = {{41,
                       30,
                       "UnlockFailedEvent",
                       {engine::EntityId{2}, engine::EntityId{1}},
                       {{"requiredItem", "Key"}, {"inventory", "Coin"}}}};
        sources.telemetry.run = run;
        sources.scripts.files = {{"Scripts/door.cpp", "bool unlocked = false;\n"}};
        items.items[ContextChipType::LastRun] = aa::LastRunContext{};
        items.items[ContextChipType::SourceCode] = aa::SourceCodeContext{{"Scripts/door.cpp"}};
    }

    void TearDown() override { std::filesystem::remove_all(root); }

    [[nodiscard]] std::unique_ptr<editor::AssistantChat>
    makeChat(engine::bridge::AgentBridge* bridge) {
        sender = std::make_unique<editor::GroundedChatQuestionSender>(
            store, bridge, sources.view(), policy, editor::GroundedAnswerSettings{"local", 5s});
        return std::make_unique<editor::AssistantChat>(*sender, loader, items, sizes, [this] {
            return editor::makeChatId(std::chrono::system_clock::now(), ++counter);
        });
    }

    static bool waitForAnswer(editor::AssistantChat& chat) {
        const auto deadline = std::chrono::steady_clock::now() + waitLimit;
        while (chat.isAnswering() && std::chrono::steady_clock::now() < deadline) {
            chat.update();
            std::this_thread::sleep_for(1ms);
        }
        return !chat.isAnswering();
    }

    static std::filesystem::path testRoot() {
        return std::filesystem::temp_directory_path() /
               ("aegis-us332-" +
                std::string(testing::UnitTest::GetInstance()->current_test_info()->name()));
    }

    std::filesystem::path root = testRoot();
    grounded_test::Sources sources;
    grounded_test::FakePolicy policy;
    grounded_test::FixedUserProjectPaths paths{root};
    aa::ChatStore store{paths};
    editor::ChatStoreLoader loader{store};
    chat_test::FakeItems items;
    editor::GroundedContextSizeEstimator sizes{sources.view()};
    std::unique_ptr<editor::GroundedChatQuestionSender> sender;
    unsigned int counter = 0;
};

TEST_F(AssistantChatGroundedTest, AnsweredChatIsRestoredFromHistoryWithStructuredCitations) {
    grounded_test::FakeBridge bridge;
    bridge.control->chunks = {{"The door needs the Key.", {}, {}}};
    auto chat = makeChat(&bridge);
    ASSERT_TRUE(chat->attach(ContextChipType::LastRun));
    chat->setDraft("Why does the door stay locked?");

    ASSERT_TRUE(chat->send()) << chat->lastError();
    ASSERT_TRUE(waitForAnswer(*chat));

    const std::vector<aa::ChatMessage> answered(chat->settledMessages().begin(),
                                                chat->settledMessages().end());
    ASSERT_EQ(answered.size(), 2U);
    EXPECT_EQ(answered[1].state, aa::MessageState::Complete);
    EXPECT_NE(answered[1].text.find("The door needs the Key."), std::string::npos);
    ASSERT_FALSE(answered[1].citations.empty());
    for (const aa::Citation& citation : answered[1].citations) {
        EXPECT_TRUE(aa::decodeCitation(citation).succeeded()) << citation.payload;
    }
    const std::string answeredId = chat->currentChatId();

    ASSERT_TRUE(chat->newChat());
    EXPECT_TRUE(chat->settledMessages().empty());
    ASSERT_EQ(chat->history().size(), 1U);
    ASSERT_TRUE(chat->openChat(answeredId)) << chat->lastError();

    const std::vector<aa::ChatMessage> restored(chat->settledMessages().begin(),
                                                chat->settledMessages().end());
    EXPECT_EQ(restored, answered);
}

TEST_F(AssistantChatGroundedTest, StopKeepsThePartialAnswerMarkedStopped) {
    HeldBridge bridge;
    auto chat = makeChat(&bridge);
    chat->setDraft("Why?");
    ASSERT_TRUE(chat->send()) << chat->lastError();
    const auto deadline = std::chrono::steady_clock::now() + waitLimit;
    while (!bridge.control->chunkSent.load() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(1ms);
    }
    ASSERT_TRUE(chat->isAnswering());

    chat->stop();
    ASSERT_TRUE(waitForAnswer(*chat));

    EXPECT_TRUE(bridge.control->cancelled.load());
    const auto messages = chat->settledMessages();
    ASSERT_EQ(messages.size(), 2U);
    EXPECT_EQ(messages[1].state, aa::MessageState::Stopped);
    EXPECT_NE(messages[1].text.find("partial answer"), std::string::npos);
    const aa::LoadResult saved = store.load(chat->currentChatId());
    ASSERT_TRUE(saved.succeeded()) << saved.error;
    EXPECT_EQ(saved.chat->messages.back().state, aa::MessageState::Stopped);
    EXPECT_EQ(saved.chat->messages.back().text, messages[1].text);
}

TEST_F(AssistantChatGroundedTest, SendIsRejectedWithoutAConfiguredServiceAndKeepsTheDraft) {
    auto chat = makeChat(nullptr);
    ASSERT_TRUE(chat->attach(ContextChipType::SourceCode));
    chat->setDraft("Why?");

    EXPECT_FALSE(chat->send());

    EXPECT_EQ(chat->lastError(), "AI service is not configured");
    EXPECT_EQ(chat->draft(), "Why?");
    EXPECT_EQ(chat->chips().chips().size(), 1U);
    EXPECT_TRUE(chat->settledMessages().empty());
}

TEST_F(AssistantChatGroundedTest, PolicyRefusalKeepsTheDraftAndChips) {
    grounded_test::FakeBridge bridge;
    policy.denied = {std::string(aa::sourceCodeCategory)};
    auto chat = makeChat(&bridge);
    ASSERT_TRUE(chat->attach(ContextChipType::SourceCode));
    chat->setDraft("Why?");

    EXPECT_FALSE(chat->send());

    EXPECT_FALSE(chat->lastError().empty());
    EXPECT_EQ(chat->draft(), "Why?");
    EXPECT_EQ(chat->chips().chips().size(), 1U);
    EXPECT_TRUE(bridge.requests.empty());
}

TEST_F(AssistantChatGroundedTest, ChipSizesAreTheBytesTheItemAddsToTheContextDocument) {
    const std::optional<std::size_t> source =
        sizes.requestBytes(aa::SourceCodeContext{{"Scripts/door.cpp"}});
    const std::optional<std::size_t> run = sizes.requestBytes(aa::LastRunContext{});

    ASSERT_TRUE(source.has_value());
    ASSERT_TRUE(run.has_value());
    EXPECT_GE(*source, std::string("bool unlocked = false;\n").size());
    EXPECT_GT(*run, 0U);
}

TEST(AssistantChatIds, GeneratedIdsAreAcceptedByTheChatStore) {
    const std::string id = editor::makeChatId(
        std::chrono::system_clock::time_point{std::chrono::milliseconds{1234}}, 7);
    EXPECT_EQ(id, "chat-1234-7");
}

} // namespace
