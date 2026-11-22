#include "assistant_chat_panel.hpp"

#include <array>
#include <cfloat>
#include <cstddef>
#include <string>

#include <imgui.h>
#include <imgui_internal.h>

#include "aegis/assistant/grounded/citations.hpp"
#include <IconsFontAwesome6.h>

namespace editor {

namespace aa = aegis::assistant;

namespace {

constexpr float messageBoxHeight = 90.0F;
constexpr double spinnerFramesPerSecond = 8.0;

struct ComposerCallbackData {
    std::string* buffer = nullptr;
    bool* enterPressed = nullptr;
};

// Grows `buffer` for ImGui and turns a plain Enter into a send request (srs.fr.33.5). Shift+Enter
// passes through, so ImGui inserts the new line at the cursor.
int composerCallback(ImGuiInputTextCallbackData* data) {
    auto* context = static_cast<ComposerCallbackData*>(data->UserData);
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        context->buffer->resize(static_cast<std::size_t>(data->BufTextLen));
        data->Buf = context->buffer->data();
        return 0;
    }
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter && data->EventChar == '\n' &&
        !ImGui::GetIO().KeyShift) {
        *context->enterPressed = true;
        return 1;
    }
    return 0;
}

[[nodiscard]] const char* spinnerFrame() {
    static constexpr std::array<const char*, 4> frames = {"|", "/", "-", "\\"};
    const auto index =
        static_cast<std::size_t>(ImGui::GetTime() * spinnerFramesPerSecond) % frames.size();
    return frames[index];
}

[[nodiscard]] const char* stateSuffix(aa::MessageState state) {
    switch (state) {
    case aa::MessageState::Stopped:
        return " (stopped)";
    case aa::MessageState::Failed:
        return " (failed)";
    case aa::MessageState::Streaming:
    case aa::MessageState::Complete:
        return "";
    }
    return "";
}

void drawCitations(const aa::ChatMessage& message) {
    for (const aa::Citation& citation : message.citations) {
        const aa::DecodedCitation decoded = aa::decodeCitation(citation);
        const std::string label = decoded.succeeded() ? aa::describeCitation(*decoded.citation)
                                                      : citation.kind + ": unreadable citation";
        ImGui::BulletText("%s", label.c_str());
    }
}

void drawMessage(const aa::ChatMessage& message) {
    const bool question = message.role == aa::MessageRole::Question;
    ImGui::TextDisabled("%s%s", question ? "You" : "Assistant", stateSuffix(message.state));
    ImGui::TextWrapped("%s", message.text.c_str());
    drawCitations(message);
    ImGui::Spacing();
}

} // namespace

void AssistantChatPanel::draw(bool focusMessageBox) {
    drawToolbar();
    ImGui::Separator();
    drawMessages();
    ImGui::Separator();
    drawChips();
    drawComposer(focusMessageBox);
}

