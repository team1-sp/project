#pragma once

#include <string>
#include <vector>

namespace editor {

// What the Editor has open: the project, the active scene and the items with unsaved changes
// (srs.fr.15.24). The Project Hub (US-13.x) and the scene stories fill it in; until then no
// project is open and nothing has unsaved changes.
class EditorSession {
public:
    // Opens a project, with no scene loaded and nothing unsaved.
    void openProject(std::string projectName);
    [[nodiscard]] bool isProjectOpen() const;
    [[nodiscard]] const std::string& projectName() const;

    // The name of the active scene; empty when no scene is loaded.
    void setActiveScene(std::string sceneName);
    [[nodiscard]] const std::string& activeSceneName() const;

    // Every loaded scene, data asset and Input Map asset with unsaved changes, and the prefab
    // open in Prefab Mode (srs.fr.15.24), by name.
    void setUnsavedItems(std::vector<std::string> items);
    [[nodiscard]] const std::vector<std::string>& unsavedItems() const;
    [[nodiscard]] bool hasUnsavedChanges() const;

    // The main window's title (srs.fr.14.2): "Aegis Editor 0.1 — <project> — <scene>", followed
    // by "*" while an item has unsaved changes. Parts that are not open are left out.
    [[nodiscard]] std::string mainWindowTitle() const;

    // The unsaved-changes check of srs.fr.15.22 before the Editor closes (srs.fr.14.3, 15.15).
    // Returns true when the Editor may close now. With unsaved items it returns false and asks
    // for the Unsaved Changes dialog (F-01), which closes the Editor itself once the user has
    // chosen; the dialog comes with US-15.x.
    [[nodiscard]] bool confirmClose();
    // True after confirmClose() found unsaved items, until clearUnsavedChangesDialogRequest().
    [[nodiscard]] bool isUnsavedChangesDialogRequested() const;
    void clearUnsavedChangesDialogRequest();

private:
    std::string projectName_;
    std::string activeSceneName_;
    std::vector<std::string> unsavedItems_;
    bool projectOpen_ = false;
    bool unsavedChangesDialogRequested_ = false;
};

} // namespace editor
