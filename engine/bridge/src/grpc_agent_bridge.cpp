#include "engine/bridge/grpc_agent_bridge.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <utility>

#include "agent_bridge.grpc.pb.h"
#include <grpcpp/grpcpp.h>

namespace engine::bridge {
namespace {

namespace protocol = ::aegis::bridge::v1;

[[nodiscard]] std::chrono::system_clock::time_point
deadlineAfter(std::chrono::milliseconds timeout) {
    return std::chrono::system_clock::now() + timeout;
}

class GrpcAssistantResponseStream final : public AssistantResponseStream {
public:
    GrpcAssistantResponseStream(std::shared_ptr<protocol::AgentBridge::Stub> stub,
                                const AssistantRequest& request, std::chrono::milliseconds timeout)
        : stub_(std::move(stub)) {
        request_.set_request_id(request.requestId);
        request_.set_model_id(request.modelId);
        request_.set_question(request.question);
        request_.set_semantic_graph_structure(request.semanticGraphStructure);
        for (const std::string& category : request.dataCategories) {
            request_.add_data_categories(category);
        }
        context_.set_deadline(deadlineAfter(timeout));
    }

    [[nodiscard]] AssistantStreamResult consume(const ChunkConsumer& consumer) override {
        if (cancelled_.load()) {
            return {AssistantStreamOutcome::Cancelled, {}};
        }

        std::unique_ptr<grpc::ClientReader<protocol::AssistantChunk>> reader =
            stub_->Answer(&context_, request_);
        protocol::AssistantChunk protocolChunk;
        while (reader->Read(&protocolChunk)) {
            AssistantChunk chunk;
            chunk.text = protocolChunk.text();
            for (const protocol::Citation& citation : protocolChunk.citations()) {
                chunk.citations.push_back({citation.kind(), citation.payload()});
            }
            for (const std::string& category : protocolChunk.data_categories()) {
                chunk.dataCategories.push_back(category);
            }
            if (!consumer(chunk)) {
                cancel();
                break;
            }
        }

        const grpc::Status status = reader->Finish();
        if (cancelled_.load() || status.error_code() == grpc::StatusCode::CANCELLED) {
            return {AssistantStreamOutcome::Cancelled, {}};
        }
        if (!status.ok()) {
            return {AssistantStreamOutcome::Failed, status.error_message()};
        }
        return {AssistantStreamOutcome::Completed, {}};
    }

    void cancel() noexcept override {
        if (!cancelled_.exchange(true)) {
            context_.TryCancel();
        }
    }

private:
    std::shared_ptr<protocol::AgentBridge::Stub> stub_;
    protocol::AssistantRequest request_;
    grpc::ClientContext context_;
    std::atomic_bool cancelled_ = false;
};

class GrpcAgentBridge final : public AgentBridge {
public:
    explicit GrpcAgentBridge(std::string serviceAddress)
        : stub_(protocol::AgentBridge::NewStub(
              grpc::CreateChannel(std::move(serviceAddress), grpc::InsecureChannelCredentials()))) {
    }

    [[nodiscard]] ExchangeResult exchange(const Observation& observation,
                                          std::chrono::milliseconds timeout) override {
        protocol::Observation request;
        request.set_game_instance_id(observation.gameInstanceId);
        request.set_sequence(observation.sequence);
        request.set_payload(observation.payload.data(), observation.payload.size());

        protocol::Action response;
        grpc::ClientContext context;
        context.set_deadline(deadlineAfter(timeout));
        const grpc::Status status = stub_->Exchange(&context, request, &response);
        if (!status.ok()) {
            return {{}, status.error_message()};
        }
        return {{response.game_instance_id(), response.sequence(), response.action_id()}, {}};
    }

    [[nodiscard]] std::unique_ptr<AssistantResponseStream>
    startAssistantResponse(const AssistantRequest& request,
                           std::chrono::milliseconds timeout) override {
        return std::make_unique<GrpcAssistantResponseStream>(stub_, request, timeout);
    }

private:
    std::shared_ptr<protocol::AgentBridge::Stub> stub_;
};

} // namespace

std::unique_ptr<AgentBridge> makeGrpcAgentBridge(std::string serviceAddress) {
    return std::make_unique<GrpcAgentBridge>(std::move(serviceAddress));
}

} // namespace engine::bridge
