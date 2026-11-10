// Sandbox: the smallest application on the engine's game loop (PB-011, PB-012). It is not the
// demo game, which the team has not chosen yet (Q-016).
//
// Usage: sandbox [--headless] [--max-ticks N] [--fps N]
//   --headless     run ticks without a window; stops after 10 000 ticks unless --max-ticks is given
//   --max-ticks N  stop after N ticks (1 to 1 000 000); without it, normal mode runs until the
//                  window is closed
//   --fps N        cap the render frame rate at N frames per second (0 to 1000, 0 = no cap;
//                  default 60). The simulation runs at 60 ticks per second whatever N is.
#include <charconv>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <system_error>

#include "engine/core/application.hpp"
#include "engine/core/frame_clock.hpp"
#include "engine/core/leak_check.hpp"
#include "engine/core/simulation.hpp"
#include "engine/core/tick.hpp"
#include "engine/core/version.hpp"
#include "engine/physics_box2d/box2d_info.hpp"
#include "engine/rendering_raylib/raylib_info.hpp"
#include "engine/rendering_raylib/raylib_window.hpp"
#include "engine/rendering_raylib/status_text_renderer.hpp"

namespace {

// The option names, default and range of --max-ticks are those srs.fr.7.33 gives a built game.
constexpr engine::Tick defaultHeadlessMaxTicks = 10'000;
constexpr engine::Tick largestMaxTicks = 1'000'000;
constexpr std::uint64_t largestFramesPerSecond = 1'000;

constexpr int exitCodeStartFailed = 1;
constexpr int exitCodeBadOption = 2;

// Stands in for the simulation until the ECS exists: a tick only records that it ran.
class TickCountingSimulation final : public engine::ISimulation {
public:
    void runTick(engine::Tick tick) override { ticksExecuted_ = tick + 1; }

    [[nodiscard]] engine::Tick ticksExecuted() const { return ticksExecuted_; }

private:
    engine::Tick ticksExecuted_ = 0;
};

// Reads a whole number from `lowest` to `largest`. Returns false for anything else.
bool parseWholeNumber(std::string_view text, std::uint64_t lowest, std::uint64_t largest,
                      std::uint64_t& value) {
    const char* const end = text.data() + text.size();
    const std::from_chars_result parsed = std::from_chars(text.data(), end, value);
    return parsed.ec == std::errc{} && parsed.ptr == end && value >= lowest && value <= largest;
}

// Fills `config` from the command line. Returns false, after naming the option on the standard
// error output, when an option is unknown or its value is not valid.
bool parseOptions(int argc, char** argv, engine::ApplicationConfig& config) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view option(argv[i]);
        std::uint64_t value = 0;
        if (option == "--headless") {
            config.headless = true;
        } else if (option == "--max-ticks" && i + 1 < argc) {
            if (!parseWholeNumber(argv[++i], 1, largestMaxTicks, value)) {
                std::fprintf(stderr, "error: --max-ticks needs a whole number from 1 to 1000000\n");
                return false;
            }
            config.maxTicks = value;
        } else if (option == "--fps" && i + 1 < argc) {
            if (!parseWholeNumber(argv[++i], 0, largestFramesPerSecond, value)) {
                std::fprintf(stderr, "error: --fps needs a whole number from 0 to 1000\n");
                return false;
            }
            config.window.targetFramesPerSecond = static_cast<int>(value);
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

    engine::ApplicationConfig config;
    config.window.title = "Aegis sandbox";
    if (!parseOptions(argc, argv, config)) {
        return exitCodeBadOption;
    }

    printVersion("engine", engine::engineVersion());
    printVersion("box2d", engine::physics_box2d::linkedBox2dVersion());
    printVersion("raylib", engine::rendering_raylib::linkedRaylibVersion());

    TickCountingSimulation simulation;
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
