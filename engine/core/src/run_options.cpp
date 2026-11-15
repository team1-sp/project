#include "engine/core/run_options.hpp"

#include <charconv>
#include <limits>
#include <system_error>

namespace engine {

namespace {

// Reads a whole number from `lowest` to `largest`, written in decimal digits only. Returns false
// for anything else, including a sign, spaces or a number that does not fit.
bool parseWholeNumber(std::string_view text, std::uint64_t lowest, std::uint64_t largest,
                      std::uint64_t& value) {
    const char* const end = text.data() + text.size();
    const std::from_chars_result parsed = std::from_chars(text.data(), end, value);
    return !text.empty() && parsed.ec == std::errc{} && parsed.ptr == end && value >= lowest &&
           value <= largest;
}

// The value that follows the option at args[index], or nullptr when it is the last argument.
const std::string_view* valueAfter(std::span<const std::string_view> args, std::size_t index) {
    return index + 1 < args.size() ? &args[index + 1] : nullptr;
}

} // namespace

RunOptionParse parseRunOption(std::span<const std::string_view> args, std::size_t& index,
                              RunOptions& options, std::string& error) {
    const std::string_view option = args[index];

    if (option == "--headless") {
        options.headless = true;
        return RunOptionParse::Parsed;
    }

    if (option == "--max-ticks") {
        const std::string_view* text = valueAfter(args, index);
        std::uint64_t value = 0;
        if (text == nullptr || !parseWholeNumber(*text, 1, largestMaxTicks, value)) {
            error = "--max-ticks needs a whole number from 1 to 1000000";
            return RunOptionParse::Invalid;
        }
        options.maxTicks = value;
        ++index;
        return RunOptionParse::Parsed;
    }

    if (option == "--seed") {
        // srs.fr.7.32: a seed is a whole number from 0 to 2^32 - 1.
        const std::string_view* text = valueAfter(args, index);
        std::uint64_t value = 0;
        if (text == nullptr ||
            !parseWholeNumber(*text, 0, std::numeric_limits<std::uint32_t>::max(), value)) {
            error = "--seed needs a whole number from 0 to 4294967295";
            return RunOptionParse::Invalid;
        }
        options.seed = static_cast<std::uint32_t>(value);
        ++index;
        return RunOptionParse::Parsed;
    }

    if (option == "--input-recording") {
        // Only the syntax is checked here; whoever plays the recording reads and validates the
        // file (srs.fr.5.12).
        const std::string_view* text = valueAfter(args, index);
        if (text == nullptr || text->empty()) {
            error = "--input-recording needs a file name";
            return RunOptionParse::Invalid;
        }
        options.inputRecordingPath = std::string(*text);
        ++index;
        return RunOptionParse::Parsed;
    }

    return RunOptionParse::NotARunOption;
}

void applyRunOptionDefaults(RunOptions& options) {
    if (options.headless && options.maxTicks == 0) {
        options.maxTicks = defaultHeadlessMaxTicks;
    }
}

bool parseRunOptions(std::span<const std::string_view> args, RunOptions& options,
                     std::string& error) {
    for (std::size_t i = 0; i < args.size(); ++i) {
        switch (parseRunOption(args, i, options, error)) {
        case RunOptionParse::Parsed:
            break;
        case RunOptionParse::Invalid:
            return false;
        case RunOptionParse::NotARunOption:
            error = "unknown option " + std::string(args[i]);
            return false;
        }
    }
    applyRunOptionDefaults(options);
    return true;
}

std::vector<std::string_view> commandLineArguments(int argc, const char* const* argv) {
    std::vector<std::string_view> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    return args;
}

} // namespace engine
