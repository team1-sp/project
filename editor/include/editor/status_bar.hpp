#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "editor/command.hpp"
#include "editor/main_menu.hpp"

namespace editor {

// What the Editor is doing (srs.fr.14.8). Play mode and Pause come with EP-17, headless runs in
// the Editor with US-7.x; until then the Editor is always in Edit mode.
enum class EditorMode {
    Edit,
    Play,
    Paused,
    Headless,
};

// The mode as the status bar shows it: "Edit mode", "Play mode", "Paused", "Headless run" (B-01,
// B-02, B-04).
[[nodiscard]] const char* editorModeText(EditorMode mode);

// Whether the Editor can reach the project's AI service (srs.fr.14.9). The AI service settings
// come with US-33.5; until then it is offline.
enum class AiServiceState {
    Offline,
    Connected,
};

// The Editor state that the status bar shows (srs.fr.14.7 to 14.11). The stories that own each
// part fill it in; until then it holds what a fresh Editor with no project has.
struct EditorStatus {
    // The newest Console message (srs.fr.14.7); empty while the Console has none. The Console log
    // comes with US-24.1.
    std::string lastConsoleMessage;
    EditorMode mode = EditorMode::Edit;
    // Ticks per second of the headless run, shown as its FPS (B-04). Used in Headless mode only.
    double headlessTicksPerSecond = 0.0;
    // Entities in the loaded scenes; empty while no scene is loaded.
    std::optional<std::size_t> entityCount;
    AiServiceState aiService = AiServiceState::Offline;
    // The connected service's short name, such as "local" (F-04); may be empty.
    std::string aiServiceName;
    // True while telemetry is recording (srs.fr.14.10).
    bool telemetryRecording = false;
    // The names of the debug flags that are on, in the order of Run > Debug Flags (srs.fr.14.11).
    std::vector<std::string> activeDebugFlags;
};

// How a status bar item's text is coloured.
enum class StatusTextColour {
    Muted,
    // The orange of the design's warnings, for active debug flags (srs.fr.14.11).
    Warning,
};

// The coloured dot drawn in front of some items.
enum class StatusDot {
    None,
    // AI service connected (srs.fr.14.9).
    Green,
    // AI service offline (srs.fr.14.9).
    Grey,
    // Telemetry recording (srs.fr.14.10).
    Red,
};

// One item of the status bar.
struct StatusItem {
    std::string text;
    StatusTextColour colour = StatusTextColour::Muted;
    StatusDot dot = StatusDot::None;
    // What clicking the item does; its label is the tooltip. An item with no click target has
    // an empty label. A target whose feature is not built yet has no enabled condition, so the
    // item does not react to the mouse (srs.fr.14.5).
    Command click;

    [[nodiscard]] bool isClickable() const { return click.isEnabled(); }
};

// The status bar at the bottom of the main window: the last Console message at the left, and at
// the right the active debug flags, REC while telemetry records, the mode, the FPS, the number
// of entities and the AI service state, in this order (B-01, B-02, B-04).
struct StatusBar {
    // Has empty text while the Console has no message.
    StatusItem message;
    std::vector<StatusItem> right;
};

// Builds the status bar for `status` (srs.fr.14.7 to 14.11). `renderFramesPerSecond` is the
// Editor's render frame rate, which is the game's FPS in Play mode. `actions` must outlive the
// returned items.
[[nodiscard]] StatusBar buildStatusBar(const EditorStatus& status, double renderFramesPerSecond,
                                       IEditorActions& actions);

// `value` with a space between groups of three digits, as the design writes numbers: "2 840".
[[nodiscard]] std::string groupDigits(std::size_t value);

} // namespace editor
