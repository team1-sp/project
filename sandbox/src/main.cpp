// Sandbox: the smallest application on the engine's game loop (PB-011, PB-012). It is not the
// demo game, which the team has not chosen yet (Q-016). It is a developer and CI host (DEC-048):
// it reads the run options a built game reads (srs.fr.1.8, 7.33) plus a few of its own.
//
// Usage: sandbox [--headless] [--max-ticks N] [--seed S] [--input-recording F] [--fps N]
//                [--fullscreen] [--fail-at-tick N]
//   --headless          run ticks without a window; stops after 10 000 ticks unless --max-ticks
//                       is given
//   --max-ticks N       stop after N ticks (1 to 1 000 000); without it, normal mode runs until
//                       the window is closed
//   --seed S            the run's seed (0 to 4 294 967 295, default 0); nothing draws random
//                       numbers yet
//   --input-recording F accepted by the parser, but refused: playback is not built yet
//   --fps N             cap the render frame rate at N frames per second (0 to 1000, 0 = no cap;
//                       default 60). The simulation runs at 60 ticks per second whatever N is.
//   --fullscreen        open the window fullscreen
//   --fail-at-tick N    tick N throws engine::RunError, to try the run error path (srs.fr.1.11)
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "engine/core/application.hpp"
#include "engine/core/frame_clock.hpp"
#include "engine/core/leak_check.hpp"
#include "engine/core/run_error.hpp"
#include "engine/core/run_options.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/version.hpp"
#include "engine/physics_box2d/box2d_info.hpp"
#include "engine/rendering_raylib/raylib_info.hpp"
#include "engine/rendering_raylib/raylib_window.hpp"
#include "engine/rendering_raylib/status_text_renderer.hpp"

namespace {

constexpr std::uint64_t largestFramesPerSecond = 1'000;

constexpr int exitCodeStartFailed = 1;
constexpr int exitCodeBadOption = 2;

// The sandbox's settings: the run options of a built game and the sandbox's own.
struct SandboxOptions {
    engine::RunOptions run;
    int framesPerSecond = 60;
    bool fullscreen = false;
    std::optional<engine::Tick> failAtTick;
};

// Stands in for the simulation until the ECS exists: a tick only records that it ran, or throws
// the run error --fail-at-tick asks for, as a system that breaks an invariant would.
class TickCountingSimulation final : public engine::ISimulation {
public:
    explicit TickCountingSimulation(std::optional<engine::Tick> failAtTick)
        : failAtTick_(failAtTick) {}

    void runTick(engine::Tick tick) override {
        if (failAtTick_ == tick) {
            throw engine::RunError("sandbox: --fail-at-tick asked this tick to fail");
        }
        ticksExecuted_ = tick + 1;
    }

    [[nodiscard]] engine::Tick ticksExecuted() const { return ticksExecuted_; }

private:
    std::optional<engine::Tick> failAtTick_;
    engine::Tick ticksExecuted_ = 0;
};

// Reads a whole number from `lowest` to `largest`. Returns false for anything else.
bool parseWholeNumber(std::string_view text, std::uint64_t lowest, std::uint64_t largest,
                      std::uint64_t& value) {
    const char* const end = text.data() + text.size();
    const std::from_chars_result parsed = std::from_chars(text.data(), end, value);
    return parsed.ec == std::errc{} && parsed.ptr == end && value >= lowest && value <= largest;
}

// Fills `options` from the command line. Returns false, after naming the option on the standard
// error output, when an option is unknown or its value is not valid (srs.fr.7.33).
bool parseOptions(const std::vector<std::string_view>& args, SandboxOptions& options) {
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string error;
        switch (engine::parseRunOption(args, i, options.run, error)) {
        case engine::RunOptionParse::Parsed:
            continue;
        case engine::RunOptionParse::Invalid:
            std::fprintf(stderr, "error: %s\n", error.c_str());
            return false;
        case engine::RunOptionParse::NotARunOption:
            break;
        }

        const std::string_view option = args[i];
        std::uint64_t value = 0;
        if (option == "--fps") {
            if (i + 1 >= args.size() ||
                !parseWholeNumber(args[++i], 0, largestFramesPerSecond, value)) {
                std::fprintf(stderr, "error: --fps needs a whole number from 0 to 1000\n");
                return false;
            }
            options.framesPerSecond = static_cast<int>(value);
        } else if (option == "--fullscreen") {
            options.fullscreen = true;
        } else if (option == "--fail-at-tick") {
            if (i + 1 >= args.size() ||
                !parseWholeNumber(args[++i], 0, engine::largestMaxTicks, value)) {
                std::fprintf(stderr,
                             "error: --fail-at-tick needs a whole number from 0 to 1000000\n");
                return false;
            }
            options.failAtTick = value;
        } else {
            std::fprintf(stderr, "error: unknown option %.*s\n", static_cast<int>(option.size()),
                         option.data());
            return false;
        }
    }

