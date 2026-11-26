#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "engine/ecs/type_registry.hpp"

namespace engine {

// What one system may touch, as its declaration states it (srs.fr.2.16), in the form the access
// check reads on every call (design/ecs-api-design.md §10): one flag per registered component
// type, by the registry's dense type index. Every failed check throws engine::RunError naming the
// system, the type and the operation; the game loop adds the tick (srs.fr.2.126, 1.11).
struct SystemAccess {
    std::string name;
    std::vector<std::uint8_t> reads;           // a declared read or write
    std::vector<std::uint8_t> writes;          // a declared write
    std::vector<std::uint8_t> writesUntracked; // a declared untracked write (D-27)
    bool createsEntities = false;
    bool destroysEntities = false;
    bool togglesEnabled = false;
    bool usesRandom = false;

    void checkRead(const ComponentTypeInfo& type, std::string_view operation) const;
    void checkWrite(const ComponentTypeInfo& type, std::string_view operation) const;
    void checkWriteUntracked(const ComponentTypeInfo& type, std::string_view operation) const;
    // `declared` is the flag the operation needs; `flagName` its describe() call.
    void checkFlag(bool declared, std::string_view flagName, std::string_view operation) const;
};

} // namespace engine
