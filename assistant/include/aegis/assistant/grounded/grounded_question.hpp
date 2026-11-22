#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "aegis/assistant/grounded/evidence_records.hpp"

// The chat-facing question model of US-11.1: the typed question text and the context items the
// user attached to it (srs.fr.11.1, srs.fr.11.6). Each item names what to retrieve; the assistant
// retrieves nothing that no item names.
namespace aegis::assistant {

// Data-category names stored with chat messages (DEC-067, srs.fr.33.59).
inline constexpr std::string_view questionCategory = "question";
inline constexpr std::string_view semanticGraphCategory = "semantic-graph";
inline constexpr std::string_view telemetryCategory = "telemetry";
inline constexpr std::string_view sourceCodeCategory = "source-code";

// srs.fr.33.34: selected entities.
struct SelectionContext {
    std::vector<engine::EntityId> entities;
};

// srs.fr.33.35: the latest run. Without a range, the last 600 ticks of the run are included.
struct LastRunContext {
    std::optional<TickRange> ticks;
};

// srs.fr.33.36: the selected entities' neighbourhood, or the whole graph when empty.
struct GraphContext {
    std::vector<engine::EntityId> focus;
};

// srs.fr.33.53: explicitly selected graph nodes.
struct GraphNodesContext {
    std::vector<std::string> nodeIds;
};

// srs.fr.33.37: exactly the listed project-relative script files.
struct SourceCodeContext {
    std::vector<std::string> files;
};

// srs.fr.33.50.
struct AssetContext {
    std::string path;
};

// srs.fr.33.51: the entry itself travels with the item.
struct LogEntryContext {
    LogEntryFacts entry;
};

// srs.fr.33.52.
struct ComponentTypeContext {
    std::string typeName;
};

using ContextItem =
    std::variant<SelectionContext, LastRunContext, GraphContext, GraphNodesContext,
                 SourceCodeContext, AssetContext, LogEntryContext, ComponentTypeContext>;

struct GroundedQuestion {
    std::string text;
    std::vector<ContextItem> attachments;
};

[[nodiscard]] std::string_view contextItemName(const ContextItem& item);

inline constexpr engine::Tick defaultLastRunTicks = 600;
inline constexpr std::size_t maximumGraphNodes = 500;
inline constexpr unsigned int graphNeighbourRelations = 2;
inline constexpr std::size_t maximumSourceBytes = 200U * 1024U;
inline constexpr std::string_view scriptsFolder = "Scripts/";

} // namespace aegis::assistant
