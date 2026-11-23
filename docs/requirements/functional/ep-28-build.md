---
epic: EP-28
group: srs.fr.28
part: editor
---

# EP-28 Build

**Epic:** As a game developer, I want to build my game as a Windows program, so that others can play it outside the editor.

This group covers the Build Settings dialog (F-06) and the build progress and result dialog (F-12 to F-12c).

## US-28.1 Build settings

**Story:** As a game developer, I want to choose the scenes and output of a build, so that the built game contains what I intend.

*Screens: F-06*

- **srs.fr.28.1** The Build Settings dialog shall list the scenes in the build, each with a checkbox that includes or excludes it.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.2** When the user drags a scene in the list, the Editor shall change the order; the first ticked scene shall be the one the game loads at start.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.3** When the user clicks Add Open Scene, the Editor shall add the active scene to the list, ticked, unless it is already listed; when the active scene has never been saved, it shall first ask for a file name as Save As… does and add the scene only after the save succeeds, leaving the list unchanged when the save is cancelled or fails.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.4** The Platform field shall show Windows as the only target platform.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.5** When Development build is on, the build shall include the stats overlay and log output.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.6** When the user clicks Browse… next to Output folder, the Editor shall open the operating system's folder picker and store the chosen folder.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.7** The Build and Build and Run buttons shall be disabled until at least one scene is ticked and an output folder is set.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.8** When the user clicks Build, the Editor shall build a Windows executable of the ticked scenes into the output folder from the saved files and show the build progress dialog (F-12); when any item has unsaved changes (srs.fr.15.24), it shall first offer Save and Build, Build Saved Version and Cancel, where Save and Build saves as stated in step 1 of srs.fr.29.21 and builds only if every save succeeded, Build Saved Version builds from the saved files and keeps the unsaved changes, and Cancel changes nothing.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.9** When the user clicks Build and Run, the Editor shall build as Build does and start the built game when the build succeeds.
  - Use cases: UC-05; Initial Plan: —

## US-28.2 Build progress and result

**Story:** As a game developer, I want to follow a build and act on its result, so that I can fix failures or run the game.

*Screens: F-12, F-12b, F-12c*

- **srs.fr.28.10** While a build runs, the build dialog shall show a progress bar and the build log.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.11** When the user clicks Cancel during a build, the Editor shall stop the build and delete only the build's staging folder (srs.fr.28.17), leaving the output folder as it was.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.12** When a build succeeds, the build dialog shall show the buttons Open Folder and Run.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.13** When the user clicks Open Folder after a successful build, the Editor shall open the output folder in the operating system's file browser.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.14** When the user clicks Run after a successful build, the Editor shall start the built game.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.15** When a build fails, the build dialog shall show the number of errors and a Show Errors button.
  - Use cases: UC-05; Initial Plan: —
- **srs.fr.28.16** When the user clicks Show Errors, the Editor shall send each build error to the Console and open the Console.
  - Use cases: UC-05; Initial Plan: —

## US-28.3 Build staging

**Story:** As a game developer, I want a cancelled build never to damage my last good build, so that I can always run the previous build.

- **srs.fr.28.17** The Editor shall build into a staging folder inside the project's Temp folder and, only when the build succeeds, replace the files listed in the previous build's manifest in the output folder with the new build and its manifest, leaving any other files in the output folder unchanged; when a file of the new build would overwrite a file in the output folder that the previous manifest does not list, the build shall fail before changing the output folder and name that file.
  - Use cases: UC-05; Initial Plan: —

## US-28.4 Publishing a build

**Story:** As a game developer, I want a failed copy into the output folder to leave a usable build, so that a disk or permission error never leaves a mix of two builds.

- **srs.fr.28.18** When replacing the output folder's files from a successful staging build (srs.fr.28.17) fails part way, the Editor shall put back the files it had already replaced from the previous build, so that the output folder holds the previous build and its manifest, and show the build as failed with an error naming the file and the reason; when a file cannot be put back, the Editor shall delete the output folder's manifest and show "Build output is incomplete: <file>".
  - Use cases: UC-05; Initial Plan: —
