#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "agent_bridge.grpc.pb.h"
#include "benchmark_runner.hpp"
#include <grpcpp/grpcpp.h>

namespace benchmark_proto = aegis::bridge::benchmark;

int main(int argc, char** argv) {
    try {
        const bridge_benchmark::Configuration config =
            bridge_benchmark::parseArguments(argc, argv, "127.0.0.1:50051");
        return bridge_benchmark::run("grpc", config, [&config](std::size_t /*clientIndex*/) {
            auto channel = grpc::CreateChannel(config.endpoint, grpc::InsecureChannelCredentials());
            auto stub = std::shared_ptr<benchmark_proto::AgentService::Stub>(
                benchmark_proto::AgentService::NewStub(channel));
            return [stub, timeout = config.timeoutMilliseconds](
                       std::uint64_t gameInstanceId, std::uint64_t sequence,
                       const std::vector<std::byte>& payload) {
                benchmark_proto::Observation request;
                request.set_game_instance_id(gameInstanceId);
                request.set_sequence(sequence);
                request.set_payload(payload.data(), payload.size());
                benchmark_proto::Action response;
                grpc::ClientContext context;
                context.set_deadline(std::chrono::system_clock::now() +
                                     std::chrono::milliseconds(timeout));
                const grpc::Status status = stub->Exchange(&context, request, &response);
                if (!status.ok()) {
                    return bridge_benchmark::Reply{0, 0, 0,
                                                   status.error_message() + " (" +
                                                       std::to_string(status.error_code()) + ")"};
                }
                return bridge_benchmark::Reply{
                    response.game_instance_id(), response.sequence(), response.action_id(), {}};
            };
        });
    } catch (const std::exception& exception) {
        std::cerr << "configuration error: " << exception.what() << '\n';
        return 1;
    }
}
