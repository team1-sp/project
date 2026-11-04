#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
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

// Synchronous seam used by one game instance. Implementations own connection and retry policy.
class AgentBridge {
public:
    virtual ~AgentBridge() = default;

    AgentBridge(const AgentBridge&) = delete;
    AgentBridge& operator=(const AgentBridge&) = delete;
    AgentBridge(AgentBridge&&) = delete;
    AgentBridge& operator=(AgentBridge&&) = delete;

    [[nodiscard]] virtual ExchangeResult exchange(const Observation& observation,
                                                  std::chrono::milliseconds timeout) = 0;

protected:
    AgentBridge() = default;
};

} // namespace engine::bridge
