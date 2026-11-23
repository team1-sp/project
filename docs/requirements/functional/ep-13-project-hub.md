---
epic: EP-13
group: srs.fr.13
part: editor
---

# EP-13 Project Hub

**Epic:** As a game developer, I want a launcher where I find, create, relink and configure projects, so that I can start working on any project in a few clicks.

The Project Hub is the launcher window that opens before a project is loaded (frames A-01 to A-06b). Hub settings apply to every project on the computer. The Hub does not check for or install Aegis updates.

## US-13.1 Hub navigation

**Story:** As a game developer, I want to move between the Hub's Projects, Templates and Settings pages, so that I can reach every Hub function from one window.

*Screens: A-01, A-04, A-05*

- **srs.fr.13.1** When the Hub opens, it shall show the Projects page (A-01) with a sidebar that holds the entries Projects, Templates and Settings.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.2** When the user clicks Projects in the Hub sidebar, the Hub shall show the recent projects list (A-01).
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.3** When the user clicks Templates in the Hub sidebar, the Hub shall show the template library (A-04).
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.4** When the user clicks Settings in the Hub sidebar, the Hub shall show the Hub settings page (A-05).
  - Use cases: UC-01; Initial Plan: —

## US-13.2 Recent projects list

**Story:** As a game developer, I want to see my recent projects and open one, so that I can continue where I left off.

*Screens: A-01*

- **srs.fr.13.5** The Projects page shall list the recent projects, one row per project, showing the project name, folder path and last-opened date, most recently opened first.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.6** When the Projects page is shown, the Hub shall check every listed folder and shall mark a row "Not found" when its folder or the folder's `aegis.project` file no longer exists.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.7** A row marked "Not found" shall show a Locate… button and a Remove button.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.8** When the user double-clicks a row whose folder exists, the Hub shall show the Loading Project screen (A-03) and then open the project in the Editor main window (B-01).
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.9** When the user double-clicks a row marked "Not found", the Hub shall not try to open the project and shall highlight that row's Locate… button.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.10** When a project is opened, the Hub shall move its row to the top of the list and update its last-opened date.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.11** When the user clicks a row's ⋮ button, the Hub shall show a menu with the items Show in Explorer and Remove from List.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.12** When the user chooses Show in Explorer from a row's ⋮ menu, the Hub shall open the operating system's file browser at the project folder.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.13** When the user chooses Remove from List from a row's ⋮ menu, the Hub shall remove the row from the list without deleting any file and show a toast with an Undo link for 5 seconds.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.14** When the user clicks Remove on a row marked "Not found", the Hub shall remove the row from the list without deleting any file and show a toast with an Undo link for 5 seconds.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.15** When the user clicks Undo in a removal toast, the Hub shall put the removed row back at its previous position.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.16** When the user clicks Locate… on a row marked "Not found", the Hub shall open the Locate Project folder picker (A-06) titled with that project's name.
  - Use cases: UC-01; Initial Plan: —

## US-13.3 Opening and creating from the Hub

**Story:** As a game developer, I want to open a project from any folder or start a new one, so that I am not limited to the recent list.

*Screens: A-01*

- **srs.fr.13.17** When the user clicks New Project on the Projects page, the Hub shall open the New Project dialog (A-02).
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.18** When the user clicks Open… on the Projects page, the Hub shall open the operating system's folder picker.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.19** When the user selects a folder that contains an `aegis.project` file, the Hub shall open that project and add it to the top of the recent projects list.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.20** When the user selects a folder that does not contain an `aegis.project` file, the Hub shall show the error "Not an Aegis project" and stay on the Projects page.
  - Use cases: UC-01; Initial Plan: —

## US-13.4 New Project dialog

**Story:** As a game developer, I want to create a project from a template with a name and location, so that I get a working project folder in one step.

*Screens: A-02*

