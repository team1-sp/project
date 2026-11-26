#include "engine/ecs/system_access.hpp"

#include "engine/core/run_error.hpp"

namespace engine {

void SystemAccess::checkRead(const ComponentTypeInfo& type, std::string_view operation) const {
    if (reads[type.index] == 0) {
        throw RunError("system " + name + ": undeclared read of " + type.name + " in " +
                       std::string(operation));
    }
}

void SystemAccess::checkWrite(const ComponentTypeInfo& type, std::string_view operation) const {
    if (writes[type.index] == 0) {
        throw RunError("system " + name + ": undeclared write of " + type.name + " in " +
                       std::string(operation));
    }
}

void SystemAccess::checkWriteUntracked(const ComponentTypeInfo& type,
                                       std::string_view operation) const {
    if (writesUntracked[type.index] == 0) {
        throw RunError("system " + name + ": undeclared untracked write of " + type.name + " in " +
                       std::string(operation));
    }
}

void SystemAccess::checkFlag(bool declared, std::string_view flagName,
                             std::string_view operation) const {
    if (!declared) {
        throw RunError("system " + name + ": " + std::string(operation) + " needs " +
                       std::string(flagName) + " in describe()");
    }
}

} // namespace engine
