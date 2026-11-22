#include "aegis/assistant/grounded/diagnosis.hpp"

#include <algorithm>
#include <map>
#include <optional>
#include <tuple>
#include <utility>

namespace aegis::assistant {
namespace {

constexpr std::string_view unlockedEventType = "UnlockedEvent";
constexpr std::string_view unlockFailedEventType = "UnlockFailedEvent";
constexpr std::string_view requiredItemKey = "requiredItem";
constexpr std::string_view inventoryKey = "inventory";

[[nodiscard]] std::string idText(engine::EntityId id) {
    return std::to_string(static_cast<std::uint64_t>(id));
}

[[nodiscard]] std::string tickText(engine::Tick tick) {
    return "tick " + std::to_string(tick);
}

[[nodiscard]] std::optional<std::string> payloadValue(const EventRecord& event,
                                                      std::string_view key) {
    for (const auto& [name, value] : event.payload) {
        if (name == key) {
            return value;
        }
    }
    return std::nullopt;
}

[[nodiscard]] bool listContains(std::string_view list, std::string_view item) {
    std::size_t begin = 0;
    while (begin <= list.size()) {
        std::size_t end = list.find(',', begin);
        if (end == std::string_view::npos) {
            end = list.size();
        }
        std::string_view entry = list.substr(begin, end - begin);
        while (!entry.empty() && entry.front() == ' ') {
            entry.remove_prefix(1);
        }
        while (!entry.empty() && entry.back() == ' ') {
            entry.remove_suffix(1);
        }
        if (!entry.empty() && entry == item) {
            return true;
        }
        begin = end + 1;
    }
    return false;
}

struct Box {
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;

    [[nodiscard]] bool contains(engine::Vec2 point) const noexcept {
        return point.x >= minX && point.x <= maxX && point.y >= minY && point.y <= maxY;
    }
};

// Axis-aligned bounds of a removed collider in world meters. The record has no rotation, so a
// rotated obstacle is approximated by its unrotated bounds.
[[nodiscard]] Box worldBounds(const ColliderRecord& collider, engine::Vec2 entityPosition) {
    const float centerX = entityPosition.x + collider.offsetMeters.x;
    const float centerY = entityPosition.y + collider.offsetMeters.y;
    switch (collider.kind) {
    case ColliderKind::Box:
        return {centerX - collider.sizeMeters.x / 2.0f, centerY - collider.sizeMeters.y / 2.0f,
                centerX + collider.sizeMeters.x / 2.0f, centerY + collider.sizeMeters.y / 2.0f};
    case ColliderKind::Circle:
        return {centerX - collider.radiusMeters, centerY - collider.radiusMeters,
                centerX + collider.radiusMeters, centerY + collider.radiusMeters};
    case ColliderKind::Polygon:
        break;
    }
    Box bounds{centerX, centerY, centerX, centerY};
    for (const engine::Vec2 vertex : collider.verticesMeters) {
        bounds.minX = std::min(bounds.minX, centerX + vertex.x);
        bounds.minY = std::min(bounds.minY, centerY + vertex.y);
        bounds.maxX = std::max(bounds.maxX, centerX + vertex.x);
        bounds.maxY = std::max(bounds.maxY, centerY + vertex.y);
    }
    return bounds;
}

class Diagnoser {
public:
    explicit Diagnoser(const AssembledContext& context)
        : context_(context)
        , evidence_(context.evidence) {}

    [[nodiscard]] Diagnosis run() {
        for (const std::string& item : context_.unavailable) {
            result_.missing.push_back(item);
        }
        const bool anyEvidence = evidence_.hasSemanticGraphData() || evidence_.run.has_value() ||
                                 !evidence_.sources.empty() || !evidence_.assets.empty() ||
                                 !evidence_.logEntries.empty();
        if (!anyEvidence) {
            result_.missing.emplace_back(
                "Semantic Graph or telemetry data: no Selection, Graph, Graph nodes, Component "
                "type or Last run context is attached");
        }
        if (evidence_.run) {
            diagnoseRun(*evidence_.run);
        }
        return std::move(result_);
    }

private:
    void cite(std::vector<GroundedCitation>& citations, GroundedCitation citation) const {
        if (checkCitation(evidence_, citation).supported &&
            std::find(citations.begin(), citations.end(), citation) == citations.end()) {
            citations.push_back(std::move(citation));
        }
    }

