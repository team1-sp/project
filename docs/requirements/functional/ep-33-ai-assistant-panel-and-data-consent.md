---
epic: EP-33
group: srs.fr.33
part: editor
---

# EP-33 AI Assistant Panel and Data Consent

**Epic:** As a game developer, I want to chat with the assistant about my game and control what data leaves my computer, so that I get grounded help without losing control of my code.

This group covers the AI Assistant panel (C-14 and C-14b), the AI menu (D-07), the AI data consent dialog (F-11) and the AI and Privacy tab of Project Settings (F-04).

## US-33.1 Opening the assistant

**Story:** As a game developer, I want to open the assistant with useful context, so that I start from the thing I am looking at.

*Screens: D-07*

- **srs.fr.33.1** When the user chooses AI ▸ Open Assistant, the Editor shall open the AI Assistant panel (C-14), or bring it to the front and focus its message box when it is already open; Ctrl+Shift+A shall toggle the panel as stated in srs.fr.17.22.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1
- **srs.fr.33.2** When the user chooses AI ▸ Ask about Selection while an entity, event or finding is selected, the Editor shall open the assistant with a context chip for the selection and the message box pre-filled with "Why…".
  - Use cases: UC-11; Initial Plan: ip.fr.11.2
- **srs.fr.33.3** When the user chooses AI ▸ Diagnose Last Run while a run report exists, the Editor shall attach the Last run context (srs.fr.33.35) to a new question that asks for a diagnosis and send it under the consent rules of srs.fr.33.41.
  - Use cases: UC-11; Initial Plan: ip.fr.11.3
- **srs.fr.33.4** When the user chooses AI ▸ AI Settings…, the Editor shall open Project Settings at the AI and Privacy tab.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1

## US-33.2 Chatting

**Story:** As a game developer, I want to ask questions, control context and stop answers, so that the conversation stays focused.

*Screens: C-14, C-14b*

- **srs.fr.33.5** When the user presses Enter in the message box, the Editor shall send the question as Send does (srs.fr.33.6), doing nothing while the message box is empty; Shift+Enter shall insert a new line.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1
- **srs.fr.33.6** When the user clicks Send, the Editor shall send the question; Send shall be disabled while the message box is empty.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1
- **srs.fr.33.7** While an answer is being prepared, the panel shall show a spinner and a Stop button.
  - Use cases: UC-11; Initial Plan: ip.nfr.1.5
- **srs.fr.33.8** When the user clicks Stop, the Editor shall cancel the running answer and keep the part received so far, marked as stopped.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1
- **srs.fr.33.9** The panel shall show the attached context items as chips (Selection, Last run, Graph, Graph nodes, Source code, Asset, Log entry or Component type), each with the size it adds to the request; clicking a chip's ✕ shall remove it from the next question.
  - Use cases: UC-11; Initial Plan: ip.fr.11.2, ip.fr.11.4
- **srs.fr.33.10** When the user clicks + beside the chips, the panel shall list the context types that are not attached and attach the chosen one.
  - Use cases: UC-11; Initial Plan: ip.fr.11.2
- **srs.fr.33.11** When the user clicks New Chat, the panel shall start an empty chat; the previous chat shall be kept in History.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1
- **srs.fr.33.12** When the user picks a chat from History ▾, the panel shall reopen it with all its questions, answers and citations.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1

## US-33.3 Answers

**Story:** As a game developer, I want to follow citations and handle suggestions safely, so that I can check and use what the assistant says.

*Screens: C-14*

- **srs.fr.33.13** Each answer shall show its citations as chips that name a state ("Tick N" or "Start"), an entity or a file and line.
  - Use cases: UC-11; Initial Plan: ip.fr.11.3
- **srs.fr.33.14** When the user clicks a state citation, the Editor shall open the Telemetry panel on the cited run with the scrubber at that tick, or at Start for a Start citation; a state citation whose run has no recording shall be disabled, with the tooltip "No telemetry recorded for this run", and one whose run was deleted shall show "Run deleted" (srs.fr.31.17).
  - Use cases: UC-11; Initial Plan: ip.fr.11.3
- **srs.fr.33.15** When the user clicks an entity citation, the Editor shall act as stated in srs.fr.33.58.
  - Use cases: UC-11; Initial Plan: ip.fr.11.3
