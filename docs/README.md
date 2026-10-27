# AI-Native Game Engine — Project Documentation

> Location: `project/docs/` (the engine/tooling repository).
> The course deliverables (Initial Plan, etc.) live in the **separate** `senior-project/docs/` repository. This folder is the *working* documentation for building the product.

This is the single entry point for every human developer and every AI agent working on the project.

## What the project is (30-second version)

A 2D game engine prototype designed to be **AI-native**. Three layers, strictly one-directional dependencies:

```
AI Assistant Layer          reads the graph + source, explains / diagnoses / suggests
        ▲
Semantic Architecture Layer turns engine runtime data into a graph
        ▲
Engine Layer                game loop, ECS, physics (via adapter), rendering, UI
```

The product is **2D only** (DEC-016); the architecture keeps a later 3D extension possible. Stack: **C++, Box2D, Raylib** (DEC-013…015). The product's unique value is the AI-native architecture (semantic graph + assistant + headless automated playtesting), not "yet another game engine". Full detail: [architecture/system-overview.md](architecture/system-overview.md).

## Start here

| You are… | Read this first |
| --- | --- |
| **An AI agent / model** | [agents/AGENT-PROTOCOL.md](agents/AGENT-PROTOCOL.md) — mandatory, every session |
| A new human developer | [architecture/system-overview.md](architecture/system-overview.md) → [guidelines/repository-structure.md](guidelines/repository-structure.md) → [process/development-process.md](process/development-process.md) |
| Debugging something | [troubleshooting/debugging-playbook.md](troubleshooting/debugging-playbook.md), then search [history/problem-log.md](history/problem-log.md) |
| Wondering "why did we do it this way?" | [history/decision-log.md](history/decision-log.md) |
| Looking for current progress | [agents/state/current-status.md](agents/state/current-status.md) |

## Folder map

```
docs/
├── README.md                      ← you are here
├── architecture/                  WHAT the system is
│   ├── system-overview.md           three layers, data flow, invariants
│   ├── engine-layer.md              loop, ECS, physics, rendering, UI, scenes
│   ├── semantic-layer.md            data / graph / relationships
│   ├── ai-assistant-layer.md        reader / LLM / user assistant
│   ├── tech-stack.md                options, status of each technology choice
│   └── glossary.md                  canonical terminology (use these words)
├── design/                        HOW specific parts are designed
│   ├── design-principles.md
│   ├── ecs-design.md
│   ├── physics-adapter-design.md
│   ├── telemetry-and-events.md
│   └── templates/design-doc-template.md
├── guidelines/                    RULES for writing code, git, docs
│   ├── coding-standards.md
│   ├── repository-structure.md
│   ├── git-workflow.md
│   ├── documentation-rules.md
│   └── genai-usage-rules.md        course rules on disclosing AI-generated text
├── process/                       HOW the team works
│   ├── development-process.md       Scrum adaptation, sprint rhythm
│   ├── roadmap.md                  phases and exit criteria
│   ├── definition-of-done.md
│   └── code-review.md
├── testing/                       HOW we verify
│   ├── test-strategy.md
│   ├── test-checklists.md
│   ├── headless-testing.md
│   └── benchmark-protocol.md       graphics framework / shader benchmark
├── troubleshooting/               HOW we fix things
│   ├── debugging-playbook.md
│   ├── common-errors.md            anticipated pitfalls (not yet encountered)
│   └── environment-setup.md
├── history/                       APPEND-ONLY memory of the project
│   ├── problem-log.md              every problem actually hit + its solution
│   ├── decision-log.md             every decision + rationale (ADR style)
│   └── genai-usage-log.md          AI usage disclosures required by the course
└── agents/                        AGENT-ONLY: coordination between models
    ├── README.md
    ├── AGENT-PROTOCOL.md           exactly how to approach every session
    ├── rules.md                    hard rules (MUST / MUST NOT)
    ├── communication-protocol.md   how agents talk to each other through files
    ├── model-compatibility.md      how to write/read docs so any model can use them
    ├── state/                      LIVE shared state
    │   ├── current-status.md
    │   ├── plan.md
    │   ├── task-board.md
    │   ├── handoff-log.md
    │   └── open-questions.md
    ├── playbooks/                  step-by-step approach per kind of work
    │   ├── implement-feature.md
    │   ├── fix-bug.md
    │   ├── record-decision.md      human decision → log → update every doc
    │   ├── write-tests.md
    │   └── update-docs.md
    └── templates/                  copy-paste entry formats
        ├── task.md
        ├── handoff.md
        ├── problem-entry.md
        └── decision-entry.md
```

## Conventions used everywhere

- **Language:** English. (Team-facing educational guides outside this folder may be Turkish; code, comments, commits and these docs are English.)
- **Terminology:** use the words in [architecture/glossary.md](architecture/glossary.md). Do not invent synonyms ("physics library" ≠ "physics engine" ≠ "physics adapter").
- **IDs:** `TASK-###`, `DEC-###`, `PRB-###`, `HND-###`, `Q-###`, `GEN-###`. Zero-padded, never reused.
- **Dates:** ISO `YYYY-MM-DD`.
- **Certainty tags:** `[VERIFIED]` (checked against code/docs/run), `[ASSUMPTION]`, `[TBD]` (decision not made yet), `[PROPOSED]` (suggested, not accepted).
- **Source of truth:** the `Accepted` entries in [history/decision-log.md](history/decision-log.md). Humans decide, agents consult (DEC-017). Course documents under `senior-project/docs/` (Initial Plan etc.) are **not** authoritative.
- **Requirement references:** the Initial Plan's requirement IDs `ip.fr.*` / `ip.nfr.*` are a *reference snapshot*. Cite them (e.g. `ip.fr.3.4`) for convenience; if one conflicts with an accepted decision, the decision wins.
- **Append-only files** (`history/*`, `agents/state/handoff-log.md`) are never rewritten, only appended to. Corrections are new entries that reference the old one.
- **Doc size:** keep each file under ~400 lines; split instead of growing.

## Who owns what

| Area | Owner | Agents may… |
| --- | --- | --- |
| `architecture/`, `design/` | Team (humans decide) | propose via `decision-log` (`Proposed`); edit to match an `Accepted` decision (propagation) |
| `guidelines/`, `process/` | Team | same as above |
| `testing/`, `troubleshooting/` | Shared | edit freely, log changes in the file's change table |
| `history/` | Shared, append-only | append |
| `agents/` | Agents (humans may edit) | edit per [agents/communication-protocol.md](agents/communication-protocol.md) |

## Team and advisor (for context)

Team 1: Işık Dönger (Project Manager), Arda Sarıtaş (Scrum Master), Arda Karabulut, Elifnur Özçelik, Zeynep Ertürk. Advisor / Product Owner: Leyla Sezer. Source: Initial Plan §3, §5, repo README.
