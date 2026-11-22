#include "editor/assistant_chat.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace editor {

namespace aa = aegis::assistant;

namespace {

constexpr std::size_t maximumTitleCharacters = 60;
constexpr std::string_view failedAnswerMessage = "The answer could not be completed.";
constexpr std::string_view rejectedSendMessage = "The question could not be sent.";

[[nodiscard]] std::string chatTitle(const aa::Chat& chat) {
    const auto question = std::find_if(
        chat.messages.begin(), chat.messages.end(),
        [](const aa::ChatMessage& message) { return message.role == aa::MessageRole::Question; });
    if (question == chat.messages.end()) {
        return chat.id;
    }
    std::string title = question->text;
    std::replace(title.begin(), title.end(), '\n', ' ');
    if (title.size() > maximumTitleCharacters) {
        title.resize(maximumTitleCharacters - 3);
        title += "...";
    }
    return title;
}

[[nodiscard]] std::unique_ptr<aa::Chat> emptyChat(const std::string& id) {
    auto chat = std::make_unique<aa::Chat>();
    chat->id = id;
    return chat;
}

} // namespace

bool isBlankMessage(std::string_view message) {
    return std::all_of(message.begin(), message.end(), [](char character) {
        return std::isspace(static_cast<unsigned char>(character)) != 0;
    });
}

EnterKeyAction enterKeyAction(std::string_view draft, bool shiftHeld) {
    if (shiftHeld) {
        return EnterKeyAction::InsertNewline;
    }
    return isBlankMessage(draft) ? EnterKeyAction::Ignore : EnterKeyAction::Send;
}

AssistantChat::AssistantChat(IChatQuestionSender& sender, const IChatLoader& loader,
                             const IContextItemProvider& items, const IContextSizeEstimator& sizes,
                             ChatIdGenerator nextChatId)
    : sender_(sender)
    , loader_(loader)
    , items_(items)
    , sizes_(sizes)
    , nextChatId_(std::move(nextChatId))
    , chat_(emptyChat(nextChatId_())) {
}

AssistantChat::~AssistantChat() {
    if (answer_ != nullptr) {
        answer_->cancel();
    }
}

void AssistantChat::setDraft(std::string draft) {
    draft_ = std::move(draft);
}

bool AssistantChat::canSend() const {
    return !isAnswering() && !isBlankMessage(draft_);
}

bool AssistantChat::send() {
    if (!canSend()) {
        return false;
    }
    const aa::GroundedQuestion question{draft_, chips_.items()};
    SendOutcome outcome = sender_.send(*chat_, question);
    if (!outcome.succeeded()) {
        lastError_ = outcome.error.empty() ? std::string(rejectedSendMessage) : outcome.error;
        return false;
    }
    // The sender appended the question and the Streaming answer before starting it; the answering
    // thread never changes the number of messages.
    answerIndex_ = chat_->messages.empty() ? 0 : chat_->messages.size() - 1;
    answer_ = std::move(outcome.answer);
    stopRequested_ = false;
    draft_.clear();
    lastError_.clear();
    return true;
}

EnterKeyAction AssistantChat::pressEnter(bool shiftHeld) {
    const EnterKeyAction action = enterKeyAction(draft_, shiftHeld);
    switch (action) {
    case EnterKeyAction::Send:
        static_cast<void>(send());
        break;
    case EnterKeyAction::InsertNewline:
        draft_ += '\n';
        break;
    case EnterKeyAction::Ignore:
        break;
    }
    return action;
}

void AssistantChat::stop() {
    if (!canStop()) {
        return;
    }
    stopRequested_ = true;
    answer_->cancel();
}

void AssistantChat::update() {
    if (answer_ != nullptr && answer_->finished()) {
        collectFinishedAnswer();
    }
}

void AssistantChat::collectFinishedAnswer() {
    const aa::AnswerResult result = answer_->result();
    answer_.reset();
    stopRequested_ = false;
    if (result.state == aa::MessageState::Failed) {
        lastError_ = result.error.empty() ? std::string(failedAnswerMessage) : result.error;
    }
}

std::span<const aa::ChatMessage> AssistantChat::settledMessages() const {
    const std::span<const aa::ChatMessage> all(chat_->messages);
    return isAnswering() ? all.first(std::min(answerIndex_, all.size())) : all;
}

void AssistantChat::removeChip(ContextChipId id) {
    chips_ = chips_.without(id);
}

std::vector<ContextChipType> AssistantChat::addableChipTypes() const {
    return chips_.unattachedTypes();
}

bool AssistantChat::canAttach(ContextChipType type) const {
    return !chips_.contains(type) && items_.itemFor(type).has_value();
}

bool AssistantChat::attach(ContextChipType type) {
    if (chips_.contains(type)) {
        return false;
    }
    std::optional<aa::ContextItem> item = items_.itemFor(type);
    if (!item) {
        return false;
    }
    chips_ = chips_.with(std::move(*item), sizes_);
    return true;
}

void AssistantChat::importEntryPoint(std::string draft,
                                     const std::vector<AssistantContext>& contexts) {
    draft_ = std::move(draft);
    for (const AssistantContext& context : contexts) {
        if (std::optional<aa::ContextItem> item = toContextItem(context)) {
            chips_ = chips_.with(std::move(*item), sizes_);
        }
    }
}

void AssistantChat::keepCurrentChatInHistory() {
    if (chat_->messages.empty()) {
        return;
    }
    std::erase_if(history_,
                  [this](const ChatHistoryEntry& entry) { return entry.chatId == chat_->id; });
    history_.insert(history_.begin(), ChatHistoryEntry{chat_->id, chatTitle(*chat_)});
}

bool AssistantChat::newChat() {
    if (!canSwitchChat()) {
        return false;
    }
    keepCurrentChatInHistory();
    chat_ = emptyChat(nextChatId_());
    lastError_.clear();
    return true;
}

bool AssistantChat::openChat(const std::string& chatId) {
    if (!canSwitchChat()) {
        return false;
    }
    if (chatId == chat_->id) {
        return true;
    }
    aa::LoadResult loaded = loader_.load(chatId);
    if (!loaded.succeeded()) {
        lastError_ = "Could not open the chat: " + loaded.error;
        return false;
    }
    keepCurrentChatInHistory();
    std::erase_if(history_,
                  [&chatId](const ChatHistoryEntry& entry) { return entry.chatId == chatId; });
    chat_ = std::make_unique<aa::Chat>(std::move(*loaded.chat));
    lastError_.clear();
    return true;
}

} // namespace editor
