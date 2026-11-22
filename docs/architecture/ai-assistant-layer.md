# AI Assistant Layer

Uses the semantic graph and project data to explain, diagnose and help with design. Sub-layers: **Architecture reading → LLM model → User assistant**.

Status: **US-11.2 core implemented**; grounded questions, the chat UI and settings remain planned.
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

The concrete project path, service address/model selection, transmission-policy values and UI are
intentionally left to their own project/settings and panel stories (#121 and #119). Citation
interpretation and grounded diagnosis remain #71; US-11.2 stores opaque citation kind/payload pairs
without inventing them.
