# Skyrim 1.7.104 compatibility candidate

This branch is a compatibility-only candidate based on the completed 0.8.32 visual baseline. It is not yet a confirmed 1.7.104 release.

## 固定した基準 / Frozen baseline

- Branch: `release/v0.8.32-complete`
- Tag: `v0.8.32-complete`
- Commit: `50587a630813be0da0ecd40dfd7f3f970e1127d8`

The candidate branch is `compat/runtime-1.7.104-candidate`.

## 変更内容 / Compatibility changes

The bundled CommonLibSSE-NG 6.1.0 snapshot receives the minimum upstream compatibility backport required for the new Address Library generation:

- Skyrim 1.7.x is classified as AE and therefore selects AE relocation IDs and the AE Address Library filename.
- The exported SKSE plugin declaration sets the Address Library v5 flag required by SKSE for Address Library v5 runtimes.
- The relocation database loader recognizes Address Library format 5 and reads its dense ID-to-offset table while retaining the existing format 1/2 loader for older runtimes.

Candidate 1 contained the first two changes. A real 1.7.104 launch reached the plugin but stopped with `Unsupported address library format: 5`, proving that the bundled CommonLib reader itself also needed the format 5 backport. Candidate 2 adds that reader without upgrading the renderer-facing CommonLib interfaces.

Candidate 3 keeps the standard depth path on Skyrim 1.7.104 when Community Shaders is absent, but also enables the existing low-settings target near-blur protection when a low-spec display setting is detected. This is an exact `1.7.104.0` runtime gate. Skyrim 1.6.1170 and every other runtime retain the completed 0.8.32 activation conditions.

No shader, preset, INI-default, or UI code was changed. Candidate 3 only expands the activation condition for the already-completed target-protection shader data on the affected 1.7.104 standalone path.

Candidate 4 separates the two low-settings decisions so the established Community Shaders depth fallback and the new standalone target guard no longer share one combined detector:

- The Community Shaders depth fallback is unchanged and still considers disabled SAO, SSR, or 64-bit HDR.
- The exact-1.7.104 standalone target guard ignores SAO and activates only when SSR or 64-bit HDR is explicitly disabled while Community Shaders is absent.
- The head guard keeps the same centre and outer radius, but its fully protected inner region increases from 72% to 82% of that radius. The body guard, close-up far guard, presets, INI defaults, and UI are unchanged.

Candidate 5 removes only the exact-`1.7.104.0` gate from the standalone target guard. Testing on Skyrim 1.6.1170 proved that setting `bUse64bitsHDRRenderTarget=0` can reproduce the same player foreground-blur problem even without Community Shaders. Therefore:

- With Community Shaders loaded, the established depth fallback remains unchanged and still considers disabled SAO, SSR, or 64-bit HDR.
- Without Community Shaders, the standard depth path remains unchanged, while the existing target guard now activates on every supported SE/AE runtime when SSR or 64-bit HDR is explicitly disabled.
- Disabled SAO alone still does not activate the standalone guard.
- No shader, guard position/radius, preset, INI-default, UI, focus, or blur calculation was changed.

Candidate 6 is a deliberately narrow visual test for the slight residual blur observed at the top of the tracked character's head when the standalone guard is active:

- The head guard keeps the same centre and outer radius.
- Its fully protected inner region increases from 82% to 88% of the existing radius, leaving the outer 12% as the feathered transition.
- The body guard, activation conditions, Community Shaders depth fallback, presets, INI defaults, UI, focus, and blur calculations are unchanged.

Candidate 7 restores the Candidate 5 head-guard feather after the wider Candidate 6 inner region showed no meaningful improvement, and adds a narrowly scoped near-focus assist for the confirmed 64-bit HDR trigger:

- The head guard's fully protected inner region returns from 88% to 82% of its unchanged radius.
- The assist is enabled only when Community Shaders is absent and `bUse64bitsHDRRenderTarget=0` is read explicitly. Disabled SSR alone does not enable it, and an unknown or enabled 64-bit HDR setting leaves it off.
- Even while enabled, the assist is applied only when target tracking supplies a valid protected target. Fixed Focus, Screen AF, and frames without a valid tracked target retain the saved value.
- During those protected target-tracking frames, the effective near-focus range is `max(saved value, 0.15 m)`. The saved preset/INI/UI value is not changed.
- Community Shaders behavior, the established low-spec depth fallback, far focus, maximum blur, bokeh, presets, INI defaults, and UI remain unchanged.

