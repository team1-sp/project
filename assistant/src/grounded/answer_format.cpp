#include "aegis/assistant/grounded/answer_format.hpp"

#include <algorithm>
#include <cstddef>
#include <sstream>

namespace aegis::assistant {
namespace {

[[nodiscard]] std::string idText(engine::EntityId id) {
    return std::to_string(static_cast<std::uint64_t>(id));
}

[[nodiscard]] std::string pointText(StatePoint point) {
    return point.isStart() ? "Start" : "tick " + std::to_string(*point.tick());
}

[[nodiscard]] std::string citationList(const std::vector<GroundedCitation>& citations) {
    if (citations.empty()) {
        return {};
    }
    std::string text = " [";
    for (std::size_t index = 0; index < citations.size(); ++index) {
        text += (index == 0 ? "" : "; ") + describeCitation(citations[index]);
    }
    return text + "]";
}

[[nodiscard]] std::string_view statusName(DebugFlagStatus status) {
    switch (status) {
    case DebugFlagStatus::NotApplied:
        return "NotApplied";
    case DebugFlagStatus::NotRecorded:
        return "NotRecorded";
    case DebugFlagStatus::NotExercised:
        return "NotExercised";
    case DebugFlagStatus::EffectUnobservable:
        return "EffectUnobservable";
    case DebugFlagStatus::CausedFailure:
        return "CausedFailure";
    }
    return "Unknown";
}

[[nodiscard]] std::string_view nodeKindName(GraphNodeKind kind) {
    switch (kind) {
    case GraphNodeKind::Entity:
        return "entity";
    case GraphNodeKind::System:
        return "system";
    case GraphNodeKind::ComponentType:
        return "component-type";
    case GraphNodeKind::Event:
        return "event";
    case GraphNodeKind::Asset:
        return "asset";
    }
    return "unknown";
}

void writeEntity(std::ostream& out, const EntityFacts& facts) {
    out << "entity id=" << idText(facts.id) << " name=\"" << facts.name << "\" scene=\""
        << facts.sourceScene << "\" runtime=" << (facts.createdAtRuntime ? "true" : "false");
}

void writeGraph(std::ostream& out, const EvidenceSet& evidence) {
    for (const EntityFacts& facts : evidence.entities) {
        writeEntity(out, facts);
        out << '\n';
    }
    for (const GraphSlice& graph : evidence.graphs) {
        out << "GRAPH nodes=" << graph.nodes.size() << " omitted=" << graph.omittedNodes << '\n';
        for (const GraphNode& node : graph.nodes) {
            out << "node " << node.id << ' ' << nodeKindName(node.kind) << " \"" << node.label
                << '"';
            if (!node.sourceFile.empty()) {
                out << " file=" << node.sourceFile;
            }
            if (node.entity) {
                out << ' ';
                writeEntity(out, *node.entity);
            }
            out << '\n';
        }
        for (const GraphEdge& edge : graph.edges) {
            out << "edge " << edge.from << " -" << edge.relation << "-> " << edge.to << '\n';
        }
    }
    for (const ComponentTypeFacts& type : evidence.componentTypes) {
        out << "component-type " << type.name << " \"" << type.semanticDescription << "\"";
        out << " declared-readers=" << type.readingSystems.size()
            << " declared-writers=" << type.writingSystems.size() << '\n';
        for (const std::string& system : type.writingSystems) {
            out << "  declares-write " << system << '\n';
        }
    }
}

void writeRun(std::ostream& out, const RunEvidence& run) {
    out << "RUN " << run.runNumber << " recorded=" << (run.recorded ? "true" : "false");
    if (run.includedTicks) {
        out << " ticks=" << run.includedTicks->first << '-' << run.includedTicks->last;
    }
    out << '\n';
    for (const DebugFlagReportLine& line : run.report) {
        out << "report \"" << debugFlagName(line.flag) << "\" "
            << (line.applied ? "applied" : "not applied: " + line.reason) << '\n';
    }
    for (const DebugInjectionRecord& record : run.injections) {
        out << "Start seq=" << record.sequence << " injection \"" << debugFlagName(record.flag)
            << "\" target=" << idText(record.target)
            << " removed-colliders=" << record.removedColliders.size()
            << " added-obstacles=" << record.addedObstacles.size() << '\n';
    }
    for (const CreationRecord& creation : run.creations) {
        out << pointText(creation.at) << " seq=" << creation.sequence << " created ";
        writeEntity(out, creation.entity);
        out << " at=(" << creation.positionMeters.x << ',' << creation.positionMeters.y << ")\n";
    }
    for (const EventRecord& event : run.events) {
        out << "tick " << event.tick << " seq=" << event.sequence << " event " << event.type;
        for (const engine::EntityId id : event.entities) {
            out << " entity=" << idText(id);
        }
        for (const auto& [key, value] : event.payload) {
            out << ' ' << key << "=\"" << value << '"';
        }
        out << '\n';
    }
    for (const FieldWriteRecord& write : run.writes) {
        out << pointText(write.at) << " seq=" << write.sequence << " write " << write.field
            << " entity=" << idText(write.entity) << " value=\"" << write.value
            << "\" by=" << write.writer << '\n';
    }
    for (const FieldSample& sample : run.samples) {
        out << "tick " << sample.tick << " sample " << sample.field
            << " entity=" << idText(sample.entity) << " value=\"" << sample.value << "\"\n";
    }
    for (const PositionSample& sample : run.positions) {
        out << "tick " << sample.tick << " position entity=" << idText(sample.entity) << " at=("
            << sample.positionMeters.x << ',' << sample.positionMeters.y << ")\n";
    }
}

void writeOtherItems(std::ostream& out, const EvidenceSet& evidence) {
    for (const AssetFacts& asset : evidence.assets) {
        out << "asset name=\"" << asset.name << "\" path=" << asset.path << " type=" << asset.type
            << " size=" << asset.sizeBytes << '\n';
        for (const auto& [key, value] : asset.importSettings) {
            out << "  import " << key << "=\"" << value << "\"\n";
        }
    }
    for (const LogEntryFacts& entry : evidence.logEntries) {
        out << "log " << entry.level;
        if (entry.tick) {
            out << " tick=" << *entry.tick;
        }
        out << " message=\"" << entry.message << "\"\n";
        if (!entry.stackTrace.empty()) {
            out << "  stack " << entry.stackTrace << '\n';
        }
    }
    for (const AttachedSourceFile& file : evidence.sources) {
        out << "SOURCE " << file.path << " hash=" << file.contentHash
            << " lines=" << file.lines.size() << '\n';
        for (std::size_t index = 0; index < file.lines.size(); ++index) {
            out << (index + 1) << "| " << file.lines[index] << '\n';
        }
    }
}

} // namespace

std::string formatDiagnosis(const Diagnosis& diagnosis) {
    std::ostringstream out;
    if (diagnosis.findings.empty() && diagnosis.debugFlags.empty()) {
        out << "The attached context does not contain the information needed to establish an "
               "answer.\n";
    }
    if (!diagnosis.findings.empty()) {
        out << "Findings from the attached evidence:\n";
        for (const Finding& finding : diagnosis.findings) {
            out << "- " << finding.statement << citationList(finding.citations) << '\n';
        }
    }
    if (!diagnosis.debugFlags.empty()) {
        out << "Debug flags:\n";
        for (const DebugFlagAssessment& flag : diagnosis.debugFlags) {
            out << "- " << flag.finding.statement << citationList(flag.finding.citations) << '\n';
        }
    }
    if (!diagnosis.missing.empty()) {
        out << unavailableHeading << '\n';
        for (const std::string& missing : diagnosis.missing) {
            out << "- " << missing << '\n';
        }
    }
    for (const Hypothesis& hypothesis : diagnosis.hypotheses) {
        out << hypothesisLabel << hypothesis.statement << citationList(hypothesis.citations)
            << '\n';
    }
    return out.str();
}

std::vector<GroundedCitation> diagnosisCitations(const Diagnosis& diagnosis) {
    std::vector<GroundedCitation> all;
    const auto add = [&all](const std::vector<GroundedCitation>& citations) {
        for (const GroundedCitation& citation : citations) {
            if (std::find(all.begin(), all.end(), citation) == all.end()) {
                all.push_back(citation);
            }
        }
    };
    for (const Finding& finding : diagnosis.findings) {
        add(finding.citations);
    }
    for (const DebugFlagAssessment& flag : diagnosis.debugFlags) {
        add(flag.finding.citations);
    }
    for (const Hypothesis& hypothesis : diagnosis.hypotheses) {
        add(hypothesis.citations);
    }
    return all;
}

std::string renderContextDocument(const AssembledContext& context, const Diagnosis& diagnosis) {
    std::ostringstream out;
    out << groundedContextMagic << '\n';
    out << "RULES\n"
           "- Use only this document; no other project content is available.\n"
           "- Cite with structured citations of kind state, entity or source whose payload lists "
           "records in this document. Citations that do not match a record are removed.\n"
           "- Cite the initial state as Start, never as tick 0.\n"
           "- Prefix any statement the records do not establish with \"Hypothesis:\".\n"
           "- When information is missing, say it is unavailable and name what is missing.\n"
           "- Name a debug flag as a cause only when DIAGNOSIS lists it as CausedFailure.\n"
           "- Never present a system's declared reads or writes as the cause of a change.\n";
    out << "QUESTION\n" << context.question << '\n';
    out << "HISTORY " << context.history.size() << '\n';
    for (const HistoryMessage& message : context.history) {
        out << (message.role == MessageRole::Question ? "user: " : "assistant: ") << message.text
            << '\n';
    }
    out << "EVIDENCE\n";
    writeGraph(out, context.evidence);
    if (context.evidence.run) {
        writeRun(out, *context.evidence.run);
    }
    writeOtherItems(out, context.evidence);
    out << "DIAGNOSIS\n";
    for (const DebugFlagAssessment& flag : diagnosis.debugFlags) {
        out << "debug-flag \"" << debugFlagName(flag.flag) << "\" " << statusName(flag.status)
            << '\n';
    }
    out << formatDiagnosis(diagnosis);
    return out.str();
}

} // namespace aegis::assistant
