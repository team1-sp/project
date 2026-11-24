---
epic: EP-11
group: srs.fr.11
part: engine
---

# EP-11 AI Assistant

**Epic:** As a game developer, I want to ask questions about my game in natural language and get grounded answers, so that I find the cause of problems faster.

These requirements refine ip.fr.11.1 to ip.fr.11.8. The Editor side is in srs.fr.33.

## US-11.1 Answering questions

**Story:** As a game developer, I want answers that cite ticks, entities and source lines, so that I can check every claim the assistant makes.

- **srs.fr.11.1** The AI Assistant Layer shall accept natural-language questions from the user through a chat interface.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1
- **srs.fr.11.2** The AI Assistant Layer shall answer questions about the current or recorded engine state, such as "Why did the unlock fail?", using the Semantic Graph.
  - Use cases: UC-11; Initial Plan: ip.fr.11.2
- **srs.fr.11.3** The AI Assistant Layer shall diagnose a reported bug from telemetry, event payloads and handler write records, and shall return with the diagnosis the ticks and entities it relies on as citations (srs.fr.11.4), citing the initial state as Start rather than as a tick. The diagnosis should not present a system's read and write declarations alone as the cause of a state change. For each debug flag of srs.fr.12.9 to srs.fr.12.12, a demo run in which the flag was applied and its effect is recorded (a removed collider, an enclosed objective or an unlock without the item) should be diagnosed by naming that effect as the cause and citing the record; when a flag was not applied (srs.fr.12.15) or its effect was not exercised or not recorded in the run, the Assistant should say so instead of attributing a failure to it.
  - Use cases: UC-11; Initial Plan: ip.fr.11.3
- **srs.fr.11.4** The AI Assistant Layer shall return each citation as a structured reference: a state citation with its run number and either a tick number or Start, the run's initial state (srs.fr.8.26); an entity citation with its entity ID, name, source scene, whether it was created at runtime and, when it comes from a run, its run number and an optional tick number or Start; or a citation of a line of a source file attached to the question with the file's content hash at the time of the question.
  - Use cases: UC-11; Initial Plan: ip.fr.11.3
- **srs.fr.11.5** When the Source code context is attached, the AI Assistant Layer shall use as additional context only the source files listed on that context item (srs.fr.33.37).
  - Use cases: UC-11; Initial Plan: ip.fr.11.4
- **srs.fr.11.6** The AI Assistant Layer shall use as context only the question text, the earlier messages of the same chat that the consent rules allow (srs.fr.33.46), and the context items attached to the question (Selection, Last run, Graph, Graph nodes, Source code, Asset, Log entry, Component type) with the content defined in srs.fr.33.34 to srs.fr.33.37 and srs.fr.33.50 to srs.fr.33.53; it shall not read any other project content.
  - Use cases: UC-11; Initial Plan: ip.fr.11.2
- **srs.fr.11.7** When the Semantic Graph or telemetry does not contain the information needed, the AI Assistant Layer should state that the information is unavailable and name what is missing, and should label a hypothesis as one.
  - Use cases: UC-11; Initial Plan: ip.fr.11.5

## US-11.2 Limits and service

**Story:** As a game developer, I want the assistant to stay read-only and to use the AI service I chose, so that it cannot break my project or leak data.

- **srs.fr.11.8** The AI Assistant Layer shall not modify engine state or project source code.
  - Use cases: UC-11; Initial Plan: ip.fr.11.6
- **srs.fr.11.9** The AI Assistant Layer should suggest design changes based on the structure of the Semantic Graph, as suggestions that the user applies by hand.
  - Use cases: UC-11; Initial Plan: ip.fr.11.7
- **srs.fr.11.10** When an AI service is configured, the AI Assistant Layer shall obtain language-model responses from it over gRPC through the AgentBridge service contract (srs.nfr.4.10).
  - Use cases: UC-11; Initial Plan: ip.fr.11.8
- **srs.fr.11.11** The AI Assistant Layer shall stop generating an answer when the user cancels the question.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1
- **srs.fr.11.12** The AI Assistant Layer shall store each chat, with its questions, answers, citations and the data categories each message carried, in the user's local area of the project folder, not with the shared project settings.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1
