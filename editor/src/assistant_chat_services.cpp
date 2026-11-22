#include "editor/assistant_chat_services.hpp"

#include <atomic>
#include <memory>
#include <thread>
#include <utility>

#include "aegis/assistant/grounded/answer_format.hpp"
#include "aegis/assistant/grounded/diagnosis.hpp"
#include "aegis/assistant/grounded/grounded_assistant.hpp"

namespace editor {

namespace aa = aegis::assistant;

namespace {

constexpr const char* measuringQuestion = "?";

// Runs one US-11.2 AnswerOperation on its own thread. AnswerOperation::run() blocks until the
// stream ends; cancel() is thread-safe and keeps the received text, marking it Stopped.
class ThreadedAnswer final : public IActiveAnswer {
public:
    explicit ThreadedAnswer(std::unique_ptr<aa::AnswerOperation> operation)
        : operation_(std::move(operation))
        , worker_([this] {
            result_ = operation_->run();
            finished_.store(true, std::memory_order_release);
        }) {}

    ThreadedAnswer(const ThreadedAnswer&) = delete;
    ThreadedAnswer& operator=(const ThreadedAnswer&) = delete;
    ThreadedAnswer(ThreadedAnswer&&) = delete;
    ThreadedAnswer& operator=(ThreadedAnswer&&) = delete;

    ~ThreadedAnswer() override {
        operation_->cancel();
        worker_.join();
    }

    [[nodiscard]] bool finished() const override {
        return finished_.load(std::memory_order_acquire);
    }

    void cancel() noexcept override { operation_->cancel(); }

    [[nodiscard]] aa::AnswerResult result() const override {
        if (!finished()) {
            return {aa::MessageState::Streaming, {}};
        }
        return result_;
    }

private:
    std::unique_ptr<aa::AnswerOperation> operation_;
    aa::AnswerResult result_;
    std::atomic_bool finished_ = false;
    // Last member: it starts after everything it uses is constructed.
    std::thread worker_;
};

// Measuring is not sending: the estimator only assembles the context document.
class MeasureEverything final : public aa::DataTransmissionPolicy {
public:
    [[nodiscard]] bool allows(std::string_view) const override { return true; }
};

// The document without a diagnosis, so the size reflects only what the item retrieves.
[[nodiscard]] std::optional<std::size_t> documentBytes(const aa::GroundedQuestion& question,
                                                       const aa::GroundedSources& sources) {
    const MeasureEverything policy;
    const aa::Chat noHistory;
    const aa::AssemblyResult assembled = aa::assembleContext(question, noHistory, sources, policy);
    if (!assembled.succeeded()) {
        return std::nullopt;
    }
    return aa::renderContextDocument(*assembled.context, aa::Diagnosis{}).size();
}

} // namespace

GroundedChatQuestionSender::GroundedChatQuestionSender(aa::ChatStore& store,
                                                       engine::bridge::AgentBridge* bridge,
                                                       aa::GroundedSources sources,
                                                       const aa::DataTransmissionPolicy& policy,
                                                       GroundedAnswerSettings settings)
    : store_(store)
    , bridge_(bridge)
    , sources_(sources)
    , policy_(policy)
    , settings_(std::move(settings)) {
}

SendOutcome GroundedChatQuestionSender::send(aa::Chat& chat, const aa::GroundedQuestion& question) {
    aa::GroundedAssistant assistant(chat, store_, bridge_, sources_, policy_, settings_.modelId,
                                    settings_.timeout);
    aa::StartAnswerResult started = assistant.ask(question);
    if (!started.succeeded()) {
        return {nullptr, started.error.empty() ? "The question could not be sent." : started.error};
    }
    return {std::make_unique<ThreadedAnswer>(std::move(started.operation)), {}};
}

aa::LoadResult ChatStoreLoader::load(const std::string& chatId) const {
    return store_.load(chatId);
}

GroundedContextSizeEstimator::GroundedContextSizeEstimator(aa::GroundedSources sources)
    : sources_(sources) {
}

std::optional<std::size_t>
GroundedContextSizeEstimator::requestBytes(const aa::ContextItem& item) const {
    // US-11.1 refuses an empty question; the same text is in both documents, so it cancels out.
    const std::optional<std::size_t> without = documentBytes({measuringQuestion, {}}, sources_);
    const std::optional<std::size_t> with = documentBytes({measuringQuestion, {item}}, sources_);
    if (!without || !with) {
        return std::nullopt;
    }
    return *with > *without ? *with - *without : 0;
}

EditorContextItemProvider::EditorContextItemProvider(const ISelectionSource& selection,
                                                     const ILatestRunSource& latestRun,
                                                     const ConsoleModel& console)
    : selection_(selection)
    , latestRun_(latestRun)
    , console_(console) {
}

std::optional<aa::ContextItem> EditorContextItemProvider::itemFor(ContextChipType type) const {
    switch (type) {
    case ContextChipType::Selection:
        return selectionContextItem(selection_.currentSelection());
    case ContextChipType::LastRun:
        if (!latestRun_.latestRun()) {
            return std::nullopt;
        }
        return aa::LastRunContext{};
    case ContextChipType::Graph: {
        aa::GraphContext graph;
        const EditorSelection selection = selection_.currentSelection();
        if (const auto* entity = std::get_if<EntitySelection>(&selection)) {
            graph.focus.push_back(entity->entity);
        }
        return graph;
    }
    case ContextChipType::LogEntry: {
        const engine::LogEntry* entry = console_.selectedEntry();
        if (entry == nullptr) {
            return std::nullopt;
        }
        return aa::LogEntryContext{
            aa::LogEntryFacts{engine::logSeverityName(entry->severity), entry->message, entry->tick,
                              entry->stackTrace.value_or(std::string()), entry->tick.has_value()}};
    }
    case ContextChipType::GraphNodes:
    case ContextChipType::SourceCode:
    case ContextChipType::Asset:
    case ContextChipType::ComponentType:
        return std::nullopt;
    }
    return std::nullopt;
}

std::string makeChatId(std::chrono::system_clock::time_point now, unsigned int counter) {
    const auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return "chat-" + std::to_string(milliseconds) + '-' + std::to_string(counter);
}

} // namespace editor
