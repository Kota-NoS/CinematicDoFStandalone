#include "DoFRenderer.h"
#include "DoFUI.h"
#include "HotkeyInput.h"
#include "Settings.h"

namespace
{
	std::atomic_bool firstMainPathLogged{ false };

	void SetupLogging()
	{
		auto directory = SKSE::log::log_directory();
		if (!directory) {
			return;
		}
		*directory /= "CinematicDoFStandalone.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(directory->string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
	}

	struct MainPostProcessingHook
	{
		static void Thunk(
			RE::ImageSpaceManager* a_imageSpaceManager,
			std::uint32_t a_unk1,
			RE::RENDER_TARGET a_target,
			void* a_unk2,
			bool a_unk3)
		{
			if (!firstMainPathLogged.exchange(true)) {
				spdlog::info("Entered main post-processing path");
			}
			CDoF::DoFRenderer::GetSingleton().Apply();
			function(a_imageSpaceManager, a_unk1, a_target, a_unk2, a_unk3);
		}
		static inline REL::Relocation<decltype(Thunk)> function;
	};

	void InstallHooks()
	{
		static std::atomic_bool installed{ false };
		if (installed.exchange(true)) {
			return;
		}

		static REL::Relocation<std::uintptr_t> mainPostProcessing{
			REL::RelocationID(100430, 107148),
			static_cast<std::ptrdiff_t>(REL::VariantOffset(0x1F0, 0x1E7, 0x206).offset())
		};
		MainPostProcessingHook::function = SKSE::GetTrampoline().write_call<5>(
			mainPostProcessing.address(),
			MainPostProcessingHook::Thunk);

		spdlog::info("Installed main post-processing hook at 0x{:X}", mainPostProcessing.address());
	}

	void MessageHandler(SKSE::MessagingInterface::Message* a_message)
	{
		if (!a_message) {
			return;
		}
		auto& renderer = CDoF::DoFRenderer::GetSingleton();
		if (a_message->type == SKSE::MessagingInterface::kPostLoad) {
			renderer.LogDisplaySettingsCheckpoint("kPostLoad");
			CDoF::UI::TryRegister();
		} else if (a_message->type == SKSE::MessagingInterface::kPostPostLoad) {
			renderer.LogDisplaySettingsCheckpoint("kPostPostLoad");
		} else if (a_message->type == SKSE::MessagingInterface::kInputLoaded) {
			// Test 7C established that the selected profile's display settings are
			// applied between kPostPostLoad and kInputLoaded, independently of
			// Community Shaders. Capture here before render-time state can diverge.
			renderer.CaptureInputLoadedDisplaySettings();
			renderer.LogDisplaySettingsCheckpoint("kInputLoaded");
		} else if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
			renderer.LogDisplaySettingsCheckpoint("kDataLoaded");
			CDoF::HotkeyInput::Register();
			// Install at DataLoaded so an optional Community Shaders thunk already
			// placed at PostPostLoad is preserved as our original call. Without
			// Community Shaders, the same hook continues through the vanilla path.
			InstallHooks();
			CDoF::UI::TryRegister();
		}
	}
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	SetupLogging();
	spdlog::info("CinematicDoFStandalone loading");

	try {
		SKSE::Init(a_skse, false);
		SKSE::AllocTrampoline(28);
		const auto settings = CDoF::LoadSettings();
		auto& renderer = CDoF::DoFRenderer::GetSingleton();
		renderer.SetSettings(settings);
		renderer.LogDisplaySettingsCheckpoint("SKSEPlugin_Load");
		CDoF::UI::Initialize(settings);
		if (!SKSE::GetMessagingInterface()->RegisterListener(MessageHandler)) {
			spdlog::critical("Failed to register SKSE message listener");
			return false;
		}
		spdlog::info("Runtime {} accepted; waiting for post-load", REL::Module::get().version().string());
		return true;
	} catch (const std::exception& error) {
		spdlog::critical("Plugin load failed: {}", error.what());
		return false;
	} catch (...) {
		spdlog::critical("Plugin load failed with an unknown exception");
		return false;
	}
}
