#include "editor/ai_privacy_settings.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <limits>
#include <system_error>
#include <utility>

namespace editor {
namespace {

constexpr std::string_view settingsHeader = "AEGIS_AI_PRIVACY 1";
constexpr std::uintmax_t maximumSettingsBytes = 64U * 1024U;

[[nodiscard]] bool parsePort(std::string_view text) {
    if (text.empty()) {
        return false;
    }
    unsigned int port = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), port);
    return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size() && port >= 1U &&
           port <= 65'535U;
}

[[nodiscard]] bool validIpv4(std::string_view text) {
    int parts = 0;
    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t end = text.find('.', start);
        const std::string_view part =
            text.substr(start, end == std::string_view::npos ? text.size() - start : end - start);
        if (part.empty() || part.size() > 3) {
            return false;
        }
        unsigned int value = 0;
        const auto parsed = std::from_chars(part.data(), part.data() + part.size(), value);
        if (parsed.ec != std::errc{} || parsed.ptr != part.data() + part.size() || value > 255U) {
            return false;
        }
        ++parts;
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return parts == 4;
}

[[nodiscard]] bool validHostname(std::string_view text) {
    if (text.empty() || text.size() > 253 || text.front() == '.' || text.back() == '.') {
        return false;
    }
    std::size_t labelStart = 0;
    while (labelStart < text.size()) {
        const std::size_t labelEnd = text.find('.', labelStart);
        const std::string_view label =
            text.substr(labelStart, labelEnd == std::string_view::npos ? text.size() - labelStart
                                                                       : labelEnd - labelStart);
        if (label.empty() || label.size() > 63 || label.front() == '-' || label.back() == '-') {
            return false;
        }
        for (const char rawCharacter : label) {
            const auto character = static_cast<unsigned char>(rawCharacter);
            if (!std::isalnum(character) && character != '-') {
                return false;
            }
        }
        if (labelEnd == std::string_view::npos) {
            break;
        }
        labelStart = labelEnd + 1;
    }
    return true;
}

[[nodiscard]] int ipv6GroupCount(std::string_view part, bool allowIpv4) {
    if (part.empty()) {
        return 0;
    }
    int groups = 0;
    std::size_t start = 0;
    while (start <= part.size()) {
        const std::size_t end = part.find(':', start);
        const std::string_view group =
            part.substr(start, end == std::string_view::npos ? part.size() - start : end - start);
        if (group.empty()) {
            return -1;
        }
        if (group.find('.') != std::string_view::npos) {
            if (!allowIpv4 || end != std::string_view::npos || !validIpv4(group)) {
                return -1;
            }
            groups += 2;
        } else {
            if (group.size() > 4) {
                return -1;
            }
            for (const char rawCharacter : group) {
                const auto character = static_cast<unsigned char>(rawCharacter);
                if (!std::isxdigit(character)) {
                    return -1;
                }
            }
            ++groups;
        }
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return groups;
}

[[nodiscard]] bool validIpv6(std::string_view text) {
    if (text.empty()) {
        return false;
    }
    const std::size_t compressed = text.find("::");
    if (compressed != std::string_view::npos &&
        text.find("::", compressed + 2) != std::string_view::npos) {
        return false;
    }
    if (compressed == std::string_view::npos) {
        return ipv6GroupCount(text, true) == 8;
    }
    const int left = ipv6GroupCount(text.substr(0, compressed), false);
    const int right = ipv6GroupCount(text.substr(compressed + 2), true);
    return left >= 0 && right >= 0 && left + right < 8;
}

[[nodiscard]] bool looksLikeIpv4(std::string_view host) {
    return std::all_of(host.begin(), host.end(), [](const unsigned char character) {
        return std::isdigit(character) || character == '.';
    });
}

[[nodiscard]] bool readBoolean(std::istream& input, std::string_view name, bool& value) {
    std::string field;
    int raw = -1;
    if (!(input >> field >> raw) || field != name || (raw != 0 && raw != 1)) {
        return false;
    }
    value = raw == 1;
    return true;
}

[[nodiscard]] bool writeSettingsFile(const std::filesystem::path& path,
                                     const AiPrivacySettings& settings) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }
    output << settingsHeader << '\n';
    output << "service_address " << std::quoted(settings.serviceAddress) << '\n';
    output << "model_identifier " << std::quoted(settings.modelIdentifier) << '\n';
    output << "allow_source_code " << (settings.allowSourceCode ? 1 : 0) << '\n';
    output << "allow_telemetry " << (settings.allowTelemetry ? 1 : 0) << '\n';
    output << "ask_before_every_send " << (settings.askBeforeEverySend ? 1 : 0) << '\n';
    output.flush();
    return static_cast<bool>(output);
}

} // namespace