## Build verification completed

- `releasedbg` x64 build: passed
- Exports present: `SKSEPlugin_Load`, `SKSEPlugin_Query`, `SKSEPlugin_Version`
- Exported `versionIndependenceEx`: `3`
  - Address Library v5 flag: set
  - no-struct-use flag: set
- Address Library declaration: set
- Real 1.7.104 Address Library database inspected:
  - Format: `5`
  - Runtime: `1.7.104.0`
  - Dense entry count and file length: consistent
  - All three relocation IDs used directly by this plugin are present: `107148`, `68617`, and `403540`

Build and database checks prove that the compatibility declaration is present and that the required IDs can be resolved. They do not prove that the game-side instructions at the two hook sites or the camera data layout are unchanged on 1.7.104.

## テスト順序 / Test order

### 1. Regression and protection test on Skyrim 1.6.1170

Use the No-INI candidate package so the existing settings remain untouched.

1. Start the game and confirm that the plugin loads without an SKSE error.
2. Open the in-game menu and confirm the saved settings and all nine presets are present.
3. Toggle DoF through the menu and assigned hotkey.
4. Check Fixed Focus, Screen AF, and Target Tracking.
5. Apply preset 7 and compare aperture shape, brightness, and near/far blur with the completed 0.8.32 build.
6. Save, reload, change cells, and enter/leave dialogue.
7. Compare FPS in the same scene and camera position.
8. Without Community Shaders, set `bUse64bitsHDRRenderTarget=0`, fully restart Skyrim, and verify that the tracked player remains sharp. The log should contain `Standalone target near-blur protection activated`.
9. Restore `bUse64bitsHDRRenderTarget=1`, fully restart Skyrim, and verify that the standalone protection line is absent and the completed 0.8.32 rendering remains unchanged.
10. Keep `CinematicDoFStandalone.log` from the SKSE log directory if anything differs.

Acceptance condition: no visual, FPS, setting, save/load, or control regression from the completed 0.8.32 build.

### 2. Candidate test on Skyrim 1.7.104

Required tester environment:

- Skyrim 1.7.104
- SKSE64 2.3.1 for Skyrim 1.7.104
- Address Library All in One for 1.7.104 (v5 database format; Nexus file v13 at the time this candidate was prepared)
- SKSE Menu Framework 3.8.0 when testing the in-game UI

Run the same checks as the 1.6.1170 regression test. In particular, confirm that enabling DoF, opening the UI, changing focus modes, and using the hotkey do not crash.

With Community Shaders absent and either SSR or 64-bit HDR disabled, verify player target tracking again. The log should contain `Standalone target near-blur protection activated`, and the player should remain protected from foreground blur while the surrounding depth of field remains unchanged. Also verify that disabling SAO alone, with SSR and 64-bit HDR enabled, does not activate this standalone guard. The established Community Shaders depth fallback must still react to SAO, SSR, or 64-bit HDR exactly as before.

If the game crashes or DoF does not render, collect:

- `CinematicDoFStandalone.log`
- the SKSE loader/plugin error message, if shown
- a crash logger report, if available
- the exact Skyrim, SKSE, and Address Library versions

## Remaining runtime risk

The plugin uses Address Library IDs, but two hooks call an instruction inside a relocated function and therefore also use a local call-site offset. Camera near/far clip values are also read at known offsets from a relocated global. These locations cannot be certified for 1.7.104 by compilation alone. They must be validated in the actual runtime before declaring official support.

Candidate 7 is therefore still a test build. Successful game launch, menu operation, DoF rendering, focus-mode changes, player-protection behavior, cell transitions, save/load, and a same-scene FPS comparison are required before the 1.7.104 package can be promoted. With Community Shaders absent, compare `bUse64bitsHDRRenderTarget=1` against `0` after a full game restart. At `0`, verify that a saved near-focus range below 0.15 m protects the tracked character while Fixed Focus, Screen AF, and frames without a valid tracked target retain their saved value. Also check foreground objects within approximately 0.15 m of the focus plane for any unwanted sharpening.

Official references:

- SKSE: https://skse.silverlock.org/
- SKSE source/readme: https://github.com/ianpatt/skse64
- Address Library: https://www.nexusmods.com/skyrimspecialedition/mods/32444
- CommonLibSSE-NG: https://github.com/alandtse/CommonLibSSE-NG
