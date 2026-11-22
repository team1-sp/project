// US-33.2: the AI Assistant chat state (srs.fr.33.5 to srs.fr.33.12), driven without a window.
#include <algorithm>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "assistant_chat_fakes.hpp"
#include "editor/assistant_chat.hpp"

namespace {

using namespace chat_test;
using editor::ContextChipType;
using editor::EnterKeyAction;

class AssistantChatTest : public testing::Test {
protected:
    AssistantChatTest() {
        items.items[ContextChipType::Selection] = aa::SelectionContext{{engine::EntityId{7}}};
        items.items[ContextChipType::Graph] = aa::GraphContext{};
        items.items[ContextChipType::LastRun] = aa::LastRunContext{};
    }

    [[nodiscard]] std::string nextId() { return "chat-" + std::to_string(++idCounter); }

    FakeSender sender;
    FakeLoader loader;
    FakeItems items;
    FakeSizes sizes;
    int idCounter = 0;
    editor::AssistantChat chat{sender, loader, items, sizes, [this] { return nextId(); }};
};

// srs.fr.33.5
TEST(AssistantComposer, WhitespaceOnlyMessagesAreEmpty) {
    EXPECT_TRUE(editor::isBlankMessage(""));
    EXPECT_TRUE(editor::isBlankMessage(" \t\n\r "));
    EXPECT_FALSE(editor::isBlankMessage("  why?  "));
}

TEST(AssistantComposer, EnterSendsShiftEnterInsertsNewlineEmptyEnterDoesNothing) {
    EXPECT_EQ(editor::enterKeyAction("Why?", false), EnterKeyAction::Send);
    EXPECT_EQ(editor::enterKeyAction("Why?", true), EnterKeyAction::InsertNewline);
    EXPECT_EQ(editor::enterKeyAction("", true), EnterKeyAction::InsertNewline);
    EXPECT_EQ(editor::enterKeyAction("", false), EnterKeyAction::Ignore);
    EXPECT_EQ(editor::enterKeyAction("  \n ", false), EnterKeyAction::Ignore);
}

TEST_F(AssistantChatTest, EnterSendsTheQuestionLikeSend) {
    chat.setDraft("Why is the door locked?");

    EXPECT_EQ(chat.pressEnter(false), EnterKeyAction::Send);

    ASSERT_EQ(sender.questions.size(), 1U);
    EXPECT_EQ(sender.questions[0].text, "Why is the door locked?");
    EXPECT_TRUE(chat.isAnswering());
}

TEST_F(AssistantChatTest, ShiftEnterInsertsANewLineAndDoesNotSend) {
    chat.setDraft("first line");

    EXPECT_EQ(chat.pressEnter(true), EnterKeyAction::InsertNewline);

    EXPECT_EQ(chat.draft(), "first line\n");
    EXPECT_TRUE(sender.questions.empty());
}

TEST_F(AssistantChatTest, EnterOnAnEmptyOrWhitespaceMessageDoesNothing) {
    EXPECT_EQ(chat.pressEnter(false), EnterKeyAction::Ignore);
    chat.setDraft("   \n\t");
    EXPECT_EQ(chat.pressEnter(false), EnterKeyAction::Ignore);

    EXPECT_TRUE(sender.questions.empty());
    EXPECT_EQ(chat.draft(), "   \n\t");
}

// srs.fr.33.6
TEST_F(AssistantChatTest, SendIsDisabledWhileTheMessageIsEmpty) {
    EXPECT_FALSE(chat.canSend());
    chat.setDraft("  ");
    EXPECT_FALSE(chat.canSend());
    EXPECT_FALSE(chat.send());
    chat.setDraft("Why?");
    EXPECT_TRUE(chat.canSend());
    EXPECT_TRUE(sender.questions.empty());
}

TEST_F(AssistantChatTest, SuccessfulSendClearsTheDraftOnceTheAnswerStartedAndKeepsChips) {
    ASSERT_TRUE(chat.attach(ContextChipType::Selection));
    chat.setDraft("Why?");

    ASSERT_TRUE(chat.send());

    ASSERT_EQ(sender.questions.size(), 1U);
    ASSERT_EQ(sender.questions[0].attachments.size(), 1U);
    EXPECT_TRUE(std::holds_alternative<aa::SelectionContext>(sender.questions[0].attachments[0]));
    EXPECT_EQ(chat.draft(), "");
    EXPECT_EQ(chat.chips().chips().size(), 1U);
    EXPECT_TRUE(chat.lastError().empty());
}

TEST_F(AssistantChatTest, RejectedSendKeepsTheDraftChipsAndChat) {
    ASSERT_TRUE(chat.attach(ContextChipType::Graph));
    chat.setDraft("Why?");
    sender.rejection = "data category is not allowed: semantic-graph";

    EXPECT_FALSE(chat.send());

    EXPECT_EQ(chat.draft(), "Why?");
    EXPECT_EQ(chat.chips().chips().size(), 1U);
    EXPECT_TRUE(chat.settledMessages().empty());
    EXPECT_FALSE(chat.isAnswering());
    EXPECT_EQ(chat.lastError(), "data category is not allowed: semantic-graph");
    EXPECT_TRUE(chat.canSend());
}

TEST_F(AssistantChatTest, NoSecondSendWhileAnAnswerIsRunning) {
    chat.setDraft("First?");
    ASSERT_TRUE(chat.send());
    chat.setDraft("Second?");

    EXPECT_FALSE(chat.canSend());
    EXPECT_FALSE(chat.send());
    EXPECT_EQ(chat.pressEnter(false), EnterKeyAction::Send);

    EXPECT_EQ(sender.questions.size(), 1U);
    EXPECT_EQ(chat.draft(), "Second?");

    sender.control->finished = true;
    chat.update();
    EXPECT_TRUE(chat.canSend());
}

// srs.fr.33.7
TEST_F(AssistantChatTest, SpinnerAndStopShowOnlyWhileAnAnswerIsPrepared) {
    EXPECT_FALSE(chat.isAnswering());
    EXPECT_FALSE(chat.canStop());

    chat.setDraft("Why?");
    ASSERT_TRUE(chat.send());
    EXPECT_TRUE(chat.isAnswering());
    EXPECT_TRUE(chat.canStop());

    chat.update();
    EXPECT_TRUE(chat.isAnswering());

    sender.control->finished = true;
    chat.update();
    EXPECT_FALSE(chat.isAnswering());
    EXPECT_FALSE(chat.canStop());
}

TEST_F(AssistantChatTest, TheAnswerBeingWrittenIsNotDrawnUntilItEnds) {
    chat.setDraft("Why?");
    ASSERT_TRUE(chat.send());

    ASSERT_EQ(chat.settledMessages().size(), 1U);
    EXPECT_EQ(chat.settledMessages()[0].role, aa::MessageRole::Question);

    sender.control->finished = true;
    chat.update();
    EXPECT_EQ(chat.settledMessages().size(), 2U);
}

// srs.fr.33.8
TEST_F(AssistantChatTest, StopCancelsTheRunningAnswerExactlyOnce) {
    chat.setDraft("Why?");
    ASSERT_TRUE(chat.send());

    chat.stop();
    chat.stop();

    EXPECT_EQ(sender.control->cancelCalls, 1);
    EXPECT_FALSE(chat.canStop());
    EXPECT_TRUE(chat.isAnswering());

    sender.control->result = {aa::MessageState::Stopped, {}};
    sender.control->finished = true;
    chat.update();
    EXPECT_FALSE(chat.isAnswering());
    EXPECT_TRUE(chat.lastError().empty());
}

TEST_F(AssistantChatTest, StopWithoutARunningAnswerDoesNothing) {
    chat.stop();
    EXPECT_FALSE(chat.isAnswering());
}

TEST_F(AssistantChatTest, AFailedAnswerReportsItsError) {
    chat.setDraft("Why?");
    ASSERT_TRUE(chat.send());
    sender.control->result = {aa::MessageState::Failed, "AI service unavailable"};
    sender.control->finished = true;

    chat.update();

    EXPECT_EQ(chat.lastError(), "AI service unavailable");
}

// srs.fr.33.9 and srs.fr.33.10
TEST_F(AssistantChatTest, RemovingAChipLeavesItOutOfTheNextQuestion) {
    ASSERT_TRUE(chat.attach(ContextChipType::Selection));
    ASSERT_TRUE(chat.attach(ContextChipType::Graph));
    const editor::ContextChipId selectionId = chat.chips().chips()[0].id;

    chat.removeChip(selectionId);
    chat.setDraft("Why?");
    ASSERT_TRUE(chat.send());

    ASSERT_EQ(sender.questions[0].attachments.size(), 1U);
    EXPECT_TRUE(std::holds_alternative<aa::GraphContext>(sender.questions[0].attachments[0]));
}

TEST_F(AssistantChatTest, PlusListsOnlyUnattachedTypesAndAttachesTheChosenOne) {
    EXPECT_EQ(chat.addableChipTypes().size(), editor::contextChipTypeCount);

    ASSERT_TRUE(chat.attach(ContextChipType::LastRun));

    const std::vector<ContextChipType> addable = chat.addableChipTypes();
    EXPECT_EQ(addable.size(), editor::contextChipTypeCount - 1);
    EXPECT_EQ(std::count(addable.begin(), addable.end(), ContextChipType::LastRun), 0);
    EXPECT_FALSE(chat.canAttach(ContextChipType::LastRun));
    EXPECT_FALSE(chat.attach(ContextChipType::LastRun));
    EXPECT_EQ(chat.chips().chips().size(), 1U);
}

TEST_F(AssistantChatTest, ATypeWithNothingToAttachCannotBeAttached) {
    EXPECT_FALSE(chat.canAttach(ContextChipType::Asset));
    EXPECT_FALSE(chat.attach(ContextChipType::Asset));
    EXPECT_TRUE(chat.chips().empty());
}

TEST_F(AssistantChatTest, EntryPointDraftAndSelectionBecomeTheComposerAndAChip) {
    editor::SelectionContext selection{editor::EntitySelection{engine::EntityId{42}}};

    chat.importEntryPoint("Why\xE2\x80\xA6", {selection});

    EXPECT_EQ(chat.draft(), "Why\xE2\x80\xA6");
    ASSERT_EQ(chat.chips().chips().size(), 1U);
    const auto& item = std::get<aa::SelectionContext>(chat.chips().chips()[0].item);
    EXPECT_EQ(item.entities, std::vector<engine::EntityId>{engine::EntityId{42}});
}

// srs.fr.33.11
TEST_F(AssistantChatTest, NewChatStartsEmptyAndKeepsThePreviousChatInHistory) {
    const std::string firstId = chat.currentChatId();
    chat.setDraft("Why is the door locked?");
    ASSERT_TRUE(chat.send());
    sender.control->finished = true;
    chat.update();

    ASSERT_TRUE(chat.newChat());

    EXPECT_TRUE(chat.settledMessages().empty());
    EXPECT_NE(chat.currentChatId(), firstId);
    ASSERT_EQ(chat.history().size(), 1U);
    EXPECT_EQ(chat.history()[0], (editor::ChatHistoryEntry{firstId, "Why is the door locked?"}));
}

TEST_F(AssistantChatTest, AnEmptyChatIsNotAddedToHistory) {
    ASSERT_TRUE(chat.newChat());
    EXPECT_TRUE(chat.history().empty());
}

TEST_F(AssistantChatTest, ChatsCannotBeSwitchedWhileAnAnswerIsRunning) {
    chat.setDraft("Why?");
    ASSERT_TRUE(chat.send());

    EXPECT_FALSE(chat.canSwitchChat());
    EXPECT_FALSE(chat.newChat());
    EXPECT_FALSE(chat.openChat("chat-old"));
}

// srs.fr.33.12
TEST_F(AssistantChatTest, PickingAChatFromHistoryRestoresAllMessagesAndCitations) {
    const aa::Citation citation{"source", "file=Scripts/door.lua\nline=12\nhash=abc"};
    aa::Chat saved;
    saved.id = "chat-old";
    saved.messages = {
        {aa::MessageRole::Question, aa::MessageState::Complete, {}, "Why?", {}, {"question"}},
        {aa::MessageRole::Answer, aa::MessageState::Stopped, {}, "Because", {citation}, {}},
    };
    loader.chats[saved.id] = saved;
    chat.setDraft("Current question");
    ASSERT_TRUE(chat.send());
    sender.control->finished = true;
    chat.update();
    const std::string currentId = chat.currentChatId();

    ASSERT_TRUE(chat.openChat("chat-old"));

    EXPECT_EQ(chat.currentChatId(), "chat-old");
    const auto messages = chat.settledMessages();
    ASSERT_EQ(messages.size(), 2U);
    EXPECT_EQ(messages[0], saved.messages[0]);
    EXPECT_EQ(messages[1], saved.messages[1]);
    ASSERT_EQ(chat.history().size(), 1U);
    EXPECT_EQ(chat.history()[0].chatId, currentId);
}

TEST_F(AssistantChatTest, AChatThatCannotBeLoadedLeavesTheOpenChatAlone) {
    const std::string currentId = chat.currentChatId();

    EXPECT_FALSE(chat.openChat("missing"));

    EXPECT_EQ(chat.currentChatId(), currentId);
    EXPECT_EQ(chat.lastError(), "Could not open the chat: chat file not found");
}

TEST_F(AssistantChatTest, DestroyingTheChatCancelsARunningAnswer) {
    std::shared_ptr<AnswerControl> control;
    {
        editor::AssistantChat scoped(sender, loader, items, sizes, [] { return "chat-x"; });
        scoped.setDraft("Why?");
        ASSERT_TRUE(scoped.send());
        control = sender.control;
    }
    EXPECT_EQ(control->cancelCalls, 1);
}

} // namespace