bool AiPrivacySettings::allows(std::string_view dataCategory) const {
    if (dataCategory == "source-code") {
        return allowSourceCode;
    }
    if (dataCategory == "telemetry") {
        return allowTelemetry;
    }
    // US-33.5 gates only these two classified project-data categories. Question text, graph
    // structure with no classified fields and future non-project categories remain usable.
    return true;
}

bool AiPrivacySettings::requiresConsent(ProjectDataCategory category) const noexcept {
    const bool allowed =
        category == ProjectDataCategory::SourceCode ? allowSourceCode : allowTelemetry;
    return askBeforeEverySend || !allowed;
}

bool isValidServiceAddress(std::string_view address) {
    std::string_view host;
    std::string_view port;
    if (address.starts_with('[')) {
        const std::size_t close = address.find(']');
        if (close == std::string_view::npos || close + 1 >= address.size() ||
            address[close + 1] != ':') {
            return false;
        }
        host = address.substr(1, close - 1);
        port = address.substr(close + 2);
        return validIpv6(host) && parsePort(port);
    }

    const std::size_t colon = address.find(':');
    if (colon == std::string_view::npos || colon == 0 ||
        address.find(':', colon + 1) != std::string_view::npos) {
        return false;
    }
    host = address.substr(0, colon);
    port = address.substr(colon + 1);
    const bool hostValid = looksLikeIpv4(host) ? validIpv4(host) : validHostname(host);
    return hostValid && parsePort(port);
}

AiPrivacySettingsStore::AiPrivacySettingsStore(std::filesystem::path userLocalProjectArea,
                                               AiPrivacySettings defaults)
    : userLocalProjectArea_(std::move(userLocalProjectArea))
    , defaults_(std::move(defaults)) {
}

std::filesystem::path AiPrivacySettingsStore::filePath() const {
    return userLocalProjectArea_ / "settings" / "ai-privacy.settings";
}

AiSettingsStoreResult AiPrivacySettingsStore::save(const AiPrivacySettings& settings) const {
    if (!settings.serviceAddress.empty() && !isValidServiceAddress(settings.serviceAddress)) {
        return {"refusing to store an invalid AI service address"};
    }
    const std::filesystem::path target = filePath();
    const std::filesystem::path temporary = target.string() + ".tmp";
    const std::filesystem::path backup = target.string() + ".bak";
    std::error_code error;
    std::filesystem::create_directories(target.parent_path(), error);
    if (error) {
        return {"could not create user-local settings directory: " + error.message()};
    }
    if (!writeSettingsFile(temporary, settings)) {
        std::filesystem::remove(temporary, error);
        return {"could not write complete AI privacy settings"};
    }

    const bool hadTarget = std::filesystem::exists(target, error) && !error;
    if (hadTarget) {
        std::filesystem::remove(backup, error);
        error.clear();
        std::filesystem::rename(target, backup, error);
        if (error) {
            std::filesystem::remove(temporary, error);
            return {"could not back up existing AI privacy settings"};
        }
    }
    error.clear();
    std::filesystem::rename(temporary, target, error);
    if (error) {
        if (hadTarget) {
            std::error_code restoreError;
            std::filesystem::rename(backup, target, restoreError);
        }
        std::filesystem::remove(temporary, error);
        return {"could not replace AI privacy settings"};
    }
    if (hadTarget) {
        std::filesystem::remove(backup, error);
    }
    return {};
}

