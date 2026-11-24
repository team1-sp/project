# Software System Attributes

## Reliability

- **srs.nfr.3.1** Repeated headless runs that meet every prerequisite of srs.fr.7.28 shall produce identical entity positions and events, compared as stated in srs.fr.7.15, at every tick number (ip.fr.7.3, srs.fr.7.6).
- **srs.nfr.3.2** If the process ends while a scene, settings or project file is being saved, reopening shall find either the complete previous version or the complete new version, never a partial file; a first save may leave no file, and failure of the storage device itself is out of scope.
- **srs.nfr.3.3** While autosave is on and its recovery writes succeed, restoring the recovery copy after a crash shall lose no more than the autosave interval set in Preferences of Edit-mode authoring work (srs.fr.15.19); changes made during Play mode are not authoring work, and after a failed recovery write the Editor shall keep the last good copy as stated in srs.fr.15.19.

## Availability

- **srs.nfr.3.4** Authoring, building, simulation, telemetry, bug detection, the Semantic Graph and stored chats shall work without a network connection; only AI requests and external web pages may be unavailable, and they shall not block local work.

## Security

- **srs.nfr.3.5** The AI Assistant Layer shall not transmit project source code or telemetry to an external AI service unless the user has explicitly allowed it (ip.nfr.2.1).
- **srs.nfr.3.6** If the AI service needs an access key, the key shall be stored in the operating system's credential store and never in project files, like the other AI and Privacy settings that stay out of the shared project settings (srs.fr.27.5, srs.nfr.3.14).
- **srs.nfr.3.7** The AI Assistant Layer shall not change game content, source code, shared project settings or simulation state (ip.fr.11.6); it may write only chats and feedback (srs.fr.11.12, srs.fr.33.20).

## Maintainability

- **srs.nfr.3.8** Only the Physics Adapter shall include Box2D headers, and only the rendering and platform/audio wrappers shall include raylib headers, with no raylib type in their public interfaces, so that either library can be replaced without changes elsewhere (srs.fr.4.24).
- **srs.nfr.3.9** All C++ code shall follow the team's C++ coding standards kept in the code repository [R3].
- **srs.nfr.3.10** All C++ code shall pass clang-format 19 with the repository's committed `.clang-format` file.
- **srs.nfr.3.11** Continuous integration shall reject a change that fails the clang-format check of srs.nfr.3.10.

## Portability

- **srs.nfr.3.12** The Engine and Editor shall run on Windows 11 (x64) and on macOS on Macs with Apple silicon (M-series) processors, in every macOS version that runs on them.
- **srs.nfr.3.13** The Engine and Editor shall use no compiler-specific language extensions, so that each platform can use the compiler it needs.
- **srs.nfr.3.14** Operating-system-specific code (file dialogs, file browser, credential store) shall be kept in one platform module, so that the platforms of srs.nfr.3.12 differ only there.
- **srs.nfr.3.15** Games built with Aegis shall run on Windows 11 (x64); other target platforms are out of scope.
