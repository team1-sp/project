#pragma once

#include <cstdint>
#include <string_view>

#include "engine/core/entity_id.hpp"
#include "engine/ecs/asset_id.hpp"
#include "engine/ecs/fixed_string.hpp"
#include "engine/ecs/tag_id.hpp"

namespace engine {

// The built-in components (design/ecs-api-design.md §5, D-12). Every entity gets all three when
// it is created and keeps them until it is destroyed; together with the enabled flag in the
// entity table they give each entity the name, enabled flag, tag, layer and optional parent of
// srs.fr.2.5. They are plain data like every component (R-8).

// The entity's name, shown in the Hierarchy and the Inspector and written to telemetry.
struct NameComponent {
    static constexpr std::string_view typeName = "Name";
    FixedString<64> name; // UTF-8; empty for a new entity
};

// The entity's tag and layer and the scene it belongs to.
struct EntityInfoComponent {
    static constexpr std::string_view typeName = "EntityInfo";
    TagId tag = untaggedTag; // one value of the project's tag list (srs.fr.2.53)
    std::uint8_t layer = 0;  // 0 to 31; 0 is the layer Default (srs.fr.2.54, 2.80)
    AssetId sourceScene{};   // the scene it was loaded from or created in (srs.fr.2.83)
};

// The entity's place in the hierarchy: its optional parent (srs.fr.2.5) and its children, in
// order, as a doubly linked list of siblings (skypjack, "ECS back and forth" part 4). The links
// are EntityIds, never handles (D-2). Only World::setParent() and CommandBuffer::setParent()
// change it; they keep both ends of every link consistent, which is why World::get() gives only
// read access to it.
struct HierarchyComponent {
    static constexpr std::string_view typeName = "Hierarchy";
    EntityId parent = noEntity;
    EntityId firstChild = noEntity;
    EntityId nextSibling = noEntity;
    EntityId prevSibling = noEntity;
    std::uint32_t childCount = 0;
};

} // namespace engine
