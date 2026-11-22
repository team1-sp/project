#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "aegis/assistant/grounded/citations.hpp"
#include "aegis/assistant/grounded/evidence_records.hpp"

namespace aegis::assistant {

// An attached script as read at the time of the question.
struct AttachedSourceFile {
    std::string path;
    std::string contentHash;
    std::vector<std::string> lines;

    bool operator==(const AttachedSourceFile&) const = default;
};

// Everything retrieved for one question, and nothing else. A citation is valid only when this set
// contains the record it points at.
struct EvidenceSet {
    std::vector<EntityFacts> entities;
    std::vector<GraphSlice> graphs;
    std::vector<ComponentTypeFacts> componentTypes;
    std::vector<AssetFacts> assets;
    std::vector<LogEntryFacts> logEntries;
    std::optional<RunEvidence> run;
    std::vector<AttachedSourceFile> sources;

    [[nodiscard]] bool hasSemanticGraphData() const noexcept;
    [[nodiscard]] bool hasRunTelemetry() const noexcept;

    [[nodiscard]] const AttachedSourceFile* source(std::string_view path) const noexcept;
    [[nodiscard]] std::optional<EntityFacts> currentEntity(engine::EntityId id) const;
    [[nodiscard]] std::optional<EntityFacts> runEntity(engine::EntityId id) const;
};

struct CitationCheck {
    bool supported = false;
    std::string reason;
};

// Validates a citation against retrieved evidence (srs.fr.11.4):
// - state: the run was retrieved and has a record at that tick, or initial-state records for Start;
// - entity: identity facts match a retrieved entity, and a run reference matches its creation
//   record and a retrieved state point;
// - source: the file is attached, the hash is its hash at question time, the line exists.
[[nodiscard]] CitationCheck checkCitation(const EvidenceSet& evidence,
                                          const GroundedCitation& citation);

} // namespace aegis::assistant
