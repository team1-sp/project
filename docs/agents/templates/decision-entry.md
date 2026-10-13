# Template — Decision entry (ADR style)

Append to `history/decision-log.md` (and add a row to its index table). **Humans decide; agents consult.** An agent writes `Accepted` only to record an explicit human decision (with who/when/quote); otherwise it writes `Proposed`, which binds nobody. After `Accepted`, run [../playbooks/record-decision.md](../playbooks/record-decision.md) so every document agrees.

Never delete or rewrite an entry. To reverse one, create a new entry and set the old status to `Superseded by DEC-###`. The only other permitted edit to an old entry is appending a dated line to its `Amendments` field (human-supplied rationale, a pinned version, a confirmation) — never changing what was decided.

```markdown
### DEC-### — Decision title (a statement, not a question)
- **Date:** YYYY-MM-DD
- **Status:** Proposed | Accepted | Rejected | Superseded by DEC-###
- **Decided by:** <human name(s)>   **Recorded by:** <agent-id or human>
- **Human's words:** "<short quote or faithful paraphrase of the decision>" (omit while Proposed)
- **Context:** the situation and forces. Link tasks / questions / requirement IDs (reference only).
- **Options considered:**
  1. Option A — pros / cons
  2. Option B — pros / cons
- **Decision:** what we will do, in one or two sentences.
- **Rationale:** why, for this project (senior project, limited time, 2D scope, headless/AI-native needs). Write `[rationale not given — ask]` if the human did not say.
- **Consequences:** what becomes easier/harder; what must change in code/docs.
- **Docs updated:** list of project files changed to match (filled during propagation).
- **Course documents affected (for humans):** files under `senior-project/docs/` that now disagree. Agents do not edit them.
- **Revisit when:** trigger or date.
- **Supersedes / Related:** DEC-###, TASK-###, Q-###, PRB-###
- **Amendments:** (dated lines added later, e.g. `2026-10-14 — rationale given by <human>: …`)
```
