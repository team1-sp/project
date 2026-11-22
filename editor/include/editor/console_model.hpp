#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "editor/status_bar.hpp"
#include "engine/core/logging.hpp"

namespace editor {

struct ConsoleCounts {
    std::size_t info = 0;
    std::size_t warning = 0;
    std::size_t error = 0;

    friend bool operator==(const ConsoleCounts&, const ConsoleCounts&) = default;
};

struct ConsoleRow {
    // Index into ConsoleModel::entries(). The model owns the entry and preserves insertion order.
    std::size_t entryIndex = 0;
    std::size_t count = 1;

    friend bool operator==(const ConsoleRow&, const ConsoleRow&) = default;
};

class IExternalCodeEditor {
public:
    virtual ~IExternalCodeEditor() = default;
    virtual bool open(const engine::SourceLocation& source) = 0;
};

class IEntityNavigator {
public:
    virtual ~IEntityNavigator() = default;
    virtual void navigateTo(engine::EntityId entity) = 0;
};

struct ConsoleActivation {
    bool externalEditorRequested = false;
    bool entityNavigationRequested = false;
};

// Window-independent Console state and view projection (srs.fr.24.1 to 24.11). The model owns
// every LogEntry in insertion order. Filters, search and collapse create index rows and never
// mutate those entries. It is used only on the Editor/game-loop thread; it performs no locking.
class ConsoleModel final : public engine::ILogSink {
public:
    using PausePlay = std::function<void()>;

    // `status` must outlive the model. `pausePlay` is injected by the Play-mode host and calls
    // Application::requestPause(); it may be empty until that host exists.
    explicit ConsoleModel(EditorStatus& status, PausePlay pausePlay = {});

    void write(engine::LogEntry entry) override;
    void clear();

    [[nodiscard]] const std::vector<engine::LogEntry>& entries() const;
    [[nodiscard]] ConsoleCounts counts() const;
    [[nodiscard]] std::string tabLabel() const;

    void setSeverityVisible(engine::LogSeverity severity, bool visible);
    [[nodiscard]] bool isSeverityVisible(engine::LogSeverity severity) const;
    void setSearch(std::string search);
    [[nodiscard]] const std::string& search() const;
    void setCollapse(bool collapse);
    [[nodiscard]] bool collapse() const;
    [[nodiscard]] std::vector<ConsoleRow> visibleRows() const;

    void select(std::size_t entryIndex);
    [[nodiscard]] std::optional<std::size_t> selectedIndex() const;
    [[nodiscard]] const engine::LogEntry* selectedEntry() const;

    [[nodiscard]] ConsoleActivation activate(std::size_t entryIndex,
                                             IExternalCodeEditor& externalEditor,
                                             IEntityNavigator& entityNavigator) const;

    void setClearOnPlay(bool clearOnPlay);
    [[nodiscard]] bool clearOnPlay() const;
    void setErrorPause(bool errorPause);
    [[nodiscard]] bool errorPause() const;

    // The smallest Play lifecycle seam until US-17.2 (#85) owns the controller. The controller
    // calls these exactly when Play begins and ends. A new run resets the first-error gate.
    void onPlayStarted();
    void onPlayStopped();

private:
    [[nodiscard]] bool matchesSearch(const engine::LogEntry& entry) const;
    void increment(engine::LogSeverity severity);

    EditorStatus& status_;
    PausePlay pausePlay_;
    std::vector<engine::LogEntry> entries_;
    ConsoleCounts counts_;
    std::optional<std::size_t> selectedIndex_;
    std::string search_;
    bool showInfo_ = true;
    bool showWarning_ = true;
    bool showError_ = true;
    bool collapse_ = false;
    bool clearOnPlay_ = true;
    bool errorPause_ = false;
    bool playActive_ = false;
    bool errorPauseTriggered_ = false;
};

} // namespace editor
