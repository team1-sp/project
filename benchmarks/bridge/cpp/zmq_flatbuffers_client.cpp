#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "agent_bridge_generated.h"
#include "benchmark_runner.hpp"
#include <flatbuffers/flatbuffers.h>
#include <zmq.hpp>

namespace benchmark_flat = Aegis::Bridge::Benchmark;

int main(int argc, char** argv) {
    try {
        const bridge_benchmark::Configuration config =
            bridge_benchmark::parseArguments(argc, argv, "tcp://127.0.0.1:5556");
        zmq::context_t context(1);
        return bridge_benchmark::run(
            "zeromq-flatbuffers", config, [&config, &context](std::size_t /*clientIndex*/) {
                auto socket = std::make_shared<zmq::socket_t>(context, zmq::socket_type::req);
                socket->set(zmq::sockopt::rcvtimeo, config.timeoutMilliseconds);
                socket->set(zmq::sockopt::sndtimeo, config.timeoutMilliseconds);
                socket->set(zmq::sockopt::linger, 0);
                socket->connect(config.endpoint);
                return [socket](std::uint64_t gameInstanceId, std::uint64_t sequence,
                                const std::vector<std::byte>& payload) {
                    try {
                        flatbuffers::FlatBufferBuilder builder(payload.size() + 64);
                        const auto encodedPayload = builder.CreateVector(
                            reinterpret_cast<const std::uint8_t*>(payload.data()), payload.size());
                        const auto request = benchmark_flat::CreateObservation(
                            builder, gameInstanceId, sequence, encodedPayload);
                        builder.Finish(request);
                        const auto sent =
                            socket->send(zmq::buffer(builder.GetBufferPointer(), builder.GetSize()),
                                         zmq::send_flags::none);
                        if (!sent) {
                            return bridge_benchmark::Reply{0, 0, 0, "send timed out"};
                        }

                        zmq::message_t message;
                        const auto received = socket->recv(message, zmq::recv_flags::none);
                        if (!received) {
                            return bridge_benchmark::Reply{0, 0, 0, "receive timed out"};
                        }
                        flatbuffers::Verifier verifier(
                            static_cast<const std::uint8_t*>(message.data()), message.size());
                        if (!verifier.VerifyBuffer<benchmark_flat::Action>(nullptr)) {
                            return bridge_benchmark::Reply{0, 0, 0,
                                                           "FlatBuffers verification failed"};
                        }
                        const auto* response =
                            flatbuffers::GetRoot<benchmark_flat::Action>(message.data());
                        return bridge_benchmark::Reply{response->game_instance_id(),
                                                       response->sequence(),
                                                       response->action_id(),
                                                       {}};
                    } catch (const zmq::error_t& error) {
                        return bridge_benchmark::Reply{0, 0, 0, error.what()};
                    }
                };
            });
    } catch (const std::exception& exception) {
        std::cerr << "configuration error: " << exception.what() << '\n';
        return 1;
    }
}
