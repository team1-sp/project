#include "aegis/assistant/chat_store.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string_view>
#include <system_error>
#include <utility>

namespace aegis::assistant {
namespace {

constexpr std::string_view magic = "AEGIS_CHAT";
constexpr std::size_t maximumFieldBytes = 16U * 1024U * 1024U;
constexpr std::size_t maximumItems = 100'000U;
constexpr std::uintmax_t maximumChatBytes = 64U * 1024U * 1024U;

[[nodiscard]] bool validChatId(std::string_view id) {
    if (id.empty()) {
        return false;
    }
    for (const char character : id) {
        const bool valid =
            (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') || character == '-' || character == '_';
        if (!valid) {
            return false;
        }
    }
    return true;
}

void writeString(std::ostream& output, std::string_view name, std::string_view value) {
    output << name << ' ' << value.size() << '\n';
    output.write(value.data(), static_cast<std::streamsize>(value.size()));
    output << '\n';
}

template <typename Enum>
void writeEnum(std::ostream& output, std::string_view name, Enum value) {
    output << name << ' ' << static_cast<unsigned int>(value) << '\n';
}

[[nodiscard]] bool parseSize(std::string_view text, std::size_t maximum, std::size_t& value) {
    if (text.empty()) {
        return false;
    }
    std::size_t parsed = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), parsed);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || parsed > maximum) {
        return false;
    }
    value = parsed;
    return true;
}

[[nodiscard]] bool readNamedSize(std::istream& input, std::string_view name, std::size_t maximum,
                                 std::size_t& value, std::string& error) {
    std::string line;
    if (!std::getline(input, line)) {
        error = "unexpected end of chat data while reading " + std::string(name);
        return false;
    }
    const std::string prefix = std::string(name) + ' ';
    if (!line.starts_with(prefix) ||
        !parseSize(std::string_view(line).substr(prefix.size()), maximum, value)) {
        error = "invalid " + std::string(name) + " field";
        return false;
    }
    return true;
}

[[nodiscard]] bool readString(std::istream& input, std::string_view name, std::string& value,
                              std::string& error) {
    std::size_t size = 0;
    if (!readNamedSize(input, name, maximumFieldBytes, size, error)) {
        return false;
    }
    value.resize(size);
    input.read(value.data(), static_cast<std::streamsize>(size));
    if (input.gcount() != static_cast<std::streamsize>(size) || input.get() != '\n') {
        error = "truncated " + std::string(name) + " field";
        return false;
    }
    return true;
}

template <typename Enum>
[[nodiscard]] bool readEnum(std::istream& input, std::string_view name, unsigned int maximum,
                            Enum& value, std::string& error) {
    std::size_t raw = 0;
    if (!readNamedSize(input, name, maximum, raw, error)) {
        return false;
    }
    value = static_cast<Enum>(raw);
    return true;
}

[[nodiscard]] bool expectLine(std::istream& input, std::string_view expected, std::string& error) {
    std::string line;
    if (!std::getline(input, line) || line != expected) {
        error = "expected " + std::string(expected);
        return false;
    }
    return true;
}

[[nodiscard]] LoadResult readChat(std::istream& input) {
    std::string header;
    if (!std::getline(input, header)) {
        return {{}, "chat data is empty"};
    }
    const std::string prefix = std::string(magic) + ' ';
    std::size_t version = 0;
    if (!header.starts_with(prefix) ||
        !parseSize(std::string_view(header).substr(prefix.size()),
                   std::numeric_limits<unsigned int>::max(), version)) {
        return {{}, "invalid chat header"};
    }
    if (version != chatFormatVersion) {
        return {{}, "unsupported chat format version " + std::to_string(version)};
    }

    Chat chat;
    chat.formatVersion = static_cast<unsigned int>(version);
    std::string error;
    if (!readString(input, "chat_id", chat.id, error) || !validChatId(chat.id)) {
        return {{}, error.empty() ? "invalid chat id" : error};
    }

    std::size_t messageCount = 0;
    if (!readNamedSize(input, "messages", maximumItems, messageCount, error)) {
        return {{}, error};
    }
    chat.messages.reserve(messageCount);
    for (std::size_t messageIndex = 0; messageIndex < messageCount; ++messageIndex) {
        ChatMessage message;
        std::size_t categoryCount = 0;
        std::size_t citationCount = 0;
        if (!expectLine(input, "message", error) ||
            !readEnum(input, "role", static_cast<unsigned int>(MessageRole::Answer), message.role,
                      error) ||
            !readEnum(input, "state", static_cast<unsigned int>(MessageState::Failed),
                      message.state, error) ||
            !readEnum(input, "content", static_cast<unsigned int>(MessageContent::DesignSuggestion),
                      message.content, error) ||
            !readString(input, "text", message.text, error) ||
            !readNamedSize(input, "categories", maximumItems, categoryCount, error)) {
            return {{}, error};
        }
        message.dataCategories.reserve(categoryCount);
        for (std::size_t index = 0; index < categoryCount; ++index) {
            std::string category;
            if (!readString(input, "category", category, error)) {
                return {{}, error};
            }
            message.dataCategories.push_back(std::move(category));
        }
        if (!readNamedSize(input, "citations", maximumItems, citationCount, error)) {
            return {{}, error};
        }
        message.citations.reserve(citationCount);
        for (std::size_t index = 0; index < citationCount; ++index) {
            Citation citation;
            if (!readString(input, "citation_kind", citation.kind, error) ||
                !readString(input, "citation_payload", citation.payload, error)) {
                return {{}, error};
            }
            message.citations.push_back(std::move(citation));
        }
        if (!expectLine(input, "end_message", error)) {
            return {{}, error};
        }
        chat.messages.push_back(std::move(message));
    }
    if (!expectLine(input, "end_chat", error)) {
        return {{}, error};
    }
    input >> std::ws;
    if (!input.eof()) {
        return {{}, "unexpected data after end_chat"};
    }
    return {std::move(chat), {}};
}