    // The engine cannot play input recordings yet (srs.fr.5.12 is not built), so the option is
    // refused before any tick rather than silently running with the input source None.
    if (!options.run.inputRecordingPath.empty()) {
        std::fprintf(stderr, "error: --input-recording: input recording playback is not built "
                             "yet\n");
        return false;
    }

    engine::applyRunOptionDefaults(options.run);
    return true;
}

double toSeconds(std::chrono::nanoseconds duration) {
    return std::chrono::duration<double>(duration).count();
}

// What the window shows: the tick counter next to real time, so it is visible that simulated
// time keeps pace with real time at any frame rate.
std::string statusText(engine::Tick ticksExecuted, const engine::FrameClock& frameClock) {
    char text[160];
    std::snprintf(
        text, sizeof(text),
        "ticks executed: %llu\nsimulated time: %.2f s\nreal time: %.2f s\nframe time: %.2f ms",
        static_cast<unsigned long long>(ticksExecuted), engine::simulatedSeconds(ticksExecuted),
        toSeconds(frameClock.totalTime()), toSeconds(frameClock.frameTime()) * 1000.0);
    return text;
}

void printVersion(const char* name, engine::Version version) {
    std::printf("%s %d.%d.%d\n", name, version.major, version.minor, version.patch);
}

} // namespace

int main(int argc, char** argv) {
    engine::enableLeakReportAtExit();

    SandboxOptions options;
    if (!parseOptions(engine::commandLineArguments(argc, argv), options)) {
        return exitCodeBadOption;
    }

    engine::ApplicationConfig config;
    config.headless = options.run.headless;
    config.maxTicks = options.run.maxTicks;
    config.window.title = "Aegis sandbox";
    config.window.targetFramesPerSecond = options.framesPerSecond;
    config.window.fullscreen = options.fullscreen;

    printVersion("engine", engine::engineVersion());
    printVersion("box2d", engine::physics_box2d::linkedBox2dVersion());
    printVersion("raylib", engine::rendering_raylib::linkedRaylibVersion());

    std::printf("mode: %s, seed: %lu\n", config.headless ? "headless" : "normal",
                static_cast<unsigned long>(options.run.seed));

    TickCountingSimulation simulation(options.failAtTick);
    // The sandbox's own measurement of real time, one reading per render frame, for display.
    engine::FrameClock frameClock;
    std::uint64_t renderFrames = 0;
    engine::rendering_raylib::StatusTextRenderer renderer([&] {
        frameClock.measureFrame();
        ++renderFrames;
        return statusText(simulation.ticksExecuted(), frameClock);
    });

    engine::Application app(simulation, renderer, engine::rendering_raylib::RaylibWindow::open);
    if (!app.init(config)) {
        return exitCodeStartFailed;
    }
    frameClock.start();
    const int exitCode = app.run();
    app.shutdown();

    std::printf("outcome: %s\n", engine::runOutcomeName(app.outcome()));
    std::printf("ticks executed: %llu (%.2f s simulated)\n",
                static_cast<unsigned long long>(app.ticksExecuted()), app.simulatedSeconds());
    if (renderFrames > 0) {
        const double realSeconds = toSeconds(frameClock.totalTime());
        std::printf("real time: %.2f s in %llu render frames (%.2f ms per frame)\n", realSeconds,
                    static_cast<unsigned long long>(renderFrames),
                    realSeconds * 1000.0 / static_cast<double>(renderFrames));
    }
    return exitCode;
}
