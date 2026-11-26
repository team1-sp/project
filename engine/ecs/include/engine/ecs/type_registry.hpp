#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

#include "engine/core/run_error.hpp"
#include "engine/ecs/component_pool.hpp"
#include "engine/ecs/field.hpp"
#include "engine/ecs/type_id.hpp"

namespace engine {

enum class ComponentKind : std::uint8_t {
    Unmanaged, // trivially copyable: pools can be copied as raw memory (D-14)
    Managed,   // owns heap memory; comes with the first managed component (§17)
};

// What the registry knows about one component type: its semantic description (srs.fr.2.12) and
// its layout (design/ecs-api-design.md §11). Which systems read and write it is derived from the
// system declarations when the schema is built (D-8), never typed here.
struct ComponentTypeInfo {
    TypeId id;
    std::string name; // T::typeName
    std::string description;
    ComponentKind kind = ComponentKind::Unmanaged;
    bool isTag = false;         // an empty type
    bool alwaysEnabled = false; // no enabled flag of its own (srs.fr.2.11; DEC-47.1)
    bool builtIn = false;       // every entity has it from creation to destruction (D-12)
    bool tracksWrites = false;  // keeps pending-write marks (D-27): Position, Velocity
    std::uint32_t size = 0;     // sizeof the component
    std::uint32_t index = 0;    // dense type index in registration order; never saved
    std::vector<FieldInfo> fields;
};

struct EnumValueInfo {
    std::string name;
    std::int64_t value = 0;
    std::string description;
};

struct EnumInfo {
    std::string name;
    std::string description;
    std::vector<EnumValueInfo> values;
};

class TypeRegistry;

namespace detail {

// Converts to any type. Never defined: it only counts the members of an aggregate.
struct AnyField {
    template <typename U>
    operator U() const; // NOLINT(google-explicit-constructor)
};

// The number of non-static data members of the aggregate T: the largest number of values that
// brace-initialise it (the technique of Boost.PFR, without the dependency). A static member such
// as typeName does not count.
template <typename T, typename... A>
consteval std::size_t memberCount() {
    if constexpr (sizeof...(A) < 64 &&
                  requires { T{std::declval<A>()..., std::declval<AnyField>()}; }) {
        return memberCount<T, A..., AnyField>();
    } else {
        return sizeof...(A);
    }
}

using PoolFactory = std::unique_ptr<IComponentPool> (*)(bool hasEnabledFlags, bool tracksWrites);

template <typename T>
std::unique_ptr<IComponentPool> makePool(bool hasEnabledFlags, bool tracksWrites) {
    return std::make_unique<ComponentPool<T>>(hasEnabledFlags, tracksWrites);
}

} // namespace detail

// Registers one component type's description and fields (design/ecs-api-design.md §11). Members
// are named through member pointers; there are no macros (D-6).
template <ComponentType T>
class ComponentBuilder {
public:
    // What the component means (srs.fr.2.12). Required (srs.fr.2.38).
    ComponentBuilder& describe(std::string_view text);
    // The component has no enabled flag of its own (srs.fr.2.11: Position and Velocity).
    ComponentBuilder& alwaysEnabled();
    // A system's non-const access marks a pending write that PhysicsSyncIn applies and clears
    // (D-27, DEC-040). For Position and Velocity.
    ComponentBuilder& tracksWrites();
    // Registers a data member as a field, with its unit and description. Every non-static data
    // member must be registered; TypeRegistry::finalize() refuses the type otherwise.
    template <typename M>
    ComponentBuilder& field(std::string_view name, M T::* member, Unit unit,
                            std::string_view description);

private:
    friend class TypeRegistry;

    ComponentBuilder(TypeRegistry& registry, std::size_t index)
        : registry_(registry)
        , index_(index) {}

    ComponentTypeInfo& info();

    TypeRegistry& registry_;
    std::size_t index_;
};

// Registers the values of an enum class used in component fields.
template <typename E>
class EnumBuilder {
public:
    EnumBuilder& value(E enumValue, std::string_view name, std::string_view description);

private:
    friend class TypeRegistry;

    EnumBuilder(TypeRegistry& registry, std::size_t index)
        : registry_(registry)
        , index_(index) {}

    TypeRegistry& registry_;
    std::size_t index_;
};

// Every component and enum type a World can hold, with its semantic description (srs.fr.2.12,
// 2.36 to 2.39; design/ecs-api-design.md §11). Types are registered, then finalize() checks them
// and freezes the registry. Any number of Worlds can share one finalized registry (D-23); nothing
// in it is global.
class TypeRegistry {
public:
    // Starts with the built-in components (D-12): Name, EntityInfo and Hierarchy.
    TypeRegistry();

    template <ComponentType T>
    ComponentBuilder<T> component();

    template <typename E>
    EnumBuilder<E> enumeration(std::string_view name, std::string_view description);

    // Checks every registered type and freezes the registry. Refused, with one RunError that
    // lists every problem, when a type or field has no description (srs.fr.2.38), two types share
    // a name or a name hash (srs.fr.2.39), two fields of a type share a name, or a component has
    // a data member that was not registered as a field.
    void finalize();
    [[nodiscard]] bool isFinalized() const { return finalized_; }

    [[nodiscard]] std::span<const ComponentTypeInfo> components() const { return components_; }
    [[nodiscard]] std::span<const EnumInfo> enumerations() const { return enums_; }
    [[nodiscard]] const ComponentTypeInfo* find(TypeId id) const;
    [[nodiscard]] const ComponentTypeInfo* find(std::string_view name) const;
    // Also checks that the type registered under T's name is T itself.
    template <ComponentType T>
    [[nodiscard]] const ComponentTypeInfo* find() const;
    [[nodiscard]] const EnumInfo* findEnum(std::string_view name) const;