- **srs.fr.33.16** When the user clicks a file citation, the Editor shall open that file at that line in the external code editor, and show the toast "File changed since this answer" when the file's content hash differs from the cited one; when no file exists at the cited path, the Editor shall open nothing, keep the citation chip and show the toast "File not found: <path>", and it shall never open another file with the same name instead.
  - Use cases: UC-11; Initial Plan: ip.fr.11.3
- **srs.fr.33.17** When the assistant reports that information is unavailable, the panel shall show an "Information unavailable" card that suggests what to record to answer the question.
  - Use cases: UC-11; Initial Plan: ip.fr.11.5
- **srs.fr.33.18** A design suggestion shall be shown as a suggestion card with Copy and Dismiss buttons and shall never be applied to the project automatically.
  - Use cases: UC-11; Initial Plan: ip.fr.11.6, ip.fr.11.7
- **srs.fr.33.19** When the user clicks Copy on an answer or suggestion, the Editor shall copy its text to the clipboard.
  - Use cases: UC-11; Initial Plan: ip.fr.11.1
- **srs.fr.33.20** When the user clicks thumbs up or thumbs down on an answer, the Editor shall store that feedback in the user's local area of the project folder and send it nowhere.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1

## US-33.4 Service state and consent

**Story:** As a game developer, I want to see when the AI service is offline and approve what data is sent, so that nothing leaves my computer without my consent.

*Screens: C-14, F-11*

- **srs.fr.33.21** The panel shall show the AI service state as a green dot when connected and a grey dot when offline. The service shall be offline when the project opens and after the Service address changes, until the health check of srs.fr.33.30, which the Editor shall then run, answers SERVING; it shall become offline again when a health check fails or a question request fails because the service is unreachable or its deadline is exceeded. Test Connection shall stay available while offline, and a successful test shall make the service connected.
  - Use cases: UC-11; Initial Plan: ip.fr.11.8
- **srs.fr.33.22** While the AI service is offline, the panel shall disable Send, the Editor shall show the toast "AI service unavailable", and every way of sending a question (Send, Enter in the message box, Diagnose Last Run and Ask about Selection) shall send nothing and keep the question and its context chips in the message box.
  - Use cases: UC-11; Initial Plan: ip.fr.11.8
- **srs.fr.33.23** When a question would send source code or telemetry to the AI service, at any address including one on the same computer, the Editor shall decide for each of the two categories separately, by srs.fr.33.41, whether to show the AI data consent dialog (F-11) before sending.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1
- **srs.fr.33.24** The AI data consent dialog shall name the category it asks about and list exactly what of it would be sent: the file names and sizes for source code, or the runs, tick ranges and data kinds for telemetry, and the total size.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1
- **srs.fr.33.25** When the user clicks Allow Once, the Editor shall send that category's data for this question only; the next question shall be checked again.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1
- **srs.fr.33.26** When the user clicks Always Allow, the Editor shall send the data and turn on the Allow sending setting of that category only.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1
- **srs.fr.33.27** When the user clicks Deny, the Editor shall send the question without any data of that category, as defined in srs.fr.33.40, and show a note in the chat that the answer may be limited.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1, ip.fr.11.5

## US-33.5 AI and Privacy settings

**Story:** As a game developer, I want to configure the AI service and data sharing per project, so that each project uses the service and privacy level I choose.

*Screens: F-04*

- **srs.fr.33.28** The AI and Privacy tab shall have a Service address field that accepts hostname:port, IPv4-address:port or [IPv6-address]:port, where port is a whole number from 1 to 65 535 and an IPv6 address shall be written in square brackets; the Editor shall check the field when it loses focus and, when the address is not of one of these forms, show "Address must be host:port" under the field and keep using the previous address.
  - Use cases: UC-11; Initial Plan: ip.fr.11.8
- **srs.fr.33.29** The AI and Privacy tab shall have a Model ▾ field that lists, by display name, the models the service returns from the model-list operation of the AgentBridge service contract, each with an identifier and a display name; the Editor shall ask for the list when the tab opens and after each successful Test Connection, with the deadline of srs.fr.33.30. After a successful request, the selected model shall be kept only when its identifier is still in the list, and otherwise cleared with "Select a model" shown; an empty list shall show "No models available". While no model is selected, sending a question shall be disabled with the tooltip "Select a model". When the request fails, the field shall keep the selected model and show "Models unavailable: <reason>" under it.
  - Use cases: UC-11; Initial Plan: ip.fr.11.8
