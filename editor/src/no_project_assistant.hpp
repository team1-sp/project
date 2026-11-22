#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "aegis/assistant/chat_store.hpp"
#include "aegis/assistant/grounded/context_assembly.hpp"
#include "aegis/assistant/grounded/evidence_sources.hpp"
#include "aegis/assistant/read_only_assistant.hpp"

// What the AI Assistant reads while the Editor has no project open, like NoProjectSimulation: an
// empty Semantic Graph, no run, no assets or scripts, no user-local chat area and no permission to
// send any data category. The Project Hub (US-13.x), the Semantic Graph, telemetry recording and
// the AI service settings (US-33.5) replace these in their stories; until then sending is refused
// before anything reaches AgentBridge, and nothing is written.
namespace editor::no_project {

class EmptyGraph final : public aegis::assistant::SemanticGraphQuery {
public:
    [[nodiscard]] std::optional<aegis::assistant::EntityFacts>
    entity(engine::EntityId) const override {
        return std::nullopt;
    }
    [[nodiscard]] aegis::assistant::GraphSlice
    neighbourhood(std::span<const engine::EntityId>, unsigned int, std::size_t) const override {
        return {};
    }
    [[nodiscard]] aegis::assistant::GraphSlice nodes(std::span<const std::string>) const override {
        return {};
    }
    [[nodiscard]] std::optional<aegis::assistant::ComponentTypeFacts>
    componentType(std::string_view) const override {
        return std::nullopt;
    }
};

class NoRuns final : public aegis::assistant::RunTelemetryQuery {
public:
    [[nodiscard]] std::optional<aegis::assistant::RunEvidence> latestRun() const override {
        return std::nullopt;
    }
};

class NoAssets final : public aegis::assistant::AssetCatalogQuery {
public:
    [[nodiscard]] std::optional<aegis::assistant::AssetFacts>
    asset(std::string_view) const override {
        return std::nullopt;
    }
};

class NoScripts final : public aegis::assistant::ProjectSourceReader {
public:
    [[nodiscard]] std::optional<std::string> readScript(std::string_view) const override {
        return std::nullopt;
    }
};

class SendNothing final : public aegis::assistant::DataTransmissionPolicy {
public:
    [[nodiscard]] bool allows(std::string_view) const override { return false; }
};

// ChatStore needs a user-local area; with no project there is none, and with no AI service no chat
// is ever saved or loaded.
class NoUserArea final : public aegis::assistant::UserProjectPaths {
public:
    [[nodiscard]] std::filesystem::path userLocalArea() const override { return {}; }
};

struct AssistantSources {
    EmptyGraph graph;
    NoRuns telemetry;
    NoAssets assets;
    NoScripts scripts;

    [[nodiscard]] aegis::assistant::GroundedSources view() const {
        return {graph, telemetry, assets, scripts};
    }
};

} // namespace editor::no_project
