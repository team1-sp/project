// Editor status bar tests (US-14.2; srs.fr.14.7 to 14.11). They use the window-free status bar
// model, so they need neither raylib nor Dear ImGui.
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/main_menu.hpp"
#include "editor/status_bar.hpp"

namespace {

using editor::AiServiceState;
using editor::EditorMode;
using editor::EditorStatus;
using editor::Panel;
using editor::StatusBar;
using editor::StatusDot;
using editor::StatusItem;
using editor::StatusTextColour;

class RecordingActions final : public editor::IEditorActions {
public:
    void exit() override { calls.push_back("exit"); }
    void showPanel(Panel panel) override {
        calls.push_back(std::string("show ") + editor::panelName(panel));
    }
    [[nodiscard]] bool canOpenAiPrivacySettings() const override { return settingsAvailable; }
    void openAiPrivacySettings() override { calls.push_back("open AI settings"); }

    std::vector<std::string> calls;
    bool settingsAvailable = false;
};

std::vector<std::string> rightTexts(const StatusBar& bar) {
    std::vector<std::string> texts;
    for (const StatusItem& item : bar.right) {
        texts.push_back(item.text);
    }
    return texts;
}

const std::string emDash = "\xE2\x80\x94";

TEST(StatusBarTest, FreshEditorShowsEditModeNoMessageAndOfflineAiService) {
    RecordingActions actions;
    const StatusBar bar = buildStatusBar(EditorStatus{}, 60.0, actions);

    EXPECT_TRUE(bar.message.text.empty());
    EXPECT_FALSE(bar.message.isClickable());
    const std::vector<std::string> expected = {"Edit mode", "FPS " + emDash, "Entities " + emDash,
                                               "AI service: offline"};
    EXPECT_EQ(rightTexts(bar), expected);
}

// srs.fr.14.7
TEST(StatusBarTest, ShowsTheLastConsoleMessageAndClickingItOpensTheConsole) {
    RecordingActions actions;
    EditorStatus status;
    status.lastConsoleMessage = "Saved DemoScene.scene";
    const StatusBar bar = buildStatusBar(status, 60.0, actions);

    EXPECT_EQ(bar.message.text, "Saved DemoScene.scene");
    ASSERT_TRUE(bar.message.isClickable());
    bar.message.click.run();
    EXPECT_EQ(actions.calls, std::vector<std::string>{"show Console"});
}

// srs.fr.14.8
TEST(StatusBarTest, ShowsTheModeTheFpsAndTheNumberOfEntities) {
    RecordingActions actions;
    EditorStatus status;
    status.entityCount = 24;

    status.mode = EditorMode::Play;
    EXPECT_EQ(
        rightTexts(buildStatusBar(status, 59.7, actions)),
        (std::vector<std::string>{"Play mode", "FPS 60", "Entities 24", "AI service: offline"}));

    status.mode = EditorMode::Paused;
    EXPECT_EQ(rightTexts(buildStatusBar(status, 60.0, actions)),
              (std::vector<std::string>{"Paused", "FPS 60", "Entities 24", "AI service: offline"}));

    status.mode = EditorMode::Headless;
    status.headlessTicksPerSecond = 2840.4;
    EXPECT_EQ(rightTexts(buildStatusBar(status, 60.0, actions)),
              (std::vector<std::string>{"Headless run", "FPS 2 840 ticks/s", "Entities 24",
                                        "AI service: offline"}));

    status.mode = EditorMode::Edit;
    EXPECT_EQ(rightTexts(buildStatusBar(status, 60.0, actions)),
              (std::vector<std::string>{"Edit mode", "FPS " + emDash, "Entities 24",
                                        "AI service: offline"}));
}

// srs.fr.14.9
TEST(StatusBarTest, ShowsTheAiServiceAsAGreenOrGreyDot) {
    RecordingActions actions;
    EditorStatus status;

    const StatusItem offline = buildStatusBar(status, 60.0, actions).right.back();
    EXPECT_EQ(offline.text, "AI service: offline");
    EXPECT_EQ(offline.dot, StatusDot::Grey);

    status.aiService = AiServiceState::Connected;
    const StatusItem connected = buildStatusBar(status, 60.0, actions).right.back();
    EXPECT_EQ(connected.text, "AI service: connected");
    EXPECT_EQ(connected.dot, StatusDot::Green);

    status.aiServiceName = "local";
    EXPECT_EQ(buildStatusBar(status, 60.0, actions).right.back().text,
              "AI service: local \xC2\xB7 connected");
}

// srs.fr.14.9: US-33.5 enables this target when a project settings controller is available.
TEST(StatusBarTest, AiServiceClickOpensAiPrivacySettingsWhenAvailable) {
    RecordingActions actions;
    actions.settingsAvailable = true;
    const StatusItem ai = buildStatusBar(EditorStatus{}, 60.0, actions).right.back();
    EXPECT_EQ(ai.click.label, "Open Project Settings at AI and Privacy");
    ASSERT_TRUE(ai.isClickable());
    ai.click.run();
    EXPECT_EQ(actions.calls, std::vector<std::string>{"open AI settings"});
}

// srs.fr.14.10
TEST(StatusBarTest, ShowsRedRecOnlyWhileTelemetryRecords) {
    RecordingActions actions;
    EditorStatus status;
    for (const StatusItem& item : buildStatusBar(status, 60.0, actions).right) {
        EXPECT_NE(item.dot, StatusDot::Red);
    }

    status.telemetryRecording = true;
    const StatusBar bar = buildStatusBar(status, 60.0, actions);
    ASSERT_EQ(bar.right.size(), 5U);
    EXPECT_EQ(bar.right.front().text, "REC telemetry");
    EXPECT_EQ(bar.right.front().dot, StatusDot::Red);
    EXPECT_EQ(bar.right.front().click.label, "Open the Telemetry panel");
    // The Telemetry panel comes with EP-30.
    EXPECT_FALSE(bar.right.front().isClickable());
}

// srs.fr.14.11
TEST(StatusBarTest, ShowsActiveDebugFlagsInOrangeFirst) {
    RecordingActions actions;
    EditorStatus status;
    for (const StatusItem& item : buildStatusBar(status, 60.0, actions).right) {
        EXPECT_EQ(item.colour, StatusTextColour::Muted);
    }

    status.activeDebugFlags = {"No collider", "Unlock without item"};
    status.telemetryRecording = true;
    const StatusBar bar = buildStatusBar(status, 60.0, actions);
    ASSERT_EQ(bar.right.size(), 6U);
    EXPECT_EQ(bar.right[0].text, "No collider, Unlock without item");
    EXPECT_EQ(bar.right[0].colour, StatusTextColour::Warning);
    EXPECT_FALSE(bar.right[0].isClickable());
    EXPECT_EQ(bar.right[1].text, "REC telemetry");
    for (std::size_t i = 1; i < bar.right.size(); ++i) {
        EXPECT_EQ(bar.right[i].colour, StatusTextColour::Muted);
    }
}

TEST(StatusBarTest, GroupsDigitsInThrees) {
    EXPECT_EQ(editor::groupDigits(0), "0");
    EXPECT_EQ(editor::groupDigits(999), "999");
    EXPECT_EQ(editor::groupDigits(1000), "1 000");
    EXPECT_EQ(editor::groupDigits(2840), "2 840");
    EXPECT_EQ(editor::groupDigits(1234567), "1 234 567");
}

TEST(StatusBarTest, NamesEveryMode) {
    EXPECT_STREQ(editor::editorModeText(EditorMode::Edit), "Edit mode");
    EXPECT_STREQ(editor::editorModeText(EditorMode::Play), "Play mode");
    EXPECT_STREQ(editor::editorModeText(EditorMode::Paused), "Paused");
    EXPECT_STREQ(editor::editorModeText(EditorMode::Headless), "Headless run");
}

} // namespace
