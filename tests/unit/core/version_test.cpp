#include <gtest/gtest.h>

#include "engine/core/version.hpp"
#include "engine/core/version_config.hpp"

TEST(EngineVersion, Called_MatchesCMakeProjectVersion) {
    const engine::Version version = engine::engineVersion();
    EXPECT_EQ(version.major, engine::build::versionMajor);
    EXPECT_EQ(version.minor, engine::build::versionMinor);
    EXPECT_EQ(version.patch, engine::build::versionPatch);
}
