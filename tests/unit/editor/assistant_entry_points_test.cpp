#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "editor/assistant_entry_points.hpp"
#include "editor/entity_navigation.hpp"
#include "editor/main_menu.hpp"

namespace {

class RunSource final : public editor::ILatestRunSource {
public:
    [[nodiscard]] std::optional<editor::LatestRun> latestRun() const override { return run; }

    std::optional<editor::LatestRun> run;
};

class RecordingConsentSender final : public editor::IConsentAwareQuestionSender {
public:
    void sendNewQuestion(editor::AssistantQuestion question) override {
        questions.push_back(std::move(question));
    }

    std::vector<editor::AssistantQuestion> questions;
};

class RecordingSettings final : public editor::IProjectSettingsOpener {
public:
    void openProjectSettings(editor::ProjectSettingsTab tab) override { openedTabs.push_back(tab); }

    std::vector<editor::ProjectSettingsTab> openedTabs;
};

class EntryPointTest : public testing::Test {
protected:
    editor::AssistantEntryPoints makeEntryPoints() {
        return editor::AssistantEntryPoints(selection, runs, questions, settings);
    }

    editor::EntityNavigation selection;
    RunSource runs;
    RecordingConsentSender questions;
    RecordingSettings settings;
};

} // namespace

TEST_F(EntryPointTest, OpenAssistant_OpensClosedPanelAndFocusesMessageBox) {
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    entryPoints.openAssistant();

    EXPECT_TRUE(entryPoints.isAssistantOpen());
    EXPECT_TRUE(entryPoints.takeMessageBoxFocusRequest());
    EXPECT_FALSE(entryPoints.takeMessageBoxFocusRequest());
}

TEST_F(EntryPointTest, OpenAssistant_FocusesMessageBoxWhenPanelIsAlreadyOpen) {
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();
    entryPoints.openAssistant();
    ASSERT_TRUE(entryPoints.takeMessageBoxFocusRequest());

    entryPoints.openAssistant();

    EXPECT_TRUE(entryPoints.isAssistantOpen());
    EXPECT_TRUE(entryPoints.takeMessageBoxFocusRequest());
}

// srs.fr.17.22: the toolbar command is the Ctrl+Shift+A route and toggles both directions.
TEST_F(EntryPointTest, CtrlShiftACommand_OpensAndClosesAssistantPanel) {
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    class Actions final : public editor::IEditorActions {
    public:
        explicit Actions(editor::AssistantEntryPoints& entryPoints)
            : entryPoints(entryPoints) {}
        void exit() override {}
        void showPanel(editor::Panel /*panel*/) override {}
        void openAssistant() override { entryPoints.openAssistant(); }
        void toggleAssistant() override { entryPoints.toggleAssistant(); }
        [[nodiscard]] bool canAskAboutSelection() const override {
            return entryPoints.canAskAboutSelection();
        }
        void askAboutSelection() override { entryPoints.askAboutSelection(); }
        [[nodiscard]] bool canDiagnoseLastRun() const override {
            return entryPoints.canDiagnoseLastRun();
        }
        void diagnoseLastRun() override { entryPoints.diagnoseLastRun(); }
        void openAiSettings() override { entryPoints.openAiSettings(); }
        editor::AssistantEntryPoints& entryPoints;
    } actions(entryPoints);
    const std::vector<editor::Command> toolbar = editor::buildToolbar(actions);
    const editor::Command& shortcutCommand = toolbar.back();

    ASSERT_TRUE(shortcutCommand.isEnabled());
    shortcutCommand.run();
    EXPECT_TRUE(entryPoints.isAssistantOpen());
    shortcutCommand.run();
    EXPECT_FALSE(entryPoints.isAssistantOpen());
}

