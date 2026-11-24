---
epic: EP-10
group: srs.fr.10
part: engine
---

# EP-10 Semantic Graph

**Epic:** As a game developer, I want a graph of entities, components, systems and events built from runtime data, so that the AI assistant understands my specific game.

These requirements refine ip.fr.10.1 to ip.fr.10.5. The Editor side is in srs.fr.32.

## US-10.1 Graph content

**Story:** As a game developer, I want the graph to hold every entity, component, system and event with their relations, so that nothing about the game's structure is hidden from the AI.

- **srs.fr.10.1** The Semantic Architecture Layer shall build a graph whose nodes represent entities, components, systems and events.
  - Use cases: UC-10; Initial Plan: ip.fr.10.1
- **srs.fr.10.2** Each node shall hold its ID, its node type and its name; an entity node shall stand for one entity, a component node for one component of one entity (identified by the entity and the component type), a system node for one registered system, and an event node for one event of one run, identified by the run and the event's sequence number (srs.fr.2.46) and also holding its tick number. The reads and writes edges of srs.fr.10.4 shall link a system node to the component nodes of the component types it declares.
  - Use cases: UC-10; Initial Plan: ip.fr.10.1
- **srs.fr.10.3** Each component node shall hold the semantic description of its component type.
  - Use cases: UC-10; Initial Plan: ip.fr.10.1
- **srs.fr.10.4** The Semantic Graph shall contain edges for these relations: entity *has* component, system *reads* component and system *writes* component (from the systems' declarations), entity *collided with* entity (from CollisionEvents), and event *triggered* component, which shall be created only from a handler write record (srs.fr.8.19), one edge per record, linking the event node to the component node chosen by srs.fr.10.12 and holding the field, the old and new values, the handling system, the callback index and the write's position.
  - Use cases: UC-10; Initial Plan: ip.fr.10.2
- **srs.fr.10.5** The Semantic Architecture Layer shall build the graph from two sources without requiring any change to game content: the structure of srs.fr.10.9 and the events of srs.fr.10.10.
  - Use cases: UC-10; Initial Plan: ip.fr.10.3
- **srs.fr.10.6** The Semantic Architecture Layer shall mark the graph as stale when the edited state of the loaded scenes or the registered types have changed, or a newer recording exists, since the graph was built.
  - Use cases: UC-10; Initial Plan: ip.fr.10.3

## US-10.2 Graph access and export

**Story:** As a game developer, I want to query and export the graph, so that the AI layer and outside tools can use it.

- **srs.fr.10.7** The Semantic Architecture Layer shall provide the AI Assistant Layer with a query interface that returns nodes by ID, by node type, by name, by tick range, and the neighbours of a node.
  - Use cases: UC-10; Initial Plan: ip.fr.10.4
- **srs.fr.10.8** The Semantic Architecture Layer shall export the graph to a JSON file whose format is documented.
  - Use cases: UC-10; Initial Plan: ip.fr.10.5

## US-10.3 Graph sources

**Story:** As a game developer, I want the graph to say which parts are current structure and which are recorded history, so that old causal evidence is never mixed with current values unnoticed.

- **srs.fr.10.9** The graph's structure (entity, component and system nodes and *has*, *reads* and *writes* edges) shall be built from the edited state of the loaded scenes and the registered types, and shall be available before any recording exists.
  - Use cases: UC-10; Initial Plan: ip.fr.10.1
- **srs.fr.10.10** Event nodes and *collided with* and *triggered* edges shall be built from one recording, the latest by default, and each shall hold that recording's run number; an event that refers to an entity with no current match (srs.fr.8.21) shall link to a historical entity node marked "historical".
  - Use cases: UC-10; Initial Plan: ip.fr.10.1
- **srs.fr.10.11** The Semantic Graph panel shall show the run number of the recording its events come from, or "No events: no recording", and component nodes shall show current field values only, never values from the recording.
  - Use cases: UC-10; Initial Plan: ip.fr.10.1

## US-10.4 Historical components

**Story:** As a game developer, I want to triggered edges to point to the component that was actually written, so that a historical write is never attached to an unrelated current component.

- **srs.fr.10.12** A *triggered* edge shall link to the current component node only when the recorded entity matches a current entity (srs.fr.8.21) that still has a component of the recorded type; when the entity matches but no longer has that component, the edge shall link to a historical component node under the current entity node; when the entity does not match, it shall link to a historical component node under the historical entity node (srs.fr.10.10); a historical component node shall be marked "historical" and hold only the type name and the values from the write records, never current values.
  - Use cases: UC-10; Initial Plan: ip.fr.10.1