AiSettingsLoadResult AiPrivacySettingsStore::load() const {
    const std::filesystem::path path = filePath();
    std::error_code error;
    if (!std::filesystem::exists(path, error)) {
        return error ? AiSettingsLoadResult{defaults_, "could not inspect AI privacy settings: " +
                                                           error.message()}
                     : AiSettingsLoadResult{defaults_, {}};
    }
    const std::uintmax_t size = std::filesystem::file_size(path, error);
    if (error || size > maximumSettingsBytes) {
        return {defaults_, error ? "could not inspect AI privacy settings: " + error.message()
                                 : "AI privacy settings file is too large"};
    }

    std::ifstream input(path, std::ios::binary);
    std::string header;
    AiPrivacySettings settings;
    std::string field;
    if (!input || !std::getline(input, header) || header != settingsHeader ||
        !(input >> field >> std::quoted(settings.serviceAddress)) || field != "service_address" ||
        !(input >> field >> std::quoted(settings.modelIdentifier)) || field != "model_identifier" ||
        !readBoolean(input, "allow_source_code", settings.allowSourceCode) ||
        !readBoolean(input, "allow_telemetry", settings.allowTelemetry) ||
        !readBoolean(input, "ask_before_every_send", settings.askBeforeEverySend)) {
        return {defaults_, "invalid AI privacy settings file"};
    }
    input >> std::ws;
    if (!input.eof()) {
        return {defaults_, "unexpected data after AI privacy settings"};
    }
    if (!settings.serviceAddress.empty() && !isValidServiceAddress(settings.serviceAddress)) {
        return {defaults_, "stored AI service address is invalid"};
    }
    return {std::move(settings), {}};
}

AgentBridgeRequestRunner::AgentBridgeRequestRunner(BridgeFactory bridgeFactory, Clock clock)
    : bridgeFactory_(std::move(bridgeFactory))
    , clock_(std::move(clock)) {
}

AgentBridgeRequestRunner::~AgentBridgeRequestRunner() = default;

void AgentBridgeRequestRunner::requestHealth(std::uint64_t requestId, std::string address,
                                             std::chrono::milliseconds timeout) {
    pending_.push_back(
        std::async(std::launch::async, [factory = bridgeFactory_, clock = clock_, requestId,
                                        address = std::move(address), timeout]() mutable {
            const auto started = clock();
            std::unique_ptr<engine::bridge::AgentBridge> bridge = factory(address);
            engine::bridge::HealthCheckResult result;
            if (bridge) {
                result = bridge->checkHealth(timeout);
            } else {
                result.failure = {engine::bridge::ServiceFailureKind::Unavailable,
                                  {},
                                  "AI service bridge is unavailable"};
            }
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(clock() - started);
            return AiServiceResponse{requestId,          AiServiceRequestKind::Health,
                                     std::move(address), elapsed,
                                     std::move(result),  {}};
        }));
}

void AgentBridgeRequestRunner::requestModels(std::uint64_t requestId, std::string address,
                                             std::chrono::milliseconds timeout) {
    pending_.push_back(
        std::async(std::launch::async, [factory = bridgeFactory_, clock = clock_, requestId,
                                        address = std::move(address), timeout]() mutable {
            const auto started = clock();
            std::unique_ptr<engine::bridge::AgentBridge> bridge = factory(address);
            engine::bridge::ModelListResult result;
            if (bridge) {
                result = bridge->listModels(timeout);
            } else {
                result.failure = {engine::bridge::ServiceFailureKind::Unavailable,
                                  {},
                                  "AI service bridge is unavailable"};
            }
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(clock() - started);
            return AiServiceResponse{
                requestId,        AiServiceRequestKind::Models, std::move(address), elapsed, {},
                std::move(result)};
        }));
}

std::vector<AiServiceResponse> AgentBridgeRequestRunner::takeCompleted() {
    std::vector<AiServiceResponse> completed;
    auto pending = pending_.begin();
    while (pending != pending_.end()) {
        if (pending->wait_for(std::chrono::milliseconds{0}) == std::future_status::ready) {
            completed.push_back(pending->get());
            pending = pending_.erase(pending);
        } else {
            ++pending;
        }
    }
    return completed;
}