    void citeRunEntity(std::vector<GroundedCitation>& citations, engine::EntityId id,
                       StatePoint point) const {
        if (const auto facts = evidence_.runEntity(id)) {
            cite(citations, EntityCitation{*facts, RunReference{evidence_.run->runNumber, point}});
        } else if (const auto current = evidence_.currentEntity(id)) {
            cite(citations, EntityCitation{*current, std::nullopt});
        }
    }

    [[nodiscard]] std::string entityLabel(engine::EntityId id) const {
        if (const auto facts = evidence_.runEntity(id)) {
            return "entity " + idText(id) + " \"" + facts->name + "\"";
        }
        if (const auto facts = evidence_.currentEntity(id)) {
            return "entity " + idText(id) + " \"" + facts->name + "\"";
        }
        return "entity " + idText(id);
    }

    [[nodiscard]] std::string rangeText(const RunEvidence& run) const {
        if (!run.includedTicks) {
            return "the run (no tick completed)";
        }
        return "ticks " + std::to_string(run.includedTicks->first) + "-" +
               std::to_string(run.includedTicks->last);
    }

    void diagnoseRun(const RunEvidence& run) {
        const std::string runText = "run " + std::to_string(run.runNumber);
        if (!run.recorded) {
            result_.missing.push_back("telemetry recording of " + runText +
                                      ": recording was off, so only its test report is available");
        } else if (run.events.empty() && run.writes.empty() && run.samples.empty() &&
                   run.positions.empty()) {
            result_.missing.push_back("event records, write records and samples of " + runText +
                                      " in " + rangeText(run));
        }
        assessDebugFlags(run);
        if (run.recorded) {
            diagnoseUnlockFailures(run);
            diagnoseFieldChanges(run);
        }
    }

    // srs.fr.11.3: a flag is named as the cause only when it was applied, its injection was
    // recorded and a retrieved record shows its effect taking place.
    void assessDebugFlags(const RunEvidence& run) {
        std::vector<DebugFlag> flags;
        for (const DebugFlagReportLine& line : run.report) {
            if (std::find(flags.begin(), flags.end(), line.flag) == flags.end()) {
                flags.push_back(line.flag);
            }
        }
        for (const DebugInjectionRecord& record : run.injections) {
            if (std::find(flags.begin(), flags.end(), record.flag) == flags.end()) {
                flags.push_back(record.flag);
            }
        }
        for (const DebugFlag flag : flags) {
            result_.debugFlags.push_back(assessFlag(run, flag));
        }
    }

    [[nodiscard]] DebugFlagAssessment assessFlag(const RunEvidence& run, DebugFlag flag) {
        const std::string name(debugFlagName(flag));
        const std::string runText = "run " + std::to_string(run.runNumber);
        const auto line =
            std::find_if(run.report.begin(), run.report.end(),
                         [flag](const DebugFlagReportLine& l) { return l.flag == flag; });
        const auto record =
            std::find_if(run.injections.begin(), run.injections.end(),
                         [flag](const DebugInjectionRecord& r) { return r.flag == flag; });

        if (line != run.report.end() && !line->applied) {
            const std::string reason = line->reason.empty() ? "" : ": " + line->reason;
            return {flag,
                    DebugFlagStatus::NotApplied,
                    {"The \"" + name + "\" debug flag was not applied to " + runText + reason +
                         ". It made no change to the run, so it did not cause a failure in it.",
                     {}}};
        }
        if (record == run.injections.end()) {
            result_.missing.push_back("the debug-injection record of \"" + name + "\" in " +
                                      runText + "'s initial state");
            return {flag,
                    DebugFlagStatus::NotRecorded,
                    {"The report says the \"" + name + "\" debug flag was applied to " + runText +
                         ", but its effect is not recorded in the run's telemetry, so a failure "
                         "cannot be attributed to it.",
                     {}}};
        }
        switch (flag) {
        case DebugFlag::NoCollider:
        case DebugFlag::OutOfBounds:
            return assessRemovedCollider(run, *record);
        case DebugFlag::UnlockWithoutItem:
            return assessUnlockWithoutItem(run, *record);
        case DebugFlag::UnreachableObjective:
            return assessEnclosedObjective(run, *record);
        }
        return {flag, DebugFlagStatus::EffectUnobservable, {}};
    }

