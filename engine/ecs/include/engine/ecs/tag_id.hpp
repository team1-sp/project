#pragma once

#include <cstdint>

namespace engine {

// One value of the project's tag list (srs.fr.2.53): its index in that list (DEC-042 item 7).
// The Phase 1 list is Untagged, Obstacle (DEC-042); Untagged is the default tag (srs.fr.2.80).
enum class TagId : std::uint32_t {};
inline constexpr TagId untaggedTag{0};

} // namespace engine
