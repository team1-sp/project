#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "engine/core/tick.hpp"

namespace engine {

// The run settings a game process takes from its command line (srs.fr.1.8, 7.33): whether it runs
// headless and, for that run, the maximum number of ticks, the seed and the input recording.
// The built game (US-28.x) and the sandbox read them with parseRunOptions(); the Editor takes the
// same settings from its mode switch and Headless Run dialog instead.
struct RunOptions {
    // --headless: no window, no render frames, ticks back to back (srs.fr.1.7, 7.1, 7.4).
    bool headless = false;
    // --max-ticks <N>: the run stops when the tick counter reaches N (srs.fr.7.7). 0 means the
    // option was not given; parseRunOptions() then sets the headless default.
    Tick maxTicks = 0;
    // --seed <S>: the seed of the gameplay random number generator (srs.fr.7.5, 7.32).
    std::uint32_t seed = 0;
    // --input-recording <file>: the input recording to play (srs.fr.5.12). Empty means the
    // input source None (srs.fr.7.3).
    std::string inputRecordingPath;
};

// srs.fr.7.33: --max-ticks defaults to 10 000 and accepts 1 to 1 000 000.
inline constexpr Tick defaultHeadlessMaxTicks = 10'000;
inline constexpr Tick largestMaxTicks = 1'000'000;

// What parseRunOption() made of one argument.
enum class RunOptionParse {
    // The argument is not a run option; the caller may know it or report it as unknown.
    NotARunOption,
    // The option and its value were read into the RunOptions.
    Parsed,
    // The argument is a run option, but its value is missing or not valid.
    Invalid,
};

// Reads the run option that starts at args[index]. On Parsed, `index` is moved to the option's
// last argument, so the caller's loop continues after it. On Invalid, `error` names the option
// and what it accepts. Applications that add options of their own call this first for each
// argument; parseRunOptions() is the version for a command line that has only run options.
[[nodiscard]] RunOptionParse parseRunOption(std::span<const std::string_view> args,
                                            std::size_t& index, RunOptions& options,
                                            std::string& error);

// Gives a headless run without --max-ticks the default of srs.fr.7.33.
void applyRunOptionDefaults(RunOptions& options);

// Reads a whole command line, without the program name, that has only run options, then applies
// the defaults. Returns false, with `error` naming the option, when an option is unknown or its
// value is missing or not valid; the caller then writes the error to the standard error output,
// runs no tick and exits with a non-zero code (srs.fr.7.33).
[[nodiscard]] bool parseRunOptions(std::span<const std::string_view> args, RunOptions& options,
                                   std::string& error);

// The arguments of main() after the program name.
[[nodiscard]] std::vector<std::string_view> commandLineArguments(int argc, const char* const* argv);

} // namespace engine
