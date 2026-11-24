#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

#include "engine/core/entity_id.hpp"
#include "engine/core/vec2.hpp"
#include "engine/ecs/asset_id.hpp"
#include "engine/ecs/fixed_string.hpp"
#include "engine/ecs/tag_id.hpp"

namespace engine {

// The kind of a component field (design/ecs-api-design.md §11). Every kind is unmanaged: a field
// is plain bytes at a fixed offset, so per-tick code, snapshots and the agent bridge can read it
// without calling anything (D-6). The managed kinds of the design (String, List) and FixedArray
// come with the first component that needs them.
enum class FieldType : std::uint8_t {
    Bool,
    Int32,
    Int64,
    UInt8,
    UInt32,
    Float,
    Double,
    Vec2,
    Enum,        // a registered enum class with an unsigned underlying type
    EntityRef,   // EntityId: a reference to another entity (srs.fr.2.42)
    AssetRef,    // AssetId
    Tag,         // TagId: one value of the project's tag list
    FixedString, // FixedString<N>; `count` is N
};

// The unit of a field's value (components.md, DEC-036).
enum class Unit : std::uint8_t {
    None,
    Meters,
    MetersPerSecond,
    Radians,
    RadiansPerSecond,
    Kilograms,
    Seconds,
    Ticks,
};

// The names the schema and the Editor show: "bool", "int32", ... and "", "m", "m/s", ...
[[nodiscard]] const char* fieldTypeName(FieldType type);
[[nodiscard]] const char* unitName(Unit unit);

// A field's value as the Inspector, scene files and the schema handle it: Enum fields hold their
// underlying value as std::int64_t, UInt8 and UInt32 fields hold std::uint32_t, FixedString fields
// hold std::string. Per-tick code never builds these; it reads fields at their offset.
using FieldValue = std::variant<bool, std::int32_t, std::int64_t, std::uint32_t, float, double,
                                Vec2, EntityId, AssetId, TagId, std::string>;

// One field of a registered component type: its meaning (srs.fr.2.12) and its layout.
struct FieldInfo {
    std::string name;
    FieldType type = FieldType::Int32;
    Unit unit = Unit::None;
    std::string description;
    FieldValue defaultValue;  // read from T{} at registration, never typed twice
    std::string enumType;     // Enum fields: the registered enumeration's name
    std::uint32_t offset = 0; // byte offset inside the component
    std::uint32_t size = 0;   // sizeof the member
    std::uint32_t count = 1;  // N for FixedString<N>, otherwise 1
};

// Reads the field from a component of its type.
[[nodiscard]] FieldValue readField(const FieldInfo& field, const void* component);
// Writes `value` into the field of a component of its type. A value of the wrong kind, an
// out-of-range number or a text longer than the field's capacity is refused: nothing changes
// and the problem is returned.
[[nodiscard]] std::optional<std::string> writeField(const FieldInfo& field, void* component,
                                                    const FieldValue& value);

namespace detail {

template <typename M>
struct FixedStringCapacity {
    static constexpr std::size_t value = 0;
};

template <std::size_t N>
struct FixedStringCapacity<FixedString<N>> {
    static constexpr std::size_t value = N;
};

template <typename>
inline constexpr bool unsupportedFieldType = false;

} // namespace detail

// The FieldType of a member type; an unsupported type does not compile.
template <typename M>
[[nodiscard]] consteval FieldType fieldTypeOf() {
    if constexpr (std::is_same_v<M, bool>) {
        return FieldType::Bool;
    } else if constexpr (std::is_same_v<M, std::int32_t>) {
        return FieldType::Int32;
    } else if constexpr (std::is_same_v<M, std::int64_t>) {
        return FieldType::Int64;
    } else if constexpr (std::is_same_v<M, std::uint8_t>) {
        return FieldType::UInt8;
    } else if constexpr (std::is_same_v<M, std::uint32_t>) {
        return FieldType::UInt32;
    } else if constexpr (std::is_same_v<M, float>) {
        return FieldType::Float;
    } else if constexpr (std::is_same_v<M, double>) {
        return FieldType::Double;
    } else if constexpr (std::is_same_v<M, Vec2>) {
        return FieldType::Vec2;
    } else if constexpr (std::is_same_v<M, EntityId>) {
        return FieldType::EntityRef;
    } else if constexpr (std::is_same_v<M, AssetId>) {
        return FieldType::AssetRef;
    } else if constexpr (std::is_same_v<M, TagId>) {
        return FieldType::Tag;
    } else if constexpr (detail::FixedStringCapacity<M>::value > 0) {
        return FieldType::FixedString;
    } else if constexpr (std::is_enum_v<M>) {
        static_assert(std::is_unsigned_v<std::underlying_type_t<M>>,
                      "enum fields need an unsigned underlying type");
        return FieldType::Enum;
    } else {
        static_assert(detail::unsupportedFieldType<M>, "unsupported component field type");
        return FieldType::Bool;
    }
}

} // namespace engine
