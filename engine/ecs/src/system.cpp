#include "engine/ecs/system.hpp"

namespace engine {

SystemDescription& SystemDescription::name(std::string_view text) {
    declaration_.name = std::string(text);
    return *this;
}

SystemDescription& SystemDescription::purpose(std::string_view text) {
    declaration_.purpose = std::string(text);
    return *this;
}

SystemDescription& SystemDescription::phase(TickPhase tickPhase) {
    declaration_.phase = tickPhase;
    return *this;
}

SystemDescription& SystemDescription::group(std::string_view groupName) {
    declaration_.group = std::string(groupName);
    return *this;
}

SystemDescription& SystemDescription::runsBefore(std::string_view systemOrGroup) {
    declaration_.runsBefore.emplace_back(systemOrGroup);
    return *this;
}

SystemDescription& SystemDescription::runsAfter(std::string_view systemOrGroup) {
    declaration_.runsAfter.emplace_back(systemOrGroup);
    return *this;
}

SystemDescription& SystemDescription::createsEntities() {
    declaration_.createsEntities = true;
    return *this;
}

SystemDescription& SystemDescription::destroysEntities() {
    declaration_.destroysEntities = true;
    return *this;
}

SystemDescription& SystemDescription::togglesEnabled() {
    declaration_.togglesEnabled = true;
    return *this;
}

SystemDescription& SystemDescription::usesRandom() {
    declaration_.usesRandom = true;
    return *this;
}

} // namespace engine
