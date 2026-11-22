#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "engine/core/entity_id.hpp"
#include "engine/core/tick.hpp"

namespace editor {

struct EntitySelection {
    engine::EntityId entity;

    bool operator==(const EntitySelection&) const = default;
};

struct EventSelection {
    std::uint64_t runNumber = 0;
    engine::Tick tick = 0;
    std::uint64_t eventIndex = 0;

    bool operator==(const EventSelection&) const = default;
};

struct FindingSelection {
    std::string findingId;

    bool operator==(const FindingSelection&) const = default;
};

using EditorSelection =
    std::variant<std::monostate, EntitySelection, EventSelection, FindingSelection>;

class ISelectionSource {
public:
    virtual ~ISelectionSource() = default;
    [[nodiscard]] virtual EditorSelection currentSelection() const = 0;
};

} // namespace editor
