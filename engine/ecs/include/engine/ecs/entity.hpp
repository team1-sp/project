#pragma once

#include <cstdint>

namespace engine {

// The runtime handle of an entity (design/ecs-api-design.md §4, D-2): an index into the World's
// entity table plus the generation of that slot. Destroying the entity increases the slot's
// generation, so every handle to it becomes stale and is detected. A handle is valid only in the
// World that created it and is never saved or written to telemetry; those use the EntityId.
struct Entity {
    static constexpr std::uint32_t invalidIndex = 0xFFFF'FFFF;
    std::uint32_t index = invalidIndex;
    std::uint32_t generation = 0;

    friend bool operator==(Entity, Entity) = default;
};

inline constexpr Entity nullEntity{};

} // namespace engine
