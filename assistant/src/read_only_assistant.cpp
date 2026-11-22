#include "aegis/assistant/read_only_assistant.hpp"

#include <algorithm>
#include <cstddef>
#include <mutex>
#include <utility>

namespace aegis::assistant {
namespace {

void addCategory(std::vector<std::string>& categories, std::string category) {
    if (std::find(categories.begin(), categories.end(), category) == categories.end()) {
        categories.push_back(std::move(category));
    }
}

[[nodiscard]] MessageState stateFor(engine::bridge::AssistantStreamOutcome outcome) {
    switch (outcome) {
    case engine::bridge::AssistantStreamOutcome::Completed:
        return MessageState::Complete;
    case engine::bridge::AssistantStreamOutcome::Cancelled:
        return MessageState::Stopped;
    case engine::bridge::AssistantStreamOutcome::Failed:
        return MessageState::Failed;
    }
    return MessageState::Failed;
}

} // namespace

class AnswerOperation::State {
public:
    State(Chat& sourceChat, ChatStore& chatStore, std::size_t targetAnswerIndex,
          std::unique_ptr<engine::bridge::AssistantResponseStream> responseStream)
        : chat(sourceChat)
        , store(chatStore)
        , answerIndex(targetAnswerIndex)
        , stream(std::move(responseStream)) {}

    [[nodiscard]] AnswerResult run() {
        {
            std::scoped_lock lock(mutex);
            if (runStarted) {
                return {chat.messages.at(answerIndex).state, "answer operation already ran"};
            }
            runStarted = true;
            if (chat.messages.at(answerIndex).state != MessageState::Streaming) {
                return {chat.messages.at(answerIndex).state, error};
            }
        }

        const engine::bridge::AssistantStreamResult streamResult = stream->consume(
            [this](const engine::bridge::AssistantChunk& chunk) { return receive(chunk); });

        std::scoped_lock lock(mutex);
        ChatMessage& answer = chat.messages.at(answerIndex);
        if (answer.state == MessageState::Streaming) {
            answer.state = stateFor(streamResult.outcome);
            if (streamResult.outcome == engine::bridge::AssistantStreamOutcome::Failed) {
                error = streamResult.error;
            }
            static_cast<void>(saveLocked());
        }
        return {answer.state, error};
    }

    void cancel() noexcept {
        bool shouldCancel = false;
        {
            std::scoped_lock lock(mutex);
            ChatMessage& answer = chat.messages.at(answerIndex);
            if (answer.state == MessageState::Streaming) {
                answer.state = MessageState::Stopped;
                static_cast<void>(saveLocked());
                shouldCancel = true;
            }
        }
        if (shouldCancel) {
            stream->cancel();
        }
    }

private:
    [[nodiscard]] bool receive(const engine::bridge::AssistantChunk& chunk) {
        std::scoped_lock lock(mutex);
        ChatMessage& answer = chat.messages.at(answerIndex);
        if (answer.state != MessageState::Streaming) {
            return false;
        }
        answer.text += chunk.text;
        for (const engine::bridge::AssistantCitation& citation : chunk.citations) {
            answer.citations.push_back({citation.kind, citation.payload});
        }
        for (const std::string& category : chunk.dataCategories) {
            addCategory(answer.dataCategories, category);
        }
        if (!saveLocked()) {
            answer.state = MessageState::Failed;
            return false;
        }
        return true;
    }

    [[nodiscard]] bool saveLocked() {
        const StoreResult result = store.save(chat);
        if (!result.succeeded()) {
            error = result.error;
            return false;
        }
        return true;
    }

    Chat& chat;
    ChatStore& store;
    std::size_t answerIndex;
    std::unique_ptr<engine::bridge::AssistantResponseStream> stream;
    std::mutex mutex;
    bool runStarted = false;
    std::string error;
};

AnswerOperation::AnswerOperation(std::unique_ptr<State> state)
    : state_(std::move(state)) {
}

AnswerOperation::~AnswerOperation() = default;

AnswerResult AnswerOperation::run() {
    return state_->run();
}

void AnswerOperation::cancel() noexcept {
    state_->cancel();
}

ReadOnlyAssistant::ReadOnlyAssistant(Chat& chat, ChatStore& store,
                                     engine::bridge::AgentBridge* configuredBridge,
                                     const SemanticGraphView& graph,
                                     const DataTransmissionPolicy& policy, std::string modelId,
                                     std::chrono::milliseconds timeout)
    : chat_(chat)
    , store_(store)
    , bridge_(configuredBridge)
    , graph_(graph)
    , policy_(policy)
    , modelId_(std::move(modelId))
    , timeout_(timeout) {
}

StartAnswerResult ReadOnlyAssistant::startQuestion(std::string question,
                                                   std::vector<std::string> dataCategories) {
    return start(std::move(question), {}, MessageContent::Conversation, std::move(dataCategories));
}

StartAnswerResult
ReadOnlyAssistant::startDesignSuggestion(std::vector<std::string> dataCategories) {
    addCategory(dataCategories, "semantic-graph");
    return start("Suggest design changes based on this Semantic Graph structure. Return only "
                 "suggestions for the user to apply manually.",
                 graph_.structureForAssistant(), MessageContent::DesignSuggestion,
                 std::move(dataCategories));
}

StartAnswerResult ReadOnlyAssistant::start(std::string question, std::string graphStructure,
                                           MessageContent answerContent,
                                           std::vector<std::string> dataCategories) {
    if (bridge_ == nullptr) {
        return {{}, "AI service is not configured"};
    }
    if (modelId_.empty()) {
        return {{}, "AI service model is not configured"};
    }
    if (question.empty()) {
        return {{}, "question is empty"};
    }
    addCategory(dataCategories, "question");
    for (const std::string& category : dataCategories) {
        if (!policy_.allows(category)) {
            return {{}, "data category is not allowed: " + category};
        }
    }

    const std::size_t answerIndex = chat_.messages.size() + 1U;
    engine::bridge::AssistantRequest request;
    request.requestId = chat_.id + '-' + std::to_string(answerIndex);
    request.modelId = modelId_;
    request.question = question;
    request.semanticGraphStructure = std::move(graphStructure);
    request.dataCategories = dataCategories;

    std::unique_ptr<engine::bridge::AssistantResponseStream> stream =
        bridge_->startAssistantResponse(request, timeout_);
    if (stream == nullptr) {
        return {{}, "AgentBridge did not create an answer stream"};
    }

    chat_.messages.push_back({MessageRole::Question,
                              MessageState::Complete,
                              MessageContent::Conversation,
                              std::move(question),
                              {},
                              dataCategories});
    chat_.messages.push_back(
        {MessageRole::Answer, MessageState::Streaming, answerContent, {}, {}, {}});
    const StoreResult storeResult = store_.save(chat_);
    if (!storeResult.succeeded()) {
        chat_.messages.pop_back();
        chat_.messages.pop_back();
        return {{}, storeResult.error};
    }

    auto state =
        std::make_unique<AnswerOperation::State>(chat_, store_, answerIndex, std::move(stream));
    return {std::unique_ptr<AnswerOperation>(new AnswerOperation(std::move(state))), {}};
}

} // namespace aegis::assistant
