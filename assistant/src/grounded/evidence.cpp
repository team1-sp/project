#include "aegis/assistant/grounded/evidence.hpp"

#include <algorithm>

namespace aegis::assistant {
namespace {

[[nodiscard]] bool hasStartRecord(const RunEvidence& run) {
    const bool startWrite = std::any_of(run.writes.begin(), run.writes.end(),
                                        [](const FieldWriteRecord& w) { return w.at.isStart(); });
    const bool startCreation = std::any_of(run.creations.begin(), run.creations.end(),
                                           [](const CreationRecord& c) { return c.at.isStart(); });
    return !run.injections.empty() || startWrite || startCreation;
}

[[nodiscard]] bool hasTickRecord(const RunEvidence& run, engine::Tick tick) {
    if (run.lastCompletedTick && tick > *run.lastCompletedTick) {
        return false;
    }
    const auto atTick = [tick](StatePoint point) { return point.tick() == tick; };
    return std::any_of(run.events.begin(), run.events.end(),
                       [tick](const EventRecord& e) { return e.tick == tick; }) ||
           std::any_of(run.writes.begin(), run.writes.end(),
                       [&](const FieldWriteRecord& w) { return atTick(w.at); }) ||
           std::any_of(run.creations.begin(), run.creations.end(),
                       [&](const CreationRecord& c) { return atTick(c.at); }) ||
           std::any_of(run.samples.begin(), run.samples.end(),
                       [tick](const FieldSample& s) { return s.tick == tick; }) ||
           std::any_of(run.positions.begin(), run.positions.end(),
                       [tick](const PositionSample& p) { return p.tick == tick; });
}

[[nodiscard]] CitationCheck checkState(const EvidenceSet& evidence, std::uint32_t runNumber,
                                       StatePoint point) {
    if (!evidence.run || evidence.run->runNumber != runNumber) {
        return {false, "run " + std::to_string(runNumber) + " is not in the attached context"};
    }
    if (!evidence.run->recorded) {
        return {false, "run " + std::to_string(runNumber) + " has no telemetry recording"};
    }
    if (point.isStart()) {
        return hasStartRecord(*evidence.run)
                   ? CitationCheck{true, {}}
                   : CitationCheck{false, "no initial-state record was retrieved"};
    }
    return hasTickRecord(*evidence.run, *point.tick())
               ? CitationCheck{true, {}}
               : CitationCheck{false,
                               "no retrieved record at " + describeStatePoint(runNumber, point)};
}

[[nodiscard]] CitationCheck checkEntity(const EvidenceSet& evidence,
                                        const EntityCitation& citation) {
    if (!citation.run) {
        const auto facts = evidence.currentEntity(citation.entity.id);
        if (!facts || *facts != citation.entity) {
            return {false, "entity is not in the attached context or its facts differ"};
        }
        return {true, {}};
    }
    if (!evidence.run || evidence.run->runNumber != citation.run->runNumber) {
        return {false, "the entity's run is not in the attached context"};
    }
    const auto facts = evidence.runEntity(citation.entity.id);
    if (!facts || *facts != citation.entity) {
        return {false, "no creation record with these entity facts in the run"};
    }
    if (citation.run->point) {
        return checkState(evidence, citation.run->runNumber, *citation.run->point);
    }
    return {true, {}};
}

[[nodiscard]] CitationCheck checkSource(const EvidenceSet& evidence,
                                        const SourceCitation& citation) {
    const AttachedSourceFile* file = evidence.source(citation.file);
    if (file == nullptr) {
        return {false, "source file is not attached: " + citation.file};
    }
    if (file->contentHash != citation.contentHash) {
        return {false, "content hash differs from the attached file"};
    }
    if (citation.line == 0 || citation.line > file->lines.size()) {
        return {false, "line is outside the attached file"};
    }
    return {true, {}};
}

} // namespace

bool EvidenceSet::hasSemanticGraphData() const noexcept {
    return !entities.empty() || !graphs.empty() || !componentTypes.empty();
}

bool EvidenceSet::hasRunTelemetry() const noexcept {
    return run.has_value() && run->recorded;
}

const AttachedSourceFile* EvidenceSet::source(std::string_view path) const noexcept {
    const auto found = std::find_if(sources.begin(), sources.end(),
                                    [path](const AttachedSourceFile& f) { return f.path == path; });
    return found == sources.end() ? nullptr : &*found;
}

std::optional<EntityFacts> EvidenceSet::currentEntity(engine::EntityId id) const {
    for (const EntityFacts& facts : entities) {
        if (facts.id == id) {
            return facts;
        }
    }
    for (const GraphSlice& graph : graphs) {
        for (const GraphNode& node : graph.nodes) {
            if (node.entity && node.entity->id == id) {
                return node.entity;
            }
        }
    }
    return std::nullopt;
}

std::optional<EntityFacts> EvidenceSet::runEntity(engine::EntityId id) const {
    if (!run) {
        return std::nullopt;
    }
    for (const CreationRecord& creation : run->creations) {
        if (creation.entity.id == id) {
            return creation.entity;
        }
    }
    return std::nullopt;
}

CitationCheck checkCitation(const EvidenceSet& evidence, const GroundedCitation& citation) {
    if (const auto* state = std::get_if<StateCitation>(&citation)) {
        return checkState(evidence, state->runNumber, state->point);
    }
    if (const auto* entity = std::get_if<EntityCitation>(&citation)) {
        return checkEntity(evidence, *entity);
    }
    return checkSource(evidence, std::get<SourceCitation>(citation));
}

} // namespace aegis::assistant
