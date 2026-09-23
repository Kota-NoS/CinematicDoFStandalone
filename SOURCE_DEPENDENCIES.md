# Source and build dependencies

This repository contains the corresponding source for CinematicDoFStandalone 1.0.2 and the CommonLibSSE-NG source snapshot used for the build. Generated binaries, build directories, package caches, and object files are intentionally excluded from Git. A separately published source archive may include the matching release DLL and PDB under `package` for source correspondence and debugging.

## Toolchain

- xmake 3.0.0 or later; the release build used xmake 3.0.9+HEAD
- Microsoft Visual C++ with C++23 support
- Windows SDK and the Direct3D 11 shader compiler libraries

From the archive root:

```powershell
xmake f -m releasedbg -a x64
xmake build CinematicDoFStandalone
```

The DLL is written to `work/artifacts/1.0.2/CinematicDoFStandalone.dll`.

## Included source dependency

- CommonLibSSE-NG 6.1.0 source snapshot: https://github.com/alandtse/CommonLibSSE-NG
  - The snapshot's `COPYING`, `EXCEPTIONS.md`, and historical `LICENSE` files are retained.
  - This project enables Skyrim SE and AE, disables VR, and uses Address Library compatibility.
  - The bundled snapshot carries the minimal compatibility backport validated for Skyrim 1.7.104: Skyrim minor versions 6 and newer are classified as AE, both supported SKSE plugin-declaration forms emit the Address Library v5 flag, and the relocation loader reads the format 5 dense ID table while preserving the older format 1/2 path.
  - The renderer separately adds display-setting-scoped target protection documented in `RUNTIME_1.7.104_VALIDATION.md`; no preset or INI schema was changed.

## Resolved xmake dependencies

These are fetched by xmake when needed and are not bundled as compiled libraries in the source archive.

- DirectXMath 2024.02: https://github.com/microsoft/DirectXMath
- DirectXTK 24.2.0: https://github.com/microsoft/DirectXTK
- spdlog 1.16.0, header-only with `std::format`: https://github.com/gabime/spdlog

Optional CommonLibSSE-NG dependencies disabled by this project's build configuration are not required for the published DLL.

## Runtime requirements not bundled

- SKSE64: https://skse.silverlock.org/
- Address Library for SKSE Plugins: https://www.nexusmods.com/skyrimspecialedition/mods/32444

## Optional integrations

- SKSE Menu Framework 3.8.0 for the in-game UI: https://www.nexusmods.com/skyrimspecialedition/mods/120352
- Community Shaders: https://github.com/community-shaders/skyrim-community-shaders

Community Shaders is not required; the plugin also works without it. If Community Shaders is installed, disable its Depth of Field effect to avoid double DoF. SKSE Menu Framework is dynamically used at runtime and is not redistributed in this package.