TEST_F(EntryPointTest, AskAboutSelection_AttachesEntitySelectionAndWhyDraftWithoutSending) {
    selection.navigateTo(engine::EntityId{17});
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    ASSERT_TRUE(entryPoints.canAskAboutSelection());
    entryPoints.askAboutSelection();

    ASSERT_EQ(entryPoints.attachedContexts().size(), 1u);
    const auto& context =
        std::get<editor::SelectionContext>(entryPoints.attachedContexts().front());
    EXPECT_EQ(std::get<editor::EntitySelection>(context.selection).entity, engine::EntityId{17});
    EXPECT_EQ(entryPoints.messageDraft(), "Why\xE2\x80\xA6");
    EXPECT_TRUE(entryPoints.isAssistantOpen());
    EXPECT_TRUE(entryPoints.takeMessageBoxFocusRequest());
    EXPECT_TRUE(questions.questions.empty());
}

TEST_F(EntryPointTest, AskAboutSelection_AcceptsEventSelection) {
    const editor::EventSelection expected{3, 90, 5};
    selection.selectEvent(expected);
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    entryPoints.askAboutSelection();

    const auto& context =
        std::get<editor::SelectionContext>(entryPoints.attachedContexts().front());
    EXPECT_EQ(std::get<editor::EventSelection>(context.selection), expected);
}

TEST_F(EntryPointTest, AskAboutSelection_AcceptsFindingSelection) {
    const editor::FindingSelection expected{"finding-12"};
    selection.selectFinding(expected);
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    entryPoints.askAboutSelection();

    const auto& context =
        std::get<editor::SelectionContext>(entryPoints.attachedContexts().front());
    EXPECT_EQ(std::get<editor::FindingSelection>(context.selection), expected);
}

TEST_F(EntryPointTest, AskAboutSelection_WithNoEligibleSelectionIsDisabledAndDoesNothing) {
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    EXPECT_FALSE(entryPoints.canAskAboutSelection());
    entryPoints.askAboutSelection();

    EXPECT_FALSE(entryPoints.isAssistantOpen());
    EXPECT_TRUE(entryPoints.messageDraft().empty());
    EXPECT_TRUE(entryPoints.attachedContexts().empty());
    EXPECT_TRUE(questions.questions.empty());
}

TEST_F(EntryPointTest, DiagnoseLastRun_WithNoReportIsDisabledAndDoesNotSend) {
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    EXPECT_FALSE(entryPoints.canDiagnoseLastRun());
    entryPoints.diagnoseLastRun();

    EXPECT_FALSE(entryPoints.isAssistantOpen());
    EXPECT_TRUE(questions.questions.empty());
}

TEST_F(EntryPointTest, DiagnoseLastRun_SendsNewQuestionOnlyThroughConsentAwareInterface) {
    runs.run = editor::LatestRun{42, 1'000};
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    ASSERT_TRUE(entryPoints.canDiagnoseLastRun());
    entryPoints.diagnoseLastRun();

    ASSERT_EQ(questions.questions.size(), 1u);
    EXPECT_EQ(questions.questions.front().text, "Diagnose the last run.");
    ASSERT_EQ(questions.questions.front().contexts.size(), 1u);
    EXPECT_TRUE(
        std::holds_alternative<editor::LastRunContext>(questions.questions.front().contexts[0]));
    EXPECT_TRUE(entryPoints.isAssistantOpen());
}

TEST_F(EntryPointTest, DiagnoseLastRun_AttachesDefinedLastRunContextWithDefaultTickRange) {
    const editor::LatestRun expected{42, 1'000};
    runs.run = expected;
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    entryPoints.diagnoseLastRun();

    const auto& context =
        std::get<editor::LastRunContext>(questions.questions.front().contexts.front());
    EXPECT_EQ(context.run, expected);
    EXPECT_EQ(context.eventAndSampleRange, (editor::TickRange{401, 1'000}));
}

TEST_F(EntryPointTest, AiSettings_OpensProjectSettingsOnAiAndPrivacyTab) {
    editor::AssistantEntryPoints entryPoints = makeEntryPoints();

    entryPoints.openAiSettings();

    EXPECT_EQ(settings.openedTabs,
              (std::vector<editor::ProjectSettingsTab>{editor::ProjectSettingsTab::AiAndPrivacy}));
}
