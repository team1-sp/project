#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "aegis/assistant/chat.hpp"
#include "aegis/assistant/grounded/evidence_records.hpp"

// Structured citations of srs.fr.11.4. They are typed values; the {kind, payload} strings of
// US-11.2's chat format and AgentBridge seam are only their serialised form (DEC-067).
namespace aegis::assistant {

// The state of a run at a tick or at Start.
struct StateCitation {
    std::uint32_t runNumber = 0;
    StatePoint point = StatePoint::start();

    bool operator==(const StateCitation&) const = default;
};

// The run an entity citation comes from, with an optional tick or Start.
struct RunReference {
    std::uint32_t runNumber = 0;
    std::optional<StatePoint> point;

    bool operator==(const RunReference&) const = default;
};

struct EntityCitation {
    EntityFacts entity;
    // Empty when the citation refers to the current (edit-time) entity.
    std::optional<RunReference> run;

    bool operator==(const EntityCitation&) const = default;
};

// A line of a source file attached to the question, with the file's content hash at the time of
// the question. Lines are numbered from 1.
struct SourceCitation {
    std::string file;
    std::uint32_t line = 0;
    std::string contentHash;

    bool operator==(const SourceCitation&) const = default;
};

using GroundedCitation = std::variant<StateCitation, EntityCitation, SourceCitation>;

inline constexpr std::string_view stateCitationKind = "state";
inline constexpr std::string_view entityCitationKind = "entity";
inline constexpr std::string_view sourceCitationKind = "source";

// Serialises a citation as a kind and a line-based "key=value" payload with fixed key order.
[[nodiscard]] Citation encodeCitation(const GroundedCitation& citation);

struct DecodedCitation {
    std::optional<GroundedCitation> citation;
    std::string error;

    [[nodiscard]] bool succeeded() const noexcept { return citation.has_value(); }
};

// Strict inverse of encodeCitation(). Unknown kinds, missing, extra or reordered keys, and
// malformed numbers are rejected with a reason; nothing is guessed from free text.
[[nodiscard]] DecodedCitation decodeCitation(std::string_view kind, std::string_view payload);
[[nodiscard]] DecodedCitation decodeCitation(const Citation& citation);

// Short human-readable label, e.g. "run 3, Start" or "run 3, tick 41".
[[nodiscard]] std::string describeStatePoint(std::uint32_t runNumber, StatePoint point);
[[nodiscard]] std::string describeCitation(const GroundedCitation& citation);

} // namespace aegis::assistant
