# Design Doc Template

Copy to `design/<topic>.md`. Keep it to 1–3 pages. Delete guidance text.

```markdown
# <Topic> Design

- **Status:** Draft | In review | Accepted | Implemented | Superseded
- **Author(s):** <name / agent-id>   **Reviewers:** <names>
- **Layer:** Engine | Semantic | AI
- **Serves:** ip.fr.x.y, ...        **Decisions:** DEC-###   **Tasks:** TASK-###

## 1. Problem
What are we solving and why now? (3–5 sentences)

## 2. Goals / Non-goals
- Goals: ...
- Non-goals: ...

## 3. Design
### 3.1 Overview (ASCII diagram)
### 3.2 Data (components, events, formats) — reads / writes / emits
### 3.3 Interfaces (signatures, language-neutral)
### 3.4 Placement in frame order (and why)
### 3.5 Headless behaviour (must be identical)
### 3.6 Telemetry (what is recorded, cost)

## 4. Alternatives considered
| Option | Pros | Cons | Why not |

## 5. Risks and open questions
- [TBD] ...

## 6. Test plan
Which tests (unit / adapter / integration / scenario) prove it; commands.

## 7. Rollout
Steps, order, migration; docs to update.

## 8. Change table
| Date | Change | By |
```

Checklist before "Accepted": invariants respected (R-1…R-9) · telemetry impact considered · headless identical · tests listed · decision entries created for any technology/architecture choice.
