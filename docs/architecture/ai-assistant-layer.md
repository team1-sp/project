# AI Assistant Layer

Uses the semantic graph and project data to explain, diagnose and help with design. Sub-layers: **Architecture reading → LLM model → User assistant**.

Status: **planned** (roadmap phase 12). Do not build before the Semantic layer provides a query API.

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

## 4. Techniques (what each is for)

| Technique | Use for | Not for |
| --- | --- | --- |
| LLM | Understanding questions, explaining evidence, suggesting repairs | Being the source of truth about engine state |
| Graph queries | Facts about entities/events/frames | Ambiguous prose interpretation |
| Embeddings / retrieval (RAG) | Finding relevant source/design documents | Exact state queries |
| Classical code analysis | Reading symbols, component access, system relationships | Interpreting lore |
| Reinforcement learning | Automated Playtest Agent only | The narrative checker |
| Fine-tuning | Not needed initially | — |

## 5. Suggested build order `[PROPOSED]` (see DEC-010, DEC-011)

1. Local script that loads a telemetry export + graph and answers 3 fixed questions without an LLM.
2. Add LLM with the compact-context prompt.
3. Add source-code retrieval.
4. Chat UI.
5. Engine ↔ AI service contract (gRPC/protobuf) if the AI service is a separate process.
6. Optional: MCP server exposing project tools to external AI clients.

## 6. Narrative Design Assistant (stretch)

LLM + RAG over game-design documents, cross-referenced with the semantic graph to audit inconsistencies between story and active rules ("Ludonarrative Checker"). Not part of the MVP; do not start without a plan entry.

## 7. How to evaluate

Use the demo game's injected bugs as ground truth: for each bug, ask the assistant the natural question ("Why can the player open the door with no key?") and check (a) correct diagnosis, (b) correct cited frames/entities, (c) correct abstention when evidence is missing. Record results in `testing/`.
