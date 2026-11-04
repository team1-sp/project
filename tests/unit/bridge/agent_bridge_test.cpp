#include <chrono>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "engine/bridge/agent_bridge.hpp"

namespace {

class EchoBridge final : public engine::bridge::AgentBridge {
public:
    [[nodiscard]] engine::bridge::ExchangeResult
    exchange(const engine::bridge::Observation& observation,
             std::chrono::milliseconds /*timeout*/) override {
        return {{observation.gameInstanceId, observation.sequence, 7}, {}};
    }
};

} // namespace

TEST(AgentBridge, Exchange_PreservesRequestAssociation) {
    const std::vector<std::byte> payload(16, std::byte{0x2A});
    const engine::bridge::Observation observation{42, 99, payload};
    EchoBridge bridge;

    const engine::bridge::ExchangeResult result =
        bridge.exchange(observation, std::chrono::milliseconds{100});

    ASSERT_TRUE(result.succeeded());
    EXPECT_EQ(result.action.gameInstanceId, observation.gameInstanceId);
    EXPECT_EQ(result.action.sequence, observation.sequence);
    EXPECT_EQ(result.action.actionId, 7);
}
