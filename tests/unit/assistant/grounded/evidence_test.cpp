#include <gtest/gtest.h>

#include "aegis/assistant/grounded/content_hash.hpp"
#include "aegis/assistant/grounded/evidence.hpp"
#include "demo_run.hpp"

namespace {

using namespace aegis::assistant;

EvidenceSet evidenceWithRun() {
    EvidenceSet evidence;
    RunEvidence run = grounded_test::baseRun();
    run.events = {{41, 20, "UnlockFailedEvent", {engine::EntityId{2}}, {}}};
    run.includedTicks = TickRange{0, 120};
    evidence.run = run;
    evidence.entities = {grounded_test::doorFacts()};
    evidence.sources = {{"Scripts/door.cpp", contentHash("a\nb\n"), {"a", "b"}}};
    return evidence;
}

TEST(EvidenceCheck, StateCitations_NeedARetrievedRecordAtThatPoint) {
    const EvidenceSet evidence = evidenceWithRun();
    using grounded_test::demoRunNumber;
    EXPECT_TRUE(
        checkCitation(evidence, StateCitation{demoRunNumber, StatePoint::start()}).supported);
    EXPECT_TRUE(
        checkCitation(evidence, StateCitation{demoRunNumber, StatePoint::atTick(41)}).supported);
    EXPECT_TRUE(
        checkCitation(evidence, StateCitation{demoRunNumber, StatePoint::atTick(0)}).supported);
    EXPECT_FALSE(
        checkCitation(evidence, StateCitation{demoRunNumber, StatePoint::atTick(42)}).supported);
    EXPECT_FALSE(
        checkCitation(evidence, StateCitation{demoRunNumber + 1, StatePoint::start()}).supported);
}

TEST(EvidenceCheck, StartAndTickZero_AreValidatedSeparately) {
    EvidenceSet evidence = evidenceWithRun();
    evidence.run->positions.clear();
    evidence.run->events.clear();
    // Only Start records remain: tick 0 has no record, Start does.
    EXPECT_TRUE(
        checkCitation(evidence, StateCitation{grounded_test::demoRunNumber, StatePoint::start()})
            .supported);
    EXPECT_FALSE(
        checkCitation(evidence, StateCitation{grounded_test::demoRunNumber, StatePoint::atTick(0)})
            .supported);
}

TEST(EvidenceCheck, UnrecordedRun_SupportsNoStateCitation) {
    EvidenceSet evidence = evidenceWithRun();
    evidence.run->recorded = false;
    EXPECT_FALSE(
        checkCitation(evidence, StateCitation{grounded_test::demoRunNumber, StatePoint::start()})
            .supported);
}

TEST(EvidenceCheck, EntityCitations_MustMatchRetrievedFacts) {
    const EvidenceSet evidence = evidenceWithRun();
    const EntityFacts door = grounded_test::doorFacts();
    EXPECT_TRUE(checkCitation(evidence, EntityCitation{door, std::nullopt}).supported);
    EntityFacts renamed = door;
    renamed.name = "Gate";
    EXPECT_FALSE(checkCitation(evidence, EntityCitation{renamed, std::nullopt}).supported);
    EntityFacts runtime = door;
    runtime.createdAtRuntime = true;
    EXPECT_FALSE(checkCitation(evidence, EntityCitation{runtime, std::nullopt}).supported);

    const RunReference atTick{grounded_test::demoRunNumber, StatePoint::atTick(41)};
    EXPECT_TRUE(checkCitation(evidence, EntityCitation{door, atTick}).supported);
    EXPECT_TRUE(checkCitation(
                    evidence,
                    EntityCitation{door, RunReference{grounded_test::demoRunNumber, std::nullopt}})
                    .supported);
    EXPECT_FALSE(
        checkCitation(evidence, EntityCitation{door, RunReference{grounded_test::demoRunNumber,
                                                                  StatePoint::atTick(77)}})
            .supported);
    EXPECT_FALSE(
        checkCitation(evidence, EntityCitation{door, RunReference{99, std::nullopt}}).supported);
}

TEST(EvidenceCheck, SourceCitations_NeedAttachedFileMatchingHashAndLine) {
    const EvidenceSet evidence = evidenceWithRun();
    const std::string hash = contentHash("a\nb\n");
    EXPECT_TRUE(checkCitation(evidence, SourceCitation{"Scripts/door.cpp", 2, hash}).supported);
    EXPECT_FALSE(checkCitation(evidence, SourceCitation{"Scripts/door.cpp", 3, hash}).supported);
    EXPECT_FALSE(
        checkCitation(evidence, SourceCitation{"Scripts/door.cpp", 1, contentHash("a\nc\n")})
            .supported);
    EXPECT_FALSE(checkCitation(evidence, SourceCitation{"Scripts/other.cpp", 1, hash}).supported);
}

} // namespace
