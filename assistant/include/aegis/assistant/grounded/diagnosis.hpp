#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "aegis/assistant/grounded/citations.hpp"
#include "aegis/assistant/grounded/context_assembly.hpp"

// Deterministic, evidence-only diagnosis (srs.fr.11.2, srs.fr.11.3, srs.fr.11.7). It derives
// findings from records in the assembled context and cites only those records; whatever it cannot
// establish becomes a named missing item or an explicitly labelled hypothesis.
namespace aegis::assistant {

struct Finding {
    std::string statement;
    std::vector<GroundedCitation> citations;
};

// Never presented as an established cause; the formatter prefixes it with "Hypothesis:".
struct Hypothesis {
    std::string statement;
    std::vector<GroundedCitation> citations;
};

enum class DebugFlagStatus {
    // The report says the flag was not applied (srs.fr.12.15).
    NotApplied,
    // The report says it was applied, but no debug-injection record exists in the recording.
    NotRecorded,
    // The injection was recorded but no retrieved record shows its effect taking place.
    NotExercised,
    // The injection was recorded but the records needed to see its effect were not retrieved.
    EffectUnobservable,
    // The injection was recorded and a retrieved record shows its effect: it is named as the cause.
    CausedFailure,
};

struct DebugFlagAssessment {
    DebugFlag flag = DebugFlag::NoCollider;
    DebugFlagStatus status = DebugFlagStatus::NotApplied;
    Finding finding;
};

struct Diagnosis {
    std::vector<Finding> findings;
    std::vector<DebugFlagAssessment> debugFlags;
    std::vector<std::string> missing;
    std::vector<Hypothesis> hypotheses;
};

[[nodiscard]] std::string_view debugFlagName(DebugFlag flag);

// Every citation in the result passes checkCitation() against `context.evidence`.
[[nodiscard]] Diagnosis diagnose(const AssembledContext& context);

} // namespace aegis::assistant
