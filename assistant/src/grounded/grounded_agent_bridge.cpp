#include "aegis/assistant/grounded/grounded_agent_bridge.hpp"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <string_view>
#include <utility>

#include "aegis/assistant/grounded/answer_format.hpp"
#include "aegis/assistant/grounded/citations.hpp"
#include "aegis/assistant/grounded/evidence.hpp"

namespace aegis::assistant {
namespace {

using engine::bridge::AssistantChunk;
using engine::bridge::AssistantCitation;
using engine::bridge::AssistantResponseStream;
using engine::bridge::AssistantStreamOutcome;
using engine::bridge::AssistantStreamResult;

[[nodiscard]] AssistantCitation toBridge(const GroundedCitation& citation) {
    Citation encoded = encodeCitation(citation);
    return {std::move(encoded.kind), std::move(encoded.payload)};
}

class GroundedResponseStream final : public AssistantResponseStream {
public:
    GroundedResponseStream(std::unique_ptr<AssistantResponseStream> configured,
                           std::shared_ptr<const GroundedTurn> turn)
        : configured_(std::move(configured))
        , turn_(std::move(turn)) {}

    [[nodiscard]] AssistantStreamResult consume(const ChunkConsumer& consumer) override {
        if (cancelled_.load()) {
            return {AssistantStreamOutcome::Cancelled, {}};
        }
        if (!consumer(openingChunk())) {
            configured_->cancel();
            return {AssistantStreamOutcome::Cancelled, {}};
        }
        std::size_t removed = 0;
        const AssistantStreamResult result = configured_->consume(
            [&](const AssistantChunk& chunk) { return consumer(filter(chunk, removed)); });
        if (result.outcome == AssistantStreamOutcome::Completed && removed > 0) {
            const AssistantChunk note{
                "\nNote: " + std::to_string(removed) +
                    " citation(s) from the AI service did not match the attached evidence and "
                    "were removed.\n",
                {},
                {}};
            if (!consumer(note)) {
                return {AssistantStreamOutcome::Cancelled, {}};
            }
        }
        return result;
    }

    void cancel() noexcept override {
        cancelled_.store(true);
        configured_->cancel();
    }

private:
    [[nodiscard]] AssistantChunk openingChunk() const {
        AssistantChunk chunk;
        chunk.text = formatDiagnosis(turn_->diagnosis) + '\n';
        for (const GroundedCitation& citation : diagnosisCitations(turn_->diagnosis)) {
            if (checkCitation(turn_->context.evidence, citation).supported) {
                chunk.citations.push_back(toBridge(citation));
            }
        }
        chunk.dataCategories = turn_->context.dataCategories;
        return chunk;
    }

    [[nodiscard]] AssistantChunk filter(const AssistantChunk& chunk, std::size_t& removed) const {
        AssistantChunk filtered;
        filtered.text = chunk.text;
        for (const AssistantCitation& citation : chunk.citations) {
            const DecodedCitation decoded = decodeCitation(citation.kind, citation.payload);
            if (decoded.succeeded() &&
                checkCitation(turn_->context.evidence, *decoded.citation).supported) {
                filtered.citations.push_back(toBridge(*decoded.citation));
            } else {
                ++removed;
            }
        }
        const std::vector<std::string>& allowed = turn_->context.dataCategories;
        for (const std::string& category : chunk.dataCategories) {
            if (category == questionCategory ||
                std::find(allowed.begin(), allowed.end(), category) != allowed.end()) {
                filtered.dataCategories.push_back(category);
            }
        }
        return filtered;
    }

    std::unique_ptr<AssistantResponseStream> configured_;
    std::shared_ptr<const GroundedTurn> turn_;
    std::atomic_bool cancelled_ = false;
};

} // namespace

GroundedAgentBridge::GroundedAgentBridge(engine::bridge::AgentBridge& configured,
                                         std::shared_ptr<const GroundedTurn> turn)
    : configured_(configured)
    , turn_(std::move(turn)) {
}

engine::bridge::ExchangeResult
GroundedAgentBridge::exchange(const engine::bridge::Observation& observation,
                              std::chrono::milliseconds timeout) {
    return configured_.exchange(observation, timeout);
}

engine::bridge::HealthCheckResult
GroundedAgentBridge::checkHealth(std::chrono::milliseconds timeout) {
    return configured_.checkHealth(timeout);
}

engine::bridge::ModelListResult GroundedAgentBridge::listModels(std::chrono::milliseconds timeout) {
    return configured_.listModels(timeout);
}

std::unique_ptr<engine::bridge::AssistantResponseStream>
GroundedAgentBridge::startAssistantResponse(const engine::bridge::AssistantRequest& request,
                                            std::chrono::milliseconds timeout) {
    if (turn_ == nullptr || request.question != turn_->context.question) {
        return nullptr;
    }
    engine::bridge::AssistantRequest grounded = request;
    grounded.semanticGraphStructure = turn_->contextDocument;
    std::unique_ptr<AssistantResponseStream> stream =
        configured_.startAssistantResponse(grounded, timeout);
    if (stream == nullptr) {
        return nullptr;
    }
    return std::make_unique<GroundedResponseStream>(std::move(stream), turn_);
}

} // namespace aegis::assistant
