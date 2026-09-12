# Third-party notices and credits

CinematicDoFStandalone contains or is derived from several open-source projects. This file records attribution and points to the license text shipped with the binary and source distributions.

## Project license

Unless a file carries a more specific notice, the project C++ integration and project-specific source are distributed under GNU GPL version 3 or later with the Community Shaders Modding Exception and GPL-3.0 Linking Exception (with Corresponding Source).

- GPL text: `COPYING`
- Additional permissions and obligations: `EXCEPTIONS.md`

The source distribution accompanies the binary distribution and includes the project source, shaders, build definition, and the CommonLibSSE-NG source used to build the plugin.

## Credits requested by Jiaye

Jiaye confirmed that publication is permitted provided that credits and licenses are preserved, and identified the following contributors:

- Jiaye: extensive edits and adaptation on top of original CinematicDOF
- Frans Bouma: original CinematicDOF
- doodlum: target focus
- ProfJack: post-processing base structure
- Community Shaders team: Community Shaders base

The permission correspondence itself is not redistributed here; the project maintainer should retain the original message as release evidence.

## CinematicDOF shader

`Shaders/CinematicDoFStandalone/DoF/dof.cs.hlsl` is derived from Cinematic Depth of Field by Frans Bouma (Otis / Infuse Project), modified by Jiaye and adapted for this plugin. The shader retains its original copyright, BSD-style redistribution conditions, disclaimer, version history, and additional credits in the source header.

- Frans Bouma, Copyright (c) 2018-2022
- License copy: `LICENSES/CinematicDOF-BSD-2-Clause.txt`
- Upstream: <https://github.com/FransBouma/OtisFX/blob/master/Shaders/CinematicDOF.fx>

The shader header additionally credits Marty McFly, Ioxa, Daodan, Prod80, and Keijiro Takahashi. The 9-tap tent filter is from KinoBokeh by Keijiro Takahashi under the MIT License:

- License copy: `LICENSES/KinoBokeh-MIT.txt`
- Upstream: <https://github.com/keijiro/KinoBokeh>

## Community Shaders

The standalone integration is based on Community Shaders and Post Processing work, including PR #2354, reference commit `0d1c62e0c8d32baf68e88a3ca0ac59a352ecac13`.

- Project: <https://github.com/community-shaders/skyrim-community-shaders>
- PR #2354: <https://github.com/community-shaders/skyrim-community-shaders/pull/2354>
- Default project license: GPL-3.0-or-later with the exceptions in `EXCEPTIONS.md`
- Small common shader helpers are covered by the Community Shaders shader MIT notice in `LICENSES/CommunityShaders-Shaders-MIT.txt`

## CommonLibSSE-NG

The plugin statically links the alandtse CommonLibSSE-NG lineage. The bundled snapshot identifies its active license as GPL-3.0-or-later with the same Modding Exception and GPL-3.0 Linking Exception.

- Project: <https://github.com/alandtse/CommonLibSSE-NG>
- Active license: `COPYING` and `EXCEPTIONS.md`
- Original MIT lineage notice retained as `LICENSES/CommonLibSSE-NG-original-MIT.txt`

## SKSE Menu Framework 3

Optional in-game controls interoperate dynamically with SKSE Menu Framework 3 through its exported API. SKSE Menu Framework itself is not bundled in the MO2 package.

- License copy retained for the API integration: `LICENSES/SKSEMenuFramework-LGPL-2.1.txt`

## Noto Sans JP

Noto Sans JP is bundled as `SKSE/Plugins/Fonts/MainFont.ttf` for the Japanese interface under the SIL Open Font License 1.1.

- Complete license: `SKSE/Plugins/Fonts/NotoSansJP-OFL.txt`
