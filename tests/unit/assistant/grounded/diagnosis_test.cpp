#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "aegis/assistant/grounded/answer_format.hpp"
#include "aegis/assistant/grounded/diagnosis.hpp"
#include "demo_run.hpp"

namespace {

using namespace aegis::assistant;
using grounded_test::baseRun;
using grounded_test::demoRunNumber;

const engine::EntityId player{1};
const engine::EntityId door{2};
const engine::EntityId crate{3};
const engine::EntityId goal{4};

AssembledContext contextWith(RunEvidence run) {
    AssembledContext context;
    context.question = "Why did the run fail?";
    context.evidence.run = limitRunToTicks(std::move(run), std::nullopt);
    return context;
}

bool contains(const std::vector<GroundedCitation>& citations, const GroundedCitation& citation) {
    return std::find(citations.begin(), citations.end(), citation) != citations.end();
}

bool anyMissingContains(const Diagnosis& diagnosis, std::string_view text) {
    return std::any_of(diagnosis.missing.begin(), diagnosis.missing.end(),
                       [text](const std::string& m) { return m.find(text) != std::string::npos; });
}

// Every citation the diagnosis returns must be backed by the retrieved evidence.
void expectAllCitationsSupported(const AssembledContext& context, const Diagnosis& diagnosis) {
    for (const GroundedCitation& citation : diagnosisCitations(diagnosis)) {
        const CitationCheck check = checkCitation(context.evidence, citation);
        EXPECT_TRUE(check.supported) << describeCitation(citation) << ": " << check.reason;
    }
}

Diagnosis diagnoseChecked(const AssembledContext& context) {
    Diagnosis diagnosis = diagnose(context);
    expectAllCitationsSupported(context, diagnosis);
    return diagnosis;
}

const DebugFlagAssessment& onlyFlag(const Diagnosis& diagnosis) {
    EXPECT_EQ(diagnosis.debugFlags.size(), 1U);
    return diagnosis.debugFlags.at(0);
}

void expectNotNamedAsCause(const Diagnosis& diagnosis, DebugFlag flag) {
    const std::string formatted = formatDiagnosis(diagnosis);
    const std::string cause = "Cause: the \"" + std::string(debugFlagName(flag)) + "\"";
    EXPECT_EQ(formatted.find(cause), std::string::npos) << formatted;
}

RunEvidence collidersRemovedRun(DebugFlag flag, engine::Vec2 moverPosition) {
    RunEvidence run = baseRun();
    run.report = {{flag, true, {}}};
    run.injections = {{flag, 5, crate, {grounded_test::crateCollider()}, {}}};
    run.positions.push_back({30, player, moverPosition});
    return run;
}

// --- srs.fr.11.3: each debug flag applied, recorded and exercised is named as the cause --------

TEST(GroundedDiagnosis, NoColliderApplied_ExercisedRecord_IsNamedAsCauseWithCitations) {
    const AssembledContext context =
        contextWith(collidersRemovedRun(DebugFlag::NoCollider, {5.2f, 0.1f}));
    const Diagnosis diagnosis = diagnoseChecked(context);
    const DebugFlagAssessment& flag = onlyFlag(diagnosis);
    EXPECT_EQ(flag.status, DebugFlagStatus::CausedFailure);
    EXPECT_NE(flag.finding.statement.find("Cause: the \"no collider\" debug flag removed the "
                                          "colliders of entity 3 \"Crate\""),
              std::string::npos)
        << flag.finding.statement;
    EXPECT_TRUE(
        contains(flag.finding.citations, StateCitation{demoRunNumber, StatePoint::start()}));
    EXPECT_TRUE(
        contains(flag.finding.citations, StateCitation{demoRunNumber, StatePoint::atTick(30)}));
    EXPECT_TRUE(contains(flag.finding.citations,
                         EntityCitation{grounded_test::obstacleFacts(),
                                        RunReference{demoRunNumber, StatePoint::start()}}));
    EXPECT_TRUE(contains(flag.finding.citations,
                         EntityCitation{grounded_test::playerFacts(),
                                        RunReference{demoRunNumber, StatePoint::atTick(30)}}));
}

TEST(GroundedDiagnosis, OutOfBoundsApplied_ExercisedRecord_IsNamedAsCause) {
    const AssembledContext context =
        contextWith(collidersRemovedRun(DebugFlag::OutOfBounds, {4.1f, -0.9f}));
    const Diagnosis diagnosis = diagnoseChecked(context);
    EXPECT_EQ(onlyFlag(diagnosis).status, DebugFlagStatus::CausedFailure);
    EXPECT_NE(onlyFlag(diagnosis).finding.statement.find("\"out of bounds\""), std::string::npos);
}

TEST(GroundedDiagnosis, UnlockWithoutItemApplied_UnlockedWithoutItem_IsNamedAsCause) {
    RunEvidence run = baseRun();
    run.report = {{DebugFlag::UnlockWithoutItem, true, {}}};
    run.injections = {{DebugFlag::UnlockWithoutItem, 5, door, {}, {}}};
    run.events = {{41,
                   30,
                   "UnlockedEvent",
                   {door, player},
                   {{"requiredItem", "Key"}, {"inventory", "Coin, Map"}}}};
    const AssembledContext context = contextWith(run);
    const Diagnosis diagnosis = diagnoseChecked(context);
    const DebugFlagAssessment& flag = onlyFlag(diagnosis);
    EXPECT_EQ(flag.status, DebugFlagStatus::CausedFailure);
    EXPECT_NE(flag.finding.statement.find("required item \"Key\" was not in the inventory"),
              std::string::npos);
    EXPECT_TRUE(
        contains(flag.finding.citations, StateCitation{demoRunNumber, StatePoint::atTick(41)}));
    EXPECT_TRUE(contains(flag.finding.citations,
                         EntityCitation{grounded_test::doorFacts(),
                                        RunReference{demoRunNumber, StatePoint::atTick(41)}}));
}

TEST(GroundedDiagnosis, UnreachableObjectiveApplied_RingCollisionRecorded_IsNamedAsCause) {
    RunEvidence run = baseRun();
    const EntityFacts ring1 = grounded_test::runtimeRingFacts(1);
    const EntityFacts ring2 = grounded_test::runtimeRingFacts(2);
    run.creations.push_back({ring1, StatePoint::start(), 6, {19.0f, 0.0f}});
    run.creations.push_back({ring2, StatePoint::start(), 7, {21.0f, 0.0f}});
    run.report = {{DebugFlag::UnreachableObjective, true, {}}};
    run.injections = {{DebugFlag::UnreachableObjective, 5, goal, {}, {ring1.id, ring2.id}}};
    run.events = {{60, 40, "CollisionEvent", {player, ring1.id}, {}}};
    const AssembledContext context = contextWith(run);
    const Diagnosis diagnosis = diagnoseChecked(context);
    const DebugFlagAssessment& flag = onlyFlag(diagnosis);
    EXPECT_EQ(flag.status, DebugFlagStatus::CausedFailure);
    EXPECT_NE(flag.finding.statement.find("enclosed entity 4 \"Goal\" with 2 added obstacles"),
              std::string::npos)
        << flag.finding.statement;
    EXPECT_TRUE(contains(flag.finding.citations,
                         EntityCitation{ring1, RunReference{demoRunNumber, StatePoint::start()}}));
    EXPECT_TRUE(ring1.createdAtRuntime);
}

// --- srs.fr.11.3 / srs.fr.12.15: flags not applied, not recorded or not exercised --------------

TEST(GroundedDiagnosis, NoColliderNotApplied_SaysSoAndAttributesNothing) {
    RunEvidence run = baseRun();
    run.report = {{DebugFlag::NoCollider, false, "no eligible obstacle"}};
    const AssembledContext context = contextWith(run);
    const Diagnosis diagnosis = diagnoseChecked(context);
    const DebugFlagAssessment& flag = onlyFlag(diagnosis);
    EXPECT_EQ(flag.status, DebugFlagStatus::NotApplied);
    EXPECT_NE(flag.finding.statement.find("was not applied to run 7: no eligible obstacle"),
              std::string::npos);
    EXPECT_NE(flag.finding.statement.find("did not cause a failure"), std::string::npos);
    expectNotNamedAsCause(diagnosis, DebugFlag::NoCollider);
}

TEST(GroundedDiagnosis, UnreachableObjectiveNotApplied_CarriesReportReason) {
    RunEvidence run = baseRun();
    run.report = {{DebugFlag::UnreachableObjective, false, "player inside the ring"}};
    // A collision with an unrelated obstacle must not be blamed on the flag.
    run.events = {{60, 40, "CollisionEvent", {player, crate}, {}}};
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    EXPECT_EQ(onlyFlag(diagnosis).status, DebugFlagStatus::NotApplied);
    EXPECT_NE(onlyFlag(diagnosis).finding.statement.find("player inside the ring"),
              std::string::npos);
    expectNotNamedAsCause(diagnosis, DebugFlag::UnreachableObjective);
}

TEST(GroundedDiagnosis, FlagAppliedButRecordingOff_IsNotRecordedAndNotBlamed) {
    RunEvidence run = baseRun();
    run.recorded = false;
    run.creations.clear();
    run.positions.clear();
    run.report = {{DebugFlag::OutOfBounds, true, {}}};
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    EXPECT_EQ(onlyFlag(diagnosis).status, DebugFlagStatus::NotRecorded);
    EXPECT_NE(onlyFlag(diagnosis).finding.statement.find("not recorded"), std::string::npos);
    EXPECT_TRUE(anyMissingContains(diagnosis, "debug-injection record of \"out of bounds\""));
    EXPECT_TRUE(anyMissingContains(diagnosis, "recording was off"));
    expectNotNamedAsCause(diagnosis, DebugFlag::OutOfBounds);
}

TEST(GroundedDiagnosis, NoColliderAppliedButNeverPassedThrough_IsNotExercised) {
    const AssembledContext context =
        contextWith(collidersRemovedRun(DebugFlag::NoCollider, {-8.0f, 0.0f}));
    const Diagnosis diagnosis = diagnoseChecked(context);
    const DebugFlagAssessment& flag = onlyFlag(diagnosis);
    EXPECT_EQ(flag.status, DebugFlagStatus::NotExercised);
    EXPECT_NE(flag.finding.statement.find("is not named as the cause"), std::string::npos);
    // The applied injection itself is still cited at Start.
    EXPECT_TRUE(
        contains(flag.finding.citations, StateCitation{demoRunNumber, StatePoint::start()}));
    expectNotNamedAsCause(diagnosis, DebugFlag::NoCollider);
}

TEST(GroundedDiagnosis, NoColliderAppliedWithoutPositionSamples_IsUnobservableAndMissingNamed) {
    RunEvidence run = collidersRemovedRun(DebugFlag::NoCollider, {5.0f, 0.0f});
    run.positions.clear();
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    EXPECT_EQ(onlyFlag(diagnosis).status, DebugFlagStatus::EffectUnobservable);
    EXPECT_TRUE(anyMissingContains(diagnosis, "position samples of run 7"));
    expectNotNamedAsCause(diagnosis, DebugFlag::NoCollider);
}

TEST(GroundedDiagnosis, UnlockWithoutItemAppliedButUnlockHadTheItem_IsNotExercised) {
    RunEvidence run = baseRun();
    run.report = {{DebugFlag::UnlockWithoutItem, true, {}}};
    run.injections = {{DebugFlag::UnlockWithoutItem, 5, door, {}, {}}};
    run.events = {{41,
                   30,
                   "UnlockedEvent",
                   {door, player},
                   {{"requiredItem", "Key"}, {"inventory", "Coin, Key"}}}};
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    EXPECT_EQ(onlyFlag(diagnosis).status, DebugFlagStatus::NotExercised);
    expectNotNamedAsCause(diagnosis, DebugFlag::UnlockWithoutItem);
}

TEST(GroundedDiagnosis, UnreachableObjectiveAppliedWithoutRingContact_IsNotExercised) {
    RunEvidence run = baseRun();
    run.report = {{DebugFlag::UnreachableObjective, true, {}}};
    run.injections = {{DebugFlag::UnreachableObjective,
                       5,
                       goal,
                       {},
                       {engine::EntityId{(std::uint64_t{1} << 32) + 1}}}};
    run.events = {{10, 40, "CollisionEvent", {player, crate}, {}}};
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    EXPECT_EQ(onlyFlag(diagnosis).status, DebugFlagStatus::NotExercised);
    expectNotNamedAsCause(diagnosis, DebugFlag::UnreachableObjective);
}

TEST(GroundedDiagnosis, InjectionOutsideTickRange_EffectOutsideRangeIsNotUsed) {
    RunEvidence run = collidersRemovedRun(DebugFlag::NoCollider, {5.0f, 0.0f});
    run.lastCompletedTick = 2000;
    run.positions = {{1500, player, {-8.0f, 0.0f}}, {30, player, {5.0f, 0.0f}}};
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    // Tick 30 is outside the default last-600-ticks range, so the pass-through was not retrieved.
    EXPECT_EQ(onlyFlag(diagnosis).status, DebugFlagStatus::NotExercised);
}

// --- srs.fr.11.2: state questions answered from records --------------------------------------

TEST(GroundedDiagnosis, UnlockFailedEvent_ExplainsMissingItemWithTickAndEntityCitations) {
    RunEvidence run = baseRun();
    run.events = {{41,
                   30,
                   "UnlockFailedEvent",
                   {door, player},
                   {{"requiredItem", "Key"}, {"inventory", "Coin"}}}};
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    ASSERT_EQ(diagnosis.findings.size(), 1U);
    EXPECT_EQ(diagnosis.findings[0].statement,
              "The unlock of entity 2 \"Door\" failed at tick 41: the required item \"Key\" was "
              "not in the inventory [Coin].");
    EXPECT_TRUE(contains(diagnosis.findings[0].citations,
                         StateCitation{demoRunNumber, StatePoint::atTick(41)}));
    EXPECT_TRUE(contains(diagnosis.findings[0].citations,
                         EntityCitation{grounded_test::playerFacts(),
                                        RunReference{demoRunNumber, StatePoint::atTick(41)}}));
}

TEST(GroundedDiagnosis, UnlockFailedEventWithoutPayload_NamesMissingPayload) {
    RunEvidence run = baseRun();
    run.events = {{41, 30, "UnlockFailedEvent", {door}, {}}};
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    EXPECT_TRUE(diagnosis.findings.empty());
    EXPECT_TRUE(anyMissingContains(diagnosis, "required item and inventory"));
}

TEST(GroundedDiagnosis, SampledChangeWithWriteRecord_CitesTheHandlerWrite) {
    RunEvidence run = baseRun();
    run.samples = {{40, door, "Unlockable.unlocked", "false"},
                   {41, door, "Unlockable.unlocked", "true"}};
    run.writes = {
        {StatePoint::atTick(41), 31, "UnlockHandler", door, "Unlockable.unlocked", "true"}};
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    ASSERT_EQ(diagnosis.findings.size(), 1U);
    EXPECT_NE(diagnosis.findings[0].statement.find("the recorded write is by UnlockHandler at "
                                                   "tick 41 (sequence 31)"),
              std::string::npos);
    EXPECT_TRUE(diagnosis.hypotheses.empty());
}

// The read/write declarations of a system are never presented as the cause (srs.fr.11.3).
TEST(GroundedDiagnosis, SampledChangeWithoutWriteRecord_DeclarationsBecomeLabelledHypothesis) {
    AssembledContext context = contextWith([] {
        RunEvidence run = baseRun();
        run.samples = {{40, door, "Unlockable.unlocked", "false"},
                       {41, door, "Unlockable.unlocked", "true"}};
        return run;
    }());
    context.evidence.componentTypes = {
        {"Unlockable", "Can be unlocked", {"unlocked"}, {"DoorSystem"}, {"DoorSystem"}}};
    const Diagnosis diagnosis = diagnoseChecked(context);
    EXPECT_TRUE(diagnosis.findings.empty());
    EXPECT_TRUE(
        anyMissingContains(diagnosis, "a write record for Unlockable.unlocked of entity 2"));
    ASSERT_EQ(diagnosis.hypotheses.size(), 1U);
    EXPECT_NE(diagnosis.hypotheses[0].statement.find("DoorSystem declare writes to Unlockable"),
              std::string::npos);

    const std::string formatted = formatDiagnosis(diagnosis);
    const std::size_t hypothesis = formatted.find(std::string(hypothesisLabel));
    ASSERT_NE(hypothesis, std::string::npos) << formatted;
    // DoorSystem only appears after the hypothesis label.
    EXPECT_GT(formatted.find("DoorSystem"), hypothesis);
}

// --- srs.fr.11.7: abstention -----------------------------------------------------------------

TEST(GroundedDiagnosis, NoAttachedEvidence_AbstainsAndNamesWhatIsMissing) {
    AssembledContext context;
    context.question = "Why did the unlock fail?";
    const Diagnosis diagnosis = diagnoseChecked(context);
    EXPECT_TRUE(diagnosis.findings.empty());
    EXPECT_TRUE(diagnosis.debugFlags.empty());
    EXPECT_TRUE(anyMissingContains(diagnosis, "Semantic Graph or telemetry data"));
    const std::string formatted = formatDiagnosis(diagnosis);
    EXPECT_EQ(formatted.rfind("The attached context does not contain the information needed", 0),
              0U);
    EXPECT_NE(formatted.find(std::string(unavailableHeading)), std::string::npos);
    EXPECT_TRUE(diagnosisCitations(diagnosis).empty());
}

TEST(GroundedDiagnosis, RecordedRunWithNoTickedRecords_NamesMissingRecords) {
    RunEvidence run = baseRun();
    run.positions.clear();
    const Diagnosis diagnosis = diagnoseChecked(contextWith(run));
    EXPECT_TRUE(anyMissingContains(diagnosis, "event records, write records and samples of run 7"));
}

TEST(GroundedDiagnosis, UnavailableContextItems_AreListedAsMissing) {
    AssembledContext context;
    context.unavailable = {"no recorded run exists"};
    const Diagnosis diagnosis = diagnoseChecked(context);
    EXPECT_TRUE(anyMissingContains(diagnosis, "no recorded run exists"));
}

TEST(GroundedDiagnosis, ContextDocument_ContainsRulesEvidenceAndFlagStatuses) {
    const AssembledContext context =
        contextWith(collidersRemovedRun(DebugFlag::NoCollider, {5.2f, 0.1f}));
    const Diagnosis diagnosis = diagnose(context);
    const std::string document = renderContextDocument(context, diagnosis);
    EXPECT_EQ(document.rfind(std::string(groundedContextMagic), 0), 0U);
    EXPECT_NE(document.find("Cite the initial state as Start, never as tick 0."),
              std::string::npos);
    EXPECT_NE(document.find("Start seq=5 injection \"no collider\" target=3"), std::string::npos);
    EXPECT_NE(document.find("debug-flag \"no collider\" CausedFailure"), std::string::npos);
    EXPECT_NE(document.find("QUESTION\nWhy did the run fail?"), std::string::npos);
}

} // namespace
