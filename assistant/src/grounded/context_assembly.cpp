#include "aegis/assistant/grounded/context_assembly.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>

#include "aegis/assistant/grounded/content_hash.hpp"

namespace aegis::assistant {
namespace {

void addUnique(std::vector<std::string>& values, std::string_view value) {
    if (std::find(values.begin(), values.end(), value) == values.end()) {
        values.emplace_back(value);
    }
}

[[nodiscard]] std::vector<std::string> splitLines(std::string_view content) {
    std::vector<std::string> lines;
    std::size_t begin = 0;
    while (begin < content.size()) {
        std::size_t end = content.find('\n', begin);
        if (end == std::string_view::npos) {
            end = content.size();
        }
        std::string_view line = content.substr(begin, end - begin);
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }
        lines.emplace_back(line);
        begin = end + 1;
    }
    return lines;
}

// srs.fr.33.36 and srs.fr.33.53: event nodes and event-derived edges are telemetry; source file
// names in any node are source code.
[[nodiscard]] std::vector<std::string> graphCategories(const GraphSlice& graph) {
    std::vector<std::string> categories{std::string(semanticGraphCategory)};
    for (const GraphNode& node : graph.nodes) {
        if (node.kind == GraphNodeKind::Event) {
            addUnique(categories, telemetryCategory);
        }
        if (!node.sourceFile.empty()) {
            addUnique(categories, sourceCodeCategory);
        }
    }
    for (const GraphEdge& edge : graph.edges) {
        if (edge.relation == "collided with" || edge.relation == "triggered") {
            addUnique(categories, telemetryCategory);
        }
    }
    return categories;
}

class Assembler {
public:
    Assembler(const GroundedSources& sources, const DataTransmissionPolicy& policy)
        : sources_(sources)
        , policy_(policy) {}

    [[nodiscard]] std::string attach(const ContextItem& item) {
        return std::visit([this](const auto& value) { return attachItem(value); }, item);
    }

    void addHistory(const Chat& chat) {
        for (const ChatMessage& message : chat.messages) {
            if (message.text.empty()) {
                continue;
            }
            const auto denied = std::find_if(
                message.dataCategories.begin(), message.dataCategories.end(),
                [this](const std::string& category) { return !policy_.allows(category); });
            if (denied != message.dataCategories.end()) {
                context_.history.push_back(
                    {message.role, "[earlier message left out: contains " + *denied + "]", true});
                continue;
            }
            context_.history.push_back({message.role, message.text, false});
            for (const std::string& category : message.dataCategories) {
                if (category != questionCategory) {
                    addUnique(context_.dataCategories, category);
                }
            }
        }
    }

    [[nodiscard]] AssembledContext take(std::string question) {
        context_.question = std::move(question);
        return std::move(context_);
    }

private:
    [[nodiscard]] std::string requireCategories(std::string_view item,
                                                const std::vector<std::string>& categories) {
        for (const std::string& category : categories) {
            if (!policy_.allows(category)) {
                return "context item " + std::string(item) + " carries data category " + category +
                       ", which is not allowed";
            }
        }
        for (const std::string& category : categories) {
            addUnique(context_.dataCategories, category);
        }
        return {};
    }

    [[nodiscard]] std::string attachItem(const SelectionContext& item) {
        if (std::string error =
                requireCategories("Selection", {std::string(semanticGraphCategory)});
            !error.empty()) {
            return error;
        }
        for (const engine::EntityId id : item.entities) {
            if (auto facts = sources_.graph.entity(id)) {
                context_.evidence.entities.push_back(std::move(*facts));
            } else {
                context_.unavailable.push_back("entity " +
                                               std::to_string(static_cast<std::uint64_t>(id)) +
                                               " is not in the Semantic Graph");
            }
        }
        return {};
    }

    [[nodiscard]] std::string attachItem(const LastRunContext& item) {
        if (std::string error = requireCategories("Last run", {std::string(telemetryCategory)});
            !error.empty()) {
            return error;
        }
        std::optional<RunEvidence> run = sources_.telemetry.latestRun();
        if (!run) {
            context_.unavailable.emplace_back("no recorded run exists");
            return {};
        }
        context_.evidence.run = limitRunToTicks(std::move(*run), item.ticks);
        return {};
    }

    [[nodiscard]] std::string attachItem(const GraphContext& item) {
        return attachGraph("Graph", sources_.graph.neighbourhood(
                                        item.focus, graphNeighbourRelations, maximumGraphNodes));
    }

    [[nodiscard]] std::string attachItem(const GraphNodesContext& item) {
        return attachGraph("Graph nodes", sources_.graph.nodes(item.nodeIds));
    }

    [[nodiscard]] std::string attachGraph(std::string_view name, GraphSlice graph) {
        if (graph.nodes.size() > maximumGraphNodes) {
            return "context item " + std::string(name) + " exceeds 500 nodes";
        }
        if (std::string error = requireCategories(name, graphCategories(graph)); !error.empty()) {
            return error;
        }
        context_.evidence.graphs.push_back(std::move(graph));
        return {};
    }