void AssistantChatPanel::drawToolbar() {
    ImGui::BeginDisabled(!chat_.canSwitchChat());
    if (ImGui::Button(ICON_FA_PLUS " New Chat")) {
        static_cast<void>(chat_.newChat());
    }
    ImGui::SameLine();
    if (ImGui::BeginCombo("##History", "History", ImGuiComboFlags_WidthFitPreview)) {
        if (chat_.history().empty()) {
            ImGui::TextDisabled("No earlier chats");
        }
        // Copy: opening a chat changes the list.
        const std::vector<ChatHistoryEntry> entries = chat_.history();
        for (const ChatHistoryEntry& entry : entries) {
            ImGui::PushID(entry.chatId.c_str());
            if (ImGui::Selectable(entry.title.c_str())) {
                static_cast<void>(chat_.openChat(entry.chatId));
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
}

void AssistantChatPanel::drawMessages() {
    const float reserved = messageBoxHeight + ImGui::GetFrameHeightWithSpacing() * 3.0F;
    if (ImGui::BeginChild("##AssistantMessages", ImVec2(0.0F, -reserved))) {
        for (const aa::ChatMessage& message : chat_.settledMessages()) {
            drawMessage(message);
        }
        drawAnswerProgress();
        if (!chat_.lastError().empty()) {
            ImGui::TextColored(ImVec4(1.0F, 0.45F, 0.4F, 1.0F), "%s", chat_.lastError().c_str());
        }
    }
    ImGui::EndChild();
}

void AssistantChatPanel::drawAnswerProgress() {
    if (!chat_.isAnswering()) {
        return;
    }
    ImGui::TextUnformatted(spinnerFrame());
    ImGui::SameLine();
    ImGui::TextUnformatted(chat_.canStop() ? "Preparing the answer..." : "Stopping...");
    ImGui::SameLine();
    ImGui::BeginDisabled(!chat_.canStop());
    if (ImGui::Button(ICON_FA_STOP " Stop")) {
        chat_.stop();
    }
    ImGui::EndDisabled();
}

void AssistantChatPanel::drawChips() {
    std::optional<ContextChipId> removed;
    for (const ContextChip& chip : chat_.chips().chips()) {
        ImGui::PushID(static_cast<int>(chip.id));
        const std::string text =
            chip.label + ": " + chip.detail + " (" + formatRequestSize(chip.requestBytes) + ")";
        ImGui::TextUnformatted(text.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(ICON_FA_XMARK)) {
            removed = chip.id;
        }
        ImGui::SameLine();
        ImGui::PopID();
    }
    if (removed) {
        chat_.removeChip(*removed);
    }

    const std::vector<ContextChipType> addable = chat_.addableChipTypes();
    ImGui::BeginDisabled(addable.empty());
    if (ImGui::SmallButton(ICON_FA_PLUS "##AddContext")) {
        ImGui::OpenPopup("AddContext");
    }
    ImGui::EndDisabled();
    if (ImGui::BeginPopup("AddContext")) {
        for (const ContextChipType type : addable) {
            const std::string label(contextChipTypeLabel(type));
            if (ImGui::MenuItem(label.c_str(), nullptr, false, chat_.canAttach(type))) {
                static_cast<void>(chat_.attach(type));
            }
        }
        ImGui::EndPopup();
    }
}

void AssistantChatPanel::drawComposer(bool focusMessageBox) {
    // Mirror model-side changes (an entry point's draft, a cleared draft after Send) when the box
    // is not being edited, or after a send cleared it.
    const ImGuiID boxId = ImGui::GetID("##AssistantMessage");
    if (chat_.draft() != buffer_ && ImGui::GetActiveID() != boxId) {
        buffer_ = chat_.draft();
    }
    if (focusMessageBox || refocusMessageBox_) {
        ImGui::SetKeyboardFocusHere();
        refocusMessageBox_ = false;
    }
    bool enterPressed = false;
    ComposerCallbackData callbackData{&buffer_, &enterPressed};
    const ImGuiInputTextFlags flags =
        ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackCharFilter;
    if (ImGui::InputTextMultiline("##AssistantMessage", buffer_.data(), buffer_.capacity() + 1,
                                  ImVec2(-FLT_MIN, messageBoxHeight), flags, composerCallback,
                                  &callbackData)) {
        chat_.setDraft(buffer_);
    }

    ImGui::BeginDisabled(!chat_.canSend());
    const bool sendClicked = ImGui::Button(ICON_FA_PAPER_PLANE " Send");
    ImGui::EndDisabled();

    bool sent = false;
    if (enterPressed) {
        // pressEnter() sends unless the message is empty; a rejected send keeps the draft.
        sent = chat_.pressEnter(false) == EnterKeyAction::Send && chat_.isAnswering();
    } else if (sendClicked) {
        sent = chat_.send();
    }
    if (sent) {
        // Drop ImGui's copy of the sent text and keep typing in the box.
        ImGui::ClearActiveID();
        buffer_.clear();
        refocusMessageBox_ = true;
    }
}

} // namespace editor
