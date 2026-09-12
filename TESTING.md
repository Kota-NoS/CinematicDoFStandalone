# CinematicDoFStandalone 0.8.32 verification

Version 0.8.32 preserves the verified Test 4 aperture implementation, replaces only `AccentuateWhites` with the bounded HDR/PBR-safe implementation, and keeps the Open Shaders FSR active-render-area fix. Spiral sampling and the fixed five-tap prefilter are not included. See `package/Documentation/RELEASE_0.8.32_JA_EN.md` for the integrated release record and known limitation.

## Test scope

- Starts from the exact public 0.8.31 source and configuration.
- Test 1 proved stability and persistence, but its mask-weight method did not produce a visible shape change in game.
- Test 2 proved that directly deforming near and far gather sample positions changes the blur shape, but the result was intentionally subtle.
- Test 3's shaped-highlight sum was still diluted by the complete gather average and was not perceptible at 0.00–0.30 in the requested in-game comparison.
- Test 4 keeps the natural shaped gather but preserves its brightest eligible aperture-shaped sample, then uses `Highlight Boost` to blend toward that peak without exceeding the sampled source highlight.
- A value of 0 preserves the natural Test 2 result.
- Exposes aperture blade count, blade roundness, shape strength, and rotation in Advanced Settings.
- Saves the five aperture settings independently in startup settings and every preset.
- Keeps aperture-shaped bokeh disabled in startup settings and the eight existing presets so their default image remains unchanged; only the new Aperture Bokeh preset enables it.
- Does not restore or expose the rejected visible-surface actor-silhouette experiments.

## Static verification

- Build `releasedbg` x64 successfully.
- Compile all 15 HLSL entry points with `fxc /Ges /T cs_5_0`.
- Confirm FileVersion and ProductVersion are `0.8.32.0`.
- Confirm `SKSEPlugin_Load`, `SKSEPlugin_Query`, and `SKSEPlugin_Version` exports.
- Confirm all five aperture keys exist in startup settings and all nine INI preset sections.
- Confirm the MO2 archive is Data-shaped (`SKSE`, `Shaders`, and `Documentation` at its root) and contains the matching DLL and PDB.
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
