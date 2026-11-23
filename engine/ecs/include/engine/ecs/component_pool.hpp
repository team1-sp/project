#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string_view>
#include <utility>
#include <vector>

#include "engine/core/entity_id.hpp"
#include "engine/ecs/entity.hpp"

namespace engine {

// The part of a component pool that does not depend on the component type, for code that treats
// every pool alike: removing a destroyed entity's components and sorting after structural changes.
class IComponentPool {
public:
    virtual ~IComponentPool() = default;

    [[nodiscard]] virtual std::string_view typeName() const = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual bool contains(Entity e) const = 0;
    // The EntityId of the entry at `pos`. Entries are in ascending EntityId whenever the pool is
    // not dirty (see sortIfDirty()).
    [[nodiscard]] virtual EntityId idAt(std::size_t pos) const = 0;
    // Removes the entity's component if it has one and says whether it had one.
    virtual bool removeIfPresent(Entity e) = 0;
    // Puts the entries back in ascending EntityId after adds and removes (D-3).
    virtual void sortIfDirty() = 0;
};

// The storage of one component type (design/ecs-api-design.md §6, D-1, D-3): a sparse set. The
// sparse array maps an entity's index to its position in three packed arrays of the same order:
// handles, EntityIds and components. add() appends and remove() moves the last entry into the
// gap, both in O(1); either can leave the packed arrays out of EntityId order, and sortIfDirty()
// sorts them again. Pools change only in direct World calls and command playback, never while a
// system or a query runs, and the World sorts every touched pool at the end of each, so systems
// always see components in ascending EntityId (coding standard rule 6).
template <typename T>
class ComponentPool final : public IComponentPool {
public:
    [[nodiscard]] std::string_view typeName() const override { return T::typeName; }
    [[nodiscard]] std::size_t size() const override { return ids_.size(); }
    [[nodiscard]] bool contains(Entity e) const override { return positionOf(e) != npos; }
    [[nodiscard]] EntityId idAt(std::size_t pos) const override { return ids_[pos]; }

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
        if (pos > 0 && ids_[pos - 1] > id) {
            dirty_ = true;
        }
        return components_.back();
    }

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
            sparse_[entities_[pos].index] = pos;
            dirty_ = true;
        }
        entities_.pop_back();
        ids_.pop_back();
        components_.pop_back();
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

    // Sorts the three packed arrays together by EntityId and rebuilds the sparse array. The
    // permutation is applied in place along its cycles, so a sort allocates nothing once the
    // order buffer has grown to the pool's size (skypjack part 5).
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
            std::size_t at = start;
            while (true) {
                const std::size_t from = order_[at];
                order_[at] = static_cast<std::uint32_t>(at);
                if (from == start) {
                    entities_[at] = entity;
                    ids_[at] = id;
                    components_[at] = std::move(component);
                    break;
                }
                entities_[at] = entities_[from];
                ids_[at] = ids_[from];
                components_[at] = std::move(components_[from]);
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
    std::vector<std::uint32_t> order_; // scratch for sortIfDirty()
    bool dirty_ = false;
};

} // namespace engine
