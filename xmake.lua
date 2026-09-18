set_xmakever("3.0.0")

local plugin_version = "1.0.1"

set_project("CinematicDoFStandalone")
set_version(plugin_version)
set_arch("x64")
set_languages("c++23")
set_warnings("allextra")
set_encodings("utf-8")
add_rules("mode.debug", "mode.releasedbg")
set_defaultmode("releasedbg")

-- Match the CommonLibSSE-NG library built for this workspace.
set_config("skyrim_se", true)
set_config("skyrim_ae", true)
set_config("skyrim_vr", false)
set_config("rex_ini", false)
set_config("rex_json", false)
set_config("rex_toml", false)
set_config("skse_xbyak", false)

includes("lib/commonlibsse-ng")

target("CinematicDoFStandalone", function()
    set_kind("shared")
    set_targetdir("work/artifacts/" .. plugin_version)
    add_deps("commonlibsse-ng")
    add_rules("commonlibsse-ng.plugin", {
        name = "CinematicDoFStandalone",
        author = "kota (@kotaSkyrim) / developed with OpenAI Codex",
        description = "Standalone cinematic depth of field with optional live controls for Skyrim SE/AE",
        version = plugin_version,
        options = {
            address_library = true,
        },
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    set_pcxxheader("src/PCH.h")
    add_defines("NOMINMAX", "WIN32_LEAN_AND_MEAN")
    add_syslinks("d3d11", "d3dcompiler", "dxgi")
end)
