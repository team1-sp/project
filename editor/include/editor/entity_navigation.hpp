#pragma once

#include <optional>

#include "editor/console_model.hpp"

namespace editor {

// Shared selection seam for Console links now and the Hierarchy/Scene/Inspector stories later.
// Navigating selects one entity; panels that display selection observe selectedEntity().
class EntityNavigation final : public IEntityNavigator {
public:
    void navigateTo(engine::EntityId entity) override;
    [[nodiscard]] std::optional<engine::EntityId> selectedEntity() const;

private:
    std::optional<engine::EntityId> selectedEntity_;
};

} // namespace editor
