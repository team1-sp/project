---
epic: EP-32
group: srs.fr.32
part: editor
---

# EP-32 Semantic Graph Panel

**Epic:** As a game developer, I want to explore and export the semantic graph visually, so that I can see how entities, components, systems and events relate.

This group covers the Semantic Graph panel (C-13 and C-13b), the Export Semantic Graph dialog (F-10) and the graph items of the AI menu (D-07).

## US-32.1 Opening and building

**Story:** As a game developer, I want to open and rebuild the graph, so that it always reflects the latest data.

*Screens: C-13, D-07*

- **srs.fr.32.1** When the user chooses AI ▸ Open Semantic Graph, the Editor shall open the Semantic Graph panel and, if the graph is stale, rebuild it first as stated in srs.fr.10.5.
  - Use cases: UC-10; Initial Plan: ip.fr.10.3
- **srs.fr.32.2** When the user chooses AI ▸ Rebuild Graph or clicks Rebuild, the Editor shall rebuild the graph as stated in srs.fr.10.5, with event nodes only when a recording exists, and show the progress in the status bar.
  - Use cases: UC-10; Initial Plan: ip.fr.10.3, ip.nfr.1.4
- **srs.fr.32.3** While the graph is stale, the Semantic Graph panel shall show a banner "Graph is out of date" with a Rebuild button.
  - Use cases: UC-10; Initial Plan: ip.fr.10.3

## US-32.2 Exploring

**Story:** As a game developer, I want to filter, search and inspect graph nodes, so that I can focus on the relations that matter.

*Screens: C-13*

- **srs.fr.32.4** The Semantic Graph panel shall draw nodes as shapes coloured by node type and edges as labelled lines.
  - Use cases: UC-10; Initial Plan: ip.fr.10.1
- **srs.fr.32.5** When the user drags the canvas or scrolls, the panel shall pan or zoom the graph.
  - Use cases: UC-10; Initial Plan: ip.fr.10.4
- **srs.fr.32.6** When the user toggles a node type filter (Entity, Component, System or Event), the panel shall show or hide nodes of that type.
  - Use cases: UC-10; Initial Plan: ip.fr.10.1
- **srs.fr.32.7** When the user toggles an edge filter (has, reads, writes, collided with, triggered), the panel shall show or hide edges of that relation.
  - Use cases: UC-10; Initial Plan: ip.fr.10.2
- **srs.fr.32.8** When the user searches, the panel shall find the nodes whose name contains the typed text, ignoring letter case, or whose ID equals it: with one match it shall centre on that node and highlight it, with several it shall list them under the search box and centre on the one the user picks, and with none it shall show "No node matches" (C-13b).
  - Use cases: UC-10; Initial Plan: ip.fr.10.4
- **srs.fr.32.9** When the user clicks a node, the Inspector shall show the node's properties and, for an entity node, the Editor shall navigate to that entity as stated in srs.fr.35.20; a historical entity node shall use the run its events come from.
  - Use cases: UC-10; Initial Plan: ip.fr.10.4
- **srs.fr.32.10** When the user double-clicks a node, the panel shall add the node's neighbours to the view.
  - Use cases: UC-10; Initial Plan: ip.fr.10.4
- **srs.fr.32.11** When the user picks Force, Hierarchical or By tick from Layout ▾, the panel shall arrange the nodes with that layout.
  - Use cases: UC-10; Initial Plan: ip.fr.10.1
- **srs.fr.32.12** When the user sets the Tick range, the panel shall show only event nodes whose tick lies in that range.
  - Use cases: UC-10; Initial Plan: ip.fr.10.4
- **srs.fr.32.13** When the user clicks Ask AI with nodes selected, the Editor shall open the AI Assistant (C-14) with those nodes as context chips.
  - Use cases: UC-10; Initial Plan: ip.fr.11.2

## US-32.3 Export

**Story:** As a game developer, I want to export the graph to a file, so that I can use it in other tools and in reports.

*Screens: F-10, D-07*

- **srs.fr.32.14** When the user clicks Export in the panel, or chooses AI ▸ Export Graph… while a graph exists, the Editor shall open the Export Semantic Graph dialog (F-10).
  - Use cases: UC-10; Initial Plan: ip.fr.10.5
- **srs.fr.32.15** The Export Semantic Graph dialog shall offer the JSON format, an Include event nodes checkbox and a file path field with Browse….
  - Use cases: UC-10; Initial Plan: ip.fr.10.5
- **srs.fr.32.16** When the user clicks Export, the Editor shall write the graph file and show a toast with an Open Folder link.
  - Use cases: UC-10; Initial Plan: ip.fr.10.5

## US-32.4 Graph export failures

**Story:** As a game developer, I want graph export to protect existing files and report failures, so that an export never overwrites work by surprise or leaves half a file.

*Screens: F-10, D-07*

- **srs.fr.32.17** When the graph export file already exists, the Editor shall ask for confirmation before replacing it; the Editor shall write the export to a temporary file and replace the target only when writing has completed, show the success toast (srs.fr.32.16) only then, and on failure delete the temporary file, leave any existing file unchanged and show an error toast naming the path and the reason.
  - Use cases: UC-10; Initial Plan: ip.fr.10.5
