#pragma once

#include <optional>

#include "editor/console_model.hpp"
#include "editor/selection.hpp"

namespace editor {

// Shared selection seam for Console links now and the Hierarchy/Scene/Inspector stories later.
// Navigating selects one entity; panels that display selection observe selectedEntity().
class EntityNavigation final : public IEntityNavigator, public ISelectionSource {
public:
    void navigateTo(engine::EntityId entity) override;
    void selectEvent(EventSelection event);
    void selectFinding(FindingSelection finding);
    void clear();
    [[nodiscard]] std::optional<engine::EntityId> selectedEntity() const;
    [[nodiscard]] EditorSelection currentSelection() const override;

private:
    EditorSelection selection_;
};

} // namespace editor
