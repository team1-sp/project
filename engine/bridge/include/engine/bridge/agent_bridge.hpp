#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace engine::bridge {

// Engine-owned values at the transport seam. Protocol-library types must not cross this boundary.
struct Observation {
    std::uint64_t gameInstanceId = 0;
    std::uint64_t sequence = 0;
    std::span<const std::byte> payload;
};

struct Action {
    std::uint64_t gameInstanceId = 0;
    std::uint64_t sequence = 0;
    std::int32_t actionId = 0;
};

struct ExchangeResult {
    Action action;
    std::string error;

    [[nodiscard]] bool succeeded() const noexcept { return error.empty(); }
};

// An opaque citation returned by the AI service. The assistant stores the kind and payload
// verbatim so later stories can add structured citation kinds without changing this seam.
struct AssistantCitation {
    std::string kind;
    std::string payload;
};

// Data sent to the configured model. Categories are stable names such as "question" and
// "semantic-graph"; keeping them as strings makes the consent/storage record forward-compatible.
struct AssistantRequest {
    std::string requestId;
    std::string modelId;
    std::string question;
    std::string semanticGraphStructure;
    std::vector<std::string> dataCategories;
};

struct AssistantChunk {
    std::string text;
    std::vector<AssistantCitation> citations;
    std::vector<std::string> dataCategories;
};

enum class AssistantStreamOutcome { Completed, Cancelled, Failed };

struct AssistantStreamResult {
    AssistantStreamOutcome outcome = AssistantStreamOutcome::Failed;
    std::string error;
};

// One server-streaming Answer RPC. cancel() is safe before, during and after consume(), and may be
// called more than once. A gRPC implementation maps it to ClientContext::TryCancel().
class AssistantResponseStream {
public:
    using ChunkConsumer = std::function<bool(const AssistantChunk&)>;

    virtual ~AssistantResponseStream() = default;

    AssistantResponseStream(const AssistantResponseStream&) = delete;
    AssistantResponseStream& operator=(const AssistantResponseStream&) = delete;
    AssistantResponseStream(AssistantResponseStream&&) = delete;
    AssistantResponseStream& operator=(AssistantResponseStream&&) = delete;

    [[nodiscard]] virtual AssistantStreamResult consume(const ChunkConsumer& consumer) = 0;
    virtual void cancel() noexcept = 0;

protected:
    AssistantResponseStream() = default;
};

// Transport-neutral seam for playtest exchange and assistant answers. Implementations own the
// connection, gRPC details and retry policy.
class AgentBridge {
public:
    virtual ~AgentBridge() = default;

    AgentBridge(const AgentBridge&) = delete;
    AgentBridge& operator=(const AgentBridge&) = delete;
    AgentBridge(AgentBridge&&) = delete;
    AgentBridge& operator=(AgentBridge&&) = delete;

    [[nodiscard]] virtual ExchangeResult exchange(const Observation& observation,
                                                  std::chrono::milliseconds timeout) = 0;

    // Starts the configured AgentBridge Answer RPC. The bridge owns transport details; callers see
    // only engine-owned values and a cancellable stream.
    [[nodiscard]] virtual std::unique_ptr<AssistantResponseStream>
    startAssistantResponse(const AssistantRequest& request, std::chrono::milliseconds timeout) = 0;

protected:
    AgentBridge() = default;
};

} // namespace engine::bridge
