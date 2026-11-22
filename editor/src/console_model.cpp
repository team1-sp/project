#include "editor/console_model.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>
#include <utility>

namespace editor {

namespace {

bool containsIgnoringCase(std::string_view text, std::string_view search) {
    if (search.empty()) {
        return true;
    }
    const auto equalIgnoringCase = [](char left, char right) {
        return std::tolower(static_cast<unsigned char>(left)) ==
               std::tolower(static_cast<unsigned char>(right));
    };
    return std::search(text.begin(), text.end(), search.begin(), search.end(), equalIgnoringCase) !=
           text.end();
}

} // namespace

ConsoleModel::ConsoleModel(EditorStatus& status, PausePlay pausePlay)
    : status_(status)
    , pausePlay_(std::move(pausePlay)) {
}

void ConsoleModel::write(engine::LogEntry entry) {
    increment(entry.severity);
    status_.lastConsoleMessage = entry.message;
    const bool triggersPause = playActive_ && errorPause_ && !errorPauseTriggered_ &&
                               entry.severity == engine::LogSeverity::Error && entry.tick;
    entries_.push_back(std::move(entry));
    if (triggersPause) {
        errorPauseTriggered_ = true;
        if (pausePlay_) {
            pausePlay_();
        }
    }
}

void ConsoleModel::clear() {
    entries_.clear();
    counts_ = {};
    selectedIndex_.reset();
    status_.lastConsoleMessage.clear();
}

const std::vector<engine::LogEntry>& ConsoleModel::entries() const {
    return entries_;
}

ConsoleCounts ConsoleModel::counts() const {
    return counts_;
}

std::string ConsoleModel::tabLabel() const {
    return "Console  I " + std::to_string(counts_.info) + "  W " + std::to_string(counts_.warning) +
           "  E " + std::to_string(counts_.error) + "###Console";
}

void ConsoleModel::setSeverityVisible(engine::LogSeverity severity, bool visible) {
    switch (severity) {
    case engine::LogSeverity::Info:
        showInfo_ = visible;
        break;
    case engine::LogSeverity::Warning:
        showWarning_ = visible;
        break;
    case engine::LogSeverity::Error:
        showError_ = visible;
        break;
    }
}

bool ConsoleModel::isSeverityVisible(engine::LogSeverity severity) const {
    switch (severity) {
    case engine::LogSeverity::Info:
        return showInfo_;
    case engine::LogSeverity::Warning:
        return showWarning_;
    case engine::LogSeverity::Error:
        return showError_;
    }
    return false;
}

void ConsoleModel::setSearch(std::string search) {
    search_ = std::move(search);
}

const std::string& ConsoleModel::search() const {
    return search_;
}

void ConsoleModel::setCollapse(bool collapse) {
    collapse_ = collapse;
}

bool ConsoleModel::collapse() const {
    return collapse_;
}

std::vector<ConsoleRow> ConsoleModel::visibleRows() const {
    std::vector<ConsoleRow> rows;
    rows.reserve(entries_.size());
    for (std::size_t index = 0; index < entries_.size(); ++index) {
        const engine::LogEntry& entry = entries_[index];
        if (!isSeverityVisible(entry.severity) || !matchesSearch(entry)) {
            continue;
        }
        if (collapse_) {
            const auto existing =
                std::find_if(rows.begin(), rows.end(), [&](const ConsoleRow& row) {
                    return entries_[row.entryIndex] == entry;
                });
            if (existing != rows.end()) {
                ++existing->count;
                continue;
            }
        }
        rows.push_back({index, 1});
    }
    return rows;
}

void ConsoleModel::select(std::size_t entryIndex) {
    if (entryIndex < entries_.size()) {
        selectedIndex_ = entryIndex;
    }
}

std::optional<std::size_t> ConsoleModel::selectedIndex() const {
    return selectedIndex_;
}

const engine::LogEntry* ConsoleModel::selectedEntry() const {
    if (!selectedIndex_ || *selectedIndex_ >= entries_.size()) {
        return nullptr;
    }
    return &entries_[*selectedIndex_];
}

ConsoleActivation ConsoleModel::activate(std::size_t entryIndex,
                                         IExternalCodeEditor& externalEditor,
                                         IEntityNavigator& entityNavigator) const {
    ConsoleActivation activation;
    if (entryIndex >= entries_.size()) {
        return activation;
    }
    const engine::LogEntry& entry = entries_[entryIndex];
    if (entry.source) {
        activation.externalEditorRequested = true;
        (void)externalEditor.open(*entry.source);
    }
    if (entry.entity) {
        entityNavigator.navigateTo(*entry.entity);
        activation.entityNavigationRequested = true;
    }
    return activation;
}

void ConsoleModel::setClearOnPlay(bool clearOnPlay) {
    clearOnPlay_ = clearOnPlay;
}

bool ConsoleModel::clearOnPlay() const {
    return clearOnPlay_;
}

void ConsoleModel::setErrorPause(bool errorPause) {
    errorPause_ = errorPause;
}

bool ConsoleModel::errorPause() const {
    return errorPause_;
}

void ConsoleModel::onPlayStarted() {
    playActive_ = true;
    errorPauseTriggered_ = false;
    if (clearOnPlay_) {
        clear();
    }
}

void ConsoleModel::onPlayStopped() {
    playActive_ = false;
}

bool ConsoleModel::matchesSearch(const engine::LogEntry& entry) const {
    return containsIgnoringCase(entry.message, search_);
}

void ConsoleModel::increment(engine::LogSeverity severity) {
    switch (severity) {
    case engine::LogSeverity::Info:
        ++counts_.info;
        break;
    case engine::LogSeverity::Warning:
        ++counts_.warning;
        break;
    case engine::LogSeverity::Error:
        ++counts_.error;
        break;
    }
}

} // namespace editor
