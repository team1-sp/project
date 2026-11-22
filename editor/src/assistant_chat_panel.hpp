#pragma once

#include <string>

#include "editor/assistant_chat.hpp"

namespace editor {

// Draws the AI Assistant panel's body with Dear ImGui (US-33.2): History, New Chat, the settled
// messages with their citations, the spinner and Stop while an answer runs, the context chips with
// their + menu, and the message box with Send. All decisions live in AssistantChat.
class AssistantChatPanel {
public:
    explicit AssistantChatPanel(AssistantChat& chat)
        : chat_(chat) {}

    // Call inside the panel's ImGui window. `focusMessageBox` moves keyboard focus to the box.
    void draw(bool focusMessageBox);

private:
    void drawToolbar();
    void drawMessages();
    void drawAnswerProgress();
    void drawChips();
    void drawComposer(bool focusMessageBox);

    AssistantChat& chat_;
    // ImGui edits this buffer; it is copied to AssistantChat after each edit.
    std::string buffer_;
    // Set after a send so the box keeps keyboard focus in the next frame.
    bool refocusMessageBox_ = false;
};

} // namespace editor
