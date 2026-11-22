#include "engine/bridge/grpc_agent_bridge.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <utility>

#include "agent_bridge.grpc.pb.h"
#include "grpc/health/v1/health.grpc.pb.h"
#include <grpcpp/grpcpp.h>

namespace engine::bridge {
namespace {

namespace protocol = ::aegis::bridge::v1;

[[nodiscard]] std::chrono::system_clock::time_point
deadlineAfter(std::chrono::milliseconds timeout) {
    return std::chrono::system_clock::now() + timeout;
}

[[nodiscard]] const char* statusName(grpc::StatusCode code) {
    switch (code) {
    case grpc::StatusCode::CANCELLED:
        return "CANCELLED";
    case grpc::StatusCode::UNKNOWN:
        return "UNKNOWN";
    case grpc::StatusCode::INVALID_ARGUMENT:
        return "INVALID_ARGUMENT";
    case grpc::StatusCode::DEADLINE_EXCEEDED:
        return "DEADLINE_EXCEEDED";
    case grpc::StatusCode::NOT_FOUND:
        return "NOT_FOUND";
    case grpc::StatusCode::ALREADY_EXISTS:
        return "ALREADY_EXISTS";
    case grpc::StatusCode::PERMISSION_DENIED:
        return "PERMISSION_DENIED";
    case grpc::StatusCode::RESOURCE_EXHAUSTED:
        return "RESOURCE_EXHAUSTED";
    case grpc::StatusCode::FAILED_PRECONDITION:
        return "FAILED_PRECONDITION";
    case grpc::StatusCode::ABORTED:
        return "ABORTED";
    case grpc::StatusCode::OUT_OF_RANGE:
        return "OUT_OF_RANGE";
    case grpc::StatusCode::UNIMPLEMENTED:
        return "UNIMPLEMENTED";
    case grpc::StatusCode::INTERNAL:
        return "INTERNAL";
    case grpc::StatusCode::UNAVAILABLE:
        return "UNAVAILABLE";
    case grpc::StatusCode::DATA_LOSS:
        return "DATA_LOSS";
    case grpc::StatusCode::UNAUTHENTICATED:
        return "UNAUTHENTICATED";
    case grpc::StatusCode::OK:
        return "OK";
    case grpc::StatusCode::DO_NOT_USE:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

[[nodiscard]] ServiceFailure mapFailure(const grpc::Status& status) {
    if (status.ok()) {
        return {};
    }
    if (status.error_code() == grpc::StatusCode::UNAVAILABLE) {
        return {ServiceFailureKind::Unavailable, {}, status.error_message()};
    }
    if (status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED) {
        return {ServiceFailureKind::DeadlineExceeded, {}, status.error_message()};
    }
    return {ServiceFailureKind::RemoteStatus, statusName(status.error_code()),
            status.error_message()};
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
        : channel_(
              grpc::CreateChannel(std::move(serviceAddress), grpc::InsecureChannelCredentials()))
        , stub_(protocol::AgentBridge::NewStub(channel_))
        , healthStub_(::grpc::health::v1::Health::NewStub(channel_)) {}

    [[nodiscard]] HealthCheckResult checkHealth(std::chrono::milliseconds timeout) override {
        ::grpc::health::v1::HealthCheckRequest request;
        request.set_service("aegis.bridge.v1.AgentBridge");
        ::grpc::health::v1::HealthCheckResponse response;
        grpc::ClientContext context;
        context.set_deadline(deadlineAfter(timeout));
        const grpc::Status status = healthStub_->Check(&context, request, &response);
        if (!status.ok()) {
            return {HealthServingStatus::Unknown, mapFailure(status)};
        }
        if (response.status() == ::grpc::health::v1::HealthCheckResponse::SERVING) {
            return {HealthServingStatus::Serving, {}};
        }
        return {HealthServingStatus::NotServing, {}};
    }

    [[nodiscard]] ModelListResult listModels(std::chrono::milliseconds timeout) override {
        protocol::ListModelsRequest request;
        protocol::ListModelsResponse response;
        grpc::ClientContext context;
        context.set_deadline(deadlineAfter(timeout));
        const grpc::Status status = stub_->ListModels(&context, request, &response);
        if (!status.ok()) {
            return {{}, mapFailure(status)};
        }
        ModelListResult result;
        result.models.reserve(static_cast<std::size_t>(response.models_size()));
        for (const protocol::Model& model : response.models()) {
            result.models.push_back({model.identifier(), model.display_name()});
        }
        return result;
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
    std::shared_ptr<grpc::Channel> channel_;
    std::shared_ptr<protocol::AgentBridge::Stub> stub_;
    std::unique_ptr<::grpc::health::v1::Health::Stub> healthStub_;
};

} // namespace

std::unique_ptr<AgentBridge> makeGrpcAgentBridge(std::string serviceAddress) {
    return std::make_unique<GrpcAgentBridge>(std::move(serviceAddress));
}

} // namespace engine::bridge
