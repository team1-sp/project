# GenAI Usage Rules (course requirement)

Source: Initial Plan template (CTIS411) as quoted in the team's materials. **Re-check the GenAI usage document on Moodle for the exact colouring rules — this file summarises, it does not replace it.**

## What the course says

- GenAI **is allowed** when preparing project reports, **if** the AI-generated parts are clearly specified by **text colouring** (rules in the Moodle GenAI usage document) **and** a discussion paragraph ends every section where GenAI was used, explaining the prompts used and how the text was edited into the overall flow.
- A section without GenAI states: "No GenAI was used in this section."
- AI-generated text used **without** clear marking is considered **plagiarism** (failing the course and disciplinary process).

## Rules for agents (S-4)

1. If your output may be pasted into a graded document, say so at the top of your answer and **do not** present it as final team prose.
2. Record each such use in `history/genai-usage-log.md` (GEN-###): date, section, tool/model, prompt, what was generated.
3. Never remove or alter an existing colour marking or disclosure paragraph in a deliverable.
4. Do not generate disclosure paragraphs that misstate what was done. The humans who edited the text must supply/confirm how it was edited.
5. Documentation in `project/docs/` is **working documentation**, not a graded deliverable, but treat anything copied from it into a deliverable as GenAI-generated.
6. Code written with AI help is not covered by the text-colouring rule, but is logged in handoffs; the team should check the course policy for code and log significant AI-generated code in `genai-usage-log.md` if required `[TBD — ask advisor/Moodle]`.

## Rules for humans

- Log the prompt you used before you forget it.
- Colour-mark AI text in the deliverable per Moodle rules; add the end-of-section discussion.
- Edit AI text so that it is accurate — requirements and decisions in deliverables must match the repo.

## Quick checklist when an AI helped with deliverable text

- [ ] GEN-### entry written
- [ ] Text colour-marked in the document
- [ ] Discussion paragraph at the end of the section (prompts + edits)
- [ ] Facts checked against `decision-log.md` / requirements
