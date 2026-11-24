# Problems: AI

**Append-only.** Scope: AgentBridge, `ai-services`, the assistant core, semantic layer, AI tests. How to use this folder: [README.md](README.md).

---

### PRB-006 — Test fixture wrote a chat file into the build folder
- **Date:** 2026-11-22
- **Area:** Tests / Assistant
- **Status:** Solved
- **Symptom:** `GroundedAssistantTest.Ask_UnauthorizedContextSendsNothingAndLeavesChatUnchanged` failed with
  `Value of: std::filesystem::exists(store.chatFilePath("grounded")) Actual: true Expected: false`,
  and `build/headless/tests/assistant/chats/grounded.chat` appeared.
- **Context:** US-11.1 tests, headless preset. The fixture declared `std::filesystem::path root;`,
  set it in `SetUp()`, and built `FixedUserProjectPaths rootPaths{root}` and `ChatStore` as members.
- **Root cause:** members are initialised before `SetUp()` runs, so the store copied an empty path
  and wrote chats relative to the test's working directory, shared by all tests.
- **Solution:** initialise `root` in its declaration from the current test name, before the paths
  and store members, and only clear it in `SetUp()`.
- **How verified:** removed the stray folder, rebuilt; all 207 tests pass and no chat file appears
  under `build/`.
- **Prevention:** in fixtures, give any member another member depends on its value in its
  declaration, never in `SetUp()`.
- **Keywords:** GoogleTest fixture, member initialisation order, SetUp, ChatStore, userLocalArea,
  temp_directory_path
- **Related:** TASK-023, DEC-067
- **Occurrences:** US-11.1 tests

---
