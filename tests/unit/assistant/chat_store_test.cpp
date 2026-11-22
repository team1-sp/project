#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "aegis/assistant/chat_store.hpp"

namespace {

class FixedUserProjectPaths final : public aegis::assistant::UserProjectPaths {
public:
    explicit FixedUserProjectPaths(std::filesystem::path path)
        : path_(std::move(path)) {}

    [[nodiscard]] std::filesystem::path userLocalArea() const override { return path_; }

private:
    std::filesystem::path path_;
};

class ChatStoreTest : public testing::Test {
protected:
    void SetUp() override {
        const testing::TestInfo* info = testing::UnitTest::GetInstance()->current_test_info();
        root_ = std::filesystem::temp_directory_path() / "aegis-chat-store-tests" /
                (std::string(info->test_suite_name()) + '-' + info->name());
        std::error_code error;
        std::filesystem::remove_all(root_, error);
        ASSERT_FALSE(error);
    }

    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }

    [[nodiscard]] std::string fileContents(const std::filesystem::path& path) const {
        std::ifstream input(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }

    std::filesystem::path root_;
};

TEST_F(ChatStoreTest, SaveAndLoad_AllMessageStatesCitationsAndCategoriesRoundTrip) {
    FixedUserProjectPaths paths(root_ / "user-local");
    aegis::assistant::ChatStore store(paths);
    const aegis::assistant::Chat chat{aegis::assistant::chatFormatVersion,
                                      "chat-17",
                                      {{aegis::assistant::MessageRole::Question,
                                        aegis::assistant::MessageState::Complete,
                                        aegis::assistant::MessageContent::Conversation,
                                        "Why?",
                                        {},
                                        {"question", "telemetry"}},
                                       {aegis::assistant::MessageRole::Answer,
                                        aegis::assistant::MessageState::Complete,
                                        aegis::assistant::MessageContent::Conversation,
                                        "Because.",
                                        {{"state", R"({"run":2,"tick":41})"}},
                                        {"model-response"}},
                                       {aegis::assistant::MessageRole::Answer,
                                        aegis::assistant::MessageState::Stopped,
                                        aegis::assistant::MessageContent::DesignSuggestion,
                                        "Partial suggestion",
                                        {{"entity", R"({"id":9})"}},
                                        {"semantic-graph"}},
                                       {aegis::assistant::MessageRole::Answer,
                                        aegis::assistant::MessageState::Streaming,
                                        aegis::assistant::MessageContent::Conversation,
                                        "Partial stream",
                                        {},
                                        {"source-code"}}}};

    ASSERT_TRUE(store.save(chat).succeeded());
    const aegis::assistant::LoadResult loaded = store.load(chat.id);

    ASSERT_TRUE(loaded.succeeded()) << loaded.error;
    EXPECT_EQ(*loaded.chat, chat);
}

TEST_F(ChatStoreTest, Save_UsesOnlyInjectedUserLocalArea) {
    const std::filesystem::path userLocal = root_ / "user-local";
    const std::filesystem::path sharedSettings = root_ / "shared-project-settings";
    FixedUserProjectPaths paths(userLocal);
    aegis::assistant::ChatStore store(paths);
    const aegis::assistant::Chat chat{aegis::assistant::chatFormatVersion, "local-chat", {}};

    ASSERT_TRUE(store.save(chat).succeeded());

    EXPECT_EQ(store.chatFilePath(chat.id), userLocal / "assistant" / "chats" / "local-chat.chat");
    EXPECT_TRUE(std::filesystem::exists(store.chatFilePath(chat.id)));
    EXPECT_FALSE(std::filesystem::exists(sharedSettings));
}

TEST_F(ChatStoreTest, Load_CorruptDataFailsWithoutChangingFile) {
    FixedUserProjectPaths paths(root_ / "user-local");
    aegis::assistant::ChatStore store(paths);
    const std::filesystem::path path = store.chatFilePath("corrupt-chat");
    std::filesystem::create_directories(path.parent_path());
    {
        std::ofstream output(path, std::ios::binary);
        output << "not a chat\n";
    }
    const std::string before = fileContents(path);

    const aegis::assistant::LoadResult loaded = store.load("corrupt-chat");
    const aegis::assistant::StoreResult saved =
        store.save({aegis::assistant::chatFormatVersion, "corrupt-chat", {}});

    EXPECT_FALSE(loaded.succeeded());
    EXPECT_EQ(loaded.error, "invalid chat header");
    EXPECT_FALSE(saved.succeeded());
    EXPECT_EQ(fileContents(path), before);
}

TEST_F(ChatStoreTest, Load_UnsupportedVersionFailsWithoutChangingFile) {
    FixedUserProjectPaths paths(root_ / "user-local");
    aegis::assistant::ChatStore store(paths);
    const std::filesystem::path path = store.chatFilePath("future-chat");
    std::filesystem::create_directories(path.parent_path());
    {
        std::ofstream output(path, std::ios::binary);
        output << "AEGIS_CHAT 999\n";
    }
    const std::string before = fileContents(path);

    const aegis::assistant::LoadResult loaded = store.load("future-chat");
    const aegis::assistant::StoreResult saved =
        store.save({aegis::assistant::chatFormatVersion, "future-chat", {}});

    EXPECT_FALSE(loaded.succeeded());
    EXPECT_EQ(loaded.error, "unsupported chat format version 999");
    EXPECT_FALSE(saved.succeeded());
    EXPECT_EQ(fileContents(path), before);
}

TEST_F(ChatStoreTest, Save_InvalidIdCannotEscapeUserLocalArea) {
    FixedUserProjectPaths paths(root_ / "user-local");
    aegis::assistant::ChatStore store(paths);

    const aegis::assistant::StoreResult result =
        store.save({aegis::assistant::chatFormatVersion, "../shared", {}});

    EXPECT_FALSE(result.succeeded());
    EXPECT_FALSE(std::filesystem::exists(root_ / "shared.chat"));
}

} // namespace
