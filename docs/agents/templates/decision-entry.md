# Template — Decision entry (ADR style)

Append to `history/decision-log.md` (and add a row to its index table). **Humans decide; agents consult.** Pick the status with the table in [../playbooks/record-decision.md](../playbooks/record-decision.md#proposed-or-accepted) (DEC-052):

- `Proposed`: you asked the human in the conversation (`DECISION NEEDED`, `Q-###`) and are still waiting for the answer.
- `Accepted` with `Approval: conversation`: the human answered in the conversation (record who, when, their words).
- `Accepted` with `Approval: review of PR #<n>`: the entry is part of a pull request and is listed under "Decisions for review" in its body. The reviewer treats it as proposed; it binds only when a human merges the PR. If the reviewer rejects or changes it, fix the entry on the branch before the merge.

After `Accepted`, run [../playbooks/record-decision.md](../playbooks/record-decision.md) so every document agrees.

Never delete or rewrite an entry once it is on `main`. Until its pull request is merged, an entry may still be changed on its branch, for example after review feedback. To reverse one on `main`, create a new entry and set the old status to `Superseded by DEC-###`. The only other permitted edit to an old entry is appending a dated line to its `Amendments` field (human-supplied rationale, a pinned version, a confirmation) — never changing what was decided.

```markdown
### DEC-### — Decision title (a statement, not a question)
- **Date:** YYYY-MM-DD
- **Status:** Proposed | Accepted | Rejected | Superseded by DEC-###
- **Decided by:** <human name(s); for `review of PR #<n>`, the human whose choice it is, or "the PR reviewer" when the PR puts it up for review>   **Recorded by:** <agent-id or human>
- **Approval:** conversation | review of PR #<n> (binds when a human merges the PR) (omit while Proposed)
- **Human's words:** "<short quote or faithful paraphrase of the decision>" (omit while Proposed, and when the approval is the PR review and nobody chose it in the conversation)
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
