#pragma once

#include <stdexcept>
#include <string>

namespace engine {

// An unrecoverable run error (srs.fr.1.11, DEC-042): a failed precondition, an access check
// (srs.fr.2.126) or an invariant a system violated, such as an invalid entity handle or a missing
// component in its own world access. Engine code throws it; only the game loop catches it. The
// loop adds the tick number when it logs the error, so the message names only what went wrong
// and where: the system, the entity ID, the component type.
//
// Invalid data read from files, recordings or the agent is recoverable (srs.nfr.5.10) and is
// logged as an error without throwing this.
class RunError : public std::runtime_error {
public:
    explicit RunError(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace engine
