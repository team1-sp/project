# Development Process

Source: Initial Plan §3 (customised Agile Scrum). Re-read that section if the team changes the process.

## 1. Model summary

- Fixed **2-week sprints**.
- **Product Owner:** the project advisor, Leyla Sezer. **Scrum Master:** Arda Sarıtaş. **Project Manager:** Işık Dönger **Development team:** Işık Dönger, Arda Sarıtaş, Arda Karabulut, Elifnur Özçelik, Zeynep Ertürk `[confirm roles]`.
- **No daily stand-ups.** One **Weekly Progress Meeting** (team only).
- **Sprint Planning** at sprint start; **Sprint Review** with the Product Owner at the end.
- **No Sprint Retrospective** (known disadvantage: process friction may persist; mitigate with the problem log and handoffs).
- Deliver a **working increment every two weeks** where possible; research-heavy sprints may deliver decisions + prototypes.
- Course milestones (documents, deliverables) are in the ClickUp Gantt chart (link in Initial Plan §4.2).

## 2. Known risks of this model (from the Initial Plan) and mitigations

| Risk | Mitigation in this repo |
| --- | --- |
| A blocker may sit for days without stand-ups | `Blocked` status + `open-questions.md`; check the board at least twice a week; raise blockers in chat immediately |
| No retrospective | Keep `problem-log.md` and `decision-log.md` honest; review them in the Weekly Progress Meeting |
| Estimates inaccurate while learning | Keep tasks ≤ 1 day of work; re-slice rather than extend; record actual outcomes in tasks |

## 3. Sprint rhythm and artefacts

| Moment | Humans | Agents |
| --- | --- | --- |
| Sprint Planning | Pick sprint goal; move tasks to `Ready` with acceptance criteria; decide open technology choices | Prepare decision briefs and task drafts beforehand (TASK-… in `Backlog`) |
| During sprint | Work tasks; Weekly Progress Meeting | Claim `Ready` tasks, follow `AGENT-PROTOCOL.md`, update state files |
| Weekly Progress Meeting | Review board, blockers, open questions, new `Proposed` decisions | Make sure `current-status.md` is current beforehand |
| Sprint Review | Demo increment to advisor; adapt backlog | Prepare demo notes and test evidence (reports from headless runs) |

## 4. Task flow

`Backlog → Ready → Claimed → Review → Done` (+ `Blocked`, `Dropped`). Definition of Ready: has acceptance criteria, no open human decision it depends on, ≤ 1 day. Definition of Done: [definition-of-done.md](definition-of-done.md).

The human-facing planning tool is ClickUp; the repo's `task-board.md` is the **agent-facing mirror**. When a ClickUp task exists, put its link in the task entry; the board must not contradict ClickUp (humans win on conflict).

## 5. Change control

Scope changes go through the advisor/Scrum Master. Technical changes that touch an invariant or an `Accepted` decision need a decision entry. Decisions are made by humans (project manager Işık Dönger consulting the team and agents) and recorded in `history/decision-log.md` (DEC-017). Course documents (Initial Plan etc.) are for the course and are not the source of truth; the Initial Plan §7 (Project Change Control) was empty in the repo on 2026-10-12 `[VERIFIED]`; update this section if the team writes it.

## 6. Communication

Team communication plan (Initial Plan §6) is currently empty `[VERIFIED 2026-10-12]`. Inside the repo, communication between agents is defined in `agents/communication-protocol.md`.

## 7. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-10-12 | Created from Initial Plan §3 | claude-code/sonnet-5.5/012r |
