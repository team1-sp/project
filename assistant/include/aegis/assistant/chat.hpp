#pragma once

#include <string>
#include <vector>

namespace aegis::assistant {

inline constexpr unsigned int chatFormatVersion = 1;

enum class MessageRole { Question, Answer };
enum class MessageState { Complete, Streaming, Stopped, Failed };
enum class MessageContent { Conversation, DesignSuggestion };

struct Citation {
    std::string kind;
    std::string payload;

    bool operator==(const Citation&) const = default;
};

struct ChatMessage {
    MessageRole role = MessageRole::Question;
    MessageState state = MessageState::Complete;
    MessageContent content = MessageContent::Conversation;
    std::string text;
    std::vector<Citation> citations;
    std::vector<std::string> dataCategories;

    bool operator==(const ChatMessage&) const = default;
};

struct Chat {
    unsigned int formatVersion = chatFormatVersion;
    std::string id;
    std::vector<ChatMessage> messages;

    bool operator==(const Chat&) const = default;
};

} // namespace aegis::assistant
