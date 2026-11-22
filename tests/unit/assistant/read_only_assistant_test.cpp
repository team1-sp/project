#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "aegis/assistant/read_only_assistant.hpp"

namespace {

using namespace std::chrono_literals;

class FixedUserProjectPaths final : public aegis::assistant::UserProjectPaths {
public:
    explicit FixedUserProjectPaths(std::filesystem::path path)
        : path_(std::move(path)) {}

    [[nodiscard]] std::filesystem::path userLocalArea() const override { return path_; }

private:
    std::filesystem::path path_;
};

class FakeGraphView final : public aegis::assistant::SemanticGraphView {
public:
    [[nodiscard]] std::string structureForAssistant() const override {
        ++readCount;
        return structure;
    }

    std::string structure = "System Movement reads Position and writes Velocity";
    mutable int readCount = 0;
};

class FakeTransmissionPolicy final : public aegis::assistant::DataTransmissionPolicy {
public:
    [[nodiscard]] bool allows(std::string_view dataCategory) const override {
        return deniedCategory.empty() || dataCategory != deniedCategory;
    }

    std::string deniedCategory;
};

struct FakeStreamControl {
    std::vector<engine::bridge::AssistantChunk> chunks;
    engine::bridge::AssistantStreamResult result{engine::bridge::AssistantStreamOutcome::Completed,
                                                 {}};
    std::function<void(std::size_t)> afterChunk;
    std::atomic_bool cancelled = false;
    std::atomic_int cancelCount = 0;
};

class FakeStream final : public engine::bridge::AssistantResponseStream {
public:
    explicit FakeStream(std::shared_ptr<FakeStreamControl> control)
        : control_(std::move(control)) {}

    [[nodiscard]] engine::bridge::AssistantStreamResult
    consume(const ChunkConsumer& consumer) override {
        if (control_->cancelled.load()) {
            return {engine::bridge::AssistantStreamOutcome::Cancelled, {}};
        }
        for (std::size_t index = 0; index < control_->chunks.size(); ++index) {
            if (control_->cancelled.load() || !consumer(control_->chunks[index])) {
                return {engine::bridge::AssistantStreamOutcome::Cancelled, {}};
            }
            if (control_->afterChunk) {
                control_->afterChunk(index);
            }
        }
        if (control_->cancelled.load()) {
            return {engine::bridge::AssistantStreamOutcome::Cancelled, {}};
        }
        return control_->result;
    }

    void cancel() noexcept override {
        if (!control_->cancelled.exchange(true)) {
            ++control_->cancelCount;
        }
    }

private:
    std::shared_ptr<FakeStreamControl> control_;
};

class FakeAgentBridge final : public engine::bridge::AgentBridge {
public:
    [[nodiscard]] engine::bridge::ExchangeResult exchange(const engine::bridge::Observation&,
                                                          std::chrono::milliseconds) override {
        ++exchangeCount;
        return {};
    }

    [[nodiscard]] engine::bridge::HealthCheckResult
    checkHealth(std::chrono::milliseconds) override {
        return {};
    }

    [[nodiscard]] engine::bridge::ModelListResult listModels(std::chrono::milliseconds) override {
        return {};
    }

    [[nodiscard]] std::unique_ptr<engine::bridge::AssistantResponseStream>
    startAssistantResponse(const engine::bridge::AssistantRequest& request,
                           std::chrono::milliseconds timeout) override {
        ++answerCount;
        lastRequest = request;
        lastTimeout = timeout;
        lastControl = std::make_shared<FakeStreamControl>();
        lastControl->chunks = nextChunks;
        lastControl->result = nextResult;
        return std::make_unique<FakeStream>(lastControl);
    }

    std::vector<engine::bridge::AssistantChunk> nextChunks;
    engine::bridge::AssistantStreamResult nextResult{
        engine::bridge::AssistantStreamOutcome::Completed, {}};
    std::shared_ptr<FakeStreamControl> lastControl;
    engine::bridge::AssistantRequest lastRequest;
    std::chrono::milliseconds lastTimeout{0};
    int answerCount = 0;
    int exchangeCount = 0;
};

class ReadOnlyAssistantTest : public testing::Test {
protected:
    void SetUp() override {
        const testing::TestInfo* info = testing::UnitTest::GetInstance()->current_test_info();
        root_ = std::filesystem::temp_directory_path() / "aegis-read-only-assistant-tests" /
                (std::string(info->test_suite_name()) + '-' + info->name());
        std::error_code error;
        std::filesystem::remove_all(root_, error);
        ASSERT_FALSE(error);
        paths_ = std::make_unique<FixedUserProjectPaths>(root_ / "user-local");
        store_ = std::make_unique<aegis::assistant::ChatStore>(*paths_);
    }

    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }

    [[nodiscard]] aegis::assistant::ReadOnlyAssistant makeAssistant() {
        return {chat_, *store_, &bridge_, graph_, policy_, "chosen-model", 30s};
    }

    std::filesystem::path root_;
    std::unique_ptr<FixedUserProjectPaths> paths_;
    std::unique_ptr<aegis::assistant::ChatStore> store_;
    aegis::assistant::Chat chat_{aegis::assistant::chatFormatVersion, "chat-72", {}};
    FakeAgentBridge bridge_;
    FakeGraphView graph_;
    FakeTransmissionPolicy policy_;
};

TEST_F(ReadOnlyAssistantTest, ConfiguredQuestion_RoutesOnlyThroughAgentBridgeAndPersistsReply) {
    bridge_.nextChunks = {{"Part one. ", {{"state", R"({"run":1,"tick":7})"}}, {"model-response"}},
                          {"Part two.", {{"entity", R"({"id":42})"}}, {"citations"}}};
    aegis::assistant::ReadOnlyAssistant assistant = makeAssistant();

    aegis::assistant::StartAnswerResult started =
        assistant.startQuestion("What happened?", {"telemetry"});
    ASSERT_TRUE(started.succeeded()) << started.error;
    const aegis::assistant::AnswerResult result = started.operation->run();

    EXPECT_EQ(result.state, aegis::assistant::MessageState::Complete);
    EXPECT_EQ(bridge_.answerCount, 1);
    EXPECT_EQ(bridge_.exchangeCount, 0);
    EXPECT_EQ(bridge_.lastRequest.modelId, "chosen-model");
    EXPECT_EQ(bridge_.lastRequest.question, "What happened?");
    EXPECT_EQ(bridge_.lastRequest.dataCategories,
              (std::vector<std::string>{"telemetry", "question"}));
    EXPECT_EQ(bridge_.lastTimeout, 30s);

    const aegis::assistant::LoadResult loaded = store_->load(chat_.id);
    ASSERT_TRUE(loaded.succeeded()) << loaded.error;
    ASSERT_EQ(loaded.chat->messages.size(), 2U);
    EXPECT_EQ(loaded.chat->messages[1].text, "Part one. Part two.");
    EXPECT_EQ(loaded.chat->messages[1].citations,
              (std::vector<aegis::assistant::Citation>{{"state", R"({"run":1,"tick":7})"},
                                                       {"entity", R"({"id":42})"}}));
    EXPECT_EQ(loaded.chat->messages[1].dataCategories,
              (std::vector<std::string>{"model-response", "citations"}));
}

TEST_F(ReadOnlyAssistantTest, DesignSuggestion_IsManualTextAndCannotMutateGraphOrProjectFiles) {
    const std::filesystem::path sourceFile = root_ / "project" / "game.cpp";
    std::filesystem::create_directories(sourceFile.parent_path());
    {
        std::ofstream output(sourceFile);
        output << "original project source\n";
    }
    bridge_.nextChunks = {
        {"Consider splitting MovementSystem into two declared systems.", {}, {"model-response"}}};
    aegis::assistant::ReadOnlyAssistant assistant = makeAssistant();

    aegis::assistant::StartAnswerResult started =
        assistant.startDesignSuggestion({"component-schema"});
    ASSERT_TRUE(started.succeeded()) << started.error;
    const aegis::assistant::AnswerResult result = started.operation->run();

    EXPECT_EQ(result.state, aegis::assistant::MessageState::Complete);
    EXPECT_EQ(graph_.readCount, 1);
    EXPECT_EQ(bridge_.lastRequest.semanticGraphStructure, graph_.structure);
    EXPECT_EQ(bridge_.exchangeCount, 0);
    ASSERT_EQ(chat_.messages.size(), 2U);
    EXPECT_EQ(chat_.messages[1].content, aegis::assistant::MessageContent::DesignSuggestion);
    EXPECT_EQ(chat_.messages[1].text,
              "Consider splitting MovementSystem into two declared systems.");
    std::ifstream input(sourceFile);
    std::string source;
    std::getline(input, source);
    EXPECT_EQ(source, "original project source");
}

