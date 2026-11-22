# AI Service and Privacy Settings

Status: **implemented for US-33.5** (srs.fr.33.28–33.33).

## Contract

The feature is split at the window boundary:

- `AiPrivacySettingsController` owns validation, state transitions, request generations,
  connection/model presentation state and immediate persistence. It has no Dear ImGui dependency.
- `MainWindowView::drawProjectSettings()` renders the AI and Privacy tab and forwards user actions
  to the controller. It does not call gRPC or make policy decisions.
- `IAiServiceRequestRunner` is asynchronous. `AgentBridgeRequestRunner` runs the address-bound
  `AgentBridge` operations off the UI thread; tests enqueue responses explicitly and in any order.
- `AgentBridge` remains the transport-neutral service seam. It now exposes `checkHealth(timeout)`
  and `listModels(timeout)` in addition to #72's answer stream. The optional gRPC target owns
  `grpc.health.v1.Health/Check`, `ListModels` protobuf mapping and status translation.

Every request carries a monotonically increasing identifier and the validated address. The
controller applies a completion only when both still match the newest request for that operation.
Changing the address invalidates both generations, clears the selected model and connected state,
and turns off both address-bound sharing permissions.

## Address and model state

`isValidServiceAddress()` accepts DNS host names, four-octet IPv4 and bracketed IPv6 followed by a
decimal port from 1 through 65 535. An invalid focus-loss commit leaves the last validated address
in use and exposes exactly `Address must be host:port`.

Opening the tab requests models with a five-second deadline. A successful health check also requests
models. Successful lists replace the presentation list and preserve only a selected identifier still
present; display names are never persisted. Empty, failed and unselected states remain distinct.
The settings model exposes `sendingEnabled()` and `sendDisabledReason()` (`Select a model`) for the
assistant sending pipeline.

Health failures are presented as `Address not valid`, `Service not reachable`, `Deadline exceeded`,
`Service not serving`, or `<gRPC status>: <message>`. A serving response presents
`Connected · N ms`; the injectable request-runner clock supplies `N`.

## Privacy and persistence

Source-code sharing, telemetry sharing and Ask before every send default off. Enabling either data
category creates a category-specific confirmation without changing the saved value; confirming
changes that category only, while cancellation changes neither. `AiPrivacySettings` implements
#72's read-only `DataTransmissionPolicy` and exposes `requiresConsent(category)` for #120; this task
does not implement #120's per-question dialog.

`AiPrivacySettingsStore` writes only the validated address, model identifier and three booleans as
`AEGIS_AI_PRIVACY 1` under:

```
<user-local-project>/settings/ai-privacy.settings
```

The user-local project area is injected, so this store cannot select shared/version-controlled
project settings. It stores no provider key, token or credential. Replacement uses temporary and
backup files; missing files load caller-supplied new-project defaults.

## Verification

`tests/unit/editor/ai_privacy_settings_test.cpp` covers address forms and boundaries, focus-loss
retention, model-list states and selection, five-second deadlines, latency/failure presentation,
privacy confirmation independence, Ask-before-send persistence, stale completions and settings
round trips. `main_menu_test.cpp` and `status_bar_test.cpp` cover the three routes to the tab.