- **srs.fr.33.30** When the user clicks Test Connection, the Editor shall send the health check of srs.fr.13.63 to the service in the Service address field with a deadline of 5 seconds, and show a green "Connected · N ms", where N is the time until the service's answer, when the service answers SERVING, or otherwise a red error naming the failure as stated in srs.fr.13.63.
  - Use cases: UC-11; Initial Plan: ip.fr.11.8
- **srs.fr.33.31** The Allow sending source code setting shall be off by default; turning it on shall ask once for confirmation that names what will be sent, and shall leave the setting off if the user cancels.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1
- **srs.fr.33.32** The Allow sending telemetry setting shall be off by default; turning it on shall ask once for confirmation that names what will be sent, and shall leave the setting off if the user cancels.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1
- **srs.fr.33.33** When Ask before every send is on, the Editor shall show the consent dialog (F-11) before every question that would send source code or telemetry, even when that category's Allow sending setting is on; Ask before every send shall be off by default.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1

## US-33.6 Context content and consent rules

**Story:** As a game developer, I want to know exactly what each context item sends and when I am asked, so that no project data leaves my computer without my consent.

- **srs.fr.33.34** The Selection context shall contain, for each selected entity, its ID, name, tag, layer, parent and children IDs and every component with its field values and semantic description, and no source code; for a selected event or bug finding it shall contain that record, which counts as telemetry.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.35** The Last run context shall contain the latest run's test report and bug findings; the header, schema (srs.fr.8.29) and initial-state records of its recording, including the creation records with their colliders (srs.fr.8.30) and the debug injection records (srs.fr.12.16); its event records; and its telemetry samples for a tick range that defaults to the last 600 ticks of the run and can be changed on the chip. The tick range shall limit only events and samples, and the initial-state records shall always be included; all of it counts as telemetry.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.36** The Graph context shall contain the Semantic Graph nodes of the selected entities and their neighbours up to two relations away, or the whole graph when nothing is selected, limited to 500 nodes: the selected entities' nodes first, then nodes in increasing number of relations away, with ties and the whole graph in ascending node ID order; only edges between included nodes shall be sent, and the chip and the request preview shall show how many nodes were left out. Event nodes, *collided with* edges and *triggered* edges shall count as telemetry, and source file names in any node shall count as source code.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.37** The Source code context shall contain only the files listed on its chip: by default the files that define the component and system types of the selected entities, plus any file the user adds from the project's Scripts folder; it shall never contain files outside the Scripts folder, and it shall list no file by default when nothing is selected.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.38** When the attached source files exceed 200 KB in total, the Editor shall disable Send and show the total size and the limit under the chips.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.39** When the user clicks a chip, the panel shall show a preview of the exact content that chip adds to the request.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.40** When a category is denied or not allowed, the outgoing request shall contain none of its data: for source code no file contents, file names, paths, line numbers or excerpts, and for telemetry no samples, event records, test reports, bug findings, tick numbers, graph event nodes or event-derived edges; this shall also hold for data that would reach the request through another context item, automatic context such as AI ▸ Diagnose Last Run, previews or the citations of earlier answers, and earlier messages shall be handled as stated in srs.fr.33.56.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.41** For each category in a question, the Editor shall show the consent dialog when Ask before every send is on or that category's Allow sending setting is off, and otherwise send that category without asking; source code and telemetry shall be asked about and decided separately, source code first, and the answer for one shall not change the other.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.42** When the user closes the consent dialog with ✕ or Esc, the Editor shall send nothing and keep the question in the message box.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.43** For each sent question, the panel shall offer Show what was sent, which lists the context items, file names, tick ranges and sizes that the request contained.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1

## US-33.7 Typed text, chat history and destinations

**Story:** As a game developer, I want to to know exactly what consent covers, including my own text and old messages, so that nothing I deny reaches the service by another path.

