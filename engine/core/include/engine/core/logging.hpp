#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "engine/core/entity_id.hpp"
#include "engine/core/tick.hpp"

namespace engine {

enum class LogSeverity {
    Info,
    Warning,
    Error,
};

[[nodiscard]] const char* logSeverityName(LogSeverity severity);

struct SourceLocation {
    std::string file;
    std::uint32_t line = 0;

    friend bool operator==(const SourceLocation&, const SourceLocation&) = default;
};

// The canonical, owning representation of one engine or Editor log message (srs.fr.24.1,
// 24.9 to 24.11). A missing tick means the message was produced outside a simulation run.
struct LogEntry {
    LogSeverity severity = LogSeverity::Info;
    std::string message;
    std::optional<Tick> tick;
    std::optional<std::string> stackTrace;
    std::optional<SourceLocation> source;
    std::optional<EntityId> entity;

    friend bool operator==(const LogEntry&, const LogEntry&) = default;
};

// Receives ownership of structured log entries. Implementations document their threading rules;
// Application calls this only on the thread running init() or run(). The sink is not owned.
class ILogSink {
public:
    virtual ~ILogSink() = default;
    virtual void write(LogEntry entry) = 0;
};

} // namespace engine
