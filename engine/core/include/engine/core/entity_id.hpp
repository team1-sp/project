#pragma once

#include <cstdint>

namespace engine {

// The entity's identity (srs.fr.2.107), as designed in ecs-api-design.md section 4.
// 0 = no entity (empty reference); 1 ... 2^32 - 1 Editor range; 2^32 and above runtime range.
// An enum class, so it cannot be mixed up with other integers.
enum class EntityId : std::uint64_t {};
inline constexpr EntityId noEntity{0};
inline constexpr EntityId firstRuntimeEntityId{std::uint64_t{1} << 32};

} // namespace engine
