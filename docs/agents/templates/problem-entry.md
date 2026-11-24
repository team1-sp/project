# Template — Problem entry

Append to the bottom of the area file in [`history/problems/`](../../history/problems/README.md) (`build-and-tooling.md`, `engine.md`, `ai.md`, `editor.md`, …) and add a row with a link to that file at the bottom of the index in `history/problems/README.md`. The id is `PRB-<n>.<k>`: `<n>` is the issue your PR closes (or the PR's number for no-issue work), `<k>` counts from 1 (DEC-148.1). Search the folder **first** — if the problem is a repeat, add an `Occurrence` line to the existing entry instead of creating a new one.

```markdown
### PRB-<n>.<k> — Short symptom-based title
- **Date:** YYYY-MM-DD
- **Reported by:** <agent-id or human>
- **Area:** ECS | Physics | Adapter | Rendering | Input | Gameplay | Telemetry | Headless | Semantic | AI | Build | Tooling | Docs | Process
- **Status:** Solved | Workaround | Open
- **Symptom:** what was observed. Paste the exact error text in a code block.
- **Context:** versions, OS, mode (normal/headless), commit/branch, how to reproduce.
- **Root cause:** the real reason (or "unknown" if Status ≠ Solved).
- **Solution:** exactly what fixed it (code/config/command). Include the minimal diff or snippet.
- **How verified:** command/test that shows it is fixed.
- **Prevention:** test, assertion, doc or rule added so it cannot silently return.
- **Keywords:** comma-separated: exact error strings, component names, library names.
- **Related:** #issue, DEC-…, other PRB-…
- **Occurrences:** (append "YYYY-MM-DD — agent — note" if it happens again)
```

Quality bar: someone who never saw the original session can fix the same problem in minutes using only this entry.
