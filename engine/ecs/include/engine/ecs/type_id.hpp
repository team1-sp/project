#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace engine {

// 64-bit FNV-1a hash of a name: the same value in every run, in the engine and in project-script
// DLLs, unlike a counter (design/ecs-api-design.md §11, D-7; skypjack part 8).
[[nodiscard]] constexpr std::uint64_t fnv1a64(std::string_view text) {
    std::uint64_t hash = 14695981039346656037ull;
    for (const char c : text) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ull;
    }
    return hash;
}

// The in-memory key of a registered type: the hash of its name. Files and exports always use the
// name; a hash collision between two names is refused like a duplicate name (srs.fr.2.39).
struct TypeId {
    std::uint64_t value = 0;

    friend bool operator==(TypeId, TypeId) = default;
    friend auto operator<=>(TypeId, TypeId) = default;
};

[[nodiscard]] constexpr TypeId typeIdFromName(std::string_view name) {
    return TypeId{fnv1a64(name)};
}

// A type the ECS can register: it names itself with `static constexpr std::string_view typeName`
// (coding standard §2: `PositionComponent` has the typeName "Position").
template <typename T>
concept EcsType = requires {
    { T::typeName } -> std::convertible_to<std::string_view>;
} && std::is_default_constructible_v<T> && std::is_move_constructible_v<T>;

template <typename T>
concept ComponentType = EcsType<T>;

template <EcsType T>
[[nodiscard]] constexpr TypeId typeIdOf() {
    return typeIdFromName(T::typeName);
}

} // namespace engine
