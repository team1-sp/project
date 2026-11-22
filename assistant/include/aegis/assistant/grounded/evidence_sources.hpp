#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "aegis/assistant/grounded/evidence_records.hpp"

// Read-only query seams the grounded assistant uses to retrieve evidence. The Semantic layer, the
// telemetry recording and the project's asset/script store implement them in their own stories;
// every operation is const and returns copies, so the assistant has no mutation path (srs.fr.11.8,
// DEC-067). Each is called only for a context item the user attached (srs.fr.11.6).
namespace aegis::assistant {

class SemanticGraphQuery {
public:
    virtual ~SemanticGraphQuery() = default;

    [[nodiscard]] virtual std::optional<EntityFacts> entity(engine::EntityId id) const = 0;

    // srs.fr.33.36: the nodes of `focus` and their neighbours up to `maxRelations` away, or the
    // whole graph when `focus` is empty, at most `maxNodes` nodes, edges only between them.
    [[nodiscard]] virtual GraphSlice neighbourhood(std::span<const engine::EntityId> focus,
                                                   unsigned int maxRelations,
                                                   std::size_t maxNodes) const = 0;

    // srs.fr.33.53: exactly the listed nodes and the edges between them.
    [[nodiscard]] virtual GraphSlice nodes(std::span<const std::string> nodeIds) const = 0;

    [[nodiscard]] virtual std::optional<ComponentTypeFacts>
    componentType(std::string_view typeName) const = 0;
};

class RunTelemetryQuery {
public:
    virtual ~RunTelemetryQuery() = default;

    // The latest run's report and recording, or nothing when no run exists. The caller limits
    // ticked records to the context item's tick range.
    [[nodiscard]] virtual std::optional<RunEvidence> latestRun() const = 0;
};

class AssetCatalogQuery {
public:
    virtual ~AssetCatalogQuery() = default;

    [[nodiscard]] virtual std::optional<AssetFacts> asset(std::string_view path) const = 0;
};

// Reads one script by its project-relative path ("Scripts/..."). The grounded assistant calls it
// only for files listed on the Source code context item (srs.fr.11.5, srs.fr.33.37).
class ProjectSourceReader {
public:
    virtual ~ProjectSourceReader() = default;

    [[nodiscard]] virtual std::optional<std::string> readScript(std::string_view path) const = 0;
};

} // namespace aegis::assistant
