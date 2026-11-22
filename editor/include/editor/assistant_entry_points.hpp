#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "editor/selection.hpp"
#include "engine/core/entity_id.hpp"
#include "engine/core/tick.hpp"

namespace editor {

struct LatestRun {
    std::uint64_t runNumber = 0;
    engine::Tick lastTick = 0;

    bool operator==(const LatestRun&) const = default;
};

class ILatestRunSource {
public:
    virtual ~ILatestRunSource() = default;
    [[nodiscard]] virtual std::optional<LatestRun> latestRun() const = 0;
};

struct TickRange {
    engine::Tick first = 0;
    engine::Tick last = 0;

    bool operator==(const TickRange&) const = default;
};

struct SelectionContext {
    EditorSelection selection;

    bool operator==(const SelectionContext&) const = default;
};

struct LastRunContext {
    // The consent-aware receiver resolves this typed run reference to the test report, findings,
    // recording header/schema/initial state (including colliders and debug injections), events and
    // telemetry samples required by srs.fr.33.35. The range limits only events and samples.
    LatestRun run;
    TickRange eventAndSampleRange;

    bool operator==(const LastRunContext&) const = default;
};

using AssistantContext = std::variant<SelectionContext, LastRunContext>;

struct AssistantQuestion {
    std::string text;
    std::vector<AssistantContext> contexts;

    bool operator==(const AssistantQuestion&) const = default;
};

// This is the only sending seam available to editor entry points. Implementations own the
// source-code/telemetry consent sequence of srs.fr.33.41 and may call the assistant only after it.
class IConsentAwareQuestionSender {
public:
    virtual ~IConsentAwareQuestionSender() = default;
    virtual void sendNewQuestion(AssistantQuestion question) = 0;
};

enum class ProjectSettingsTab { AiAndPrivacy };

class IProjectSettingsOpener {
public:
    virtual ~IProjectSettingsOpener() = default;
    virtual void openProjectSettings(ProjectSettingsTab tab) = 0;
};

class ProjectSettingsState final : public IProjectSettingsOpener {
public:
    void openProjectSettings(ProjectSettingsTab tab) override;
    void close();
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] ProjectSettingsTab selectedTab() const;

private:
    bool open_ = false;
    ProjectSettingsTab selectedTab_ = ProjectSettingsTab::AiAndPrivacy;
};

// Window-independent state and orchestration for US-33.1. Dear ImGui only reflects this state.
class AssistantEntryPoints {
public:
    AssistantEntryPoints(const ISelectionSource& selection, const ILatestRunSource& latestRun,
                         IConsentAwareQuestionSender& questions,
                         IProjectSettingsOpener& projectSettings);

    void openAssistant();
    void toggleAssistant();
    void closeAssistant();
    [[nodiscard]] bool isAssistantOpen() const;
    [[nodiscard]] bool takeMessageBoxFocusRequest();

    [[nodiscard]] bool canAskAboutSelection() const;
    void askAboutSelection();
    [[nodiscard]] bool canDiagnoseLastRun() const;
    void diagnoseLastRun();
    void openAiSettings();

    [[nodiscard]] const std::string& messageDraft() const;
    void setMessageDraft(std::string draft);
    [[nodiscard]] const std::vector<AssistantContext>& attachedContexts() const;

private:
    [[nodiscard]] static LastRunContext makeLastRunContext(LatestRun run);

    const ISelectionSource& selection_;
    const ILatestRunSource& latestRun_;
    IConsentAwareQuestionSender& questions_;
    IProjectSettingsOpener& projectSettings_;
    bool assistantOpen_ = false;
    bool focusMessageBox_ = false;
    std::string draft_;
    std::vector<AssistantContext> contexts_;
};

} // namespace editor
