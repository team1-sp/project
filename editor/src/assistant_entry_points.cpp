#include "editor/assistant_entry_points.hpp"

#include <utility>

namespace editor {

namespace {

constexpr engine::Tick defaultLastRunTickCount = 600;

bool eligibleSelection(const EditorSelection& selection) {
    return !std::holds_alternative<std::monostate>(selection);
}

} // namespace

void ProjectSettingsState::openProjectSettings(ProjectSettingsTab tab) {
    selectedTab_ = tab;
    open_ = true;
}

void ProjectSettingsState::close() {
    open_ = false;
}

bool ProjectSettingsState::isOpen() const {
    return open_;
}

ProjectSettingsTab ProjectSettingsState::selectedTab() const {
    return selectedTab_;
}

AssistantEntryPoints::AssistantEntryPoints(const ISelectionSource& selection,
                                           const ILatestRunSource& latestRun,
                                           IConsentAwareQuestionSender& questions,
                                           IProjectSettingsOpener& projectSettings)
    : selection_(selection)
    , latestRun_(latestRun)
    , questions_(questions)
    , projectSettings_(projectSettings) {
}

void AssistantEntryPoints::openAssistant() {
    assistantOpen_ = true;
    focusMessageBox_ = true;
}

void AssistantEntryPoints::toggleAssistant() {
    if (assistantOpen_) {
        closeAssistant();
    } else {
        openAssistant();
    }
}

void AssistantEntryPoints::closeAssistant() {
    assistantOpen_ = false;
    focusMessageBox_ = false;
}

bool AssistantEntryPoints::isAssistantOpen() const {
    return assistantOpen_;
}

bool AssistantEntryPoints::takeMessageBoxFocusRequest() {
    const bool requested = focusMessageBox_;
    focusMessageBox_ = false;
    return requested;
}

bool AssistantEntryPoints::canAskAboutSelection() const {
    return eligibleSelection(selection_.currentSelection());
}

void AssistantEntryPoints::askAboutSelection() {
    const EditorSelection selection = selection_.currentSelection();
    if (!eligibleSelection(selection)) {
        return;
    }
    openAssistant();
    contexts_ = {SelectionContext{selection}};
    draft_ = "Why\xE2\x80\xA6";
}

bool AssistantEntryPoints::canDiagnoseLastRun() const {
    return latestRun_.latestRun().has_value();
}

void AssistantEntryPoints::diagnoseLastRun() {
    std::optional<LatestRun> run = latestRun_.latestRun();
    if (!run) {
        return;
    }

    openAssistant();
    draft_ = "Diagnose the last run.";
    contexts_ = {makeLastRunContext(std::move(*run))};
    questions_.sendNewQuestion(AssistantQuestion{draft_, contexts_});
}

void AssistantEntryPoints::openAiSettings() {
    projectSettings_.openProjectSettings(ProjectSettingsTab::AiAndPrivacy);
}

const std::string& AssistantEntryPoints::messageDraft() const {
    return draft_;
}

void AssistantEntryPoints::setMessageDraft(std::string draft) {
    draft_ = std::move(draft);
}

const std::vector<AssistantContext>& AssistantEntryPoints::attachedContexts() const {
    return contexts_;
}

LastRunContext AssistantEntryPoints::makeLastRunContext(LatestRun run) {
    const engine::Tick first = run.lastTick >= defaultLastRunTickCount - 1
                                   ? run.lastTick - (defaultLastRunTickCount - 1)
                                   : 0;
    return LastRunContext{run, TickRange{first, run.lastTick}};
}

} // namespace editor
