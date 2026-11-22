#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "aegis/assistant/chat.hpp"
#include "aegis/assistant/grounded/evidence.hpp"
#include "aegis/assistant/grounded/evidence_sources.hpp"
#include "aegis/assistant/grounded/grounded_question.hpp"
#include "aegis/assistant/read_only_assistant.hpp"

namespace aegis::assistant {

// The read-only sources a question's context items may be retrieved from. Held by reference; the
// owner keeps them alive while a question is assembled.
struct GroundedSources {
    const SemanticGraphQuery& graph;
    const RunTelemetryQuery& telemetry;
    const AssetCatalogQuery& assets;
    const ProjectSourceReader& scripts;
};

// An earlier message of the same chat as it goes into the request (srs.fr.33.46, srs.fr.33.56).
struct HistoryMessage {
    MessageRole role = MessageRole::Question;
    std::string text;
    bool leftOut = false;

    bool operator==(const HistoryMessage&) const = default;
};

// Everything the assistant may use for one question (srs.fr.11.6): the typed text, the earlier
// messages consent allows, and the evidence retrieved for the attached items.
struct AssembledContext {
    std::string question;
    std::vector<HistoryMessage> history;
    EvidenceSet evidence;
    // Categories the request carries besides the question (srs.fr.33.59), in first-seen order.
    std::vector<std::string> dataCategories;
    // What an attached item asked for but the sources do not have, e.g. "no recorded run".
    std::vector<std::string> unavailable;
};

struct AssemblyResult {
    std::optional<AssembledContext> context;
    std::string error;

    [[nodiscard]] bool succeeded() const noexcept { return context.has_value() && error.empty(); }
};

// Builds the context of one question. It reads only what the attached items name, through the
// const sources, and refuses (with an error naming the item) any item whose data category the
// policy does not allow or whose source path is outside the Scripts folder. Earlier messages that
// carry a disallowed category are replaced by "[earlier message left out: contains <category>]".
[[nodiscard]] AssemblyResult assembleContext(const GroundedQuestion& question, const Chat& chat,
                                             const GroundedSources& sources,
                                             const DataTransmissionPolicy& policy);

// srs.fr.33.37: a project-relative path inside "Scripts/" with no ".", ".." or empty segment, no
// backslash, drive or absolute prefix. Returns the reason when the path is refused.
[[nodiscard]] std::optional<std::string> refuseScriptPath(std::string_view path);

// Limits ticked records to `range` (default: the run's last 600 ticks) and keeps every Start
// record (srs.fr.33.35).
[[nodiscard]] RunEvidence limitRunToTicks(RunEvidence run, std::optional<TickRange> range);

} // namespace aegis::assistant