    [[nodiscard]] Finding injectionFinding(const RunEvidence& run,
                                           const DebugInjectionRecord& record,
                                           std::string statement) const {
        Finding finding{std::move(statement), {}};
        cite(finding.citations, StateCitation{run.runNumber, StatePoint::start()});
        citeRunEntity(finding.citations, record.target, StatePoint::start());
        return finding;
    }

    [[nodiscard]] DebugFlagAssessment notExercised(const RunEvidence& run,
                                                   const DebugInjectionRecord& record,
                                                   std::string_view change) const {
        const std::string name(debugFlagName(record.flag));
        return {record.flag, DebugFlagStatus::NotExercised,
                injectionFinding(run, record,
                                 "The \"" + name + "\" debug flag was applied at Start of run " +
                                     std::to_string(run.runNumber) + " (" + std::string(change) +
                                     "), but no record in " + rangeText(run) +
                                     " shows that effect taking place, so it is not named as the "
                                     "cause of a failure.")};
    }

    [[nodiscard]] DebugFlagAssessment assessRemovedCollider(const RunEvidence& run,
                                                            const DebugInjectionRecord& record) {
        const std::string name(debugFlagName(record.flag));
        const std::string change = "colliders of " + entityLabel(record.target) + " removed";
        const auto creation =
            std::find_if(run.creations.begin(), run.creations.end(),
                         [&](const CreationRecord& c) { return c.entity.id == record.target; });
        if (creation == run.creations.end() || record.removedColliders.empty()) {
            result_.missing.push_back("the creation record and removed colliders of " +
                                      entityLabel(record.target) + " in run " +
                                      std::to_string(run.runNumber));
            return {
                record.flag, DebugFlagStatus::EffectUnobservable,
                injectionFinding(run, record,
                                 "The \"" + name + "\" debug flag was applied (" + change +
                                     "), but the records needed to see its effect are missing.")};
        }
        if (run.positions.empty()) {
            result_.missing.push_back("position samples of run " + std::to_string(run.runNumber) +
                                      " in " + rangeText(run));
            return {record.flag, DebugFlagStatus::EffectUnobservable,
                    injectionFinding(run, record,
                                     "The \"" + name + "\" debug flag was applied (" + change +
                                         "), but no position samples were retrieved, so whether "
                                         "anything passed through the obstacle is unknown.")};
        }
        for (const PositionSample& sample : run.positions) {
            if (sample.entity == record.target) {
                continue;
            }
            for (const ColliderRecord& collider : record.removedColliders) {
                if (worldBounds(collider, creation->positionMeters)
                        .contains(sample.positionMeters)) {
                    Finding finding = injectionFinding(
                        run, record,
                        "Cause: the \"" + name + "\" debug flag removed the colliders of " +
                            entityLabel(record.target) + " at Start, and " +
                            entityLabel(sample.entity) +
                            " was inside the removed collider's area "
                            "at " +
                            tickText(sample.tick) + ".");
                    cite(finding.citations,
                         StateCitation{run.runNumber, StatePoint::atTick(sample.tick)});
                    citeRunEntity(finding.citations, sample.entity,
                                  StatePoint::atTick(sample.tick));
                    return {record.flag, DebugFlagStatus::CausedFailure, std::move(finding)};
                }
            }
        }
        return notExercised(run, record, change);
    }

