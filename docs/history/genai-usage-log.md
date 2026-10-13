# GenAI Usage Log

**Append-only.** The course (CTIS411) allows GenAI in project reports **only if** AI-generated parts are clearly marked (colour coding per the Moodle GenAI usage document) and every section where it was used ends with a discussion of the prompts and how the text was edited. Unmarked AI text in a graded document is treated as plagiarism. This log is the working record that makes the end-of-section disclosures easy and accurate.

Rules: see [../guidelines/genai-usage-rules.md](../guidelines/genai-usage-rules.md). This log covers **text for course deliverables** and, optionally, significant AI-generated code (for the team's own transparency). Routine agent work on the codebase is tracked in `handoff-log.md`.

## Entry format

```markdown
### GEN-### — Short title
- **Date:** YYYY-MM-DD
- **Deliverable / section:** e.g. Initial Plan §4.1, or "code: PhysicsAdapter"
- **Tool and model:** e.g. Claude Sonnet 5.5, ChatGPT, Gemini Pro
- **Prompt(s) (verbatim or faithful summary):** ...
- **What was generated:** ...
- **How it was edited by the team:** ...
- **Human owner who takes responsibility:** name
- **Marked in the document?** Yes (colour) / No — must fix
```

## Entries

### GEN-001 — Developer documentation scaffold (`project/docs/`)
- **Date:** 2026-10-12
- **Deliverable / section:** None — working documentation for developers and agents; **not** a course deliverable.
- **Tool and model:** Claude Sonnet 5.5 (Claude Code session)
- **Prompt (summary):** Create structured .md docs for architecture, rules, design, process, testing, error solving, agent-only coordination docs, a problem history log and a decision history log.
- **What was generated:** The whole `project/docs/` tree.
- **How it was edited by the team:** `[TBD]` pending review.
- **Human owner who takes responsibility:** Işık Dönger (requester) `[confirm]`
- **Marked in the document?** n/a (not a graded deliverable). If any of this text is later reused in a graded document, it must be marked and logged again.

---
