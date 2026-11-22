#include "editor/assistant_chat_context.hpp"

#include <algorithm>
#include <cstdio>
#include <type_traits>
#include <utility>

namespace editor {

namespace aa = aegis::assistant;

namespace {

constexpr std::size_t maximumDetailCharacters = 48;
constexpr double bytesPerKilobyte = 1024.0;

[[nodiscard]] std::string countOf(std::size_t count, std::string_view one, std::string_view many) {
    return std::to_string(count) + ' ' + std::string(count == 1 ? one : many);
}

[[nodiscard]] std::string shortened(std::string text) {
    if (text.size() <= maximumDetailCharacters) {
        return text;
    }
    text.resize(maximumDetailCharacters - 3);
    return text + "...";
}

[[nodiscard]] std::string detailOf(const aa::SelectionContext& item) {
    return countOf(item.entities.size(), "entity", "entities");
}

[[nodiscard]] std::string detailOf(const aa::LastRunContext& item) {
    if (!item.ticks) {
        return "last " + std::to_string(aa::defaultLastRunTicks) + " ticks";
    }
    return "ticks " + std::to_string(item.ticks->first) + '-' + std::to_string(item.ticks->last);
}

[[nodiscard]] std::string detailOf(const aa::GraphContext& item) {
    return item.focus.empty() ? "whole graph"
                              : "around " + countOf(item.focus.size(), "entity", "entities");
}

[[nodiscard]] std::string detailOf(const aa::GraphNodesContext& item) {
    return countOf(item.nodeIds.size(), "node", "nodes");
}

[[nodiscard]] std::string detailOf(const aa::SourceCodeContext& item) {
    return item.files.size() == 1 ? shortened(item.files.front())
                                  : countOf(item.files.size(), "file", "files");
}

[[nodiscard]] std::string detailOf(const aa::AssetContext& item) {
    return shortened(item.path);
}

[[nodiscard]] std::string detailOf(const aa::LogEntryContext& item) {
    return shortened(item.entry.message);
}

[[nodiscard]] std::string detailOf(const aa::ComponentTypeContext& item) {
    return shortened(item.typeName);
}

[[nodiscard]] std::string oneDecimal(double value, const char* unit) {
    char text[32] = {};
    std::snprintf(text, sizeof text, "%.1f %s", value, unit);
    return text;
}

} // namespace

std::string_view contextChipTypeLabel(ContextChipType type) {
    switch (type) {
    case ContextChipType::Selection:
        return "Selection";
    case ContextChipType::LastRun:
        return "Last run";
    case ContextChipType::Graph:
        return "Graph";
    case ContextChipType::GraphNodes:
        return "Graph nodes";
    case ContextChipType::SourceCode:
        return "Source code";
    case ContextChipType::Asset:
        return "Asset";
    case ContextChipType::LogEntry:
        return "Log entry";
    case ContextChipType::ComponentType:
        return "Component type";
    }
    return "Context";
}

ContextChipType contextChipTypeOf(const aa::ContextItem& item) {
    return static_cast<ContextChipType>(item.index());
}

std::string contextChipDetail(const aa::ContextItem& item) {
    return std::visit([](const auto& value) { return detailOf(value); }, item);
}

std::string formatRequestSize(std::optional<std::size_t> bytes) {
    if (!bytes) {
        return "\xE2\x80\x94"; // em dash
    }
    if (*bytes < 1024U) {
        return std::to_string(*bytes) + " B";
    }
    const double kilobytes = static_cast<double>(*bytes) / bytesPerKilobyte;
    if (kilobytes < bytesPerKilobyte) {
        return oneDecimal(kilobytes, "KB");
    }
    return oneDecimal(kilobytes / bytesPerKilobyte, "MB");
}

bool ContextChipSet::contains(ContextChipType type) const {
    return std::any_of(chips_.begin(), chips_.end(),
                       [type](const ContextChip& chip) { return chip.type == type; });
}

std::vector<ContextChipType> ContextChipSet::unattachedTypes() const {
    std::vector<ContextChipType> types;
    for (const ContextChipType type : allContextChipTypes) {
        if (!contains(type)) {
            types.push_back(type);
        }
    }
    return types;
}

ContextChipSet ContextChipSet::with(aa::ContextItem item,
                                    const IContextSizeEstimator& sizes) const {
    ContextChipSet next = *this;
    const ContextChipType type = contextChipTypeOf(item);
    ContextChip chip{next.nextId_++,
                     type,
                     std::string(contextChipTypeLabel(type)),
                     contextChipDetail(item),
                     sizes.requestBytes(item),
                     std::move(item)};
    const auto existing =
        std::find_if(next.chips_.begin(), next.chips_.end(),
                     [type](const ContextChip& attached) { return attached.type == type; });
    if (existing != next.chips_.end()) {
        *existing = std::move(chip);
    } else {
        next.chips_.push_back(std::move(chip));
    }
    return next;
}

ContextChipSet ContextChipSet::without(ContextChipId id) const {
    ContextChipSet next = *this;
    std::erase_if(next.chips_, [id](const ContextChip& chip) { return chip.id == id; });
    return next;
}

std::vector<aa::ContextItem> ContextChipSet::items() const {
    std::vector<aa::ContextItem> result;
    result.reserve(chips_.size());
    for (const ContextChip& chip : chips_) {
        result.push_back(chip.item);
    }
    return result;
}

std::optional<aa::ContextItem> selectionContextItem(const EditorSelection& selection) {
    if (const auto* entity = std::get_if<EntitySelection>(&selection)) {
        return aa::SelectionContext{{entity->entity}};
    }
    return std::nullopt;
}

std::optional<aa::ContextItem> toContextItem(const AssistantContext& context) {
    if (const auto* selection = std::get_if<SelectionContext>(&context)) {
        return selectionContextItem(selection->selection);
    }
    const auto& lastRun = std::get<LastRunContext>(context);
    return aa::LastRunContext{
        aa::TickRange{lastRun.eventAndSampleRange.first, lastRun.eventAndSampleRange.last}};
}

} // namespace editor
