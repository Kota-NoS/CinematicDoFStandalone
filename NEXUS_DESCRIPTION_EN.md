# About this mod

CinematicDoFStandalone is an SKSE plugin that makes Cinematic DoF available as a standalone effect. It is based on Jiaye's Community Shaders AIO work, but Community Shaders itself is not required.

Version 1.0.0 is the first stable release. It combines the completed aperture-shaped bokeh, official Skyrim 1.7.104 and Address Library v5 support, and narrowly scoped tracked-subject protection for affected low-setting configurations while preserving Skyrim 1.6.1170 and the established rendering baseline.

## Features

- Standalone SKSE integration with English and Japanese UI
- Three focus modes: Fixed Focus, Screen AF, and Target Tracking
- Console-selected target tracking plus automatic player selection and tracking
- Nine editable presets with bundled defaults
- Independent near/far in-focus ranges and near/far maximum blur controls
- Automatic focus on the conversation subject during dialogue
- Option to disable near blur in first person
- An optional, unassigned-by-default keyboard hotkey for toggling DoF
- Aperture-shaped bokeh with adjustable blade count, roundness, strength, rotation, and highlight emphasis

## Requirements

- Skyrim AE 1.6.1170 or 1.7.104 (the runtimes used for in-game release testing)
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.8.0 only if you want the in-game UI

The build targets Skyrim SE and AE, and the distributed build was tested in game on AE 1.6.1170 and 1.7.104. Version 1.7.104 uses the Address Library v5 database format supported by this release. Community Shaders is optional. If it is installed, disable its Depth of Field effect to avoid applying two DoF effects at once.

Without Community Shaders, tracked-subject protection activates when SSR or 64-bit HDR is disabled. When 64-bit HDR is explicitly disabled and a valid tracked target exists, the effective near-focus range is floored at 0.15 m without changing the saved value. The protection is disabled in a normal standalone configuration with both 64-bit HDR and SSR enabled.

## Installation

1. For a new installation, install the `Full-MO2` package. It includes the INI with the bundled defaults.
2. When updating from an older version, use the INI-free `Update-Merge-MO2` package and choose `Merge` into the existing mod in MO2. `Replace` also deletes the existing INI.
3. Installing the `Full-MO2` package over an existing setup replaces the user's saved settings with the bundled defaults.
4. If Community Shaders is installed, disable Depth of Field in Community Shaders.

For Japanese UI text, set `EnableJapanese=true` under `[Fonts]` in `Data/SKSE/Plugins/SKSEMenuFramework.ini`.

## Basic use

With SKSE Menu Framework installed, press F1 in game and open `Cinematic DoF Standalone` → `Depth of Field`.

- Fixed Focus directly sets the distance from the camera to the focus plane.
- Screen AF samples depth at the selected screen X/Y position.
- Target Tracking uses projected head-node distance for actors and projected reference/anchor distance for non-actors.
- Target Focus Offset moves the focus plane toward the camera with negative values and farther away with positive values.
- A practical lens workflow is to set broad depth separation with focal length, refine the in-focus range with the F-number, then set final strength with the near/far maximum blur controls.
- DoF Hotkey: press the hotkey button beside `Enable DoF`, then press the keyboard key you want to assign. Esc cancels assignment; Backspace or Delete clears it.

The key assignment is saved to the INI and is not part of a preset. Toggling DoF with the hotkey does not automatically save the enabled state. If the same key is used by Skyrim or another mod, both actions may run.

## Presets and saving

The nine slots are Gameplay, Cinematic, View Focus, Photo Portrait, Photo Wide, First-Person Photo, Aperture Bokeh, Custom 1, and Custom 2. Existing Custom 1/2 values keep their named INI sections and stable internal IDs while moving visually to slots 8/9, so an upgrade does not overwrite them.

- Preset number / Apply: applies the stored settings to the current image. It does not change the preset slot or startup settings.
- Store Current: saves the current DoF, dialogue-focus, and target-tracking values in the selected preset slot. It does not change startup settings or the UI language.
- Restore Defaults: press twice to apply the bundled defaults without saving.
- Save Startup Settings: saves the current DoF, dialogue-focus, target-tracking, and UI-language values for the next launch. It does not overwrite any preset slot.
- Reload INI: reloads the saved startup settings and preset slots.

## Credits and license

- kota (@kotaSkyrim): CinematicDoFStandalone
- Jiaye: extensive edits and adaptation of the original CinematicDOF work
- Frans Bouma (Otis / Infuse Project): original CinematicDOF shader
- doodlum: target focus
- ProfJack: post-processing base structure
- Community Shaders team: Community Shaders base
- OpenAI Codex: development assistance

Unless a file carries a more specific notice, this mod is distributed under GPL-3.0-or-later with the Community Shaders Modding Exception and the GPL-3.0 Linking Exception with Corresponding Source. See `COPYING`, `EXCEPTIONS.md`, `THIRD_PARTY_NOTICES.md`, `LICENSES`, and the bundled font's OFL for the complete terms and third-party notices.

The complete corresponding source is provided in a separate source archive with the same version number. See `SOURCE_DEPENDENCIES.md` inside that archive for build and dependency-source information.
