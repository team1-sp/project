// Editor: the application that hosts the engine and owns its main window (DEC-048). Games,
// including the demo game, are built and run inside it; they never host the engine themselves.
// Panels, projects and the run controls follow in later backlog items (PB-110 to PB-113).
//
// Usage: editor [--max-ticks N]
//   --max-ticks N  close the window after N ticks (1 to 1 000 000); without it, the editor runs
//                  until the window is closed
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <system_error>

#include "engine/core/application.hpp"
#include "engine/core/leak_check.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/version.hpp"
#include "engine/rendering_raylib/raylib_window.hpp"
#include "engine/rendering_raylib/status_text_renderer.hpp"

namespace {

constexpr engine::Tick largestMaxTicks = 1'000'000;

constexpr int exitCodeStartFailed = 1;
constexpr int exitCodeBadOption = 2;

// No project is open yet, so a tick has nothing to simulate.
class NoProjectSimulation final : public engine::ISimulation {
public:
    void runTick(engine::Tick /*tick*/) override {}
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

} // namespace

int main(int argc, char** argv) {
    engine::enableLeakReportAtExit();

    engine::ApplicationConfig config;
    config.window.widthPixels = 1280;
    config.window.heightPixels = 720;
    config.window.title = "Aegis Editor";
    if (!parseOptions(argc, argv, config)) {
        return exitCodeBadOption;
    }

    const engine::Version version = engine::engineVersion();
    std::printf("editor %d.%d.%d\n", version.major, version.minor, version.patch);

    NoProjectSimulation simulation;
    engine::rendering_raylib::StatusTextRenderer renderer(
        [] { return std::string("No project open"); });

    engine::Application app(simulation, renderer, engine::rendering_raylib::RaylibWindow::open);
    if (!app.init(config)) {
        return exitCodeStartFailed;
    }
    const int exitCode = app.run();
    app.shutdown();
    return exitCode;
}
