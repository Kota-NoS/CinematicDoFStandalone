# CinematicDoFStandalone 0.8.31 release verification

## Release scope

- Keeps the 0.8.28 rendering path and HLSL behavior.
- Uses projected head-node distance for actor target tracking and projected reference/anchor distance for non-actors.
- Does not expose or select the retained visible-surface focus experiment.
- Uses the INI saved by the maintainer on 2026-09-01 15:28 as the distributed configuration.
- Synchronizes all eight compiled Restore Defaults presets with the distributed INI.
- Separates Apply, Store Current, and Save Startup Settings so each changes only its named destination.
- Removes the persistent unsaved-startup-settings notice from the bottom of the UI.
- Provides an unassigned-by-default keyboard hotkey for toggling DoF. The assignment is saved separately from presets, while the toggled enabled state is not saved automatically.

## Static verification

- Build `releasedbg` x64 successfully.
- Confirm FileVersion and ProductVersion are `0.8.31.0`.
- Confirm `SKSEPlugin_Load`, `SKSEPlugin_Query`, and `SKSEPlugin_Version` exports.
- Confirm the supplied and packaged INI files have SHA-256 `8B61DC03CD2F95E373027FD32220AB02C60930CB186D01C07C7A4DE2981FFBDF`.
- Compare every field in the eight INI preset sections with the compiled Restore Defaults values.
- Confirm the release archives exclude PDB, object, build, work, and `.xmake` content.
- Confirm the main archive is Data-shaped (`SKSE`, `Shaders`, and `Documentation` at its root).

## In-game verification completed on Skyrim AE 1.6.1170

- Standalone operation without Community Shaders.
- DoF OFF/ON rendering, including keyboard hotkey assignment, cancellation, clearing, persistence after restart, and normal-play toggling.
- Fixed Focus, Screen AF, player tracking, console-selected actor tracking, and console-selected non-actor tracking.
- Actor head-node distance and non-actor reference/anchor distance tracking.
- Japanese and English UI, near/far in-focus ranges, all eight presets, Store Current, Restore Defaults, Save Startup Settings, and Reload INI.
- Independent Apply, Store Current, and Save Startup Settings behavior.
- Bundled defaults and representative Low and Medium Skyrim depth-setting scenes.
- Bright subjects against dark backgrounds, dark subjects against bright skies or windows, layered distant scenery, rain, fog, snow, and night lighting.
- Strong near/far blur compositions and the expected ability to compensate for close-up body-depth differences with the in-focus range controls.
