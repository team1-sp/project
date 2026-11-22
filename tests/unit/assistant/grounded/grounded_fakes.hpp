#pragma once

// Deterministic, in-memory fakes for the US-11.1 grounded assistant tests. No network, model or
// engine run is involved.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "aegis/assistant/chat_store.hpp"
#include "aegis/assistant/grounded/context_assembly.hpp"
#include "aegis/assistant/grounded/evidence_sources.hpp"
#include "aegis/assistant/read_only_assistant.hpp"
#include "engine/bridge/agent_bridge.hpp"

namespace grounded_test {

using aegis::assistant::AssetFacts;
using aegis::assistant::ComponentTypeFacts;
using aegis::assistant::EntityFacts;
using aegis::assistant::GraphSlice;
using aegis::assistant::RunEvidence;

inline engine::EntityId entityId(std::uint64_t value) {
    return engine::EntityId{value};
}

class FakeGraph final : public aegis::assistant::SemanticGraphQuery {
public:
    [[nodiscard]] std::optional<EntityFacts> entity(engine::EntityId id) const override {
        ++calls;
        for (const EntityFacts& facts : entities) {
            if (facts.id == id) {
                return facts;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] GraphSlice neighbourhood(std::span<const engine::EntityId> focus,
                                           unsigned int maxRelations,
                                           std::size_t maxNodes) const override {
        ++calls;
        lastFocus.assign(focus.begin(), focus.end());
        lastMaxRelations = maxRelations;
        lastMaxNodes = maxNodes;
        return slice;
    }

    [[nodiscard]] GraphSlice nodes(std::span<const std::string> nodeIds) const override {
        ++calls;
        GraphSlice selected;
        for (const auto& node : slice.nodes) {
            if (std::find(nodeIds.begin(), nodeIds.end(), node.id) != nodeIds.end()) {
                selected.nodes.push_back(node);
            }
        }
        return selected;
    }

    [[nodiscard]] std::optional<ComponentTypeFacts>
    componentType(std::string_view typeName) const override {
        ++calls;
        for (const ComponentTypeFacts& type : types) {
            if (type.name == typeName) {
                return type;
            }
        }
        return std::nullopt;
    }

    std::vector<EntityFacts> entities;
    GraphSlice slice;
    std::vector<ComponentTypeFacts> types;
    mutable int calls = 0;
    mutable std::vector<engine::EntityId> lastFocus;
    mutable unsigned int lastMaxRelations = 0;
    mutable std::size_t lastMaxNodes = 0;
};

class FakeTelemetry final : public aegis::assistant::RunTelemetryQuery {
public:
    [[nodiscard]] std::optional<RunEvidence> latestRun() const override {
        ++calls;
        return run;
    }

    std::optional<RunEvidence> run;
    mutable int calls = 0;
};

class FakeAssets final : public aegis::assistant::AssetCatalogQuery {
public:
    [[nodiscard]] std::optional<AssetFacts> asset(std::string_view path) const override {
        ++calls;
        for (const AssetFacts& facts : assets) {
            if (facts.path == path) {
                return facts;
            }
        }
        return std::nullopt;
    }

    std::vector<AssetFacts> assets;
    mutable int calls = 0;
};

class FakeScripts final : public aegis::assistant::ProjectSourceReader {
public:
    [[nodiscard]] std::optional<std::string> readScript(std::string_view path) const override {
        readPaths.emplace_back(path);
        const auto found = files.find(std::string(path));
        if (found == files.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    std::map<std::string, std::string> files;
    mutable std::vector<std::string> readPaths;
};

class FakePolicy final : public aegis::assistant::DataTransmissionPolicy {
public:
    [[nodiscard]] bool allows(std::string_view category) const override {
        return denied.find(std::string(category)) == denied.end();
    }

    std::set<std::string> denied;
};

struct Sources {
    FakeGraph graph;
    FakeTelemetry telemetry;
    FakeAssets assets;
    FakeScripts scripts;

    [[nodiscard]] aegis::assistant::GroundedSources view() const {
        return {graph, telemetry, assets, scripts};
    }
};

class FixedUserProjectPaths final : public aegis::assistant::UserProjectPaths {
public:
    explicit FixedUserProjectPaths(std::filesystem::path path)
        : path_(std::move(path)) {}

    [[nodiscard]] std::filesystem::path userLocalArea() const override { return path_; }

private:
    std::filesystem::path path_;
};

struct FakeStreamControl {
    std::vector<engine::bridge::AssistantChunk> chunks;
    engine::bridge::AssistantStreamResult result{engine::bridge::AssistantStreamOutcome::Completed,
                                                 {}};
    std::atomic_bool cancelled = false;
    std::atomic_int consumeCount = 0;
};

class FakeStream final : public engine::bridge::AssistantResponseStream {
public:
    explicit FakeStream(std::shared_ptr<FakeStreamControl> control)
        : control_(std::move(control)) {}

    [[nodiscard]] engine::bridge::AssistantStreamResult
    consume(const ChunkConsumer& consumer) override {
        ++control_->consumeCount;
        for (const auto& chunk : control_->chunks) {
            if (control_->cancelled.load() || !consumer(chunk)) {
                return {engine::bridge::AssistantStreamOutcome::Cancelled, {}};
            }
        }
        return control_->result;
    }

    void cancel() noexcept override { control_->cancelled.store(true); }

private:
    std::shared_ptr<FakeStreamControl> control_;
};

class FakeBridge final : public engine::bridge::AgentBridge {
public:
    [[nodiscard]] engine::bridge::ExchangeResult exchange(const engine::bridge::Observation&,
                                                          std::chrono::milliseconds) override {
        return {{}, "not used"};
    }

    [[nodiscard]] std::unique_ptr<engine::bridge::AssistantResponseStream>
    startAssistantResponse(const engine::bridge::AssistantRequest& request,
                           std::chrono::milliseconds) override {
        requests.push_back(request);
        return std::make_unique<FakeStream>(control);
    }

    std::shared_ptr<FakeStreamControl> control = std::make_shared<FakeStreamControl>();
    std::vector<engine::bridge::AssistantRequest> requests;
};

} // namespace grounded_test
