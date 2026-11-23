#pragma once

#include <cstdint>

namespace engine {

// The identity of an asset, such as a scene: the 64-bit FNV-1a hash of the asset's name
// (DEC-042 item 7). The asset registry keeps the name, so telemetry and the schema can show it.
// An enum class, so it cannot be mixed up with other integers or with EntityId.
enum class AssetId : std::uint64_t {};
inline constexpr AssetId noAsset{0};

} // namespace engine
