#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "agent_bridge.pb.h"
#include "benchmark_runner.hpp"
#include <zmq.h>

namespace benchmark_proto = aegis::bridge::benchmark;

namespace {

struct ContextDeleter {
    void operator()(void* context) const noexcept { zmq_ctx_term(context); }
};

struct SocketDeleter {
    void operator()(void* socket) const noexcept { zmq_close(socket); }
};

using Context = std::unique_ptr<void, ContextDeleter>;
using Socket = std::shared_ptr<void>;

std::string zmqError() {
    return std::string(zmq_strerror(errno));
}

} // namespace

int main(int argc, char** argv) {
    try {
        const bridge_benchmark::Configuration config =
            bridge_benchmark::parseArguments(argc, argv, "tcp://127.0.0.1:5555");
        Context context(zmq_ctx_new());
        if (!context) {
            throw std::runtime_error("zmq_ctx_new: " + zmqError());
        }
        return bridge_benchmark::run(
            "zeromq", config, [&config, rawContext = context.get()](std::size_t /*clientIndex*/) {
                Socket socket(zmq_socket(rawContext, ZMQ_REQ), SocketDeleter{});
                if (!socket) {
                    throw std::runtime_error("zmq_socket: " + zmqError());
                }
                const int timeout = config.timeoutMilliseconds;
                const int linger = 0;
                if (zmq_setsockopt(socket.get(), ZMQ_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
                    zmq_setsockopt(socket.get(), ZMQ_SNDTIMEO, &timeout, sizeof(timeout)) != 0 ||
                    zmq_setsockopt(socket.get(), ZMQ_LINGER, &linger, sizeof(linger)) != 0 ||
                    zmq_connect(socket.get(), config.endpoint.c_str()) != 0) {
                    throw std::runtime_error("ZeroMQ socket setup: " + zmqError());
                }
                return [socket](std::uint64_t gameInstanceId, std::uint64_t sequence,
                                const std::vector<std::byte>& payload) {
                    benchmark_proto::Observation request;
                    request.set_game_instance_id(gameInstanceId);
                    request.set_sequence(sequence);
                    request.set_payload(payload.data(), payload.size());
                    std::string encoded;
                    if (!request.SerializeToString(&encoded)) {
                        return bridge_benchmark::Reply{0, 0, 0, "protobuf serialization failed"};
                    }
                    if (zmq_send(socket.get(), encoded.data(), encoded.size(), 0) < 0) {
                        return bridge_benchmark::Reply{0, 0, 0, "send: " + zmqError()};
                    }

                    zmq_msg_t message;
                    if (zmq_msg_init(&message) != 0) {
                        return bridge_benchmark::Reply{0, 0, 0, "message init: " + zmqError()};
                    }
                    const int received = zmq_msg_recv(&message, socket.get(), 0);
                    if (received < 0) {
                        const std::string error = "receive: " + zmqError();
                        zmq_msg_close(&message);
                        return bridge_benchmark::Reply{0, 0, 0, error};
                    }
                    benchmark_proto::Action response;
                    const bool parsed = response.ParseFromArray(zmq_msg_data(&message), received);
                    zmq_msg_close(&message);
                    if (!parsed) {
                        return bridge_benchmark::Reply{0, 0, 0, "protobuf parse failed"};
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
