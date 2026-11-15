// Run options of a game process (US-1.2; srs.fr.1.8, 7.32, 7.33).
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "engine/core/run_options.hpp"

namespace {

using Args = std::vector<std::string_view>;

// Parses `args` into fresh RunOptions; `error` receives the message when parsing fails.
bool parse(const Args& args, engine::RunOptions& options, std::string& error) {
    return engine::parseRunOptions(args, options, error);
}

} // namespace

TEST(RunOptions, NoOptions_NormalModeWithNoTickLimitSeedZeroAndNoRecording) {
    engine::RunOptions options;
    std::string error;

    ASSERT_TRUE(parse({}, options, error));

    EXPECT_FALSE(options.headless);
    EXPECT_EQ(options.maxTicks, 0u);
    EXPECT_EQ(options.seed, 0u);
    EXPECT_TRUE(options.inputRecordingPath.empty());
}

// srs.fr.7.33: --max-ticks defaults to 10 000 in a headless run.
TEST(RunOptions, HeadlessWithoutMaxTicks_MaxTicksIsTenThousand) {
    engine::RunOptions options;
    std::string error;

    ASSERT_TRUE(parse({"--headless"}, options, error));

    EXPECT_TRUE(options.headless);
    EXPECT_EQ(options.maxTicks, 10'000u);
}

TEST(RunOptions, AllOptionsGiven_AllAreRead) {
    engine::RunOptions options;
    std::string error;

    ASSERT_TRUE(parse(
        {"--headless", "--max-ticks", "600", "--seed", "42", "--input-recording", "runs/run-7.rec"},
        options, error));

    EXPECT_TRUE(options.headless);
    EXPECT_EQ(options.maxTicks, 600u);
    EXPECT_EQ(options.seed, 42u);
    EXPECT_EQ(options.inputRecordingPath, "runs/run-7.rec");
}

TEST(RunOptions, OptionsInAnyOrder_AreRead) {
    engine::RunOptions options;
    std::string error;

    ASSERT_TRUE(parse({"--seed", "7", "--max-ticks", "1", "--headless"}, options, error));

    EXPECT_TRUE(options.headless);
    EXPECT_EQ(options.maxTicks, 1u);
    EXPECT_EQ(options.seed, 7u);
}

// srs.fr.7.33: --max-ticks accepts 1 to 1 000 000; srs.fr.7.32: a seed is 0 to 2^32 - 1.
TEST(RunOptions, RangeLimits_AreAccepted) {
    engine::RunOptions options;
    std::string error;

    ASSERT_TRUE(parse({"--max-ticks", "1000000", "--seed", "4294967295"}, options, error));

    EXPECT_EQ(options.maxTicks, 1'000'000u);
    EXPECT_EQ(options.seed, 4'294'967'295u);
}

// srs.fr.7.33: an invalid value is refused with an error that names the option.
TEST(RunOptions, InvalidMaxTicks_FailsNamingTheOption) {
    for (const std::string_view value : {"0", "1000001", "-5", "ten", "12x", "", " 5", "+5"}) {
        SCOPED_TRACE(std::string(value));
        engine::RunOptions options;
        std::string error;

        EXPECT_FALSE(parse({"--headless", "--max-ticks", value}, options, error));
        EXPECT_NE(error.find("--max-ticks"), std::string::npos);
    }
}

TEST(RunOptions, InvalidSeed_FailsNamingTheOption) {
    for (const std::string_view value : {"4294967296", "-1", "seed", "1.5", ""}) {
        SCOPED_TRACE(std::string(value));
        engine::RunOptions options;
        std::string error;

        EXPECT_FALSE(parse({"--seed", value}, options, error));
        EXPECT_NE(error.find("--seed"), std::string::npos);
    }
}

TEST(RunOptions, OptionWithoutItsValue_FailsNamingTheOption) {
    for (const std::string_view option : {"--max-ticks", "--seed", "--input-recording"}) {
        SCOPED_TRACE(std::string(option));
        engine::RunOptions options;
        std::string error;

        EXPECT_FALSE(parse({"--headless", option}, options, error));
        EXPECT_NE(error.find(option), std::string::npos);
    }
}

TEST(RunOptions, EmptyInputRecordingName_Fails) {
    engine::RunOptions options;
    std::string error;

    EXPECT_FALSE(parse({"--input-recording", ""}, options, error));
    EXPECT_NE(error.find("--input-recording"), std::string::npos);
}

TEST(RunOptions, UnknownOption_FailsNamingIt) {
    engine::RunOptions options;
    std::string error;

    EXPECT_FALSE(parse({"--headless", "--speed", "2"}, options, error));
    EXPECT_NE(error.find("--speed"), std::string::npos);
}

// Applications with options of their own read the run options one argument at a time.
TEST(RunOptions, ParseRunOption_LeavesOtherOptionsToTheCaller) {
    const Args args{"--fps", "30", "--max-ticks", "5"};
    engine::RunOptions options;
    std::string error;
    std::size_t index = 0;

    EXPECT_EQ(engine::parseRunOption(args, index, options, error),
              engine::RunOptionParse::NotARunOption);
    EXPECT_EQ(index, 0u);

    index = 2;
    EXPECT_EQ(engine::parseRunOption(args, index, options, error), engine::RunOptionParse::Parsed);
    EXPECT_EQ(index, 3u);
    EXPECT_EQ(options.maxTicks, 5u);
}

TEST(RunOptions, CommandLineArguments_SkipsTheProgramName) {
    const char* const argv[] = {"game.exe", "--headless", "--seed", "3"};

    const std::vector<std::string_view> args = engine::commandLineArguments(4, argv);

    EXPECT_EQ(args, (std::vector<std::string_view>{"--headless", "--seed", "3"}));
}
