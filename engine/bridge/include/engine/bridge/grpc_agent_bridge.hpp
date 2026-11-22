#pragma once

#include <memory>
#include <string>

#include "engine/bridge/agent_bridge.hpp"

namespace engine::bridge {

// Creates the production gRPC/protobuf AgentBridge for a validated host:port service address.
// Transport dependencies remain in the optional engine_bridge_grpc target (DEC-038).
[[nodiscard]] std::unique_ptr<AgentBridge> makeGrpcAgentBridge(std::string serviceAddress);

} // namespace engine::bridge