    [[nodiscard]] DebugFlagAssessment assessUnlockWithoutItem(const RunEvidence& run,
                                                              const DebugInjectionRecord& record) {
        const std::string name(debugFlagName(record.flag));
        for (const EventRecord& event : run.events) {
            if (event.type != unlockedEventType ||
                std::find(event.entities.begin(), event.entities.end(), record.target) ==
                    event.entities.end()) {
                continue;
            }
            const auto required = payloadValue(event, requiredItemKey);
            const auto inventory = payloadValue(event, inventoryKey);
            if (!required || !inventory || listContains(*inventory, *required)) {
                continue;
            }
            Finding finding = injectionFinding(
                run, record,
                "Cause: the \"" + name + "\" debug flag unlocked " + entityLabel(record.target) +
                    " at " + tickText(event.tick) + " although the required item \"" + *required +
                    "\" was not in the inventory [" + *inventory + "].");
            cite(finding.citations, StateCitation{run.runNumber, StatePoint::atTick(event.tick)});
            for (const engine::EntityId id : event.entities) {
                citeRunEntity(finding.citations, id, StatePoint::atTick(event.tick));
            }
            return {record.flag, DebugFlagStatus::CausedFailure, std::move(finding)};
        }
        return notExercised(run, record, "acts on " + entityLabel(record.target));
    }

    [[nodiscard]] DebugFlagAssessment assessEnclosedObjective(const RunEvidence& run,
                                                              const DebugInjectionRecord& record) {
        const std::string name(debugFlagName(record.flag));
        const std::string change = std::to_string(record.addedObstacles.size()) +
                                   " obstacles added around " + entityLabel(record.target);
        for (const EventRecord& event : run.events) {
            const auto ring =
                std::find_first_of(event.entities.begin(), event.entities.end(),
                                   record.addedObstacles.begin(), record.addedObstacles.end());
            if (ring == event.entities.end()) {
                continue;
            }
            const auto other = std::find_if(
                event.entities.begin(), event.entities.end(), [&](engine::EntityId id) {
                    return std::find(record.addedObstacles.begin(), record.addedObstacles.end(),
                                     id) == record.addedObstacles.end();
                });
            if (other == event.entities.end()) {
                continue;
            }
            Finding finding = injectionFinding(
                run, record,
                "Cause: the \"" + name + "\" debug flag enclosed " + entityLabel(record.target) +
                    " with " + std::to_string(record.addedObstacles.size()) +
                    " added obstacles at Start, and " + entityLabel(*other) + " was stopped by " +
                    entityLabel(*ring) + " (" + event.type + ") at " + tickText(event.tick) + ".");
            cite(finding.citations, StateCitation{run.runNumber, StatePoint::atTick(event.tick)});
            citeRunEntity(finding.citations, *other, StatePoint::atTick(event.tick));
            citeRunEntity(finding.citations, *ring, StatePoint::start());
            return {record.flag, DebugFlagStatus::CausedFailure, std::move(finding)};
        }
        return notExercised(run, record, change);
    }

    // srs.fr.11.2: "Why did the unlock fail?" is answered from the UnlockFailedEvent payload.
    void diagnoseUnlockFailures(const RunEvidence& run) {
        for (const EventRecord& event : run.events) {
            if (event.type != unlockFailedEventType) {
                continue;
            }
            const auto required = payloadValue(event, requiredItemKey);
            const auto inventory = payloadValue(event, inventoryKey);
            const std::string subject =
                event.entities.empty() ? "an Unlockable" : entityLabel(event.entities.front());
            if (!required || !inventory) {
                result_.missing.push_back("the required item and inventory in the " +
                                          std::string(unlockFailedEventType) + " payload at " +
                                          tickText(event.tick));
                continue;
            }
            Finding finding{"The unlock of " + subject + " failed at " + tickText(event.tick) +
                                ": the required item \"" + *required +
                                "\" was not in the inventory [" + *inventory + "].",
                            {}};
            cite(finding.citations, StateCitation{run.runNumber, StatePoint::atTick(event.tick)});
            for (const engine::EntityId id : event.entities) {
                citeRunEntity(finding.citations, id, StatePoint::atTick(event.tick));
            }
            result_.findings.push_back(std::move(finding));
        }
    }