[[nodiscard]] StoreResult writeChat(const Chat& chat, const std::filesystem::path& path) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return {"could not open temporary chat file"};
    }
    output << magic << ' ' << chatFormatVersion << '\n';
    writeString(output, "chat_id", chat.id);
    output << "messages " << chat.messages.size() << '\n';
    for (const ChatMessage& message : chat.messages) {
        output << "message\n";
        writeEnum(output, "role", message.role);
        writeEnum(output, "state", message.state);
        writeEnum(output, "content", message.content);
        writeString(output, "text", message.text);
        output << "categories " << message.dataCategories.size() << '\n';
        for (const std::string& category : message.dataCategories) {
            writeString(output, "category", category);
        }
        output << "citations " << message.citations.size() << '\n';
        for (const Citation& citation : message.citations) {
            writeString(output, "citation_kind", citation.kind);
            writeString(output, "citation_payload", citation.payload);
        }
        output << "end_message\n";
    }
    output << "end_chat\n";
    output.flush();
    if (!output) {
        return {"could not write complete chat data"};
    }
    return {};
}

} // namespace

std::filesystem::path ChatStore::chatFilePath(const std::string& chatId) const {
    if (!validChatId(chatId)) {
        return {};
    }
    return paths_.userLocalArea() / "assistant" / "chats" / (chatId + ".chat");
}

StoreResult ChatStore::save(const Chat& chat) const {
    if (chat.formatVersion != chatFormatVersion) {
        return {"cannot save unsupported chat format version"};
    }
    const std::filesystem::path destination = chatFilePath(chat.id);
    if (destination.empty()) {
        return {"invalid chat id"};
    }

    std::error_code fileError;
    if (std::filesystem::exists(destination, fileError)) {
        const std::uintmax_t existingBytes = std::filesystem::file_size(destination, fileError);
        if (fileError) {
            return {"could not inspect existing chat size: " + fileError.message()};
        }
        if (existingBytes > maximumChatBytes) {
            return {"refusing to overwrite existing chat: chat data is too large"};
        }
        std::ifstream existing(destination, std::ios::binary);
        const LoadResult existingResult = readChat(existing);
        if (!existingResult.succeeded()) {
            return {"refusing to overwrite existing chat: " + existingResult.error};
        }
    }
    if (fileError) {
        return {"could not inspect existing chat: " + fileError.message()};
    }

    std::filesystem::create_directories(destination.parent_path(), fileError);
    if (fileError) {
        return {"could not create user-local chat directory: " + fileError.message()};
    }

    const std::filesystem::path temporary = destination.string() + ".tmp";
    const std::filesystem::path backup = destination.string() + ".bak";
    if (std::filesystem::exists(temporary, fileError) ||
        std::filesystem::exists(backup, fileError)) {
        return {"chat recovery file already exists"};
    }
    if (fileError) {
        return {"could not inspect chat recovery files: " + fileError.message()};
    }

    const StoreResult writeResult = writeChat(chat, temporary);
    if (!writeResult.succeeded()) {
        std::filesystem::remove(temporary, fileError);
        return writeResult;
    }
    const std::uintmax_t writtenBytes = std::filesystem::file_size(temporary, fileError);
    if (fileError || writtenBytes > maximumChatBytes) {
        std::error_code cleanupError;
        std::filesystem::remove(temporary, cleanupError);
        return {fileError ? "could not inspect temporary chat size: " + fileError.message()
                          : "chat data is too large"};
    }

    const bool hadDestination = std::filesystem::exists(destination, fileError);
    if (fileError) {
        std::filesystem::remove(temporary, fileError);
        return {"could not inspect chat destination: " + fileError.message()};
    }
    if (hadDestination) {
        std::filesystem::rename(destination, backup, fileError);
        if (fileError) {
            const std::string message = fileError.message();
            std::error_code cleanupError;
            std::filesystem::remove(temporary, cleanupError);
            return {"could not prepare existing chat for replacement: " + message};
        }
    }

    std::filesystem::rename(temporary, destination, fileError);
    if (fileError) {
        const std::string message = fileError.message();
        if (hadDestination) {
            std::error_code restoreError;
            std::filesystem::rename(backup, destination, restoreError);
        }
        std::error_code cleanupError;
        std::filesystem::remove(temporary, cleanupError);
        return {"could not install updated chat: " + message};
    }
    if (hadDestination) {
        std::filesystem::remove(backup, fileError);
        if (fileError) {
            return {"chat saved but old backup could not be removed: " + fileError.message()};
        }
    }
    return {};
}

LoadResult ChatStore::load(const std::string& chatId) const {
    const std::filesystem::path path = chatFilePath(chatId);
    if (path.empty()) {
        return {{}, "invalid chat id"};
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {{}, "chat file not found"};
    }
    std::error_code fileError;
    const std::uintmax_t bytes = std::filesystem::file_size(path, fileError);
    if (fileError) {
        return {{}, "could not inspect chat size: " + fileError.message()};
    }
    if (bytes > maximumChatBytes) {
        return {{}, "chat data is too large"};
    }
    LoadResult result = readChat(input);
    if (result.succeeded() && result.chat->id != chatId) {
        return {{}, "chat id does not match its file name"};
    }
    return result;
}

} // namespace aegis::assistant
