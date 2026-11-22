#include "editor/entity_navigation.hpp"

#include <utility>

namespace editor {

void EntityNavigation::navigateTo(engine::EntityId entity) {
    selection_ = EntitySelection{entity};
}

void EntityNavigation::selectEvent(EventSelection event) {
    selection_ = std::move(event);
}

void EntityNavigation::selectFinding(FindingSelection finding) {
    selection_ = std::move(finding);
}

void EntityNavigation::clear() {
    selection_ = std::monostate{};
}

std::optional<engine::EntityId> EntityNavigation::selectedEntity() const {
    const auto* entity = std::get_if<EntitySelection>(&selection_);
    return entity != nullptr ? std::optional<engine::EntityId>{entity->entity} : std::nullopt;
}

EditorSelection EntityNavigation::currentSelection() const {
    return selection_;
}

} // namespace editor
