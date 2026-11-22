#include "editor/entity_navigation.hpp"

namespace editor {

void EntityNavigation::navigateTo(engine::EntityId entity) {
    selectedEntity_ = entity;
}

std::optional<engine::EntityId> EntityNavigation::selectedEntity() const {
    return selectedEntity_;
}

} // namespace editor
