---
epic: EP-26
group: srs.fr.26
part: editor
---

# EP-26 Components Panel

**Epic:** As a game developer, I want an overview of every component type in the project, so that I can see where each component is used and by which systems.

This group covers the Components panel in the bottom dock (C-15) and its no-results state (C-15b). Counts refer to the open scene.

## US-26.1 Component list

**Story:** As a game developer, I want to filter, search and sort the component types, so that I find the component I need.

*Screens: C-15, C-15b*

- **srs.fr.26.1** The Components panel shall list every registered component type with its name, category, fields, number of entities that use it, the systems that read it, the systems that write it and where it is defined.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.10.1
- **srs.fr.26.2** When the user clicks All, the Components panel shall list every component type and show the All filter in blue.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.10.1
- **srs.fr.26.3** When the user clicks Built in, the Components panel shall list only the engine's built-in component types.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.10.1
- **srs.fr.26.4** When the user clicks Project, the Components panel shall list only the component types defined in the project's scripts.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.10.1
- **srs.fr.26.5** When the user types in Search components, the Components panel shall list only the component types whose name, field names or semantic description contain the typed text.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.10.1
- **srs.fr.26.6** When no component type matches the search, the Components panel shall show the no-results state (C-15b) with the buttons Clear Search and ＋ New Component Script….
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.10.1
- **srs.fr.26.7** When the user clicks Clear Search, the Components panel shall empty the search field and show the full list again.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.10.1
- **srs.fr.26.8** When the user clicks a column header, the Components panel shall sort by that column; clicking it again shall reverse the order, and a ▾ mark shall show on the sorted column.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.10.1
- **srs.fr.26.9** When the user clicks ↻ Refresh, the Editor shall re-read the component registry and update every count.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2, ip.fr.10.1

## US-26.2 Component actions

**Story:** As a game developer, I want to act on a component type from the list or details pane, so that I can jump from a type to its entities, graph, source or the AI.

*Screens: C-15*

- **srs.fr.26.10** When the user clicks a row, the Components panel shall select it and show in the details pane the component's semantic description, its fields with types and defaults, the entities that use it, its systems and its events.
  - Use cases: UC-02, UC-10, UC-11; Initial Plan: —
- **srs.fr.26.11** When the user double-clicks a row or clicks Select Entities, the Editor shall select every entity that has this component, show them in the Hierarchy and Scene view, and show a multi-edit Inspector when there is more than one.
  - Use cases: UC-02, UC-10, UC-11; Initial Plan: ip.fr.2.4
- **srs.fr.26.12** When the user right-clicks a row, the Components panel shall show a menu with the items Select Entities, Show in Graph, Open Source and Copy Name.
  - Use cases: UC-02, UC-10, UC-11; Initial Plan: —
- **srs.fr.26.13** When the user chooses Copy Name, the Editor shall copy the component type's name to the clipboard.
  - Use cases: UC-02, UC-10, UC-11; Initial Plan: —
- **srs.fr.26.14** When the user clicks Show in Graph, the Editor shall open the Semantic Graph (C-13) filtered to the component nodes of this component type (srs.fr.10.2) with all of them highlighted, or show "No components of this type in the graph" when there are none.
  - Use cases: UC-02, UC-10, UC-11; Initial Plan: —
- **srs.fr.26.15** When the user clicks Open Source for a project component, the Editor shall open its header file in the external code editor.
  - Use cases: UC-02, UC-10, UC-11; Initial Plan: —
- **srs.fr.26.16** When the user clicks Open Source for a built-in component, the Editor shall open its read-only documentation page.
  - Use cases: UC-02, UC-10, UC-11; Initial Plan: —
- **srs.fr.26.17** When the user clicks ✦ Ask AI, the Editor shall open the AI Assistant (C-14) with this component as a context chip.
  - Use cases: UC-02, UC-10, UC-11; Initial Plan: ip.fr.11.2

## US-26.3 New component script

**Story:** As a game developer, I want to create a new component type from the panel, so that I can add my own data types without leaving the editor.

*Screens: C-15, C-15b*

- **srs.fr.26.18** When the user clicks ＋ New Component Script…, the Editor shall open the New Script dialog (F-17) for a component type; in the no-results state the Name field shall be pre-filled from the search text.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.26.19** The New Script dialog shall accept a component name only as stated in srs.fr.36.3 and shall show the reason under the field otherwise.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
- **srs.fr.26.20** When the user clicks Create, the Editor shall create the script as stated in srs.fr.36.4; the type shall not be registered until the project scripts build successfully.
  - Use cases: UC-02; Initial Plan: ip.fr.2.2
