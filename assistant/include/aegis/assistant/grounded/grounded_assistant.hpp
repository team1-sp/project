#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "aegis/assistant/chat.hpp"
#include "aegis/assistant/chat_store.hpp"
#include "aegis/assistant/grounded/context_assembly.hpp"
#include "aegis/assistant/grounded/grounded_agent_bridge.hpp"
#include "aegis/assistant/grounded/grounded_question.hpp"
#include "aegis/assistant/read_only_assistant.hpp"
#include "engine/bridge/agent_bridge.hpp"

namespace aegis::assistant {

struct PrepareResult {
    std::shared_ptr<const GroundedTurn> turn;
    std::string error;

    [[nodiscard]] bool succeeded() const noexcept { return turn != nullptr && error.empty(); }
};

// Chat-facing application API of US-11.1 (srs.fr.11.1 to srs.fr.11.7). A chat UI (#119) passes
// the typed question and its attached context items; the answer streams into the chat through
// US-11.2's ReadOnlyAssistant, AnswerOperation and ChatStore. It holds only const query sources
// and the read-only transmission policy, so answering cannot change engine state or project
// files; the only write is the user-local chat file.
class GroundedAssistant {
public:
    GroundedAssistant(Chat& chat, ChatStore& store, engine::bridge::AgentBridge* configuredBridge,
                      GroundedSources sources, const DataTransmissionPolicy& policy,
                      std::string modelId, std::chrono::milliseconds timeout);

    // Assembles context and diagnosis without contacting the AI service or changing the chat.
    [[nodiscard]] PrepareResult prepare(const GroundedQuestion& question) const;

    // Prepares the question and starts the answer. On error nothing is sent and the chat is
    // unchanged.
    [[nodiscard]] StartAnswerResult ask(const GroundedQuestion& question);

private:
    Chat& chat_;
    ChatStore& store_;
    engine::bridge::AgentBridge* bridge_;
    GroundedSources sources_;
    const DataTransmissionPolicy& policy_;
    std::string modelId_;
    std::chrono::milliseconds timeout_;
};

} // namespace aegis::assistant
