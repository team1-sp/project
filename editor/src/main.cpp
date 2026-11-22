// Editor: the application that hosts the engine and owns its main window (DEC-048). Games,
// including the demo game, are built and run inside it; they never host the engine themselves.
// The main window has the menu bar, toolbar, panels and status bar of srs.fr.14.1 (US-14.1),
// drawn with Dear ImGui (DEC-063); the status bar shows the Editor's state (US-14.2) and the
// panels' contents come with their own stories.
//
// Usage: editor [--max-ticks N]
//   --max-ticks N  close the window after N ticks (1 to 1 000 000); without it, the editor runs
//                  until the window is closed
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "editor/assistant_entry_points.hpp"
#include "editor/console_model.hpp"
#include "editor/editor_session.hpp"
#include "editor/entity_navigation.hpp"
#include "editor/external_code_editor.hpp"
#include "editor/status_bar.hpp"
#include "engine/core/application.hpp"
#include "engine/core/leak_check.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/tick_phase.hpp"
#include "engine/core/version.hpp"
#include "engine/rendering_raylib/imgui_renderer.hpp"
#include "engine/rendering_raylib/raylib_window.hpp"
#include "main_window_view.hpp"
#include "theme.hpp"

namespace {

constexpr engine::Tick largestMaxTicks = 1'000'000;

constexpr int exitCodeStartFailed = 1;
constexpr int exitCodeBadOption = 2;

// No project is open yet, so a tick has nothing to simulate.
class NoProjectSimulation final : public engine::ISimulation {
public:
    void runPhase(engine::TickPhase /*phase*/, const engine::TickContext& /*context*/) override {}
};

class NoLatestRunSource final : public editor::ILatestRunSource {
public:
    [[nodiscard]] std::optional<editor::LatestRun> latestRun() const override {
        return std::nullopt;
    }
};

// The run-report and consent-dialog stories provide the production consumer. Until both exist,
// Diagnose Last Run is disabled because NoLatestRunSource has no report; this queue ensures a
// future source still cannot reach an AI service except through the consent-aware seam.
class PendingConsentQuestionQueue final : public editor::IConsentAwareQuestionSender {
public:
    void sendNewQuestion(editor::AssistantQuestion question) override {
        pending_ = std::move(question);
    }

private:
    std::optional<editor::AssistantQuestion> pending_;
};

// Fills `config` from the command line. Returns false, after naming the option on the standard
// error output, when an option is unknown or its value is not valid.
bool parseOptions(int argc, char** argv, engine::ApplicationConfig& config) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view option(argv[i]);
        if (option == "--max-ticks" && i + 1 < argc) {
            const std::string_view text(argv[++i]);
            std::uint64_t value = 0;
            const char* const end = text.data() + text.size();
            const std::from_chars_result parsed = std::from_chars(text.data(), end, value);
            if (parsed.ec != std::errc{} || parsed.ptr != end || value < 1 ||
                value > largestMaxTicks) {
                std::fprintf(stderr, "error: --max-ticks needs a whole number from 1 to 1000000\n");
                return false;
            }
            config.maxTicks = value;
        } else {
            std::fprintf(stderr, "error: unknown or incomplete option %s\n", argv[i]);
            return false;
        }
    }
    return true;
}

std::string configuredCodeEditor() {
#ifdef _WIN32
    char* value = nullptr;
    std::size_t valueSize = 0;
    const errno_t result = _dupenv_s(&value, &valueSize, "AEGIS_CODE_EDITOR");
    std::string executable = result == 0 && value != nullptr ? value : "code";
    std::free(value);
    return executable;
#else
    const char* value = std::getenv("AEGIS_CODE_EDITOR");
    return value != nullptr ? value : "code";
#endif
}

} // namespace

int main(int argc, char** argv) {
    engine::enableLeakReportAtExit();

    editor::EditorSession session;
    // What the status bar shows (srs.fr.14.7 to 14.11); the Console, Play mode, scene, AI
    // service, telemetry and debug flag stories fill it in.
    editor::EditorStatus status;
    engine::Application* runningApp = nullptr;
    editor::ConsoleModel console(status, [&runningApp] {
        if (runningApp != nullptr) {
            (void)runningApp->requestPause();
        }
    });
    editor::ProcessExternalCodeEditor externalEditor(configuredCodeEditor());
    editor::EntityNavigation selection;
    NoLatestRunSource latestRun;
    PendingConsentQuestionQueue questions;
    editor::ProjectSettingsState projectSettings;
    editor::AssistantEntryPoints assistant(selection, latestRun, questions, projectSettings);

    engine::ApplicationConfig config;
    config.window.widthPixels = 1280;
    config.window.heightPixels = 720;
    config.window.title = session.mainWindowTitle();
    // Escape stops a headless run in the Editor (D-06); it must not close the main window.
    config.window.escapeClosesWindow = false;
    // Closing the main window applies the unsaved-changes check first (srs.fr.14.3).
    config.confirmClose = [&session] { return session.confirmClose(); };
    config.logSink = &console;
    if (!parseOptions(argc, argv, config)) {
        return exitCodeBadOption;
    }

    const engine::Version version = engine::engineVersion();
    std::printf("editor %d.%d.%d\n", version.major, version.minor, version.patch);

    NoProjectSimulation simulation;
    editor::MainWindowView mainWindow(
        session, status, console, assistant, projectSettings, externalEditor, selection,
        [&runningApp] { runningApp->requestClose(); },
        [&runningApp](const std::string& title) { runningApp->setWindowTitle(title); });
    engine::rendering_raylib::ImGuiRenderer renderer([&mainWindow] { mainWindow.draw(); },
                                                     editor::applyTheme);

    engine::Application app(simulation, renderer, engine::rendering_raylib::RaylibWindow::open);
    runningApp = &app;
    if (!app.init(config)) {
        return exitCodeStartFailed;
    }
    const int exitCode = app.run();
    app.shutdown();
    return exitCode;
}
