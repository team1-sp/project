#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "aegis/assistant/grounded/context_assembly.hpp"
#include "aegis/assistant/grounded/diagnosis.hpp"

namespace aegis::assistant {

inline constexpr std::string_view groundedContextMagic = "AEGIS_GROUNDED_CONTEXT 1";
inline constexpr std::string_view hypothesisLabel =
    "Hypothesis (not established by the evidence): ";
inline constexpr std::string_view unavailableHeading = "Information not available:";

// The evidence-based part of an answer: findings, debug-flag assessments, what is missing and
// labelled hypotheses. Citations travel separately as typed values (diagnosisCitations()).
[[nodiscard]] std::string formatDiagnosis(const Diagnosis& diagnosis);

// Every citation of the diagnosis, in order of first use, without duplicates.
[[nodiscard]] std::vector<GroundedCitation> diagnosisCitations(const Diagnosis& diagnosis);

// The versioned grounded-context document sent to the AI service. It contains only the assembled
// context (question, permitted history, retrieved evidence) and the deterministic diagnosis.
[[nodiscard]] std::string renderContextDocument(const AssembledContext& context,
                                                const Diagnosis& diagnosis);

} // namespace aegis::assistant
