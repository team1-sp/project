#include "editor/command.hpp"

#include <array>
#include <cstddef>

namespace editor {

namespace {

// Indexed by Key, in the order of its enumerators.
constexpr std::array<const char*, 30> keyNames = {
    "A", "B", "C", "D", "E", "F", "H", "N", "O", "P", "Q",  "R",  "S",   "T",   "V",
    "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "F2", "F4", "Del", "Esc", ",",
};

} // namespace

Platform currentPlatform() {
#if defined(__APPLE__)
    return Platform::MacOS;
#else
    return Platform::Windows;
#endif
}

std::string shortcutText(const Shortcut& shortcut, Platform platform) {
    std::string text;
    if (shortcut.ctrl) {
        text += platform == Platform::MacOS ? "Cmd+" : "Ctrl+";
    }
    if (shortcut.shift) {
        text += "Shift+";
    }
    if (shortcut.alt) {
        text += "Alt+";
    }
    text += keyNames.at(static_cast<std::size_t>(shortcut.key));
    return text;
}

bool Command::isEnabled() const {
    return enabled && run && enabled();
}

std::optional<Shortcut> Command::shortcutOn(Platform platform) const {
    if (platform == Platform::MacOS && macosShortcut) {
        return macosShortcut;
    }
    return shortcut;
}

} // namespace editor