    // A sampled value change is attributed only to a recorded write; read/write declarations alone
    // become a labelled hypothesis (srs.fr.11.3).
    void diagnoseFieldChanges(const RunEvidence& run) {
        using Key = std::tuple<std::uint64_t, std::string>;
        std::map<Key, std::vector<const FieldSample*>> series;
        for (const FieldSample& sample : run.samples) {
            series[{static_cast<std::uint64_t>(sample.entity), sample.field}].push_back(&sample);
        }
        for (auto& [key, samples] : series) {
            std::stable_sort(
                samples.begin(), samples.end(),
                [](const FieldSample* a, const FieldSample* b) { return a->tick < b->tick; });
            for (std::size_t index = 1; index < samples.size(); ++index) {
                const FieldSample& before = *samples[index - 1];
                const FieldSample& after = *samples[index];
                if (before.value != after.value) {
                    explainChange(run, before, after);
                }
            }
        }
    }

    void explainChange(const RunEvidence& run, const FieldSample& before,
                       const FieldSample& after) {
        const std::string change = before.field + " of " + entityLabel(after.entity) +
                                   " changed from \"" + before.value + "\" at " +
                                   tickText(before.tick) + " to \"" + after.value + "\" at " +
                                   tickText(after.tick);
        const FieldWriteRecord* write = nullptr;
        for (const FieldWriteRecord& candidate : run.writes) {
            const auto tick = candidate.at.tick();
            if (candidate.entity == after.entity && candidate.field == after.field && tick &&
                *tick > before.tick && *tick <= after.tick) {
                write = &candidate;
            }
        }
        if (write != nullptr) {
            Finding finding{change + "; the recorded write is by " + write->writer + " at " +
                                tickText(*write->at.tick()) + " (sequence " +
                                std::to_string(write->sequence) + ").",
                            {}};
            cite(finding.citations, StateCitation{run.runNumber, write->at});
            citeRunEntity(finding.citations, after.entity, write->at);
            result_.findings.push_back(std::move(finding));
            return;
        }
        result_.missing.push_back("a write record for " + after.field + " of " +
                                  entityLabel(after.entity) + " after " + tickText(before.tick) +
                                  " and up to " + tickText(after.tick));
        const std::string component = after.field.substr(0, after.field.find('.'));
        for (const ComponentTypeFacts& type : evidence_.componentTypes) {
            if (type.name != component || type.writingSystems.empty()) {
                continue;
            }
            std::string writers;
            for (const std::string& system : type.writingSystems) {
                writers += (writers.empty() ? "" : ", ") + system;
            }
            Hypothesis hypothesis{
                change + ". " + writers + " declare writes to " + component +
                    ", but no write record shows which, if any, made this change.",
                {}};
            cite(hypothesis.citations,
                 StateCitation{run.runNumber, StatePoint::atTick(after.tick)});
            result_.hypotheses.push_back(std::move(hypothesis));
        }
    }

    const AssembledContext& context_;
    const EvidenceSet& evidence_;
    Diagnosis result_;
};

} // namespace

std::string_view debugFlagName(DebugFlag flag) {
    switch (flag) {
    case DebugFlag::NoCollider:
        return "no collider";
    case DebugFlag::UnlockWithoutItem:
        return "unlock without item";
    case DebugFlag::UnreachableObjective:
        return "unreachable objective";
    case DebugFlag::OutOfBounds:
        return "out of bounds";
    }
    return "unknown";
}

Diagnosis diagnose(const AssembledContext& context) {
    return Diagnoser(context).run();
}

} // namespace aegis::assistant