- **srs.fr.13.21** The New Project dialog shall show one card per available template, starting with the built-in templates Empty 2D and the demo game template, followed by the user's custom templates.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.22** When the user clicks a template card, the dialog shall select that card and deselect any other card.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.23** The dialog's Location field shall be filled with the default project location from the Hub settings when the dialog opens.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.24** When the user clicks Browse… next to the Location field, the dialog shall open the operating system's folder picker and put the chosen folder into the Location field.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.25** The dialog shall show, below the fields, a preview of the full project path (Location followed by Name) that updates on every change to either field.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.26** When the project path points to a folder that exists and is not empty, the dialog shall show the error "Folder already exists and is not empty" in red under the Name field.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.27** When the Name field contains a character that is not allowed in folder names, the dialog shall show the error "Name contains characters that are not allowed" in red under the Name field.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.28** The Create button shall be disabled until a template is selected, the Name field is not empty, the Location folder exists and no field shows an error.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.29** When the user clicks Create, the Hub shall create the project folder, copy the selected template's files into it, write an `aegis.project` file that holds a newly generated project ID and the project name, add the project to the recent list and show the Loading Project screen (A-03).
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.30** When any step of creating the project fails (creating the folder, copying the template's files or writing the `aegis.project` file), the Hub shall show an error that names the step and includes the operating system's reason, remove the files and folders this attempt created while keeping anything that was there before, add no recent-list entry and keep the dialog open.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1
- **srs.fr.13.31** When the user clicks Cancel or presses Esc in the New Project dialog, the Hub shall close the dialog without creating anything.
  - Use cases: UC-01; Initial Plan: ip.fr.12.1

## US-13.5 Loading a project

**Story:** As a game developer, I want to see loading progress and be able to cancel, so that I know the editor is working and can back out.

*Screens: A-03*

- **srs.fr.13.32** While a project loads, the Hub shall show the Loading Project screen with a progress bar, the percentage done and the name of the current step (assets or scene).
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.33** When the user clicks Cancel on the Loading Project screen, the Hub shall stop loading, leave the project files unchanged and return to the Projects page.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.34** When loading fails, the Hub shall return to the Projects page and show an error toast that names the file that could not be loaded.
  - Use cases: UC-01; Initial Plan: —

## US-13.6 Templates page

**Story:** As a game developer, I want to browse, import and manage project templates, so that I can reuse project setups.

*Screens: A-04*

- **srs.fr.13.35** The Templates page shall show the built-in templates and, under the heading My templates, the user's custom templates, each as a card with a name, a thumbnail and a short description.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.36** When the user clicks a template card, the Hub shall select it, give it a blue border and show its name, description and contents in the details pane.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.37** When the user double-clicks a template card, the Hub shall behave as if Use Template were clicked for that template.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.38** When the user clicks Use Template, the Hub shall open the New Project dialog (A-02) with that template selected, the location filled from the Hub's default project location (srs.fr.13.23) and the name filled as stated in srs.fr.13.81.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.39** When the user clicks Show in Explorer for a custom template, the Hub shall open the operating system's file browser at the template's folder.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.40** When the user clicks Show in Explorer for a built-in template, the Hub shall open the read-only copy of the template in the install folder.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.41** When the user types in Search templates, the Hub shall show only the cards whose name or description contains the typed text, ignoring letter case, updating on every keystroke.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.42** When no template matches the search text, the Hub shall show the text "No templates match".
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.43** When the user clicks Import… or the Import Template… card, the Hub shall open a picker that accepts a `.aegistemplate` file or a folder that contains a `template.json` file.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.44** When the user imports a valid template, the Hub shall copy it into the Documents/Aegis/Templates folder and add its card under My templates.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.45** A template shall be valid when its `template.json` file is a JSON object whose name is a non-empty string, and the template holds at least one `.scene` file; a `.aegistemplate` file shall be a ZIP archive with `template.json` at its root. When the user imports a file or folder that is not a valid template, the Hub shall show an error toast that states which of these conditions failed, shall not add a card and shall leave the Templates folder unchanged.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.46** When the user clicks Open Templates Folder, the Hub shall open the Documents/Aegis/Templates folder in the operating system's file browser.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.47** When the user right-clicks a custom template card, the Hub shall show a menu with the items Rename, Duplicate and Delete.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.48** When the user right-clicks a built-in template card, the Hub shall show a menu with only the item Duplicate.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.49** When the user chooses Rename for a custom template, the Hub shall make the card's name editable in place; Enter saves the new name and Esc cancels; a name that is empty after leading and trailing spaces are removed, or that equals another template's name compared without regard to case, shall be refused with the old name kept and the reason shown under the card.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.50** When the user chooses Duplicate for a template, the Hub shall create a custom copy named after the original with " (copy)" added and show its card under My templates.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.51** When the user chooses Delete for a custom template, the Hub shall ask for confirmation (F-02) and, if confirmed, delete the template's folder and remove its card.
  - Use cases: UC-01; Initial Plan: —

## US-13.7 Hub settings

**Story:** As a game developer, I want to set preferences that apply to every project, so that the Hub behaves the way I want on this computer.

*Screens: A-05*

- **srs.fr.13.52** The Hub shall save every change on the Settings page immediately to the Hub settings file, without a Save button.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.53** When the user clicks Browse… next to Default project location, the Hub shall open the operating system's folder picker and store the chosen folder as the default for New Project and Locate.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.54** When the default project location folder does not exist, the Settings page shall show a yellow hint "Folder not found" under the field.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.55** The On start setting shall offer the values Show the Hub and Open last project, and shall take effect at the next launch.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.56** When On start is Open last project and that project's folder exists, the Hub shall open that project directly at launch; otherwise it shall show the Projects page.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.57** The Language setting shall list English as the only available language.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.58** The Theme setting shall offer Dark and shall show Light greyed out with the note "not available yet"; it shall be the same per-user setting as the Theme field in Preferences (srs.fr.27.30).
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.59** When the user changes UI scale to 90, 100, 125 or 150 %, the Hub shall rescale at once and store the value in the same per-user setting as the UI scale field in Preferences (srs.fr.27.31), which the Editor uses when a project opens.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.60** When the user clicks Open next to Install folder, the Hub shall open the install folder in the operating system's file browser.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.61** The Send anonymous usage data setting shall be off by default; when on, the Hub shall send, once when it closes, only the Aegis version, the operating system family and the number of times each Hub command was used in that session, with no user, project or computer identifier, file name or path, scene content, source code or telemetry; when off, it shall neither collect nor send these counts.
  - Use cases: UC-01; Initial Plan: ip.nfr.2.1
- **srs.fr.13.62** The AI service address field shall hold the default AI service address that new projects use.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.63** When the user clicks Test next to the AI service address, the Hub shall send a health check to the service with a deadline of 5 seconds, using the gRPC Health Checking Protocol (grpc.health.v1, Check) over the AI service connection (srs.fr.11.10), and show the green text "reachable" under the field when the service answers SERVING, or a red error under the field that names the failure: address not valid (srs.fr.33.28), service not reachable, deadline exceeded, service not serving, or the error status and message returned by the service.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.64** When the user clicks Clear Recent List, the Hub shall ask for confirmation and, if confirmed, empty the recent projects list without deleting any project files.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.65** When the user clicks Remove Missing Projects, the Hub shall remove only the rows marked "Not found" and update the count shown in the hint.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.66** When the user clicks Reset to Defaults, the Hub shall ask for confirmation and, if confirmed, restore every Hub setting to its factory default (srs.fr.13.84), not to a last-used value.
  - Use cases: UC-01; Initial Plan: —

## US-13.8 Locate Project

**Story:** As a game developer, I want to point a moved project's entry at its new folder, so that I do not lose a project from my list when I move it.

*Screens: A-06, A-06b*

- **srs.fr.13.67** The Locate Project picker shall open in the parent folder of the project's old path or, if that folder no longer exists, in the default project location.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.68** When the user clicks ←, the picker shall go back to the previously shown folder.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.69** When the user clicks ↑, the picker shall go up one folder level.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.70** When the user types or pastes a path into the path field and presses Enter, the picker shall show that folder.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.71** When the path in the path field does not exist, the picker shall give the field a red outline and keep showing the current folder.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.72** When the user clicks a folder in the folder tree, the picker shall show that folder.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.73** When the user types in the picker's Search box, the picker shall show only the entries of the current folder whose names contain the typed text.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.74** When the user double-clicks a folder in the list, the picker shall open that folder and update the validation line.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.75** When the user clicks New Folder, the picker shall create a folder in the current location and make its name editable in place.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.76** When the selected folder contains an `aegis.project` file whose project ID (srs.fr.13.82) equals the ID stored for the recent-list entry, the validation line shall be green and the Select Folder button shall be enabled, whatever the project's current name.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.77** When the selected folder contains no `aegis.project` file, the validation line shall be yellow with the text "No aegis.project in this folder" and the Select Folder button shall be disabled.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.78** When the selected folder contains an `aegis.project` file with a different project ID, the validation line shall say "This folder holds a different project: <name>", and Select Folder shall ask "Replace this entry with <name>?"; if confirmed, the entry shall be relinked to that project, taking its ID and name.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.79** When the user clicks Select Folder, the Hub shall relink the recent-list entry to the new folder, show the row as normal with the new path and show the toast "Project located".
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.80** When the user clicks Cancel or ✕, or presses Esc, the picker shall close without changes and the row shall stay "Not found".
  - Use cases: UC-01; Initial Plan: —

## US-13.9 Project defaults and identity

**Story:** As a game developer, I want a new project to have a sensible name and every project a stable identity, so that locating a project never relinks the wrong one.

- **srs.fr.13.81** The New Project dialog shall fill its Name field with "New Project", or with "New Project N" using the lowest N from 2 up whose folder does not yet exist in the location.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.82** The project ID in `aegis.project` shall identify a project; the recent list shall store each entry's project ID, and the project name shall be used only for display, so that a renamed project keeps its identity and two projects with the same name stay distinct.
  - Use cases: UC-01; Initial Plan: —

## US-13.10 Invalid project files

**Story:** As a game developer, I want to to be told why a project cannot be opened, so that a damaged project file never opens half-way or replaces a recent entry.

- **srs.fr.13.83** When an `aegis.project` file cannot be read, is not valid JSON, or has no project ID or no name, the Hub shall not open the project or relink a recent-list entry to it (srs.fr.13.19, srs.fr.13.76, srs.fr.13.78), and shall show "Cannot open project: <reason>" with the file's path.
  - Use cases: UC-01; Initial Plan: —

## US-13.11 Factory defaults of Hub settings

**Story:** As a game developer, I want to to know the value every Hub setting starts with, so that Reset to Defaults has a fixed, testable result.

- **srs.fr.13.84** The factory defaults of the Hub settings shall be: Default project location Documents/Aegis/Projects; On start Show the Hub; Language English; Theme Dark; UI scale 100 %; Send anonymous usage data off; and an empty AI service address.
  - Use cases: UC-01; Initial Plan: —

## US-13.12 Template name conflicts

**Story:** As a game developer, I want importing or duplicating templates to never overwrite one, so that every template I add keeps its own name and files.

- **srs.fr.13.85** When the user imports a valid template (srs.fr.13.44) whose name, or folder name in the Templates folder, is already used, the Hub shall add " (2)", " (3)" and so on, the lowest number not in use, to both, and shall never overwrite another template; when the copy fails part way, the Hub shall delete the partly copied folder, add no card and show an error toast naming the file and the reason.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.13.86** When the name of a duplicate (srs.fr.13.50) is already used, the Hub shall add " 2", " 3" and so on after "(copy)", the lowest number not in use; a duplication that fails part way shall be cleaned up as stated in srs.fr.13.85.
  - Use cases: UC-01; Initial Plan: —

## US-13.13 Folder tree arrows

**Story:** As a game developer, I want to expand and collapse folders in the picker, so that I can reach nested folders.

- **srs.fr.13.87** When the user clicks a tree arrow in the picker's folder tree, the picker shall expand or collapse that folder's subfolders.
  - Use cases: UC-01; Initial Plan: —
