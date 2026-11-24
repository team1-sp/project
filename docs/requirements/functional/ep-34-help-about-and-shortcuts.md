---
epic: EP-34
group: srs.fr.34
part: editor
---

# EP-34 Help, About and Shortcuts

**Epic:** As a game developer, I want to reach help, version information and a shortcut list, so that I can learn the editor and report problems.

This group covers the Help menu (D-09), the About dialog (F-14) and the shortcut reference (F-15).

## US-34.1 Help menu

**Story:** As a game developer, I want to open documentation and report bugs, so that I can get help when stuck.

*Screens: D-09*

- **srs.fr.34.1** When the user chooses Help ▸ Documentation, the Editor shall open the documentation in the default web browser.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.34.2** When the user chooses Help ▸ Keyboard Shortcuts, the Editor shall open the shortcut reference (F-15).
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.34.3** When the user chooses Help ▸ Report a Bug, the Editor shall open the project's GitHub new-issue page in the default web browser.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.34.4** When the user chooses Help ▸ About Aegis, the Editor shall open the About dialog (F-14).
  - Use cases: UC-01; Initial Plan: —

## US-34.2 About and shortcut reference

**Story:** As a game developer, I want to see version details and every shortcut, so that I can report my setup and learn faster ways to work.

*Screens: F-14, F-15*

- **srs.fr.34.5** The About dialog shall show the Aegis version, the raylib version (6.2), the Box2D version (3.1.1) and the licences of the third-party libraries.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.34.6** The shortcut reference shall list every command and its shortcut, grouped by menu.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.34.7** When the user types in the shortcut reference's Search box, it shall show only the commands whose name or key matches.
  - Use cases: UC-01; Initial Plan: —
- **srs.fr.34.8** When the user clicks Edit in Preferences, the Editor shall open Preferences (F-05) at the Shortcuts list.
  - Use cases: UC-01; Initial Plan: —
