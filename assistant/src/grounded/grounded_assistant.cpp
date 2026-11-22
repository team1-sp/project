#include "aegis/assistant/grounded/grounded_assistant.hpp"

#include <utility>

#include "aegis/assistant/grounded/answer_format.hpp"
#include "aegis/assistant/grounded/diagnosis.hpp"

namespace aegis::assistant {
namespace {

// ReadOnlyAssistant needs a SemanticGraphView; for a grounded question it is the turn's document,
// which contains only the assembled context.
class TurnGraphView final : public SemanticGraphView {
public:
    explicit TurnGraphView(const GroundedTurn& turn)
        : turn_(turn) {}

    [[nodiscard]] std::string structureForAssistant() const override {
        return turn_.contextDocument;
    }

private:
    const GroundedTurn& turn_;
};

} // namespace

GroundedAssistant::GroundedAssistant(Chat& chat, ChatStore& store,
                                     engine::bridge::AgentBridge* configuredBridge,
                                     GroundedSources sources, const DataTransmissionPolicy& policy,
                                     std::string modelId, std::chrono::milliseconds timeout)
    : chat_(chat)
    , store_(store)
    , bridge_(configuredBridge)
    , sources_(sources)
    , policy_(policy)
    , modelId_(std::move(modelId))
    , timeout_(timeout) {
}

PrepareResult GroundedAssistant::prepare(const GroundedQuestion& question) const {
    AssemblyResult assembled = assembleContext(question, chat_, sources_, policy_);
    if (!assembled.succeeded()) {
        return {nullptr, std::move(assembled.error)};
    }
    auto turn = std::make_shared<GroundedTurn>();
    turn->context = std::move(*assembled.context);
    turn->diagnosis = diagnose(turn->context);
    turn->contextDocument = renderContextDocument(turn->context, turn->diagnosis);
    return {std::move(turn), {}};
}

StartAnswerResult GroundedAssistant::ask(const GroundedQuestion& question) {
    if (bridge_ == nullptr) {
        return {{}, "AI service is not configured"};
    }
    PrepareResult prepared = prepare(question);
    if (!prepared.succeeded()) {
        return {{}, std::move(prepared.error)};
    }
    GroundedAgentBridge grounded(*bridge_, prepared.turn);
    const TurnGraphView view(*prepared.turn);
    ReadOnlyAssistant assistant(chat_, store_, &grounded, view, policy_, modelId_, timeout_);
    return assistant.startQuestion(question.text, prepared.turn->context.dataCategories);
}

} // namespace aegis::assistant
