#pragma once

#include <functional>
#include <optional>
#include <string>

namespace editor {

// The operating systems the Editor runs on (DEC-029). Keyboard shortcuts read differently on
// each (srs.fr.14.6).
enum class Platform {
    Windows,
    MacOS,
};

// The platform this Editor was built for.
[[nodiscard]] Platform currentPlatform();

// The keys that Editor shortcuts use with or without modifiers.
enum class Key {
    // clang-format off
    A, B, C, D, E, F, H, N, O, P, Q, R, S, T, V, W, X, Y, Z,
    Digit1, Digit2, Digit3, Digit4, Digit5, Digit6,
    F2, F4, Delete, Escape, Comma,
    // clang-format on
};

// A keyboard shortcut as the Windows Editor writes it. `ctrl` stands for Cmd on macOS
// (srs.fr.14.6).
struct Shortcut {
    Key key = Key::A;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
};

// The shortcut as menus show it on `platform`: "Ctrl+Shift+S" on Windows and "Cmd+Shift+S" on
// macOS (srs.fr.14.6).
[[nodiscard]] std::string shortcutText(const Shortcut& shortcut, Platform platform);

// Something the user can choose in a menu or on the toolbar.
struct Command {
    std::string label;
    // Empty when the command has no shortcut.
    std::optional<Shortcut> shortcut;
    // Replaces `shortcut` on macOS, where some commands use another key (File > Exit is Cmd+Q,
    // srs.fr.15.15).
    std::optional<Shortcut> macosShortcut;
    // The condition under which the command is enabled. Empty when the feature behind the
    // command is not built yet, which keeps it disabled.
    std::function<bool()> enabled;
    // What choosing the command does.
    std::function<void()> run;

    // True when the command can be chosen now. A disabled command is drawn in muted text and
    // with no hover effect (srs.fr.14.5).
    [[nodiscard]] bool isEnabled() const;
    // The shortcut on `platform`, if the command has one there.
    [[nodiscard]] std::optional<Shortcut> shortcutOn(Platform platform) const;
};

} // namespace editor