AiPrivacySettingsController::AiPrivacySettingsController(AiPrivacySettingsStore& store,
                                                         IAiServiceRequestRunner& requests)
    : store_(store)
    , requests_(requests) {
    AiSettingsLoadResult loaded = store_.load();
    settings_ = std::move(loaded.settings);
    addressDraft_ = settings_.serviceAddress;
    persistenceError_ = std::move(loaded.error);
}

void AiPrivacySettingsController::setAddressDraft(std::string address) {
    addressDraft_ = std::move(address);
}

bool AiPrivacySettingsController::commitAddressOnFocusLoss() {
    if (!isValidServiceAddress(addressDraft_)) {
        addressError_ = "Address must be host:port";
        return false;
    }
    addressError_.clear();
    if (addressDraft_ == settings_.serviceAddress) {
        return true;
    }
    settings_.serviceAddress = addressDraft_;
    settings_.modelIdentifier.clear();
    settings_.allowSourceCode = false;
    settings_.allowTelemetry = false;
    models_.clear();
    modelListState_ = ModelListState::NotRequested;
    modelsError_.clear();
    connectionState_ = ConnectionState::Offline;
    connectionText_.clear();
    healthRequestId_.reset();
    modelsRequestId_.reset();
    persist();
    return true;
}

void AiPrivacySettingsController::openTab() {
    requestModels();
}

void AiPrivacySettingsController::testConnection() {
    if (addressDraft_ != settings_.serviceAddress) {
        if (!commitAddressOnFocusLoss()) {
            connectionState_ = ConnectionState::Failed;
            connectionText_ = "Address not valid";
            healthRequestId_.reset();
            return;
        }
    }
    if (!isValidServiceAddress(settings_.serviceAddress)) {
        connectionState_ = ConnectionState::Failed;
        connectionText_ = "Address not valid";
        healthRequestId_.reset();
        return;
    }
    const std::uint64_t requestId = nextRequestId_++;
    healthRequestId_ = requestId;
    connectionState_ = ConnectionState::Checking;
    connectionText_.clear();
    requests_.requestHealth(requestId, settings_.serviceAddress, aiServiceDeadline);
}

void AiPrivacySettingsController::requestModels() {
    if (!isValidServiceAddress(settings_.serviceAddress)) {
        modelListState_ = ModelListState::Failed;
        modelsError_ = "Models unavailable: Address not valid";
        modelsRequestId_.reset();
        return;
    }
    const std::uint64_t requestId = nextRequestId_++;
    modelsRequestId_ = requestId;
    modelListState_ = ModelListState::Loading;
    modelsError_.clear();
    requests_.requestModels(requestId, settings_.serviceAddress, aiServiceDeadline);
}

void AiPrivacySettingsController::poll() {
    for (const AiServiceResponse& response : requests_.takeCompleted()) {
        if (response.address != settings_.serviceAddress) {
            continue;
        }
        if (response.kind == AiServiceRequestKind::Health) {
            if (healthRequestId_ && response.requestId == *healthRequestId_) {
                applyHealth(response);
            }
        } else if (modelsRequestId_ && response.requestId == *modelsRequestId_) {
            applyModels(response);
        }
    }
}

std::string
AiPrivacySettingsController::failureText(const engine::bridge::ServiceFailure& failure) const {
    using engine::bridge::ServiceFailureKind;
    switch (failure.kind) {
    case ServiceFailureKind::None:
        return {};
    case ServiceFailureKind::Unavailable:
        return "Service not reachable";
    case ServiceFailureKind::DeadlineExceeded:
        return "Deadline exceeded";
    case ServiceFailureKind::RemoteStatus:
        return failure.status + (failure.message.empty() ? std::string() : ": " + failure.message);
    }
    return "Unknown service failure";
}

