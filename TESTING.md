# CinematicDoFStandalone 1.0.0 verification

Version 1.0.0 promotes the completed 0.8.32 rendering baseline to the first stable release, adds official Skyrim 1.7.104 and Address Library v5 compatibility, and integrates the display-setting-scoped target protection validated through Candidate 7. See `package/Documentation/RELEASE_1.0.0_JA_EN.md` and `RUNTIME_1.7.104_VALIDATION.md`.

## Rendering baseline retained from 0.8.32

- Starts from the exact public 0.8.31 source and configuration.
- Test 1 proved stability and persistence, but its mask-weight method did not produce a visible shape change in game.
- Test 2 proved that directly deforming near and far gather sample positions changes the blur shape, but the result was intentionally subtle.
- Test 3's shaped-highlight sum was still diluted by the complete gather average and was not perceptible at 0.00–0.30 in the requested in-game comparison.
- Test 4 keeps the natural shaped gather but preserves its brightest eligible aperture-shaped sample, then uses `Highlight Boost` to blend toward that peak without exceeding the sampled source highlight.
- A value of 0 preserves the natural Test 2 result.
- Exposes aperture blade count, blade roundness, shape strength, and rotation in Advanced Settings.
- Saves the five aperture settings independently in startup settings and every preset.
- Keeps aperture-shaped bokeh disabled in startup settings and the other bundled presets so their default image remains unchanged; only the Aperture Bokeh preset enables it.
- Does not restore or expose the rejected visible-surface actor-silhouette experiments.

## 1.0.0 compatibility and protection scope

- Skyrim 1.7.x is classified as AE and uses the AE Address Library path.
- The SKSE plugin declaration sets the Address Library v5 flag.
- The relocation reader accepts the Address Library v5 dense table while retaining the v1/v2 path used by earlier runtimes.
- With Community Shaders loaded, the established depth fallback still considers disabled SAO, SSR, or 64-bit HDR.
- Without Community Shaders, the standard depth path remains in use. The tracked-subject guard activates only when SSR or 64-bit HDR is disabled; disabled SAO alone does not activate it.
- The head guard retains its established centre and outer radius, with 82% of that radius fully protected.
- Without Community Shaders, explicitly disabled 64-bit HDR and a valid tracked target set the effective near-focus range to `max(saved value, 0.15 m)`. The saved INI, preset, and UI value are unchanged.
- Fixed Focus, Screen AF, invalid/no-target frames, and normal standalone configurations with SSR and 64-bit HDR enabled do not receive the 0.15 m assist.

## Static verification

- Build `releasedbg` x64 successfully.
- Compile all 15 HLSL entry points with `fxc /Ges /T cs_5_0`.
- Confirm FileVersion and ProductVersion are `1.0.0.0`.
- Confirm `SKSEPlugin_Load`, `SKSEPlugin_Query`, and `SKSEPlugin_Version` exports.
- Confirm the Address Library and v5 version-independence declaration flags.
- Confirm the three relocation IDs used directly by the plugin exist in the Skyrim 1.7.104 database.
- Confirm all five aperture keys exist in startup settings and all nine INI preset sections.
- Confirm the MO2 archive is Data-shaped (`SKSE`, `Shaders`, and `Documentation` at its root) and contains the matching DLL. Keep build symbols outside the user-facing runtime archives.
- Confirm the source archive contains the matching source tree and package while excluding generated build, object, cache, `work`, and `.xmake` directories.

## Completed in-game verification on Skyrim AE 1.6.1170

- Ordinary camera movement was checked without an abrupt visible Mip transition.
- Large far bokeh was checked without renewed splitting or double contours at the tested settings.
- Standard circular blur was checked without becoming unnaturally soft.
- Conversation focus, target tracking, and fixed focus were checked.
- Indoor/outdoor, day/night, strong-highlight, and PBR scenes were checked.
- Native AA and the available FSR presets were checked.
- Frame rate and transient stutter were checked without a new issue being observed.
- Settings and all nine presets were checked across save, reload, and restart.
- Dialogue-only DoF was checked from normal-gameplay OFF through conversation start/end, including immediate master-switch/hotkey shutdown and restoration of normal DoF.
- English/Japanese selection was checked for immediate switching and persistence.
- An older INI with customized `Preset.Custom1` and `Preset.Custom2` values was checked to retain those values in UI slots 8 and 9 while the independent Aperture Bokeh slot was added at 7.

## Completed 1.0.0 protection verification on Skyrim AE 1.6.1170

- With Community Shaders absent, changing only `bUse64bitsHDRRenderTarget` to `0` reproduced the tracked-character foreground-blur problem before protection and activated the final protection in 1.0.0.
- Restoring `bUse64bitsHDRRenderTarget=1` with SSR enabled disabled both the standalone target guard and HDR near-focus assist, as confirmed by the runtime log.
- The 0.15 m effective floor was compared against a saved 0.00 m value at ordinary and very close foreground distances. Ordinary framing was visually equivalent; the limited close-range difference was accepted in exchange for reliable subject protection.

## Completed in-game verification on Skyrim AE 1.7.104

- The plugin loaded with SKSE64 2.3.1 and the matching Address Library v5 database.
- Menu operation, all focus modes, DoF rendering, and aperture-shaped bokeh operated without a runtime compatibility failure.
- The final 82% guard plus HDR-scoped 0.15 m assist produced the intended sharp subject while retaining surrounding depth-of-field rendering.

## Known limitation

At low `Blur Quality`, very large blur discs can split into visible rings or points because the gather does not have enough sample density. Raise `Blur Quality` or reduce the near/far maximum blur when necessary.

## Pending 1.0.1 Test 7D input-loaded HDR classification

- Test 7C established that `bUse64bitsHDRRenderTarget=1` remains disabled through `kPostPostLoad` and becomes enabled by `kInputLoaded`, identically with Community Shaders enabled and disabled. This is the game's profile-application boundary, not a Community Shaders runtime write.
- Test 7D captures the 64-bit HDR classification at `kInputLoaded`, before the first DoF frame, and retains the lifecycle checkpoint logs for verification.
- A valid tracked actor uses the same depth-confirmed projected guard with or without Community Shaders and with either 64-bit HDR setting.
- The guard protects only pixels near the tracked visible-surface depth. The previous close-up head override that bypassed depth is absent on both near and far CoC paths.
- With Community Shaders and input-loaded 64-bit HDR disabled, a valid tracked actor additionally receives an effective near-focus floor of `max(saved value, 0.17 m)` without changing the saved INI, preset, or UI value.
- Disabled SAO or SSR alone does not activate the 0.17 m floor, and non-actor targets do not receive it.
- Community Shaders with input-loaded 64-bit HDR enabled uses the unified actor guard without the 0.17 m floor.
- Standalone actors use the unified guard in every display configuration. Non-actor console targets retain the legacy SSR/HDR-disabled guard condition, while HDR disabled also applies the 0.15 m near-focus floor.
- Verify close portraits at screen centre and near both screen edges, then compare `f/3.4` and `f/22`: face, torso, and hands should remain protected without a circular or band-shaped sharp region around the neck.