    // A new, empty pool for a registered type; World creates one per type.
    [[nodiscard]] std::unique_ptr<IComponentPool> makePool(const ComponentTypeInfo& info) const;

private:
    template <ComponentType>
    friend class ComponentBuilder;
    template <typename>
    friend class EnumBuilder;

    struct CppType {
        std::type_index type;
        detail::PoolFactory makePool;
        std::size_t memberCount;
    };

    void checkOpen(std::string_view operation) const;
    [[nodiscard]] const std::string* enumNameOf(const std::type_info& type) const;

    std::vector<ComponentTypeInfo> components_;
    std::vector<CppType> cppTypes_; // parallel to components_
    std::vector<EnumInfo> enums_;
    std::vector<std::type_index> enumTypes_; // parallel to enums_
    std::vector<std::string> problems_;      // found while registering, reported by finalize()
    std::vector<std::pair<TypeId, std::uint32_t>> byId_; // sorted by finalize()
    bool finalized_ = false;
};

// --- templates ---

template <ComponentType T>
ComponentBuilder<T> TypeRegistry::component() {
    static_assert(std::is_aggregate_v<T>, "components are aggregates (coding standard §4)");
    static_assert(std::is_trivially_copyable_v<T>,
                  "managed components (std::string, std::vector) come later (design §17)");
    checkOpen("TypeRegistry::component");
    ComponentTypeInfo info;
    info.id = typeIdOf<T>();
    info.name = std::string(T::typeName);
    info.isTag = std::is_empty_v<T>;
    info.size = static_cast<std::uint32_t>(sizeof(T));
    info.index = static_cast<std::uint32_t>(components_.size());
    components_.push_back(std::move(info));
    cppTypes_.push_back(
        CppType{std::type_index(typeid(T)), &detail::makePool<T>, detail::memberCount<T>()});
    return ComponentBuilder<T>(*this, components_.size() - 1);
}

template <ComponentType T>
const ComponentTypeInfo* TypeRegistry::find() const {
    const ComponentTypeInfo* info = find(typeIdOf<T>());
    if (info != nullptr && cppTypes_[info->index].type != std::type_index(typeid(T))) {
        throw RunError("TypeRegistry: the type registered as " + info->name +
                       " is a different C++ type");
    }
    return info;
}

template <typename E>
EnumBuilder<E> TypeRegistry::enumeration(std::string_view name, std::string_view description) {
    static_assert(std::is_enum_v<E> && std::is_unsigned_v<std::underlying_type_t<E>>,
                  "enum fields need an enum class with an unsigned underlying type");
    checkOpen("TypeRegistry::enumeration");
    enums_.push_back(EnumInfo{std::string(name), std::string(description), {}});
    enumTypes_.emplace_back(typeid(E));
    return EnumBuilder<E>(*this, enums_.size() - 1);
}

template <ComponentType T>
ComponentTypeInfo& ComponentBuilder<T>::info() {
    return registry_.components_[index_];
}

template <ComponentType T>
ComponentBuilder<T>& ComponentBuilder<T>::describe(std::string_view text) {
    registry_.checkOpen("ComponentBuilder::describe");
    info().description = std::string(text);
    return *this;
}

template <ComponentType T>
ComponentBuilder<T>& ComponentBuilder<T>::tracksWrites() {
    registry_.checkOpen("ComponentBuilder::tracksWrites");
    info().tracksWrites = true;
    return *this;
}

template <ComponentType T>
ComponentBuilder<T>& ComponentBuilder<T>::alwaysEnabled() {
    registry_.checkOpen("ComponentBuilder::alwaysEnabled");
    info().alwaysEnabled = true;
    return *this;
}

template <ComponentType T>
template <typename M>
ComponentBuilder<T>& ComponentBuilder<T>::field(std::string_view name, M T::* member, Unit unit,
                                                std::string_view description) {
    registry_.checkOpen("ComponentBuilder::field");
    constexpr FieldType type = fieldTypeOf<M>();
    FieldInfo entry;
    entry.name = std::string(name);
    entry.type = type;
    entry.unit = unit;
    entry.description = std::string(description);
    entry.size = static_cast<std::uint32_t>(sizeof(M));
    if constexpr (type == FieldType::FixedString) {
        static_assert(std::is_standard_layout_v<M> && sizeof(M) == M::capacity + 1,
                      "FixedString<N> must be N chars followed by its length byte");
        entry.count = static_cast<std::uint32_t>(M::capacity);
    }
    if constexpr (type == FieldType::Enum) {
        const std::string* enumName = registry_.enumNameOf(typeid(M));
        if (enumName == nullptr) {
            registry_.problems_.push_back("field " + info().name + "." + entry.name +
                                          " uses an enum that is not registered");
        } else {
            entry.enumType = *enumName;
        }
    }
    // The offset from a default-constructed instance, which also gives the default value.
    const T probe{};
    entry.offset = static_cast<std::uint32_t>(reinterpret_cast<const std::byte*>(&(probe.*member)) -
                                              reinterpret_cast<const std::byte*>(&probe));
    entry.defaultValue = readField(entry, &probe);
    info().fields.push_back(std::move(entry));
    return *this;
}

template <typename E>
EnumBuilder<E>& EnumBuilder<E>::value(E enumValue, std::string_view name,
                                      std::string_view description) {
    registry_.checkOpen("EnumBuilder::value");
    registry_.enums_[index_].values.push_back(EnumValueInfo{
        std::string(name), static_cast<std::int64_t>(enumValue), std::string(description)});
    return *this;
}

} // namespace engine
