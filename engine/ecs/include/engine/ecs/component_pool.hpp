#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/ecs/entity.hpp"

namespace engine {

// The part of a component pool that does not depend on the component type, for code that treats
// every pool alike: removing a destroyed entity's components, sorting after structural changes,
// the command buffer, scene files and export (design/ecs-api-design.md §6).
class IComponentPool {
public:
    virtual ~IComponentPool() = default;

    [[nodiscard]] virtual std::string_view typeName() const = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual bool contains(Entity e) const = 0;
    // The EntityId of the entry at `pos`. Entries are in ascending EntityId whenever the pool is
    // not dirty (see sortIfDirty()).
    [[nodiscard]] virtual EntityId idAt(std::size_t pos) const = 0;
    // Whether the component at `pos` is enabled; always true for a type without enabled flags.
    [[nodiscard]] virtual bool enabledAt(std::size_t pos) const = 0;
    // The component of entity `e` as raw memory, or nullptr when it has none.
    [[nodiscard]] virtual const void* rawOf(Entity e) const = 0;
    [[nodiscard]] virtual void* rawOf(Entity e) = 0;
    // Adds a component of the live entity `e`, whose EntityId is `id`, copied from `bytes`, the
    // raw memory of a value of the pool's type. The caller checks that `e` has none yet.
    virtual void* addRaw(Entity e, EntityId id, const void* bytes) = 0;
    // Adds a default-constructed component, as T{} gives it.
    virtual void* addDefault(Entity e, EntityId id) = 0;
    // Removes the entity's component if it has one and says whether it had one.
    virtual bool removeIfPresent(Entity e) = 0;
    // Puts the entries back in ascending EntityId after adds and removes (D-3).
    virtual void sortIfDirty() = 0;
};

// The storage of one component type (design/ecs-api-design.md §6, D-1, D-3, D-13): a sparse set.
// The sparse array maps an entity's index to its position in the packed arrays, which all have
// the same order: handles, EntityIds, components and, for a type that has them, enabled flags.
// add() appends and remove() moves the last entry into the gap, both in O(1); either can leave the
// packed arrays out of EntityId order, and sortIfDirty() sorts them again. Pools change only in
// direct World calls and command playback, never while a system or a query runs, and the World
// sorts every touched pool at the end of each, so systems always see components in ascending
// EntityId (coding standard rule 6).
template <typename T>
class ComponentPool final : public IComponentPool {
public:
    // Types with enabled flags keep one per component (srs.fr.2.11); a new component is enabled.
    explicit ComponentPool(bool hasEnabledFlags = false)
        : hasEnabledFlags_(hasEnabledFlags) {}

    [[nodiscard]] std::string_view typeName() const override { return T::typeName; }
    [[nodiscard]] std::size_t size() const override { return ids_.size(); }
    [[nodiscard]] bool contains(Entity e) const override { return positionOf(e) != npos; }
    [[nodiscard]] EntityId idAt(std::size_t pos) const override { return ids_[pos]; }
    [[nodiscard]] bool hasEnabledFlags() const { return hasEnabledFlags_; }

    [[nodiscard]] bool enabledAt(std::size_t pos) const override {
        return !hasEnabledFlags_ || enabled_[pos] != 0;
    }

    // Whether `e` has the component and it is enabled.
    [[nodiscard]] bool containsEnabled(Entity e) const {
        const std::uint32_t pos = positionOf(e);
        return pos != npos && enabledAt(pos);
    }

    // `e` must have the component and the type must have enabled flags.
    void setEnabled(Entity e, bool enabled) {
        enabled_[positionOf(e)] = static_cast<std::uint8_t>(enabled ? 1 : 0);
    }

    // Adds the component of the live entity `e`, whose EntityId is `id`. The caller checks that
    // `e` has no component of this type yet (srs.fr.2.10).
    T& add(Entity e, EntityId id, T value) {
        if (e.index >= sparse_.size()) {
            sparse_.resize(static_cast<std::size_t>(e.index) + 1, npos);
        }
        const std::size_t pos = ids_.size();
        sparse_[e.index] = static_cast<std::uint32_t>(pos);
        entities_.push_back(e);
        ids_.push_back(id);
        components_.push_back(std::move(value));
        if (hasEnabledFlags_) {
            enabled_.push_back(1);
        }
        if (pos > 0 && ids_[pos - 1] > id) {
            dirty_ = true;
        }
        return components_.back();
    }

    void* addRaw(Entity e, EntityId id, const void* bytes) override {
        static_assert(std::is_trivially_copyable_v<T>, "only unmanaged components are copied raw");
        T value{};
        std::memcpy(static_cast<void*>(&value), bytes, sizeof(T));
        return &add(e, id, value);
    }