- **srs.fr.33.44** The question text the user types shall be sent as typed, without filtering, and shall not be subject to consent; the request preview shall label it as the user's own text.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.45** Consent shall apply to every piece of project data that the Editor adds to a request: context items, automatic context such as Diagnose Last Run, and earlier messages of the chat that carried project data.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.46** The Editor shall store with each chat message the data categories it carries by srs.fr.33.59; when a category is denied or not allowed for a new question, earlier messages that carry it shall be left out of the request and replaced by the placeholder "[earlier message left out: contains <category>]".
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.47** Before sending, the panel shall offer Preview Request, which shows the exact text and data of the outgoing request after the consent rules are applied; Show what was sent (srs.fr.33.43) shall show the same content afterwards.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.48** Reopening a stored chat shall not send any of its earlier data except under the consent rules of srs.fr.33.41.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.49** The Allow sending settings shall belong to the service address for which they were given; when the Service address changes, the Editor shall turn both Allow sending settings off and ask for consent again on the next question that would send project data.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.50** The Asset context (srs.fr.23.36) shall contain the asset's name, path, type, size and import settings and never the file's contents, also for a script asset; a script asset's name and path shall count as source code, and the fields of any other asset shall count as neither category.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.51** The Log entry context (srs.fr.24.18) shall contain the entry's level, message and tick, and its stack trace when it has one; the stack trace, which names source files and lines, shall count as source code, and an entry logged during a run shall count as telemetry.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.52** The Component type context (srs.fr.26.17) shall contain the type's name, semantic description and fields and the systems that read and write it, and no source code.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1
- **srs.fr.33.53** The Graph nodes context (srs.fr.32.13) shall contain the selected nodes and the edges between them, classified as in srs.fr.33.36: event nodes, *collided with* edges and *triggered* edges shall count as telemetry, and source file names in any node shall count as source code.
  - Use cases: UC-11; Initial Plan: ip.fr.11.4, ip.nfr.2.1

## US-33.8 Field-level classification and history order

**Story:** As a game developer, I want to consent to apply field by field and to old messages in a fixed order, so that denied data never slips through by another path.

- **srs.fr.33.54** The Editor shall classify every field of every structured context item, preview and citation as source code, telemetry or neither, using srs.fr.33.34 to srs.fr.33.37 and srs.fr.33.50 to srs.fr.33.53, and shall remove the fields of a denied category one by one before sending; earlier messages shall instead be kept or left out whole by srs.fr.33.56, and the current typed question shall always be sent (srs.fr.33.44).
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1
- **srs.fr.33.55** A log entry's message shall count as source code when it names a source file or line, and as telemetry when it was logged during a run.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1
- **srs.fr.33.56** For earlier messages of a chat, the Editor shall apply these rules in order: (1) attachments of earlier questions shall never be resent; (2) an earlier answer shall carry the categories of srs.fr.33.59 and shall be left out when any of them is denied or not allowed; (3) an earlier question's typed text shall be sent unless its message carries a category that is now denied, in which case it shall be left out (srs.fr.33.46); (4) the current typed question shall always be sent (srs.fr.33.44).
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1

## US-33.9 AI settings scope

**Story:** As a game developer, I want to AI settings to be personal and to apply at once, so that changing the AI service never touches shared project files.

- **srs.fr.33.57** Every setting of the AI and Privacy tab (Service address, Model, the Allow sending settings and Ask before every send) shall be stored for the current user in that project, outside the shared project settings, with the Hub's default AI service address (srs.fr.13.62) as the Service address of a new project.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1

## US-33.10 Entity citation navigation

**Story:** As a game developer, I want an entity citation always to lead somewhere sensible, so that it never selects an unrelated current entity.

- **srs.fr.33.58** When the user clicks an entity citation, the Editor shall navigate to it as stated in srs.fr.35.20, using the citation's run number and tick, where a citation marked as created at runtime never matches a current entity.
  - Use cases: UC-11; Initial Plan: ip.fr.11.3

## US-33.11 History categories

**Story:** As a game developer, I want to denied data never to come back through an answer derived from it, so that consent holds through the whole chat.

- **srs.fr.33.59** A chat message shall carry the union of the categories of its own attachments and citations and of every earlier message that its request included; an answer shall also carry every category of the request it answers, including the attachments of its question, whether or not it cites them, so that categories pass on transitively through the chat history.
  - Use cases: UC-11; Initial Plan: ip.nfr.2.1
