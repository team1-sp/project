#pragma once

#include <chrono>
#include <cstddef>
#include <optional>
#include <string>

#include "aegis/assistant/chat_store.hpp"
#include "aegis/assistant/grounded/context_assembly.hpp"
#include "aegis/assistant/read_only_assistant.hpp"
#include "editor/assistant_chat.hpp"
#include "editor/assistant_chat_context.hpp"
#include "editor/console_model.hpp"
#include "editor/selection.hpp"
#include "engine/bridge/agent_bridge.hpp"

// Production implementations of the AssistantChat seams. They use only the public interfaces of
// US-11.1 (GroundedAssistant), US-11.2 (AnswerOperation, ChatStore) and US-33.1 (selection and
// latest-run sources); nothing here contacts an AI service except through AgentBridge.
namespace editor {

struct GroundedAnswerSettings {
    std::string modelId;
    std::chrono::milliseconds timeout{std::chrono::seconds(60)};
};

// Asks through US-11.1's GroundedAssistant::ask() and runs the returned US-11.2 AnswerOperation on
// its own thread. Cancelling calls AnswerOperation::cancel(). `bridge` may be null while no AI
// service is configured; every send is then rejected with the assistant's reason.
class GroundedChatQuestionSender final : public IChatQuestionSender {
public:
    GroundedChatQuestionSender(aegis::assistant::ChatStore& store,
                               engine::bridge::AgentBridge* bridge,
                               aegis::assistant::GroundedSources sources,
                               const aegis::assistant::DataTransmissionPolicy& policy,
                               GroundedAnswerSettings settings);

    [[nodiscard]] SendOutcome send(aegis::assistant::Chat& chat,
                                   const aegis::assistant::GroundedQuestion& question) override;

private:
    aegis::assistant::ChatStore& store_;
    engine::bridge::AgentBridge* bridge_;
    aegis::assistant::GroundedSources sources_;
    const aegis::assistant::DataTransmissionPolicy& policy_;
    GroundedAnswerSettings settings_;
};

class ChatStoreLoader final : public IChatLoader {
public:
    explicit ChatStoreLoader(const aegis::assistant::ChatStore& store)
        : store_(store) {}

    [[nodiscard]] aegis::assistant::LoadResult load(const std::string& chatId) const override;

private:
    const aegis::assistant::ChatStore& store_;
};

// Measures an item as the bytes its evidence adds to US-11.1's context document: the document
// assembled with the item minus the document without it, both rendered without a diagnosis. It
// reads only the const sources and contacts no AI service; measuring is not sending, so it
// ignores the data-sharing policy.
class GroundedContextSizeEstimator final : public IContextSizeEstimator {
public:
    explicit GroundedContextSizeEstimator(aegis::assistant::GroundedSources sources);

    [[nodiscard]] std::optional<std::size_t>
    requestBytes(const aegis::assistant::ContextItem& item) const override;

private:
    aegis::assistant::GroundedSources sources_;
};

// Builds items from the Editor's state. Selection and Graph come from the current entity
// selection, Last run from the latest run, Log entry from the Console's selected entry. Graph
// nodes, Source code, Asset and Component type have no Editor selection yet (their panels belong
// to later stories), so they return nothing.
class EditorContextItemProvider final : public IContextItemProvider {
public:
    EditorContextItemProvider(const ISelectionSource& selection, const ILatestRunSource& latestRun,
                              const ConsoleModel& console);

    [[nodiscard]] std::optional<aegis::assistant::ContextItem>
    itemFor(ContextChipType type) const override;

private:
    const ISelectionSource& selection_;
    const ILatestRunSource& latestRun_;
    const ConsoleModel& console_;
};

// "chat-<milliseconds since epoch>-<counter>", accepted by ChatStore.
[[nodiscard]] std::string makeChatId(std::chrono::system_clock::time_point now,
                                     unsigned int counter);

} // namespace editor
