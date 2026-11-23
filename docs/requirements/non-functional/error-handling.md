# Error Handling Requirements

The table below states how Aegis responds to each type of error. *Actor errors* come from the user or from another system; *system errors* come from a defect or failure inside Aegis.

**Table 49** Error Handling Requirements

| ID | Error | Source | Response |
| --- | --- | --- | --- |
| srs.nfr.5.1 | Invalid value typed into a field (not a number, out of range, illegal name) | Actor | Warn user: the field flashes red or shows the error under it, and the previous value is kept. |
| srs.nfr.5.2 | Project folder moved or missing, or a folder that is not an Aegis project is opened | Actor | Warn user: the row is marked "Not found" with Locate… and Remove, or "Not an Aegis project" is shown. |
| srs.nfr.5.3 | Unsupported or invalid file imported (asset or template) | Actor | Warn user with an error toast; nothing is copied. |
| srs.nfr.5.4 | A scene file names an unknown component type | Actor | Log and proceed: the rest of the scene loads, the component's data is kept unchanged as a non-executing placeholder that is saved back and restored when the type is registered (srs.fr.2.21, srs.fr.2.101), and a warning names the entity and type. |
| srs.nfr.5.5 | A referenced asset file is missing | Actor | Substitute and keep the reference: a missing sprite is drawn as a placeholder, missing audio plays silence, a missing data asset follows srs.fr.2.27 and a missing prefab or base follows srs.fr.2.35, srs.fr.2.98, srs.fr.2.108 and srs.fr.2.125; a warning is logged. |
| srs.nfr.5.6 | Saving a file fails (disk full, no permission) | Actor | Warn user with the item and the reason; the item keeps its unsaved state and its previous file, other saves that succeeded are kept (srs.fr.15.23), and the user can retry without limit. |
| srs.nfr.5.7 | AI service unreachable or times out | Actor | Warn user ("AI service unavailable"), keep the question and disable sending (srs.fr.33.22); the question request's deadline is the one of the AgentBridge contract (srs.nfr.4.11), and a successful Test Connection restores sending (srs.fr.33.21); the rest of the Editor keeps working. |
| srs.nfr.5.8 | AI service returns an answer without answer text, or with citations that do not decode into the forms of srs.fr.11.4 | Actor | Log and proceed: the answer is shown as an error card, no citation chips are made from invalid citations, and the raw response is written to the log; the answer schema is part of the AgentBridge contract (srs.nfr.4.11). |
| srs.nfr.5.9 | Build fails | Actor | Warn user: the build dialog shows the error count and Show Errors sends the errors to the Console. |
| srs.nfr.5.10 | An entity ID or asset reference read from a scene, prefab, data asset, input recording or agent action is invalid, or a system writes a non-finite physics value | System | Log and proceed: the operation is rejected before it changes any state, the previous valid value is kept and never passed to physics, an error names the operation, ID or value and tick, and other entities continue. An invalid entity handle or missing component in a system's own world access is an invariant violation and is unrecoverable (srs.fr.1.11). |
| srs.nfr.5.11 | A recoverable exception in the Engine during a headless run | System | Stop the run with outcome "Error" and the message, finalize its report and recording when possible, and keep the Editor running; a failed precondition or access check is handled as stated in srs.fr.1.11. |
| srs.nfr.5.12 | The simulation cannot keep up with real time | System | Log and proceed: extra ticks are dropped and a warning is logged (srs.fr.1.4). |
| srs.nfr.5.13 | Unrecoverable failure of the Engine (out of memory, graphics cannot start) | System | Shut down: when possible, write the reason and the last tick ("before tick 0" at start-up) to the log; exit with a non-zero code (srs.fr.1.11); the Editor's recovery copies are kept, and an Editor not affected by the failure keeps running. |
