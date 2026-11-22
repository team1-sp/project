# AI Assistant Layer

Uses the semantic graph and project data to explain, diagnose and help with design. Sub-layers: **Architecture reading → LLM model → User assistant**.

Status: **US-11.2 core, US-33.5 AI/privacy settings, US-11.1 grounded answers and the US-33.2 chat
state implemented**; consent dialogs and offline handling remain planned.
The Semantic layer has no query implementation yet, so US-11.2 depends only on the minimal
read-only `SemanticGraphView` structure seam (DEC-067).

## 1. Capabilities (ip.fr.11.*)

| Capability | Requirement |
| --- | --- |
| Accept natural-language questions via a chat interface | ip.fr.11.1 |
| Answer questions about current/recorded engine state, e.g. "Why did the door not open?" | ip.fr.11.2 |
| Diagnose a reported bug from telemetry and event history, **citing frames and entities** | ip.fr.11.3 |
| Use the project's source code as extra context | ip.fr.11.4 |
| Say "information unavailable" instead of guessing | ip.fr.11.5 |
| **Never modify engine state or source code** | ip.fr.11.6 |
| Suggest design changes from graph structure (should) | ip.fr.11.7 |
| Get LLM responses from an external service over gRPC (may) | ip.fr.11.8 |

## 2. Sub-layers

```
User assistant layer      chat UI, conversation, formatting, citations
        ↓
LLM model layer           prompt assembly, model provider (local or cloud), guardrails
        ↓
Architecture reading layer graph queries + source retrieval → structured context
        ↓
Semantic graph / telemetry / source code
```

- **Architecture reading:** turns a question into graph queries (entities, components, events, frames) and source lookups; returns a compact, cited context.
- **LLM model:** receives only the compact context + the question. Must be instructed to cite frames/entities and to abstain when evidence is missing.
- **User assistant:** presents answer + evidence (frame numbers, entity names) so the user can verify.

## 3. Principles

1. **Evidence first.** An answer without a frame/entity citation is only allowed when explicitly labelled as general knowledge.
2. **Abstain over guess** (ip.fr.11.5).
3. **Read-only** toward the engine and source (ip.fr.11.6). Any "suggested change" is text for a human.
4. **Privacy default-off** for external services: no source code or telemetry leaves the machine unless the user enabled it in configuration (ip.nfr.2.1).
5. **Latency target:** answer within 30 s for ≥ 90% of questions, excluding service outages (ip.nfr.1.5).
6. **Model-agnostic.** Provider behind an interface so a local model or a different vendor can be used.
7. **No own model; user-supplied provider** (DEC-045). We do not build or train a model. Users bring their own provider account and API key (stored in the OS credential store); the project guarantees only free-tier or local use.

## 4. Techniques (what each is for)

| Technique | Use for | Not for |
| --- | --- | --- |
| LLM | Understanding questions, explaining evidence, suggesting repairs | Being the source of truth about engine state |
| Graph queries | Facts about entities/events/frames | Ambiguous prose interpretation |
| Embeddings / retrieval (RAG) | Finding relevant source/design documents | Exact state queries |
| Classical code analysis | Reading symbols, component access, system relationships | Interpreting lore |
| Reinforcement learning | Automated Playtest Agent only | The narrative checker |
| Fine-tuning | Not needed initially | — |

## 5. Suggested build order `[PROPOSED]` (see DEC-010; protocol decided in DEC-038)

1. Local script that loads a telemetry export + graph and answers 3 fixed questions without an LLM.
2. Add LLM with the compact-context prompt.
3. Add source-code retrieval.
4. Chat UI.
5. Engine ↔ AI service contract: gRPC/protobuf behind `AgentBridge` (DEC-038).
6. Optional: MCP server exposing project tools to external AI clients.

## 6. Narrative Design Assistant (stretch)

LLM + RAG over game-design documents, cross-referenced with the semantic graph to audit inconsistencies between story and active rules ("Ludonarrative Checker"). Not part of the MVP; do not start without a plan entry.

## 7. How to evaluate

Use the demo game's injected bugs as ground truth: for each bug, ask the assistant the natural question ("Why can the player open the door with no key?") and check (a) correct diagnosis, (b) correct cited frames/entities, (c) correct abstention when evidence is missing. Record results in `testing/`.

## 8. Implemented US-11.2 boundary

The window-free C++ target `aegis::assistant` implements the contract needed before the UI and
grounded-diagnosis stories:

- `ReadOnlyAssistant` receives only a `const SemanticGraphView`, read-only
  `DataTransmissionPolicy`, `AgentBridge` and `ChatStore`. It has no `World`, project-source writer
  or general project filesystem capability. It rejects a request before AgentBridge sees any data
  category the policy disallows. A design suggestion is
  stored as `MessageContent::DesignSuggestion` text for manual application; there is no execute or
  apply operation (srs.fr.11.8–11.9).
- A configured question calls the `AgentBridge::startAssistantResponse()` server-streaming operation.
  Protocol-library types stay inside optional `engine::bridge_grpc`; the production protobuf contract
  is `proto/agent_bridge.proto`. No service means no request or chat mutation (srs.fr.11.10, DEC-038).
- `AnswerOperation::cancel()` changes a streaming answer to `Stopped`, persists the part received and
  calls the stream's idempotent cancellation. The gRPC implementation maps this to
  `ClientContext::TryCancel()`; cancellation before consume, during streaming and after completion is
  safe (srs.fr.11.11).