TEST_F(ReadOnlyAssistantTest, CancelBeforeResponse_IsStoppedAndIdempotent) {
    bridge_.nextChunks = {{"must not arrive", {}, {}}};
    aegis::assistant::ReadOnlyAssistant assistant = makeAssistant();
    aegis::assistant::StartAnswerResult started = assistant.startQuestion("Stop it", {});
    ASSERT_TRUE(started.succeeded()) << started.error;

    started.operation->cancel();
    started.operation->cancel();
    const aegis::assistant::AnswerResult result = started.operation->run();

    EXPECT_EQ(result.state, aegis::assistant::MessageState::Stopped);
    EXPECT_EQ(bridge_.lastControl->cancelCount.load(), 1);
    EXPECT_TRUE(chat_.messages[1].text.empty());
    const aegis::assistant::LoadResult loaded = store_->load(chat_.id);
    ASSERT_TRUE(loaded.succeeded()) << loaded.error;
    EXPECT_EQ(loaded.chat->messages[1].state, aegis::assistant::MessageState::Stopped);
}

TEST_F(ReadOnlyAssistantTest, CancelDuringStreaming_PersistsOnlyReceivedPart) {
    bridge_.nextChunks = {{"received", {{"state", "tick:5"}}, {"telemetry"}},
                          {" must not arrive", {}, {}}};
    aegis::assistant::ReadOnlyAssistant assistant = makeAssistant();
    aegis::assistant::StartAnswerResult started = assistant.startQuestion("Stop midway", {});
    ASSERT_TRUE(started.succeeded()) << started.error;
    aegis::assistant::AnswerOperation* operation = started.operation.get();
    bridge_.lastControl->afterChunk = [operation](std::size_t index) {
        if (index == 0U) {
            operation->cancel();
            operation->cancel();
        }
    };

    const aegis::assistant::AnswerResult result = started.operation->run();

    EXPECT_EQ(result.state, aegis::assistant::MessageState::Stopped);
    EXPECT_EQ(bridge_.lastControl->cancelCount.load(), 1);
    EXPECT_EQ(chat_.messages[1].text, "received");
    const aegis::assistant::LoadResult loaded = store_->load(chat_.id);
    ASSERT_TRUE(loaded.succeeded()) << loaded.error;
    EXPECT_EQ(loaded.chat->messages[1].text, "received");
    EXPECT_EQ(loaded.chat->messages[1].citations,
              (std::vector<aegis::assistant::Citation>{{"state", "tick:5"}}));
    EXPECT_EQ(loaded.chat->messages[1].dataCategories, (std::vector<std::string>{"telemetry"}));
}

TEST_F(ReadOnlyAssistantTest, CancelAfterCompletion_DoesNotChangeCompletedAnswer) {
    bridge_.nextChunks = {{"done", {}, {"model-response"}}};
    aegis::assistant::ReadOnlyAssistant assistant = makeAssistant();
    aegis::assistant::StartAnswerResult started = assistant.startQuestion("Finish", {});
    ASSERT_TRUE(started.succeeded()) << started.error;
    ASSERT_EQ(started.operation->run().state, aegis::assistant::MessageState::Complete);

    started.operation->cancel();
    started.operation->cancel();

    EXPECT_EQ(bridge_.lastControl->cancelCount.load(), 0);
    EXPECT_EQ(chat_.messages[1].state, aegis::assistant::MessageState::Complete);
    const aegis::assistant::LoadResult loaded = store_->load(chat_.id);
    ASSERT_TRUE(loaded.succeeded()) << loaded.error;
    EXPECT_EQ(loaded.chat->messages[1].state, aegis::assistant::MessageState::Complete);
}

TEST_F(ReadOnlyAssistantTest, UnconfiguredService_DoesNotStartOrPersistQuestion) {
    aegis::assistant::ReadOnlyAssistant assistant(chat_, *store_, nullptr, graph_, policy_, "model",
                                                  30s);

    const aegis::assistant::StartAnswerResult started = assistant.startQuestion("Question", {});

    EXPECT_FALSE(started.succeeded());
    EXPECT_EQ(started.error, "AI service is not configured");
    EXPECT_TRUE(chat_.messages.empty());
    EXPECT_FALSE(std::filesystem::exists(store_->chatFilePath(chat_.id)));
}

TEST_F(ReadOnlyAssistantTest, DisallowedDataCategory_NeverReachesAgentBridgeOrStorage) {
    policy_.deniedCategory = "telemetry";
    aegis::assistant::ReadOnlyAssistant assistant = makeAssistant();

    const aegis::assistant::StartAnswerResult started =
        assistant.startQuestion("Private question", {"telemetry"});

    EXPECT_FALSE(started.succeeded());
    EXPECT_EQ(started.error, "data category is not allowed: telemetry");
    EXPECT_EQ(bridge_.answerCount, 0);
    EXPECT_TRUE(chat_.messages.empty());
    EXPECT_FALSE(std::filesystem::exists(store_->chatFilePath(chat_.id)));
}

} // namespace
