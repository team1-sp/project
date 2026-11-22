#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "aegis/assistant/chat.hpp"
#include "aegis/assistant/chat_store.hpp"
#include "aegis/assistant/grounded/grounded_question.hpp"
#include "aegis/assistant/read_only_assistant.hpp"
#include "editor/assistant_chat_context.hpp"
#include "editor/assistant_entry_points.hpp"

// Window-free chat state of the AI Assistant panel (US-33.2, srs.fr.33.5 to srs.fr.33.12). It
// owns the composer draft, the context chips, the open chat and the History list, and reaches the
// assistant only through the injected sender, so consent and offline rules (US-33.3) can decorate
// sending without this class knowing. Dear ImGui only draws this state.
namespace editor {

// srs.fr.33.5: a message of only whitespace counts as empty.
[[nodiscard]] bool isBlankMessage(std::string_view message);

enum class EnterKeyAction { Send, InsertNewline, Ignore };

// srs.fr.33.5: Enter sends, Shift+Enter inserts a new line, Enter on an empty message does nothing.
[[nodiscard]] EnterKeyAction enterKeyAction(std::string_view draft, bool shiftHeld);

// A running answer. US-11.2's AnswerOperation streams into the chat on another thread; until
// finished() returns true, only the answer message is written, and only by that thread.
class IActiveAnswer {
public:
    virtual ~IActiveAnswer() = default;
    [[nodiscard]] virtual bool finished() const = 0;
    // Forwards to US-11.2's cancellation; safe to call at any time.
    virtual void cancel() noexcept = 0;
    // Valid once finished() returned true.
    [[nodiscard]] virtual aegis::assistant::AnswerResult result() const = 0;
};

struct SendOutcome {
    std::unique_ptr<IActiveAnswer> answer;
    std::string error;

    [[nodiscard]] bool succeeded() const noexcept { return answer != nullptr && error.empty(); }
};

// The panel's only sending seam. On success the implementation has appended the question and a
// Streaming answer to `chat`; on error the chat is unchanged.
class IChatQuestionSender {
public:
    virtual ~IChatQuestionSender() = default;
    [[nodiscard]] virtual SendOutcome send(aegis::assistant::Chat& chat,
                                           const aegis::assistant::GroundedQuestion& question) = 0;
};

// Reads a saved chat back (US-11.2's ChatStore in production).
class IChatLoader {
public:
    virtual ~IChatLoader() = default;
    [[nodiscard]] virtual aegis::assistant::LoadResult load(const std::string& chatId) const = 0;
};

struct ChatHistoryEntry {
    std::string chatId;
    // The chat's first question, shortened.
    std::string title;

    bool operator==(const ChatHistoryEntry&) const = default;
};

class AssistantChat {
public:
    // Returns a new id accepted by ChatStore ([A-Za-z0-9_-]+), unique within the project.
    using ChatIdGenerator = std::function<std::string()>;

    AssistantChat(IChatQuestionSender& sender, const IChatLoader& loader,
                  const IContextItemProvider& items, const IContextSizeEstimator& sizes,
                  ChatIdGenerator nextChatId);

    AssistantChat(const AssistantChat&) = delete;
    AssistantChat& operator=(const AssistantChat&) = delete;
    AssistantChat(AssistantChat&&) = delete;
    AssistantChat& operator=(AssistantChat&&) = delete;
    // Cancels a running answer and waits for it to end.
    ~AssistantChat();

    // Composer.
    [[nodiscard]] const std::string& draft() const noexcept { return draft_; }
    void setDraft(std::string draft);
    // srs.fr.33.6: false while the message is empty or an answer is running.
    [[nodiscard]] bool canSend() const;
    // srs.fr.33.6. On success the draft is cleared and the chips are kept for the next question; on
    // a rejected send the draft, chips and chat are unchanged and lastError() says why.
    bool send();
    // srs.fr.33.5. Returns what the key did.
    EnterKeyAction pressEnter(bool shiftHeld);
    // The last rejection or answer failure; cleared by the next successful send.
    [[nodiscard]] const std::string& lastError() const noexcept { return lastError_; }

    // Answer progress (srs.fr.33.7, srs.fr.33.8).
    [[nodiscard]] bool isAnswering() const noexcept { return answer_ != nullptr; }
    [[nodiscard]] bool canStop() const noexcept { return isAnswering() && !stopRequested_; }
    // Cancels the running answer once; the part received so far stays, marked Stopped.
    void stop();
    // Call once per frame: collects a finished answer.
    void update();

    // The messages the panel may draw now: every message, except the answer still being written by
    // the answering thread.
    [[nodiscard]] std::span<const aegis::assistant::ChatMessage> settledMessages() const;
    [[nodiscard]] const std::string& currentChatId() const noexcept { return chat_->id; }

    // Context chips (srs.fr.33.9, srs.fr.33.10).
    [[nodiscard]] const ContextChipSet& chips() const noexcept { return chips_; }
    void removeChip(ContextChipId id);
    [[nodiscard]] std::vector<ContextChipType> addableChipTypes() const;
    [[nodiscard]] bool canAttach(ContextChipType type) const;
    bool attach(ContextChipType type);
    // Takes the draft and contexts a US-33.1 entry point (Ask about selection) prepared.
    void importEntryPoint(std::string draft, const std::vector<AssistantContext>& contexts);

    // Chats (srs.fr.33.11, srs.fr.33.12). Switching waits until no answer is running.
    [[nodiscard]] bool canSwitchChat() const noexcept { return !isAnswering(); }
    bool newChat();
    // Earlier chats, most recent first; the open chat is not listed.
    [[nodiscard]] const std::vector<ChatHistoryEntry>& history() const noexcept { return history_; }
    bool openChat(const std::string& chatId);

private:
    void collectFinishedAnswer();
    void keepCurrentChatInHistory();

    IChatQuestionSender& sender_;
    const IChatLoader& loader_;
    const IContextItemProvider& items_;
    const IContextSizeEstimator& sizes_;
    ChatIdGenerator nextChatId_;

    // Heap-owned so its address stays fixed while an answer writes into it.
    std::unique_ptr<aegis::assistant::Chat> chat_;
    std::string draft_;
    ContextChipSet chips_;
    std::vector<ChatHistoryEntry> history_;
    std::unique_ptr<IActiveAnswer> answer_;
    std::size_t answerIndex_ = 0;
    bool stopRequested_ = false;
    std::string lastError_;
};

} // namespace editor
