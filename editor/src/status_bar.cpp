#include "editor/status_bar.hpp"

#include <cmath>
#include <cstddef>
#include <string>
#include <utility>

namespace editor {

namespace {

// U+2014 EM DASH: a value that does not exist now (B-01 shows "FPS —" in Edit mode).
constexpr const char* noValue = "\xE2\x80\x94";

std::size_t roundedCount(double value) {
    if (!(value > 0.0)) {
        return 0;
    }
    return static_cast<std::size_t>(std::llround(value));
}

std::string framesPerSecondText(const EditorStatus& status, double renderFramesPerSecond) {
    switch (status.mode) {
    case EditorMode::Edit:
        // No game is running, so it has no frame rate (B-01).
        return std::string("FPS ") + noValue;
    case EditorMode::Play:
    case EditorMode::Paused:
        return "FPS " + groupDigits(roundedCount(renderFramesPerSecond));
    case EditorMode::Headless:
        // A headless run renders nothing; its speed is the tick rate (B-04).
        return "FPS " + groupDigits(roundedCount(status.headlessTicksPerSecond)) + " ticks/s";
    }
    return std::string("FPS ") + noValue;
}

std::string joined(const std::vector<std::string>& names, const char* separator) {
    std::string text;
    for (const std::string& name : names) {
        if (!text.empty()) {
            text += separator;
        }
        text += name;
    }
    return text;
}

} // namespace

const char* editorModeText(EditorMode mode) {
    switch (mode) {
    case EditorMode::Edit:
        return "Edit mode";
    case EditorMode::Play:
        return "Play mode";
    case EditorMode::Paused:
        return "Paused";
    case EditorMode::Headless:
        return "Headless run";
    }
    return "Edit mode";
}

std::string groupDigits(std::size_t value) {
    std::string digits = std::to_string(value);
    std::string text;
    const std::size_t count = digits.size();
    for (std::size_t i = 0; i < count; ++i) {
        if (i > 0 && (count - i) % 3 == 0) {
            text += ' ';
        }
        text += digits[i];
    }
    return text;
}

StatusBar buildStatusBar(const EditorStatus& status, double renderFramesPerSecond,
                         IEditorActions& actions) {
    StatusBar bar;

    // srs.fr.14.7: the last Console message; clicking it opens the Console panel.
    if (!status.lastConsoleMessage.empty()) {
        bar.message.text = status.lastConsoleMessage;
        bar.message.click.label = "Open the Console panel";
        bar.message.click.enabled = [] { return true; };
        bar.message.click.run = [&actions] { actions.showPanel(Panel::Console); };
    }

    // srs.fr.14.11: the active debug flags in orange.
    if (!status.activeDebugFlags.empty()) {
        StatusItem flags;
        flags.text = joined(status.activeDebugFlags, ", ");
        flags.colour = StatusTextColour::Warning;
        bar.right.push_back(std::move(flags));
    }

    // srs.fr.14.10: REC while telemetry records; clicking it opens the Telemetry panel, which
    // comes with EP-30, so the click stays off until then.
    if (status.telemetryRecording) {
        StatusItem rec;
        rec.text = "REC telemetry";
        rec.dot = StatusDot::Red;
        rec.click.label = "Open the Telemetry panel";
        bar.right.push_back(std::move(rec));
    }

    // srs.fr.14.8: the mode, the FPS and the number of entities.
    StatusItem mode;
    mode.text = editorModeText(status.mode);
    bar.right.push_back(std::move(mode));

    StatusItem fps;
    fps.text = framesPerSecondText(status, renderFramesPerSecond);
    bar.right.push_back(std::move(fps));

    StatusItem entities;
    entities.text = "Entities " +
                    (status.entityCount ? groupDigits(*status.entityCount) : std::string(noValue));
    bar.right.push_back(std::move(entities));

    // srs.fr.14.9: the AI service state; clicking it opens Project Settings at the AI and Privacy
    // tab (F-04), which comes with US-33.5, so the click stays off until then.
    StatusItem ai;
    if (status.aiService == AiServiceState::Connected) {
        ai.text = status.aiServiceName.empty()
                      ? std::string("AI service: connected")
                      : "AI service: " + status.aiServiceName + " \xC2\xB7 connected";
        ai.dot = StatusDot::Green;
    } else {
        ai.text = "AI service: offline";
        ai.dot = StatusDot::Grey;
    }
    ai.click.label = "Open Project Settings at AI and Privacy";
    bar.right.push_back(std::move(ai));

    return bar;
}

} // namespace editor
