#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "editor/ai_privacy_settings.hpp"

namespace {

using namespace std::chrono_literals;

struct RecordedRequest {
    std::uint64_t id = 0;
    editor::AiServiceRequestKind kind = editor::AiServiceRequestKind::Models;
    std::string address;
    std::chrono::milliseconds timeout{0};
};

class FakeRequests final : public editor::IAiServiceRequestRunner {
public:
    void requestHealth(std::uint64_t requestId, std::string address,
                       std::chrono::milliseconds timeout) override {
        requests.push_back(
            {requestId, editor::AiServiceRequestKind::Health, std::move(address), timeout});
    }

    void requestModels(std::uint64_t requestId, std::string address,
                       std::chrono::milliseconds timeout) override {
        requests.push_back(
            {requestId, editor::AiServiceRequestKind::Models, std::move(address), timeout});
    }

    [[nodiscard]] std::vector<editor::AiServiceResponse> takeCompleted() override {
        return std::exchange(completed, {});
    }

    void completeModels(std::size_t requestIndex, engine::bridge::ModelListResult result) {
        const RecordedRequest& request = requests.at(requestIndex);
        completed.push_back(
            {request.id, request.kind, request.address, 0ms, {}, std::move(result)});
    }

    void completeHealth(std::size_t requestIndex, engine::bridge::HealthCheckResult result,
                        std::chrono::milliseconds latency) {
        const RecordedRequest& request = requests.at(requestIndex);
        completed.push_back(
            {request.id, request.kind, request.address, latency, std::move(result), {}});
    }

    std::vector<RecordedRequest> requests;
    std::vector<editor::AiServiceResponse> completed;
};

class AiPrivacySettingsTest : public testing::Test {
protected:
    void SetUp() override {
        const testing::TestInfo* info = testing::UnitTest::GetInstance()->current_test_info();
        root_ = std::filesystem::temp_directory_path() / "aegis-ai-privacy-tests" /
                (std::string(info->test_suite_name()) + '-' + info->name());
        std::error_code error;
        std::filesystem::remove_all(root_, error);
        ASSERT_FALSE(error);
        editor::AiPrivacySettings defaults;
        defaults.serviceAddress = "localhost:50051";
        store_ = std::make_unique<editor::AiPrivacySettingsStore>(root_ / "user-local",
                                                                  std::move(defaults));
        controller_ = std::make_unique<editor::AiPrivacySettingsController>(*store_, requests_);
    }

    void TearDown() override {
        controller_.reset();
        store_.reset();
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }

    std::filesystem::path root_;
    FakeRequests requests_;
    std::unique_ptr<editor::AiPrivacySettingsStore> store_;
    std::unique_ptr<editor::AiPrivacySettingsController> controller_;
};

TEST(ServiceAddressValidation, AcceptsHostnameIpv4AndBracketedIpv6) {
    EXPECT_TRUE(editor::isValidServiceAddress("localhost:1"));
    EXPECT_TRUE(editor::isValidServiceAddress("ai-service.example.com:65535"));
    EXPECT_TRUE(editor::isValidServiceAddress("127.0.0.1:50051"));
    EXPECT_TRUE(editor::isValidServiceAddress("255.255.255.255:65535"));
    EXPECT_TRUE(editor::isValidServiceAddress("[::1]:1"));
    EXPECT_TRUE(editor::isValidServiceAddress("[2001:db8:0:1::abcd]:50051"));
    EXPECT_TRUE(editor::isValidServiceAddress("[::ffff:192.0.2.128]:65535"));
}

TEST(ServiceAddressValidation, RejectsMalformedHostsAddressesAndPorts) {
    EXPECT_FALSE(editor::isValidServiceAddress("localhost"));
    EXPECT_FALSE(editor::isValidServiceAddress(":50051"));
    EXPECT_FALSE(editor::isValidServiceAddress("bad host:50051"));
    EXPECT_FALSE(editor::isValidServiceAddress("-host:50051"));
    EXPECT_FALSE(editor::isValidServiceAddress("256.1.1.1:50051"));
    EXPECT_FALSE(editor::isValidServiceAddress("127.0.0:50051"));
    EXPECT_FALSE(editor::isValidServiceAddress("::1:50051"));
    EXPECT_FALSE(editor::isValidServiceAddress("[2001:::1]:50051"));
    EXPECT_FALSE(editor::isValidServiceAddress("[2001:db8::1]50051"));
    EXPECT_FALSE(editor::isValidServiceAddress("localhost:0"));
    EXPECT_FALSE(editor::isValidServiceAddress("localhost:65536"));
    EXPECT_FALSE(editor::isValidServiceAddress("localhost:+1"));
    EXPECT_FALSE(editor::isValidServiceAddress("localhost:1.5"));
    EXPECT_FALSE(editor::isValidServiceAddress("localhost:abc"));
}

TEST_F(AiPrivacySettingsTest, InvalidAddressRetainsPreviousValidAddress) {
    controller_->setAddressDraft("not-an-address");

    EXPECT_FALSE(controller_->commitAddressOnFocusLoss());
    EXPECT_EQ(controller_->addressError(), "Address must be host:port");
    EXPECT_EQ(controller_->settings().serviceAddress, "localhost:50051");
}

TEST_F(AiPrivacySettingsTest, InvalidDraftMakesTestConnectionFailWithoutCallingOldAddress) {
    controller_->setAddressDraft("bad address");

    controller_->testConnection();

    EXPECT_TRUE(requests_.requests.empty());
    EXPECT_EQ(controller_->settings().serviceAddress, "localhost:50051");
    EXPECT_EQ(controller_->connectionState(), editor::ConnectionState::Failed);
    EXPECT_EQ(controller_->connectionText(), "Address not valid");
}

TEST_F(AiPrivacySettingsTest, TabOpenRequestsModelsWithFiveSecondDeadline) {
    controller_->openTab();

    ASSERT_EQ(requests_.requests.size(), 1U);
    EXPECT_EQ(requests_.requests[0].kind, editor::AiServiceRequestKind::Models);
    EXPECT_EQ(requests_.requests[0].address, "localhost:50051");
    EXPECT_EQ(requests_.requests[0].timeout, 5s);
    EXPECT_EQ(controller_->modelListState(), editor::ModelListState::Loading);
}

TEST_F(AiPrivacySettingsTest, SuccessfulEmptyAndFailedListsHaveDistinctStates) {
    controller_->openTab();
    requests_.completeModels(0, {{{"m1", "Model One"}}, {}});
    controller_->poll();
    EXPECT_EQ(controller_->modelListState(), editor::ModelListState::Available);
    EXPECT_TRUE(controller_->selectModel("m1"));
    EXPECT_EQ(controller_->modelFieldText(), "Model One");

    controller_->openTab();
    requests_.completeModels(1, {{}, {}});
    controller_->poll();
    EXPECT_EQ(controller_->modelListState(), editor::ModelListState::Empty);
    EXPECT_EQ(controller_->modelFieldText(), "No models available");
    EXPECT_TRUE(controller_->settings().modelIdentifier.empty());

    controller_->openTab();
    requests_.completeModels(
        2, {{}, {engine::bridge::ServiceFailureKind::Unavailable, {}, "connection refused"}});
    controller_->poll();
    EXPECT_EQ(controller_->modelListState(), editor::ModelListState::Failed);
    EXPECT_EQ(controller_->modelsError(), "Models unavailable: Service not reachable");
}

TEST_F(AiPrivacySettingsTest, SuccessfulListPreservesOnlyAvailableSelectedIdentifier) {
    controller_->openTab();
    requests_.completeModels(0, {{{"m1", "First"}, {"m2", "Second"}}, {}});
    controller_->poll();
    ASSERT_TRUE(controller_->selectModel("m2"));

    controller_->openTab();
    requests_.completeModels(1, {{{"m2", "Second renamed"}}, {}});
    controller_->poll();
    EXPECT_EQ(controller_->settings().modelIdentifier, "m2");
    EXPECT_EQ(controller_->modelFieldText(), "Second renamed");

    controller_->openTab();
    requests_.completeModels(2, {{{"m3", "Third"}}, {}});
    controller_->poll();
    EXPECT_TRUE(controller_->settings().modelIdentifier.empty());
    EXPECT_EQ(controller_->modelFieldText(), "Select a model");
}

TEST_F(AiPrivacySettingsTest, FailedListKeepsSelectedModelAndDisablesOnlyWhenNoneSelected) {
    controller_->openTab();
    requests_.completeModels(0, {{{"m1", "First"}}, {}});
    controller_->poll();
    ASSERT_TRUE(controller_->selectModel("m1"));

    controller_->openTab();
    requests_.completeModels(
        1, {{}, {engine::bridge::ServiceFailureKind::DeadlineExceeded, {}, "timeout"}});
    controller_->poll();

    EXPECT_EQ(controller_->settings().modelIdentifier, "m1");
    EXPECT_TRUE(controller_->sendingEnabled());
    EXPECT_TRUE(controller_->sendDisabledReason().empty());

    controller_->setAddressDraft("new.example:50052");
    ASSERT_TRUE(controller_->commitAddressOnFocusLoss());
    EXPECT_FALSE(controller_->sendingEnabled());
    EXPECT_EQ(controller_->sendDisabledReason(), "Select a model");
}

TEST_F(AiPrivacySettingsTest, TestConnectionMapsTimeoutServingLatencyAndSpecificFailures) {
    controller_->testConnection();
    ASSERT_EQ(requests_.requests[0].timeout, 5s);
    requests_.completeHealth(
        0,
        {engine::bridge::HealthServingStatus::Unknown,
         {engine::bridge::ServiceFailureKind::DeadlineExceeded, {}, "deadline"}},
        5s);
    controller_->poll();
    EXPECT_EQ(controller_->connectionState(), editor::ConnectionState::Failed);
    EXPECT_EQ(controller_->connectionText(), "Deadline exceeded");

    controller_->testConnection();
    requests_.completeHealth(1, {engine::bridge::HealthServingStatus::Serving, {}}, 42ms);
    controller_->poll();
    EXPECT_EQ(controller_->connectionState(), editor::ConnectionState::Connected);
    EXPECT_EQ(controller_->connectionText(), "Connected \xC2\xB7 42 ms");
    ASSERT_EQ(requests_.requests.size(), 3U);
    EXPECT_EQ(requests_.requests[2].kind, editor::AiServiceRequestKind::Models);

    requests_.completeModels(2, {{}, {}});
    controller_->poll();
    controller_->testConnection();
    requests_.completeHealth(3, {engine::bridge::HealthServingStatus::NotServing, {}}, 7ms);
    controller_->poll();
    EXPECT_EQ(controller_->connectionText(), "Service not serving");

    controller_->testConnection();
    requests_.completeHealth(
        4,
        {engine::bridge::HealthServingStatus::Unknown,
         {engine::bridge::ServiceFailureKind::RemoteStatus, "UNAUTHENTICATED", "bad key"}},
        3ms);
    controller_->poll();
    EXPECT_EQ(controller_->connectionText(), "UNAUTHENTICATED: bad key");
}

TEST_F(AiPrivacySettingsTest, PrivacySettingsDefaultOffAndConfirmIndependently) {
    EXPECT_FALSE(controller_->settings().allowSourceCode);
    EXPECT_FALSE(controller_->settings().allowTelemetry);
    EXPECT_FALSE(controller_->settings().askBeforeEverySend);

    controller_->requestAllow(editor::ProjectDataCategory::SourceCode, true);
    EXPECT_EQ(controller_->pendingConfirmation(), editor::PrivacyConfirmation::SourceCode);
    EXPECT_NE(controller_->confirmationText().find("file contents"), std::string::npos);
    controller_->cancelPrivacyChange();
    EXPECT_FALSE(controller_->settings().allowSourceCode);

    controller_->requestAllow(editor::ProjectDataCategory::Telemetry, true);
    EXPECT_EQ(controller_->pendingConfirmation(), editor::PrivacyConfirmation::Telemetry);
    EXPECT_NE(controller_->confirmationText().find("telemetry samples"), std::string::npos);
    controller_->confirmPrivacyChange();
    EXPECT_FALSE(controller_->settings().allowSourceCode);
    EXPECT_TRUE(controller_->settings().allowTelemetry);
}

TEST_F(AiPrivacySettingsTest, AddressChangeTurnsBothAddressBoundPermissionsOff) {
    controller_->requestAllow(editor::ProjectDataCategory::SourceCode, true);
    controller_->confirmPrivacyChange();
    controller_->requestAllow(editor::ProjectDataCategory::Telemetry, true);
    controller_->confirmPrivacyChange();
    ASSERT_TRUE(controller_->settings().allowSourceCode);
    ASSERT_TRUE(controller_->settings().allowTelemetry);

    controller_->setAddressDraft("other.example:50052");
    ASSERT_TRUE(controller_->commitAddressOnFocusLoss());

    EXPECT_FALSE(controller_->settings().allowSourceCode);
    EXPECT_FALSE(controller_->settings().allowTelemetry);
}

TEST_F(AiPrivacySettingsTest, AskBeforeEverySendAndPolicyPersistPerProject) {
    controller_->requestAllow(editor::ProjectDataCategory::SourceCode, true);
    controller_->confirmPrivacyChange();
    controller_->setAskBeforeEverySend(true);

    EXPECT_TRUE(controller_->settings().allows("source-code"));
    EXPECT_TRUE(controller_->settings().allows("semantic-graph"));
    EXPECT_TRUE(controller_->settings().requiresConsent(editor::ProjectDataCategory::SourceCode));
    EXPECT_TRUE(controller_->settings().requiresConsent(editor::ProjectDataCategory::Telemetry));

    editor::AiPrivacySettingsController reloaded(*store_, requests_);
    EXPECT_TRUE(reloaded.settings().allowSourceCode);
    EXPECT_FALSE(reloaded.settings().allowTelemetry);
    EXPECT_TRUE(reloaded.settings().askBeforeEverySend);
}

TEST_F(AiPrivacySettingsTest, AddressChangeIgnoresStaleAsynchronousResponses) {
    controller_->openTab();
    controller_->setAddressDraft("new.example:50052");
    ASSERT_TRUE(controller_->commitAddressOnFocusLoss());
    controller_->openTab();

    requests_.completeModels(0, {{{"old", "Old address model"}}, {}});
    controller_->poll();
    EXPECT_EQ(controller_->modelListState(), editor::ModelListState::Loading);
    EXPECT_TRUE(controller_->models().empty());

    requests_.completeModels(1, {{{"new", "New address model"}}, {}});
    controller_->poll();
    ASSERT_EQ(controller_->models().size(), 1U);
    EXPECT_EQ(controller_->models()[0].identifier, "new");
}

TEST_F(AiPrivacySettingsTest, NewerHealthRequestWinsWhenResponsesCompleteOutOfOrder) {
    controller_->testConnection();
    controller_->testConnection();
    ASSERT_EQ(requests_.requests.size(), 2U);

    requests_.completeHealth(0, {engine::bridge::HealthServingStatus::Serving, {}}, 10ms);
    controller_->poll();
    EXPECT_EQ(controller_->connectionState(), editor::ConnectionState::Checking);

    requests_.completeHealth(1,
                             {engine::bridge::HealthServingStatus::Unknown,
                              {engine::bridge::ServiceFailureKind::Unavailable, {}, "refused"}},
                             20ms);
    controller_->poll();
    EXPECT_EQ(controller_->connectionState(), editor::ConnectionState::Failed);
    EXPECT_EQ(controller_->connectionText(), "Service not reachable");
}

TEST_F(AiPrivacySettingsTest, ProjectSettingsRoundTripUsesUserLocalSettingsArea) {
    editor::AiPrivacySettings settings;
    settings.serviceAddress = "[2001:db8::1]:65535";
    settings.modelIdentifier = "model-id-not-display-name";
    settings.allowSourceCode = true;
    settings.allowTelemetry = false;
    settings.askBeforeEverySend = true;

    ASSERT_TRUE(store_->save(settings).succeeded());
    const editor::AiSettingsLoadResult loaded = store_->load();
    ASSERT_TRUE(loaded.succeeded()) << loaded.error;
    EXPECT_EQ(loaded.settings.serviceAddress, settings.serviceAddress);
    EXPECT_EQ(loaded.settings.modelIdentifier, settings.modelIdentifier);
    EXPECT_TRUE(loaded.settings.allowSourceCode);
    EXPECT_FALSE(loaded.settings.allowTelemetry);
    EXPECT_TRUE(loaded.settings.askBeforeEverySend);
    EXPECT_EQ(store_->filePath(), root_ / "user-local" / "settings" / "ai-privacy.settings");
}

} // namespace