    void* addDefault(Entity e, EntityId id) override { return &add(e, id, T{}); }

    bool removeIfPresent(Entity e) override {
        const std::uint32_t pos = positionOf(e);
        if (pos == npos) {
            return false;
        }
        const std::size_t last = ids_.size() - 1;
        if (pos != last) {
            entities_[pos] = entities_[last];
            ids_[pos] = ids_[last];
            components_[pos] = std::move(components_[last]);
            if (hasEnabledFlags_) {
                enabled_[pos] = enabled_[last];
            }
            sparse_[entities_[pos].index] = pos;
            dirty_ = true;
        }
        entities_.pop_back();
        ids_.pop_back();
        components_.pop_back();
        if (hasEnabledFlags_) {
            enabled_.pop_back();
        }
        sparse_[e.index] = npos;
        return true;
    }

    [[nodiscard]] T* tryGet(Entity e) {
        const std::uint32_t pos = positionOf(e);
        return pos == npos ? nullptr : &components_[pos];
    }

    [[nodiscard]] const T* tryGet(Entity e) const {
        const std::uint32_t pos = positionOf(e);
        return pos == npos ? nullptr : &components_[pos];
    }

    [[nodiscard]] const void* rawOf(Entity e) const override { return tryGet(e); }
    [[nodiscard]] void* rawOf(Entity e) override { return tryGet(e); }

    // The packed arrays, in ascending EntityId when the pool is not dirty. For queries and export.
    [[nodiscard]] const Entity& entityAt(std::size_t pos) const { return entities_[pos]; }
    [[nodiscard]] T& componentAt(std::size_t pos) { return components_[pos]; }
    [[nodiscard]] const T& componentAt(std::size_t pos) const { return components_[pos]; }

    // Sorts the packed arrays together by EntityId and rebuilds the sparse array. The permutation
    // is applied in place along its cycles, so a sort allocates nothing once the order buffer has
    // grown to the pool's size (skypjack part 5).
    void sortIfDirty() override {
        if (!dirty_) {
            return;
        }
        dirty_ = false;
        const std::size_t n = ids_.size();
        order_.resize(n);
        std::iota(order_.begin(), order_.end(), std::uint32_t{0});
        std::sort(order_.begin(), order_.end(),
                  [this](std::uint32_t a, std::uint32_t b) { return ids_[a] < ids_[b]; });
        // order_[i] is the old position of the entry that belongs at position i. Placing an
        // entry marks its position done by setting order_[i] = i.
        for (std::size_t start = 0; start < n; ++start) {
            if (order_[start] == start) {
                continue;
            }
            const Entity entity = entities_[start];
            const EntityId id = ids_[start];
            T component = std::move(components_[start]);
            const std::uint8_t enabled = hasEnabledFlags_ ? enabled_[start] : std::uint8_t{1};
            std::size_t at = start;
            while (true) {
                const std::size_t from = order_[at];
                order_[at] = static_cast<std::uint32_t>(at);
                if (from == start) {
                    entities_[at] = entity;
                    ids_[at] = id;
                    components_[at] = std::move(component);
                    if (hasEnabledFlags_) {
                        enabled_[at] = enabled;
                    }
                    break;
                }
                entities_[at] = entities_[from];
                ids_[at] = ids_[from];
                components_[at] = std::move(components_[from]);
                if (hasEnabledFlags_) {
                    enabled_[at] = enabled_[from];
                }
                at = from;
            }
        }
        for (std::size_t pos = 0; pos < n; ++pos) {
            sparse_[entities_[pos].index] = static_cast<std::uint32_t>(pos);
        }
    }

private:
    static constexpr std::uint32_t npos = 0xFFFF'FFFF;

    [[nodiscard]] std::uint32_t positionOf(Entity e) const {
        if (e.index >= sparse_.size()) {
            return npos;
        }
        const std::uint32_t pos = sparse_[e.index];
        if (pos == npos || entities_[pos] != e) {
            return npos;
        }
        return pos;
    }

    std::vector<std::uint32_t> sparse_; // entity index -> position, npos when absent
    std::vector<Entity> entities_;      // packed arrays, all in the same order
    std::vector<EntityId> ids_;
    std::vector<T> components_;
    std::vector<std::uint8_t> enabled_; // empty for a type without enabled flags
    std::vector<std::uint32_t> order_;  // scratch for sortIfDirty()
    bool hasEnabledFlags_ = false;
    bool dirty_ = false;
};

} // namespace engine
