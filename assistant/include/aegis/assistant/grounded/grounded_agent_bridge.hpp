#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "aegis/assistant/grounded/context_assembly.hpp"
#include "aegis/assistant/grounded/diagnosis.hpp"
#include "engine/bridge/agent_bridge.hpp"

namespace aegis::assistant {

// One prepared question: its assembled context, deterministic diagnosis and the context document
// the AI service receives.
struct GroundedTurn {
    AssembledContext context;
    Diagnosis diagnosis;
    std::string contextDocument;
};

// Decorates the configured AgentBridge for one grounded question without changing the US-11.2
// seam (DEC-067). It
// - puts the grounded-context document into the request's semanticGraphStructure field;
// - streams the deterministic diagnosis first, with its typed citations;
// - lets through only AI-service citations that decode as typed citations and pass
//   checkCitation() against the turn's evidence, re-encoded canonically, and reports how many
//   were removed;
// - keeps only data categories the request carried.
// Requests for any other question get no stream. exchange(), checkHealth() and listModels() are
// forwarded unchanged to the configured bridge.
class GroundedAgentBridge final : public engine::bridge::AgentBridge {
public:
    GroundedAgentBridge(engine::bridge::AgentBridge& configured,
                        std::shared_ptr<const GroundedTurn> turn);

    [[nodiscard]] engine::bridge::ExchangeResult
    exchange(const engine::bridge::Observation& observation,
             std::chrono::milliseconds timeout) override;

    [[nodiscard]] engine::bridge::HealthCheckResult
    checkHealth(std::chrono::milliseconds timeout) override;

    [[nodiscard]] engine::bridge::ModelListResult
    listModels(std::chrono::milliseconds timeout) override;

    [[nodiscard]] std::unique_ptr<engine::bridge::AssistantResponseStream>
    startAssistantResponse(const engine::bridge::AssistantRequest& request,
                           std::chrono::milliseconds timeout) override;

private:
    engine::bridge::AgentBridge& configured_;
    std::shared_ptr<const GroundedTurn> turn_;
};

} // namespace aegis::assistant
