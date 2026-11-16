// Tick phases (US-1.3, srs.fr.1.12).
#include <cstddef>
#include <string>

#include <gtest/gtest.h>

#include "engine/core/tick_phase.hpp"

using engine::TickPhase;

TEST(TickPhases, Order_IsTheOrderOfTheSrs) {
    ASSERT_EQ(engine::tickPhases.size(), 7u);
    EXPECT_EQ(engine::tickPhases[0], TickPhase::InputApplication);
    EXPECT_EQ(engine::tickPhases[1], TickPhase::GameplayInput);
    EXPECT_EQ(engine::tickPhases[2], TickPhase::PhysicsSyncIn);
    EXPECT_EQ(engine::tickPhases[3], TickPhase::PhysicsStep);
    EXPECT_EQ(engine::tickPhases[4], TickPhase::PhysicsSyncOut);
    EXPECT_EQ(engine::tickPhases[5], TickPhase::GameplayRules);
    EXPECT_EQ(engine::tickPhases[6], TickPhase::TelemetryRecording);
}

TEST(TickPhases, EnumValues_FollowTheExecutionOrder) {
    for (std::size_t i = 0; i < engine::tickPhases.size(); ++i) {
        EXPECT_EQ(static_cast<std::size_t>(engine::tickPhases[i]), i);
    }
}

TEST(TickPhaseName, EveryPhase_HasTheNameTheSchemaUses) {
    EXPECT_STREQ(engine::tickPhaseName(TickPhase::InputApplication), "InputApplication");
    EXPECT_STREQ(engine::tickPhaseName(TickPhase::GameplayInput), "GameplayInput");
    EXPECT_STREQ(engine::tickPhaseName(TickPhase::PhysicsSyncIn), "PhysicsSyncIn");
    EXPECT_STREQ(engine::tickPhaseName(TickPhase::PhysicsStep), "PhysicsStep");
    EXPECT_STREQ(engine::tickPhaseName(TickPhase::PhysicsSyncOut), "PhysicsSyncOut");
    EXPECT_STREQ(engine::tickPhaseName(TickPhase::GameplayRules), "GameplayRules");
    EXPECT_STREQ(engine::tickPhaseName(TickPhase::TelemetryRecording), "TelemetryRecording");
}