    [[nodiscard]] std::string attachItem(const SourceCodeContext& item) {
        if (item.files.empty()) {
            return {};
        }
        for (const std::string& path : item.files) {
            if (auto refusal = refuseScriptPath(path)) {
                return "context item Source code: " + *refusal;
            }
        }
        if (std::string error = requireCategories("Source code", {std::string(sourceCodeCategory)});
            !error.empty()) {
            return error;
        }
        std::size_t totalBytes = 0;
        for (const std::string& path : item.files) {
            if (context_.evidence.source(path) != nullptr) {
                continue;
            }
            std::optional<std::string> content = sources_.scripts.readScript(path);
            if (!content) {
                return "context item Source code: cannot read " + path;
            }
            totalBytes += content->size();
            if (totalBytes > maximumSourceBytes) {
                return "context item Source code exceeds 200 KB";
            }
            context_.evidence.sources.push_back(
                {path, contentHash(*content), splitLines(*content)});
        }
        return {};
    }

    [[nodiscard]] std::string attachItem(const AssetContext& item) {
        std::optional<AssetFacts> asset = sources_.assets.asset(item.path);
        if (!asset) {
            context_.unavailable.push_back("asset " + item.path + " is not in the project");
            return {};
        }
        std::vector<std::string> categories;
        if (asset->isScript) {
            categories.emplace_back(sourceCodeCategory);
        }
        if (std::string error = requireCategories("Asset", categories); !error.empty()) {
            return error;
        }
        context_.evidence.assets.push_back(std::move(*asset));
        return {};
    }

    [[nodiscard]] std::string attachItem(const LogEntryContext& item) {
        std::vector<std::string> categories;
        if (!item.entry.stackTrace.empty()) {
            categories.emplace_back(sourceCodeCategory);
        }
        if (item.entry.loggedDuringRun) {
            categories.emplace_back(telemetryCategory);
        }
        if (std::string error = requireCategories("Log entry", categories); !error.empty()) {
            return error;
        }
        context_.evidence.logEntries.push_back(item.entry);
        return {};
    }

    [[nodiscard]] std::string attachItem(const ComponentTypeContext& item) {
        if (std::string error =
                requireCategories("Component type", {std::string(semanticGraphCategory)});
            !error.empty()) {
            return error;
        }
        if (auto facts = sources_.graph.componentType(item.typeName)) {
            context_.evidence.componentTypes.push_back(std::move(*facts));
        } else {
            context_.unavailable.push_back("component type " + item.typeName +
                                           " is not in the Semantic Graph");
        }
        return {};
    }

    const GroundedSources& sources_;
    const DataTransmissionPolicy& policy_;
    AssembledContext context_;
};

template <typename Record, typename Keep>
[[nodiscard]] std::vector<Record> keepIf(std::vector<Record> records, Keep keep) {
    std::vector<Record> kept;
    kept.reserve(records.size());
    std::copy_if(std::make_move_iterator(records.begin()), std::make_move_iterator(records.end()),
                 std::back_inserter(kept), keep);
    return kept;
}

} // namespace

std::string_view contextItemName(const ContextItem& item) {
    constexpr std::string_view names[] = {"Selection",   "Last run", "Graph",     "Graph nodes",
                                          "Source code", "Asset",    "Log entry", "Component type"};
    return names[item.index()];
}

std::optional<std::string> refuseScriptPath(std::string_view path) {
    if (!path.starts_with(scriptsFolder) || path.size() == scriptsFolder.size()) {
        return "file is outside the Scripts folder: " + std::string(path);
    }
    if (path.find('\\') != std::string_view::npos || path.find(':') != std::string_view::npos ||
        path.find('\0') != std::string_view::npos) {
        return "file path is not a project-relative script path: " + std::string(path);
    }
    std::size_t begin = 0;
    while (begin <= path.size()) {
        std::size_t end = path.find('/', begin);
        if (end == std::string_view::npos) {
            end = path.size();
        }
        const std::string_view segment = path.substr(begin, end - begin);
        if (segment.empty() || segment == "." || segment == "..") {
            return "file path is not a project-relative script path: " + std::string(path);
        }
        begin = end + 1;
    }
    return std::nullopt;
}

RunEvidence limitRunToTicks(RunEvidence run, std::optional<TickRange> range) {
    if (!range && run.lastCompletedTick) {
        const engine::Tick last = *run.lastCompletedTick;
        const engine::Tick first = last >= defaultLastRunTicks ? last - defaultLastRunTicks + 1 : 0;
        range = TickRange{first, last};
    }
    run.includedTicks = range;
    const auto inRange = [range](engine::Tick tick) { return range && range->contains(tick); };
    const auto pointKept = [&](StatePoint point) {
        return point.isStart() || inRange(*point.tick());
    };

    run.events =
        keepIf(std::move(run.events), [&](const EventRecord& r) { return inRange(r.tick); });
    run.writes =
        keepIf(std::move(run.writes), [&](const FieldWriteRecord& r) { return pointKept(r.at); });
    run.creations =
        keepIf(std::move(run.creations), [&](const CreationRecord& r) { return pointKept(r.at); });
    run.samples =
        keepIf(std::move(run.samples), [&](const FieldSample& r) { return inRange(r.tick); });
    run.positions =
        keepIf(std::move(run.positions), [&](const PositionSample& r) { return inRange(r.tick); });
    return run;
}

AssemblyResult assembleContext(const GroundedQuestion& question, const Chat& chat,
                               const GroundedSources& sources,
                               const DataTransmissionPolicy& policy) {
    if (question.text.empty()) {
        return {std::nullopt, "question is empty"};
    }
    Assembler assembler(sources, policy);
    for (const ContextItem& item : question.attachments) {
        if (std::string error = assembler.attach(item); !error.empty()) {
            return {std::nullopt, std::move(error)};
        }
    }
    assembler.addHistory(chat);
    return {assembler.take(question.text), {}};
}

} // namespace aegis::assistant
