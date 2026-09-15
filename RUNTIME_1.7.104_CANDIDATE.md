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

No files under `src`, `package/Shaders`, or `package/SKSE` were changed. DoF rendering, aperture shape, presets, INI defaults, and UI behavior remain identical to the completed 0.8.32 baseline.

## Build verification completed

- `releasedbg` x64 build: passed
- Exports present: `SKSEPlugin_Load`, `SKSEPlugin_Query`, `SKSEPlugin_Version`
- Exported `versionIndependenceEx`: `3`
  - Address Library v5 flag: set
  - no-struct-use flag: set
- Address Library declaration: set

Build success proves that the compatibility declaration is present; it does not prove that the game-side hook sites are unchanged on 1.7.104.

## テスト順序 / Test order

### 1. Regression test on Skyrim 1.6.1170

Use the No-INI candidate package so the existing settings remain untouched.

1. Start the game and confirm that the plugin loads without an SKSE error.
2. Open the in-game menu and confirm the saved settings and all nine presets are present.
3. Toggle DoF through the menu and assigned hotkey.
4. Check Fixed Focus, Screen AF, and Target Tracking.
5. Apply preset 7 and compare aperture shape, brightness, and near/far blur with the completed 0.8.32 build.
6. Save, reload, change cells, and enter/leave dialogue.
7. Compare FPS in the same scene and camera position.
8. Keep `CinematicDoFStandalone.log` from the SKSE log directory if anything differs.

Acceptance condition: no visual, FPS, setting, save/load, or control regression from the completed 0.8.32 build.

### 2. Candidate test on Skyrim 1.7.104

Required tester environment:

- Skyrim 1.7.104
- SKSE64 2.3.1 for Skyrim 1.7.104
- Address Library All in One for 1.7.104 (v5 database format; Nexus file v13 at the time this candidate was prepared)
- SKSE Menu Framework 3.8.0 when testing the in-game UI

Run the same checks as the 1.6.1170 regression test. In particular, confirm that enabling DoF, opening the UI, changing focus modes, and using the hotkey do not crash.

If the game crashes or DoF does not render, collect:

- `CinematicDoFStandalone.log`
- the SKSE loader/plugin error message, if shown
- a crash logger report, if available
- the exact Skyrim, SKSE, and Address Library versions

## Remaining runtime risk

The plugin uses Address Library IDs, but two hooks call an instruction inside a relocated function and therefore also use a local call-site offset. Camera near/far clip values are also read at known offsets from a relocated global. These locations cannot be certified for 1.7.104 by compilation alone. They must be validated in the actual runtime before declaring official support.

Official references:

- SKSE: https://skse.silverlock.org/
- SKSE source/readme: https://github.com/ianpatt/skse64
- Address Library: https://www.nexusmods.com/skyrimspecialedition/mods/32444
- CommonLibSSE-NG: https://github.com/alandtse/CommonLibSSE-NG
