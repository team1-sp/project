# Console and Structured Logging

Status: implemented for US-24.1 (srs.fr.24.1 to 24.11); design choices are carried for review as
DEC-067.

## Contract and ownership

`engine::LogEntry` in `engine/core/logging.hpp` is the only log-entry representation. It owns its
message and optional stack trace and source path, and carries:

- `LogSeverity` (`Info`, `Warning`, or `Error`);
- the complete message;
- an optional simulation tick (absent only outside a run);
- an optional stack trace;
- an optional source file and one-based line;
- an optional `EntityId`.

`ILogSink::write(LogEntry)` transfers an entry to its destination. `ApplicationConfig::logSink` is
an optional, non-owning pointer which must outlive `init()` and `run()`. `Application` calls it only
on its own thread. Without a sink, command-line and headless behavior is unchanged: messages go to
stderr. `Application` attaches its current tick to run warnings and `RunError`s.

The Editor owns one `ConsoleModel` for its lifetime. The model implements `ILogSink`, owns a
`std::vector<LogEntry>` in insertion order, and is used only on the Editor/game-loop thread. It has
no global instance and no locking. Each accepted entry updates severity counters and
`EditorStatus.lastConsoleMessage`; Clear empties both entries and counters, clears selection, and
removes the status message.

## View projection

Severity toggles, search, and Collapse operate on a projection returned as `ConsoleRow` values.
Each row contains an index into the owned entry vector and a display count. They never erase,
reorder, or change entries.

Search is a case-insensitive substring search over the complete message. An empty search matches
all messages. Collapse preserves the first occurrence's insertion position and groups entries only
when every `LogEntry` field is equal: severity, complete message, tick, stack trace, source file and
line, and entity. In particular, equal text from different ticks does not collapse because its
inspection context differs.

Selecting a row stores its entry index. The detail pane reads that canonical entry and shows its
complete message, optional tick, and optional stack trace. Double-click activation asks both
applicable injected services: `IExternalCodeEditor` for a source location and `IEntityNavigator`
for an entity.

## Play lifecycle and Error Pause

`ConsoleModel::onPlayStarted()` is the event seam owned by the future Play controller in US-17.2
(#85). It resets the first-error gate and clears entries when Clear on Play is enabled.
`onPlayStopped()` ends the run scope. This is the smallest integration point needed before that
controller exists; #85 calls it at the same transition that starts/stops its game-loop run.

While Error Pause is enabled, the first Error with a tick after `onPlayStarted()` calls the injected
pause callback once. The Editor wires that callback to `Application::requestPause()`. If the error
is logged inside a tick, the existing loop completes that tick, takes the request at its boundary,
drops the render frame's remaining scheduled ticks, and continues rendering in the paused state
(DEC-060). Clearing the Console does not reset the gate; only another Play start does.

## Platform and navigation seams

`ProcessExternalCodeEditor` launches without a shell, so paths are passed as one argument and no
test starts an application. The executable is `AEGIS_CODE_EDITOR` when set and otherwise `code`;
the request is `--goto <file>:<line>`. A later Preferences story can supply the same constructor
from project settings without changing the Console.

`EntityNavigation` is the minimal shared selection state until the Hierarchy, Scene, and Inspector
selection stories land. Navigating selects the referenced `EntityId`; the Console view also focuses
the Hierarchy tab. Scene-qualified and historical references belong to the later srs.fr.35.20+
stories and can replace the interface parameter without changing stored `LogEntry` ownership.

## Tests

`tests/unit/editor/console_model_test.cpp` covers entry ownership, optional ticks, counters and tab
label, Clear and Clear on Play, full-field collapse identity, independent filters, search, details,
status updates, first-error gating, and injected activation requests. `ApplicationTest` verifies a
structured headless `RunError` and that Error Pause stops a multi-tick render frame at the error
tick boundary. All tests build without raylib or a window.
