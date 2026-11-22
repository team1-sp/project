#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "aegis/assistant/grounded/grounded_question.hpp"
#include "editor/assistant_entry_points.hpp"
#include "editor/selection.hpp"

// Context chips of the AI Assistant panel (US-33.2, srs.fr.33.9 and srs.fr.33.10). A chip wraps
// one of US-11.1's typed context items, so the chip list is exactly what the next question
// carries. No Dear ImGui here; the panel only draws this state.
namespace editor {

// One value per alternative of aegis::assistant::ContextItem, in the same order.
enum class ContextChipType : std::uint8_t {
    Selection,
    LastRun,
    Graph,
    GraphNodes,
    SourceCode,
    Asset,
    LogEntry,
    ComponentType,
};

inline constexpr std::size_t contextChipTypeCount = 8;
static_assert(std::variant_size_v<aegis::assistant::ContextItem> == contextChipTypeCount,
              "every US-11.1 context item needs a chip type");

inline constexpr std::array<ContextChipType, contextChipTypeCount> allContextChipTypes = {
    ContextChipType::Selection,  ContextChipType::LastRun,       ContextChipType::Graph,
    ContextChipType::GraphNodes, ContextChipType::SourceCode,    ContextChipType::Asset,
    ContextChipType::LogEntry,   ContextChipType::ComponentType,
};

// "Selection", "Last run", "Graph", "Graph nodes", "Source code", "Asset", "Log entry" or
// "Component type".
[[nodiscard]] std::string_view contextChipTypeLabel(ContextChipType type);
[[nodiscard]] ContextChipType contextChipTypeOf(const aegis::assistant::ContextItem& item);
// What the item names, e.g. "2 entities", "ticks 0-599" or "Scripts/door.lua".
[[nodiscard]] std::string contextChipDetail(const aegis::assistant::ContextItem& item);
// "—" when unknown, otherwise "512 B", "1.5 KB" or "2.0 MB".
[[nodiscard]] std::string formatRequestSize(std::optional<std::size_t> bytes);

using ContextChipId = std::uint64_t;

struct ContextChip {
    // Stable for the chip's lifetime and never reused within a chip set.
    ContextChipId id = 0;
    ContextChipType type = ContextChipType::Selection;
    std::string label;
    std::string detail;
    // Bytes this item adds to the request; empty when it could not be measured.
    std::optional<std::size_t> requestBytes;
    aegis::assistant::ContextItem item;
};

// Measures how many bytes an item adds to the request without contacting the AI service.
class IContextSizeEstimator {
public:
    virtual ~IContextSizeEstimator() = default;
    [[nodiscard]] virtual std::optional<std::size_t>
    requestBytes(const aegis::assistant::ContextItem& item) const = 0;
};

// Builds the item a chip type attaches from the Editor's current state (selection, latest run,
// Console entry...). Returns nothing when the Editor has nothing of that type to attach.
class IContextItemProvider {
public:
    virtual ~IContextItemProvider() = default;
    [[nodiscard]] virtual std::optional<aegis::assistant::ContextItem>
    itemFor(ContextChipType type) const = 0;
};

// An immutable list of at most one chip per type. Every change returns a new set.
class ContextChipSet {
public:
    [[nodiscard]] const std::vector<ContextChip>& chips() const noexcept { return chips_; }
    [[nodiscard]] bool empty() const noexcept { return chips_.empty(); }
    [[nodiscard]] bool contains(ContextChipType type) const;
    // srs.fr.33.10: the types the + control lists.
    [[nodiscard]] std::vector<ContextChipType> unattachedTypes() const;
    // Attaches `item`, replacing a chip of the same type. The new chip gets a fresh id.
    [[nodiscard]] ContextChipSet with(aegis::assistant::ContextItem item,
                                      const IContextSizeEstimator& sizes) const;
    // srs.fr.33.9: removes the chip from the next question. Unknown ids leave the set unchanged.
    [[nodiscard]] ContextChipSet without(ContextChipId id) const;
    // The items the next question carries, in chip order.
    [[nodiscard]] std::vector<aegis::assistant::ContextItem> items() const;

private:
    std::vector<ContextChip> chips_;
    ContextChipId nextId_ = 1;
};

// Converts a context attached by a US-33.1 entry point into US-11.1's item. Event and finding
// selections have no US-11.1 item yet, so they convert to nothing.
[[nodiscard]] std::optional<aegis::assistant::ContextItem>
toContextItem(const AssistantContext& context);
[[nodiscard]] std::optional<aegis::assistant::ContextItem>
selectionContextItem(const EditorSelection& selection);

} // namespace editor
