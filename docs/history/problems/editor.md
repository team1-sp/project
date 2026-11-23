# Problems: Editor

**Append-only.** Scope: Editor panels and models, Dear ImGui, the assistant UI. How to use this folder: [README.md](README.md).

---

### PRB-007 — Chip sizes were missing or zero when measured through GroundedAssistant::prepare()
- **Date:** 2026-11-22
- **Area:** Editor / Assistant
- **Status:** Solved
- **Symptom:** `AssistantChatGroundedTest.ChipSizesAreTheBytesTheItemAddsToTheContextDocument` failed
  first with `Value of: source.has_value() Actual: false Expected: true`, then with
  `Expected: (*source) >= (std::string("bool unlocked = false;\n").size()), actual: 0 vs 23`.
- **Context:** US-33.2 `GroundedContextSizeEstimator`, headless preset.
- **Root cause:** `assembleContext()` refuses an empty question (`question is empty`); after giving
  it a text, the diagnosis in the context document shortened when evidence was attached, cancelling
  the source file's bytes.
- **Solution:** measure with a fixed placeholder question and render both documents with
  `renderContextDocument(context, Diagnosis{})`, so only the item's evidence differs.
- **How verified:** the test passes; headless 258/258 and macOS Debug 271/271 with warnings as errors.
- **Prevention:** measure one item's contribution against an otherwise identical document.
- **Keywords:** context chip size, assembleContext, question is empty, renderContextDocument, diagnosis
- **Related:** TASK-025, DEC-071, DEC-070
- **Occurrences:** US-33.2 tests

---
