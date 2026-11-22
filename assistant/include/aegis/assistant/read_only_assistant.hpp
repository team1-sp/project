#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "aegis/assistant/chat.hpp"
#include "aegis/assistant/chat_store.hpp"
#include "engine/bridge/agent_bridge.hpp"

namespace aegis::assistant {

// The only Semantic Graph capability exposed to the assistant in US-11.2. It returns structure for
// context and has no mutation operation.
class SemanticGraphView {
public:
    virtual ~SemanticGraphView() = default;

    [[nodiscard]] virtual std::string structureForAssistant() const = 0;
};

// Read-only view of the project's data-sharing choices. US-33.5 supplies the implementation; the
// assistant core refuses a request before AgentBridge sees any disallowed category.
class DataTransmissionPolicy {
public:
    virtual ~DataTransmissionPolicy() = default;

    [[nodiscard]] virtual bool allows(std::string_view dataCategory) const = 0;
};

struct AnswerResult {
    MessageState state = MessageState::Failed;
    std::string error;
};

class AnswerOperation {
public:
    ~AnswerOperation();

    AnswerOperation(const AnswerOperation&) = delete;
    AnswerOperation& operator=(const AnswerOperation&) = delete;
    AnswerOperation(AnswerOperation&&) = delete;
    AnswerOperation& operator=(AnswerOperation&&) = delete;

    [[nodiscard]] AnswerResult run();
    void cancel() noexcept;

private:
    class State;
    explicit AnswerOperation(std::unique_ptr<State> state);

    std::unique_ptr<State> state_;

    friend class ReadOnlyAssistant;
};

struct StartAnswerResult {
    std::unique_ptr<AnswerOperation> operation;
    std::string error;

    [[nodiscard]] bool succeeded() const noexcept { return operation != nullptr && error.empty(); }
};

// Window-free assistant orchestration. It can read graph structure and call AgentBridge, but has no
// engine-state, project-source or filesystem mutation capability beyond the injected ChatStore.
class ReadOnlyAssistant {
public:
    ReadOnlyAssistant(Chat& chat, ChatStore& store, engine::bridge::AgentBridge* configuredBridge,
                      const SemanticGraphView& graph, const DataTransmissionPolicy& policy,
                      std::string modelId, std::chrono::milliseconds timeout);

    [[nodiscard]] StartAnswerResult startQuestion(std::string question,
                                                  std::vector<std::string> dataCategories);
    [[nodiscard]] StartAnswerResult startDesignSuggestion(std::vector<std::string> dataCategories);

private:
    [[nodiscard]] StartAnswerResult start(std::string question, std::string graphStructure,
                                          MessageContent answerContent,
                                          std::vector<std::string> dataCategories);

    Chat& chat_;
    ChatStore& store_;
    engine::bridge::AgentBridge* bridge_;
    const SemanticGraphView& graph_;
    const DataTransmissionPolicy& policy_;
    std::string modelId_;
    std::chrono::milliseconds timeout_;
};

} // namespace aegis::assistant
