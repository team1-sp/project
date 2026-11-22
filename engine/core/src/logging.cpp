#include "engine/core/logging.hpp"

namespace engine {

const char* logSeverityName(LogSeverity severity) {
    switch (severity) {
    case LogSeverity::Info:
        return "Info";
    case LogSeverity::Warning:
        return "Warning";
    case LogSeverity::Error:
        return "Error";
    }
    return "Unknown";
}

} // namespace engine
