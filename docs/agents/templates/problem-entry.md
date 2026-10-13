# Template — Problem entry

Append to `history/problem-log.md` (and add a row to its index table). Search the log **first** — if the problem is a repeat, add an `Occurrence` line to the existing entry instead of creating a new one.

```markdown
### PRB-### — Short symptom-based title
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
- **Related:** TASK-###, DEC-###, other PRB-###
- **Occurrences:** (append "YYYY-MM-DD — agent — note" if it happens again)
```

Quality bar: someone who never saw the original session can fix the same problem in minutes using only this entry.