- `ChatStore` writes format `AEGIS_CHAT 1` beneath
  `<UserProjectPaths::userLocalArea()>/assistant/chats`. Every message records role, state, content
  kind, text, citations and data-category names. Replacement uses a temporary file and backup;
  corrupt or unsupported existing files are rejected rather than overwritten (srs.fr.11.12).

US-33.5 supplies the validated address, model identifier and `DataTransmissionPolicy` implementation
from a window-free controller. It extends the transport-neutral bridge with the standard gRPC health
check and model discovery, keeps transport work off the UI thread, and persists no secrets under the
injected user-local project settings area. See
[`design/ai-privacy-settings.md`](../design/ai-privacy-settings.md). The per-question consent dialog
remains #120; citation interpretation and grounded diagnosis are US-11.1 (§9).

## 9. Implemented US-11.1 grounded answers

`aegis::assistant` adds the grounded pipeline under `assistant/include/aegis/assistant/grounded/`
(DEC-070). Each stage is a separate unit:

| Stage | Unit | What it does |
| --- | --- | --- |
| Question model | `GroundedQuestion`, `ContextItem` | Typed question text plus the attached items: Selection, Last run, Graph, Graph nodes, Source code, Asset, Log entry, Component type (srs.fr.11.1, 11.6). |
| Evidence retrieval | `SemanticGraphQuery`, `RunTelemetryQuery`, `AssetCatalogQuery`, `ProjectSourceReader`; `assembleContext()` | Const query seams returning copies; called only for attached items. Enforces the policy per category, the `Scripts/` allowlist and the 200 KB limit, limits Last run to its tick range (default 600 ticks, Start always kept), and applies the earlier-message consent rules (srs.fr.11.5, 11.6, 33.46, 33.56, 33.59). `ScriptsFolderReader` is the production file reader; it never writes and refuses paths that resolve outside `Scripts/`. |
| Diagnosis | `diagnose()` | Deterministic findings from records only: unlock failures from event payloads, value changes from handler write records, and debug-flag assessments (not applied, not recorded, not exercised, effect not observable, cause). Declarations alone become labelled hypotheses; missing records are named (srs.fr.11.2, 11.3, 11.7). |
| Citations | `StateCitation`, `EntityCitation`, `SourceCitation`; `encodeCitation()`/`decodeCitation()`; `checkCitation()` | Typed values; Start is distinct from tick 0; source citations carry the `sha256:` content hash at question time. A citation is returned only if it matches retrieved evidence (srs.fr.11.4). |
| Formatting | `formatDiagnosis()`, `renderContextDocument()` | The deterministic answer text and the versioned `AEGIS_GROUNDED_CONTEXT 1` document for the AI service. |
| Transport | `GroundedAgentBridge`, `GroundedAssistant` | The chat-facing API. `ask()` assembles, diagnoses, then runs US-11.2's `ReadOnlyAssistant` through a decorator of the configured bridge that sends the document in `semanticGraphStructure`, streams the diagnosis first and drops AI-service citations that do not match the evidence. |

The concrete Semantic Graph, telemetry recording, asset catalogue and the EP-33 context chips are
not built yet; they implement these seams in their own stories. Tests use deterministic fakes and no
network or model. The production `DataTransmissionPolicy` is US-33.5's `AiPrivacySettings`;
`GroundedAgentBridge` forwards `checkHealth()` and `listModels()` unchanged to the bridge it wraps.
Consent dialogs, field-by-field removal of denied data (srs.fr.33.54) and request preview remain
#120 and EP-33; the chat panel is US-33.2 (§10).

## 10. Implemented US-33.2 chat panel state

The Editor's AI Assistant panel keeps its state in `editor_core` (DEC-071), window-free and tested
headlessly; `assistant_chat_panel.cpp` only draws it with Dear ImGui.

| Unit | What it does |
| --- | --- |
| `AssistantChat` | Composer draft (Enter sends, Shift+Enter is a new line, whitespace-only is empty), Send/Stop state, one running answer at a time, New Chat and History. A rejected send keeps the draft and chips; a successful one clears the draft and keeps the chips. |
| `ContextChipSet`, `ContextChip` | One chip per US-11.1 `ContextItem` type with a stable id, label, detail and the bytes it adds to the request; changes return a new set. The + menu lists only types not attached. |
| `IChatQuestionSender` | The only sending seam, so US-33.3 can add consent and offline rules around it. `GroundedChatQuestionSender` calls `GroundedAssistant::ask()` and runs US-11.2's `AnswerOperation::run()` on a worker thread; Stop calls `AnswerOperation::cancel()` once. |
| `ChatStoreLoader` | History reopens chats through US-11.2's `ChatStore::load()`; citations stay `{kind, payload}` and are decoded with `decodeCitation()` only for display. |
| `GroundedContextSizeEstimator` | A chip's size is the length of the `renderContextDocument()` output with the item minus without it, both without a diagnosis; it reads only the const sources. |
| `EditorContextItemProvider` | Selection, Graph, Last run and the selected Console entry; the other types have no Editor source yet. |

While an answer runs, the panel draws every message except the one the worker thread writes, then
the whole chat once the operation has finished; switching chats waits for the answer to end. The
History list holds the chats opened in this Editor session; `ChatStore` has no listing operation, so
chats saved in earlier sessions are not listed yet.
