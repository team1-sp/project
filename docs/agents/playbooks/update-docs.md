# Playbook — Update Documentation

Docs are code: wrong docs are bugs. See also [guidelines/documentation-rules.md](../../guidelines/documentation-rules.md).

## When you must update docs

| You changed… | Update… |
| --- | --- |
| Public interface of a layer / adapter | `architecture/*`, `design/physics-adapter-design.md` or relevant design doc |
| A component, event or system | `design/ecs-design.md`, `design/telemetry-and-events.md` |
| Build/run steps | `troubleshooting/environment-setup.md` |
| Frame order | `architecture/engine-layer.md` + decision entry (rule R-9) |
| A human decision (any) | Follow [record-decision.md](record-decision.md): log it, then update **every** doc that depended on the old belief |
| Project state | `agents/state/*` |

## Steps

1. Identify every doc that mentions what you changed (grep the old name/behaviour).
2. Edit with the smallest change that makes it true. Don't restyle.
3. If the doc has a change table, add a row (date, summary, agent).
4. If the doc is human-owned (`architecture/`, `design/`, `guidelines/`, `process/`) and the change is *not* a consequence of an `Accepted` decision, **do not decide it yourself**: tell the human (`DECISION NEEDED`), log a `Proposed` decision and add a `Q-###`.
5. Check links still resolve (relative paths).
6. For state docs update `last_updated` / `updated_by`.

## Tone and shape

- Imperative, short sentences, tables for comparisons, fenced blocks for diagrams/commands.
- Prefer ASCII diagrams (they survive any viewer and any model).
- Use glossary terms. Tag certainty (`[TBD]`, `[VERIFIED]`).
- Never document intended behaviour as if it existed; say "planned" or `[PROPOSED]`.

## Don't touch

- `senior-project/docs/` course deliverables (rule S-3). They are not the truth; list conflicts for the humans.
- Past entries in `history/` (rule P-6).
