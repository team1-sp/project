#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "aegis/assistant/chat.hpp"

namespace aegis::assistant {

// Supplied by the project layer. It deliberately exposes only the user's local project area, so
// chat persistence cannot accidentally select shared project settings.
class UserProjectPaths {
public:
    virtual ~UserProjectPaths() = default;

    [[nodiscard]] virtual std::filesystem::path userLocalArea() const = 0;
};

struct StoreResult {
    std::string error;

    [[nodiscard]] bool succeeded() const noexcept { return error.empty(); }
};

struct LoadResult {
    std::optional<Chat> chat;
    std::string error;

    [[nodiscard]] bool succeeded() const noexcept { return chat.has_value() && error.empty(); }
};

// Versioned, atomic chat persistence under <user-local>/assistant/chats. Existing corrupt or
// unsupported files are never overwritten.
class ChatStore {
public:
    explicit ChatStore(const UserProjectPaths& paths)
        : paths_(paths) {}

    [[nodiscard]] StoreResult save(const Chat& chat) const;
    [[nodiscard]] LoadResult load(const std::string& chatId) const;
    [[nodiscard]] std::filesystem::path chatFilePath(const std::string& chatId) const;

private:
    const UserProjectPaths& paths_;
};

} // namespace aegis::assistant