void AiPrivacySettingsController::applyHealth(const AiServiceResponse& response) {
    healthRequestId_.reset();
    if (response.health.failure.failed()) {
        connectionState_ = ConnectionState::Failed;
        connectionText_ = failureText(response.health.failure);
        return;
    }
    if (response.health.status != engine::bridge::HealthServingStatus::Serving) {
        connectionState_ = ConnectionState::Failed;
        connectionText_ = "Service not serving";
        return;
    }
    connectionState_ = ConnectionState::Connected;
    connectedLatency_ = response.latency;
    connectionText_ = "Connected \xC2\xB7 " + std::to_string(connectedLatency_.count()) + " ms";
    requestModels();
}

void AiPrivacySettingsController::applyModels(const AiServiceResponse& response) {
    modelsRequestId_.reset();
    if (response.models.failure.failed()) {
        modelListState_ = ModelListState::Failed;
        modelsError_ = "Models unavailable: " + failureText(response.models.failure);
        return;
    }
    models_ = response.models.models;
    modelListState_ = models_.empty() ? ModelListState::Empty : ModelListState::Available;
    modelsError_.clear();
    const auto selected = std::find_if(models_.begin(), models_.end(), [this](const auto& model) {
        return model.identifier == settings_.modelIdentifier;
    });
    if (!settings_.modelIdentifier.empty() && selected == models_.end()) {
        settings_.modelIdentifier.clear();
        persist();
    }
}

std::string AiPrivacySettingsController::modelFieldText() const {
    if (!settings_.modelIdentifier.empty()) {
        const auto selected =
            std::find_if(models_.begin(), models_.end(), [this](const auto& model) {
                return model.identifier == settings_.modelIdentifier;
            });
        if (selected != models_.end()) {
            return selected->displayName;
        }
        return settings_.modelIdentifier;
    }
    return modelListState_ == ModelListState::Empty ? "No models available" : "Select a model";
}

bool AiPrivacySettingsController::selectModel(std::string_view identifier) {
    const auto model = std::find_if(models_.begin(), models_.end(), [identifier](const auto& item) {
        return item.identifier == identifier;
    });
    if (model == models_.end()) {
        return false;
    }
    settings_.modelIdentifier = model->identifier;
    persist();
    return true;
}

bool AiPrivacySettingsController::sendingEnabled() const noexcept {
    return !settings_.modelIdentifier.empty();
}

std::string_view AiPrivacySettingsController::sendDisabledReason() const noexcept {
    return sendingEnabled() ? std::string_view{} : std::string_view{"Select a model"};
}

void AiPrivacySettingsController::requestAllow(ProjectDataCategory category, bool allow) {
    bool& value = category == ProjectDataCategory::SourceCode ? settings_.allowSourceCode
                                                              : settings_.allowTelemetry;
    if (!allow) {
        value = false;
        pendingConfirmation_ = PrivacyConfirmation::None;
        persist();
        return;
    }
    if (value) {
        return;
    }
    pendingConfirmation_ = category == ProjectDataCategory::SourceCode
                               ? PrivacyConfirmation::SourceCode
                               : PrivacyConfirmation::Telemetry;
}

std::string AiPrivacySettingsController::confirmationText() const {
    if (pendingConfirmation_ == PrivacyConfirmation::SourceCode) {
        return "Allow sending source code names, paths, line references and file contents to the "
               "configured AI service?";
    }
    if (pendingConfirmation_ == PrivacyConfirmation::Telemetry) {
        return "Allow sending run reports, tick ranges, events and telemetry samples to the "
               "configured AI service?";
    }
    return {};
}

void AiPrivacySettingsController::confirmPrivacyChange() {
    if (pendingConfirmation_ == PrivacyConfirmation::SourceCode) {
        settings_.allowSourceCode = true;
    } else if (pendingConfirmation_ == PrivacyConfirmation::Telemetry) {
        settings_.allowTelemetry = true;
    }
    pendingConfirmation_ = PrivacyConfirmation::None;
    persist();
}

void AiPrivacySettingsController::cancelPrivacyChange() {
    pendingConfirmation_ = PrivacyConfirmation::None;
}

void AiPrivacySettingsController::setAskBeforeEverySend(bool enabled) {
    settings_.askBeforeEverySend = enabled;
    persist();
}

void AiPrivacySettingsController::persist() {
    persistenceError_ = store_.save(settings_).error;
}

} // namespace editor
