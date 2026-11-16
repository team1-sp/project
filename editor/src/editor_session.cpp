#include "editor/editor_session.hpp"

#include <utility>

#include "engine/core/version.hpp"

namespace editor {

void EditorSession::openProject(std::string projectName) {
    projectName_ = std::move(projectName);
    activeSceneName_.clear();
    unsavedItems_.clear();
    projectOpen_ = true;
}

bool EditorSession::isProjectOpen() const {
    return projectOpen_;
}

const std::string& EditorSession::projectName() const {
    return projectName_;
}

void EditorSession::setActiveScene(std::string sceneName) {
    activeSceneName_ = std::move(sceneName);
}

const std::string& EditorSession::activeSceneName() const {
    return activeSceneName_;
}

void EditorSession::setUnsavedItems(std::vector<std::string> items) {
    unsavedItems_ = std::move(items);
}

const std::vector<std::string>& EditorSession::unsavedItems() const {
    return unsavedItems_;
}

bool EditorSession::hasUnsavedChanges() const {
    return !unsavedItems_.empty();
}

std::string EditorSession::mainWindowTitle() const {
    // The separator of the B-01 design, an em dash between spaces, in UTF-8.
    constexpr const char* separator = " \xE2\x80\x94 ";

    const engine::Version version = engine::engineVersion();
    std::string title =
        "Aegis Editor " + std::to_string(version.major) + "." + std::to_string(version.minor);
    if (projectOpen_) {
        title += separator + projectName_;
        if (!activeSceneName_.empty()) {
            title += separator + activeSceneName_;
        }
    }
    if (hasUnsavedChanges()) {
        title += "*";
    }
    return title;
}

bool EditorSession::confirmClose() {
    if (!hasUnsavedChanges()) {
        return true;
    }
    unsavedChangesDialogRequested_ = true;
    return false;
}

bool EditorSession::isUnsavedChangesDialogRequested() const {
    return unsavedChangesDialogRequested_;
}

void EditorSession::clearUnsavedChangesDialogRequest() {
    unsavedChangesDialogRequested_ = false;
}

} // namespace editor
