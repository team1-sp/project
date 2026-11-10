// Sandbox: the smallest application on the engine's game loop (PB-011). It is not the demo game,
// which the team has not chosen yet (Q-016).
//
// Usage: sandbox [--headless] [--max-ticks N]
//   --headless     run ticks without a window; stops after 10 000 ticks unless --max-ticks is given
//   --max-ticks N  stop after N ticks (1 to 1 000 000); without it, normal mode runs until the
//                  window is closed
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <system_error>

#include "engine/core/application.hpp"
#include "engine/core/leak_check.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/version.hpp"
#include "engine/physics_box2d/box2d_info.hpp"
#include "engine/rendering_raylib/raylib_info.hpp"
#include "engine/rendering_raylib/raylib_window.hpp"
#include "engine/rendering_raylib/status_text_renderer.hpp"

namespace {

// The option names, default and range are those srs.fr.7.33 gives a built game.
constexpr std::uint64_t defaultHeadlessMaxTicks = 10'000;
constexpr std::uint64_t largestMaxTicks = 1'000'000;

constexpr int exitCodeStartFailed = 1;
constexpr int exitCodeBadOption = 2;

// Stands in for the simulation until the ECS exists: a tick only counts itself.
class TickCountingSimulation final : public engine::ISimulation {
public:
    void runTick() override { ++ticksExecuted_; }

    [[nodiscard]] std::uint64_t ticksExecuted() const { return ticksExecuted_; }

private:
    std::uint64_t ticksExecuted_ = 0;
};

// Fills `config` from the command line. Returns false, after naming the option on the standard
// error output, when an option is unknown or its value is not valid.
bool parseOptions(int argc, char** argv, engine::ApplicationConfig& config) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view option(argv[i]);
        if (option == "--headless") {
            config.headless = true;
        } else if (option == "--max-ticks" && i + 1 < argc) {
            const std::string_view value(argv[++i]);
            const char* const valueEnd = value.data() + value.size();
            std::uint64_t maxTicks = 0;
            const std::from_chars_result parsed = std::from_chars(value.data(), valueEnd, maxTicks);
            if (parsed.ec != std::errc{} || parsed.ptr != valueEnd || maxTicks < 1 ||
                maxTicks > largestMaxTicks) {
                std::fprintf(stderr, "error: --max-ticks needs a whole number from 1 to 1000000\n");
                return false;
            }
            config.maxTicks = maxTicks;
        } else {
            std::fprintf(stderr, "error: unknown or incomplete option %s\n", argv[i]);
            return false;
        }
    }

    if (config.headless && config.maxTicks == 0) {
        config.maxTicks = defaultHeadlessMaxTicks;
    }
    return true;
}

void printVersion(const char* name, engine::Version version) {
    std::printf("%s %d.%d.%d\n", name, version.major, version.minor, version.patch);
}

} // namespace

int main(int argc, char** argv) {
    engine::enableLeakReportAtExit();

    engine::ApplicationConfig config;
    config.window.title = "Aegis sandbox";
    if (!parseOptions(argc, argv, config)) {
        return exitCodeBadOption;
    }

    printVersion("engine", engine::engineVersion());
    printVersion("box2d", engine::physics_box2d::linkedBox2dVersion());
    printVersion("raylib", engine::rendering_raylib::linkedRaylibVersion());

    TickCountingSimulation simulation;
    engine::rendering_raylib::StatusTextRenderer renderer(
        [&simulation] { return "ticks executed: " + std::to_string(simulation.ticksExecuted()); });

    engine::Application app(simulation, renderer, engine::rendering_raylib::RaylibWindow::open);
    if (!app.init(config)) {
        return exitCodeStartFailed;
    }
    const int exitCode = app.run();
    app.shutdown();

    std::printf("ticks executed: %llu\n",
                static_cast<unsigned long long>(simulation.ticksExecuted()));
    return exitCode;
}
