#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "aegis/assistant/read_only_assistant.hpp"
#include "engine/bridge/agent_bridge.hpp"

namespace editor {

inline constexpr std::chrono::milliseconds aiServiceDeadline{5'000};

enum class ProjectDataCategory { SourceCode, Telemetry };

// Per-user, per-project AI settings. It implements the read-only policy seam consumed by the
// assistant and exposes the consent decision that US-33.4/#120 will use before sending.
class AiPrivacySettings final : public aegis::assistant::DataTransmissionPolicy {
public:
    std::string serviceAddress;
    std::string modelIdentifier;
    bool allowSourceCode = false;
    bool allowTelemetry = false;
    bool askBeforeEverySend = false;

    [[nodiscard]] bool allows(std::string_view dataCategory) const override;
    [[nodiscard]] bool requiresConsent(ProjectDataCategory category) const noexcept;
};

[[nodiscard]] bool isValidServiceAddress(std::string_view address);

struct AiSettingsStoreResult {
    std::string error;
    [[nodiscard]] bool succeeded() const noexcept { return error.empty(); }
};

struct AiSettingsLoadResult {
    AiPrivacySettings settings;
    std::string error;
    [[nodiscard]] bool succeeded() const noexcept { return error.empty(); }
};

// Versioned persistence under <user-local-project>/settings/ai-privacy.settings. The caller
// supplies the user-local project area so this store cannot select shared project settings.
class AiPrivacySettingsStore {
public:
    explicit AiPrivacySettingsStore(std::filesystem::path userLocalProjectArea,
                                    AiPrivacySettings defaults = {});

    [[nodiscard]] AiSettingsStoreResult save(const AiPrivacySettings& settings) const;
    [[nodiscard]] AiSettingsLoadResult load() const;
    [[nodiscard]] std::filesystem::path filePath() const;

private:
    std::filesystem::path userLocalProjectArea_;
    AiPrivacySettings defaults_;
};

enum class AiServiceRequestKind { Health, Models };

struct AiServiceResponse {
    std::uint64_t requestId = 0;
    AiServiceRequestKind kind = AiServiceRequestKind::Models;
    std::string address;
    std::chrono::milliseconds latency{0};
    engine::bridge::HealthCheckResult health;
    engine::bridge::ModelListResult models;
};

// Asynchronous seam: production work happens off the UI thread; tests enqueue completions in any
// order to prove stale-address responses are ignored.
class IAiServiceRequestRunner {
public:
    virtual ~IAiServiceRequestRunner() = default;

    virtual void requestHealth(std::uint64_t requestId, std::string address,
                               std::chrono::milliseconds timeout) = 0;
    virtual void requestModels(std::uint64_t requestId, std::string address,
                               std::chrono::milliseconds timeout) = 0;
    [[nodiscard]] virtual std::vector<AiServiceResponse> takeCompleted() = 0;
};

// Adapter from the UI-facing asynchronous seam to #72's address-bound AgentBridge abstraction.
// The factory is injectable, so editor_core does not link gRPC and unit tests need no network.
class AgentBridgeRequestRunner final : public IAiServiceRequestRunner {
public:
    using BridgeFactory = std::function<std::unique_ptr<engine::bridge::AgentBridge>(std::string)>;
    using Clock = std::function<std::chrono::steady_clock::time_point()>;

    explicit AgentBridgeRequestRunner(BridgeFactory bridgeFactory,
                                      Clock clock = std::chrono::steady_clock::now);
    ~AgentBridgeRequestRunner() override;

    void requestHealth(std::uint64_t requestId, std::string address,
                       std::chrono::milliseconds timeout) override;
    void requestModels(std::uint64_t requestId, std::string address,
                       std::chrono::milliseconds timeout) override;
    [[nodiscard]] std::vector<AiServiceResponse> takeCompleted() override;

private:
    BridgeFactory bridgeFactory_;
    Clock clock_;
    std::vector<std::future<AiServiceResponse>> pending_;
};

enum class ModelListState { NotRequested, Loading, Available, Empty, Failed };
enum class ConnectionState { Offline, Checking, Connected, Failed };
enum class PrivacyConfirmation { None, SourceCode, Telemetry };

// Window-independent controller for validation, persistence and service-response transitions.
class AiPrivacySettingsController {
public:
    AiPrivacySettingsController(AiPrivacySettingsStore& store, IAiServiceRequestRunner& requests);

    [[nodiscard]] const AiPrivacySettings& settings() const noexcept { return settings_; }
    [[nodiscard]] const std::string& addressDraft() const noexcept { return addressDraft_; }
    void setAddressDraft(std::string address);
    [[nodiscard]] bool commitAddressOnFocusLoss();
    [[nodiscard]] const std::string& addressError() const noexcept { return addressError_; }

    void openTab();
    void testConnection();
    void poll();

    [[nodiscard]] ModelListState modelListState() const noexcept { return modelListState_; }
    [[nodiscard]] const std::vector<engine::bridge::AvailableModel>& models() const noexcept {
        return models_;
    }
    [[nodiscard]] const std::string& modelsError() const noexcept { return modelsError_; }
    [[nodiscard]] std::string modelFieldText() const;
    [[nodiscard]] bool selectModel(std::string_view identifier);
    [[nodiscard]] bool sendingEnabled() const noexcept;
    [[nodiscard]] std::string_view sendDisabledReason() const noexcept;

    [[nodiscard]] ConnectionState connectionState() const noexcept { return connectionState_; }
    [[nodiscard]] const std::string& connectionText() const noexcept { return connectionText_; }
    [[nodiscard]] std::chrono::milliseconds connectedLatency() const noexcept {
        return connectedLatency_;
    }

    void requestAllow(ProjectDataCategory category, bool allow);
    [[nodiscard]] PrivacyConfirmation pendingConfirmation() const noexcept {
        return pendingConfirmation_;
    }
    [[nodiscard]] std::string confirmationText() const;
    void confirmPrivacyChange();
    void cancelPrivacyChange();
    void setAskBeforeEverySend(bool enabled);

    [[nodiscard]] const std::string& persistenceError() const noexcept { return persistenceError_; }

private:
    void requestModels();
    void applyHealth(const AiServiceResponse& response);
    void applyModels(const AiServiceResponse& response);
    void persist();
    [[nodiscard]] std::string failureText(const engine::bridge::ServiceFailure& failure) const;

    AiPrivacySettingsStore& store_;
    IAiServiceRequestRunner& requests_;
    AiPrivacySettings settings_;
    std::string addressDraft_;
    std::string addressError_;
    std::vector<engine::bridge::AvailableModel> models_;
    ModelListState modelListState_ = ModelListState::NotRequested;
    std::string modelsError_;
    ConnectionState connectionState_ = ConnectionState::Offline;
    std::string connectionText_;
    std::chrono::milliseconds connectedLatency_{0};
    PrivacyConfirmation pendingConfirmation_ = PrivacyConfirmation::None;
    std::string persistenceError_;
    std::uint64_t nextRequestId_ = 1;
    std::optional<std::uint64_t> healthRequestId_;
    std::optional<std::uint64_t> modelsRequestId_;
};

} // namespace editor
