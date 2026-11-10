#include "DoFUI.h"

#include "DoFRenderer.h"
#include "SubjectMask.h"
#include "MenuFramework.h"
#include "TDMIntegration.h"

#include <format>

namespace
{
	using namespace CDoF;

	Settings uiSettings{};
	TargetFocusSettings targetFocusSettings{};
	TDMFocusSettings tdmFocusSettings{};
	SneakFocusSettings sneakFocusSettings{};
	ModeSettings modeSettings{};
	SilhouetteSettings silhouetteSettings{};
	DepthFrameSettings depthFrameSettings{};
	ExperimentalTransparencySettings experimentalTransparencySettings{};
	InterfaceSettings interfaceSettings{};
	HotkeySettings hotkeySettings{};
	std::mutex uiMutex;
	bool initialized{};
	bool registered{};
	bool showAdvanced{};
	enum class HotkeyCaptureTarget
	{
		kNone,
		kDoF,
		kSilhouette
	};
	HotkeyCaptureTarget hotkeyCaptureTarget{ HotkeyCaptureTarget::kNone };
	MenuFramework::WindowInterface* silhouetteWindow{};
	MenuFramework::WindowInterface* depthFrameWindow{};
	MenuFramework::WindowInterface* focusPickerWindow{};
	bool focusPickerActive{};
	bool focusPickerAcceptInput{};
	bool focusPickerFirstPerson{};
	float focusPickerOriginalX{ 0.5F };
	float focusPickerOriginalY{ 0.5F };
	bool focusPickerOriginalAutoFocus{};
	bool focusPickerOriginalConsoleEnabled{};
	bool japaneseFontEnabled{};
	std::optional<std::size_t> editingPresetIndex{};
	std::optional<std::size_t> pendingPresetResetIndex{};
	std::string statusTextEnglish{ "Values apply immediately. Use Save Startup Settings to keep them." };
	std::string statusTextJapanese{ "変更はすぐ反映されます。次回起動にも残す場合は「次回起動設定を保存」を押してください。" };

	const char* Localized(const char* a_english, const char* a_japanese)
	{
		return interfaceSettings.japanese ? a_japanese : a_english;
	}

	bool IsFirstPersonView()
	{
		const auto camera = RE::PlayerCamera::GetSingleton();
		return camera && camera->IsInFirstPerson();
	}

	bool IsFirstPersonScreenAFFallbackActive()
	{
		return IsFirstPersonView() &&
			targetFocusSettings.consoleEnabled &&
			targetFocusSettings.targetSource == TargetFocusSource::kPlayer &&
			targetFocusSettings.useFirstPersonScreenAFFallback;
	}

	float& ScreenFocusX(Settings& a_settings, bool a_firstPerson)
	{
		return a_firstPerson ? a_settings.firstPersonFocusX : a_settings.focusX;
	}

	float& ScreenFocusY(Settings& a_settings, bool a_firstPerson)
	{
		return a_firstPerson ? a_settings.firstPersonFocusY : a_settings.focusY;
	}

	void SetStatus(std::string a_english, std::string a_japanese)
	{
		statusTextEnglish = std::move(a_english);
		statusTextJapanese = std::move(a_japanese);
	}

	bool CheckboxWithHelp(
		const char* a_englishLabel,
		const char* a_japaneseLabel,
		bool* a_value,
		const char* a_englishHelp,
		const char* a_japaneseHelp)
	{
		const auto label = std::string(Localized(a_englishLabel, a_japaneseLabel));
		const auto changed = MenuFramework::Checkbox(label.c_str(), a_value);
		MenuFramework::ItemTooltip(Localized(a_englishHelp, a_japaneseHelp));
		return changed;
	}

	bool SliderWithHelp(
		const char* a_englishLabel,
		const char* a_japaneseLabel,
		float* a_value,
		float a_min,
		float a_max,
		const char* a_format,
		const char* a_englishHelp,
		const char* a_japaneseHelp)
	{
		const auto label = std::string(Localized(a_englishLabel, a_japaneseLabel));
		const auto changed = MenuFramework::SliderFloat(label.c_str(), a_value, a_min, a_max, a_format);
		MenuFramework::ItemTooltip(Localized(a_englishHelp, a_japaneseHelp));
		return changed;
	}

	bool ColorEditWithHelp(
		const char* a_englishLabel,
		const char* a_japaneseLabel,
		std::array<float, 3>& a_color,
		const char* a_englishHelp,
		const char* a_japaneseHelp)
	{
		const auto label = std::string(Localized(a_englishLabel, a_japaneseLabel));
		const auto changed = MenuFramework::ColorEdit3(label.c_str(), a_color.data());
		MenuFramework::ItemTooltip(Localized(a_englishHelp, a_japaneseHelp));
		if (changed) {
			for (auto& component : a_color) {
				component = std::clamp(component, 0.0F, 1.0F);
			}
		}
		return changed;
	}

	bool ApertureBladeSlider(Settings& a_settings)
	{
		float bladeCount = static_cast<float>(a_settings.apertureBlades);
		const auto changed = SliderWithHelp(
			"Aperture Blades", "絞り羽根枚数", &bladeCount, 3.0F, 12.0F, "%.0f",
			"Sets the number of aperture blades. Fewer blades make the polygonal shape easier to see on strongly defocused bright points.",
			"絞り羽根の枚数です。枚数を少なくすると、大きくぼけた明るい点で多角形の形が見えやすくなります。");
		if (changed) {
			a_settings.apertureBlades = static_cast<std::uint32_t>(
				std::clamp(std::lround(bladeCount), 3L, 12L));
		}
		return changed;
	}

	bool StarburstRaySelector(Settings& a_settings)
	{
		bool changed = false;
		MenuFramework::Text(Localized("Diffraction Rays", "光条の本数"));
		MenuFramework::SameLine();
		constexpr std::array<std::uint32_t, 6> rayCounts{ 4U, 5U, 6U, 8U, 10U, 12U };
		for (std::size_t index = 0; index < rayCounts.size(); ++index) {
			if (index != 0)
				MenuFramework::SameLine();
			const auto rays = rayCounts[index];
			const auto label = std::format("{}{}##StarburstRays{}", a_settings.starburstRays == rays ? "*" : "", rays, rays);
			if (MenuFramework::Button(label.c_str())) {
				a_settings.starburstRays = rays;
				changed = true;
			}
		}
		MenuFramework::ItemTooltip(Localized(
			"Selects the total number of diffraction rays. Odd counts such as five are generated as individually spaced directions.",
			"回折光条の総本数です。5本などの奇数も、各方向を等間隔に個別生成します。"));
		return changed;
	}

	bool SquaredRangeSliderWithHelp(
		const char* a_englishLabel,
		const char* a_japaneseLabel,
		float* a_valueMeters,
		float a_maxMeters,
		const char* a_englishHelp,
		const char* a_japaneseHelp)
	{
		const auto clampedMeters = std::clamp(*a_valueMeters, 0.0F, a_maxMeters);
		float sliderPosition = a_maxMeters > 0.0F ? std::sqrt(clampedMeters / a_maxMeters) : 0.0F;
		const auto valueText = std::format("{:.2f} m", clampedMeters);
		const auto label = std::string(Localized(a_englishLabel, a_japaneseLabel));
		const auto changed = MenuFramework::SliderFloat(
			label.c_str(), &sliderPosition, 0.0F, 1.0F, valueText.c_str());
		if (changed) {
			sliderPosition = std::clamp(sliderPosition, 0.0F, 1.0F);
			*a_valueMeters = a_maxMeters * sliderPosition * sliderPosition;
		}
		MenuFramework::ItemTooltip(Localized(a_englishHelp, a_japaneseHelp));
		return changed;
	}

	bool LogDistanceSliderWithHelp(
		const char* a_englishLabel,
		const char* a_japaneseLabel,
		float* a_valueMeters,
		float a_minMeters,
		float a_maxMeters,
		const char* a_englishHelp,
		const char* a_japaneseHelp)
	{
		const auto clampedMeters = std::clamp(*a_valueMeters, a_minMeters, a_maxMeters);
		const auto logRange = std::log(a_maxMeters / a_minMeters);
		float sliderPosition = std::log(clampedMeters / a_minMeters) / logRange;
		const auto valueText = std::format("{:.2f} m", clampedMeters);
		const auto label = std::string(Localized(a_englishLabel, a_japaneseLabel));
		const auto changed = MenuFramework::SliderFloat(
			label.c_str(), &sliderPosition, 0.0F, 1.0F, valueText.c_str());
		if (changed) {
			sliderPosition = std::clamp(sliderPosition, 0.0F, 1.0F);
			*a_valueMeters = a_minMeters * std::exp(logRange * sliderPosition);
		}
		MenuFramework::ItemTooltip(Localized(a_englishHelp, a_japaneseHelp));
		return changed;
	}

	bool ChoiceButton(
		const char* a_englishLabel,
		const char* a_japaneseLabel,
		const char* a_id,
		bool a_selected,
		const char* a_englishHelp,
		const char* a_japaneseHelp)
	{
		const auto visible = std::string(Localized(a_englishLabel, a_japaneseLabel));
		const auto label = (a_selected ? "[" + visible + "]" : visible) + "##" + a_id;
		const auto pressed = MenuFramework::Button(label.c_str());
		MenuFramework::ItemTooltip(Localized(a_englishHelp, a_japaneseHelp));
		return pressed;
	}

	bool IsJapaneseFontEnabled()
	{
		wchar_t value[16]{};
		GetPrivateProfileStringW(
			L"Fonts",
			L"EnableJapanese",
			L"false",
			value,
			static_cast<DWORD>(std::size(value)),
			L"Data\\SKSE\\Plugins\\SKSEMenuFramework.ini");
		std::wstring normalized(value);
		std::ranges::transform(normalized, normalized.begin(), [](wchar_t a_character) {
			return static_cast<wchar_t>(std::towlower(a_character));
		});
		return normalized == L"true" || normalized == L"yes" || normalized == L"on" || normalized == L"1";
	}

	bool PushLocalizedFont()
	{
		return interfaceSettings.japanese && MenuFramework::PushFont("MainFont.ttf");
	}

	void RenderLanguageControls()
	{
		MenuFramework::SameLine();
		const auto englishLabel = interfaceSettings.japanese ?
			"English##InterfaceEnglish" : "[English]##InterfaceEnglish";
		if (MenuFramework::Button(englishLabel) && interfaceSettings.japanese) {
			interfaceSettings.japanese = false;
			SetStatus(
				"Interface language changed (use Save Startup Settings to keep it).",
				"表示言語を変更しました（次回起動にも残すには「次回起動設定を保存」）。");
		}
		MenuFramework::ItemTooltip("Switch the interface to English. The selected language is shown in brackets.");
		MenuFramework::SameLine();
		const auto japaneseLabel = interfaceSettings.japanese ?
			"[日本語]##InterfaceJapanese" : "日本語##InterfaceJapanese";
		if (MenuFramework::Button(japaneseLabel) && !interfaceSettings.japanese) {
			interfaceSettings.japanese = true;
			SetStatus(
				"Interface language changed (use Save Startup Settings to keep it).",
				"表示言語を変更しました（次回起動にも残すには「次回起動設定を保存」）。");
		}
		MenuFramework::ItemTooltip(Localized(
			"Switch the interface to Japanese. The selected language is shown in brackets.",
			"表示を日本語へ切り替えます。選択中の言語は角括弧で表示されます。"));
		if (interfaceSettings.japanese && !japaneseFontEnabled) {
			MenuFramework::Text("Japanese glyphs are disabled in SKSE Menu Framework.");
			MenuFramework::Text("Set [Fonts] EnableJapanese=true in SKSEMenuFramework.ini, then restart Skyrim.");
		}
	}

	struct PresetSlot
	{
		std::uint32_t storageId;
		const char* name;
		const char* japaneseName;
		const wchar_t* section;
		const wchar_t* legacySection;
		Settings defaults;
		Settings values;
		TargetFocusSettings targetDefaults;
		TargetFocusSettings targetValues;
	};

	void ApplyTargetFocus();

	void ApplyLive()
	{
		DoFRenderer::GetSingleton().SetSettings(uiSettings);
		DoFRenderer::GetSingleton().SetModeSettings(modeSettings);
		DoFRenderer::GetSingleton().SetSilhouetteSettings(silhouetteSettings);
		DoFRenderer::GetSingleton().SetDepthFrameSettings(depthFrameSettings);
		ApplyTargetFocus();
		SetStatus("Live values changed (not saved).", "現在値を変更しました（INI未保存）。");
	}

	void FinishFocusPicker(bool a_confirmed, bool a_reopenMainWindow)
	{
		if (!a_confirmed) {
			ScreenFocusX(uiSettings, focusPickerFirstPerson) = focusPickerOriginalX;
			ScreenFocusY(uiSettings, focusPickerFirstPerson) = focusPickerOriginalY;
			uiSettings.autoFocus = focusPickerOriginalAutoFocus;
			targetFocusSettings.consoleEnabled = focusPickerOriginalConsoleEnabled;
		}
		DoFRenderer::GetSingleton().SetSettings(uiSettings);
		ApplyTargetFocus();
		DoFRenderer::GetSingleton().SetFocusPickerActive(false);
		focusPickerActive = false;
		focusPickerAcceptInput = false;
		if (focusPickerWindow) {
			focusPickerWindow->isOpen = false;
		}
		if (a_reopenMainWindow) {
			if (auto* mainWindow = MenuFramework::GetMainWindow()) {
				mainWindow->isOpen = true;
			}
		}
		SetStatus(
			a_confirmed ?
				"Screen AF position selected (not saved)." :
				"Screen AF position selection cancelled.",
			a_confirmed ?
				"画面位置AFのピント位置を決定しました（INI未保存）。" :
				"画面位置AFのピント位置選択を中止しました。");
	}

	void BeginFocusPicker()
	{
		if (!focusPickerWindow) {
			SetStatus(
				"Screen AF visual selection is unavailable in this Menu Framework version.",
				"このMenu Frameworkでは画面位置AFの視覚選択を利用できません。");
			return;
		}
		focusPickerFirstPerson = IsFirstPersonView();
		focusPickerOriginalX = ScreenFocusX(uiSettings, focusPickerFirstPerson);
		focusPickerOriginalY = ScreenFocusY(uiSettings, focusPickerFirstPerson);
		focusPickerOriginalAutoFocus = uiSettings.autoFocus;
		focusPickerOriginalConsoleEnabled = targetFocusSettings.consoleEnabled;
		uiSettings.autoFocus = true;
		targetFocusSettings.consoleEnabled = false;
		DoFRenderer::GetSingleton().SetSettings(uiSettings);
		ApplyTargetFocus();
		DoFRenderer::GetSingleton().SetFocusPickerActive(true);
		focusPickerActive = true;
		// The picker window may render later in the same ImGui frame as this
		// button. Ignore that frame so the activation click cannot confirm itself.
		focusPickerAcceptInput = false;
		focusPickerWindow->isOpen = true;
		if (auto* mainWindow = MenuFramework::GetMainWindow()) {
			mainWindow->isOpen = false;
		}
		SetStatus(
			"Move the mouse, then left-click to confirm. Right-click or Esc cancels.",
			"マウスで位置を動かし、左クリックで決定します。右クリックまたはEscで中止します。");
	}

	constexpr std::uint32_t kEscapeKey{ 0x01 };
	constexpr std::uint32_t kBackspaceKey{ 0x0E };
	constexpr std::uint32_t kDeleteKey{ 0xD3 };

	std::string HotkeyName(std::uint32_t a_keyCode)
	{
		if (a_keyCode == 0) {
			return Localized("Unassigned", "未設定");
		}

		if (auto* inputManager = RE::BSInputDeviceManager::GetSingleton()) {
			RE::BSFixedString keyName;
			inputManager->GetButtonNameFromID(
				RE::INPUT_DEVICE::kKeyboard,
				static_cast<std::int32_t>(a_keyCode),
				keyName);
			if (keyName.c_str() && keyName.c_str()[0] != '\0') {
				return keyName.c_str();
			}
		}

		return std::format("Key 0x{:02X}", a_keyCode);
	}

	void BeginHotkeyCapture(HotkeyCaptureTarget a_target)
	{
		hotkeyCaptureTarget = a_target;
		SetStatus(
			"Press a keyboard key to assign it. Esc cancels; Backspace or Delete clears it.",
			"登録するキーボードのキーを押してください。Escで中止、BackspaceまたはDeleteで解除します。");
	}

	void RenderDoFHotkeyControl()
	{
		MenuFramework::SameLine();
		const auto label = hotkeyCaptureTarget == HotkeyCaptureTarget::kDoF ?
			std::string(Localized("Press a key...", "キーを押してください...")) + "##ToggleDoFHotkey" :
			std::string(Localized("Hotkey: ", "ホットキー：")) + HotkeyName(hotkeySettings.toggleDoFKey) +
				"##ToggleDoFHotkey";
		if (MenuFramework::Button(label.c_str())) {
			BeginHotkeyCapture(HotkeyCaptureTarget::kDoF);
		}
		MenuFramework::ItemTooltip(Localized(
			"Assigns a keyboard hotkey that toggles DoF without saving the enabled state. Esc cancels assignment; Backspace or Delete clears it. Avoid keys used by Skyrim or other mods because both actions may run.",
			"DoFのON/OFFを切り替えるキーボードのホットキーです。切替状態は自動保存しません。Escで登録を中止し、BackspaceまたはDeleteで解除します。Skyrimや他MODと同じキーでは両方の操作が実行される場合があります。"));
	}

	void RenderSilhouetteHotkeyControl()
	{
		const auto label = hotkeyCaptureTarget == HotkeyCaptureTarget::kSilhouette ?
			std::string(Localized("Press a key...", "キーを押してください...")) + "##ToggleSilhouetteHotkey" :
			std::string(Localized("Hotkey: ", "ホットキー：")) +
				HotkeyName(hotkeySettings.toggleSilhouetteKey) + "##ToggleSilhouetteHotkey";
		if (MenuFramework::Button(label.c_str())) {
			BeginHotkeyCapture(HotkeyCaptureTarget::kSilhouette);
		}
		MenuFramework::ItemTooltip(Localized(
			"Assigns a keyboard hotkey that toggles silhouette mode even while this window is closed. The enabled state is not saved automatically. Esc cancels assignment; Backspace or Delete clears it.",
			"この小窓を閉じていてもシルエット撮影モードをON/OFFできるキーボードのホットキーです。切替状態は自動保存しません。Escで登録を中止し、BackspaceまたはDeleteで解除します。"));
	}

	Settings GameplayPreset()
	{
		Settings preset{};
		preset.autoFocus = false;
		preset.transitionSpeed = 0.9F;
		preset.manualFocusMeters = 46.09F;
		preset.focalLength = 20.6F;
		preset.fNumber = 6.4F;
		preset.farPlaneMaxBlur = 1.91F;
		preset.nearPlaneMaxBlur = 0.0F;
		preset.blurQuality = 5.0F;
		preset.nearFarDistanceCompensation = 3.98F;
		preset.bokehBusyFactor = 0.25F;
		preset.keepSkySharp = true;
		return preset;
	}

	Settings CinematicPreset()
	{
		Settings preset{};
		preset.autoFocus = false;
		preset.transitionSpeed = 0.9F;
		preset.manualFocusMeters = 46.09F;
		preset.focalLength = 52.6F;
		preset.fNumber = 15.2F;
		preset.nearFocusRangeMeters = 0.0740728F;
		preset.farFocusRangeMeters = 64.7113F;
		preset.farPlaneMaxBlur = 1.49F;
		preset.nearPlaneMaxBlur = 2.45F;
		preset.blurQuality = 5.0F;
		preset.nearFarDistanceCompensation = 3.94F;
		preset.bokehBusyFactor = 0.0F;
		preset.keepSkySharp = true;
		return preset;
	}

	Settings PhotoPortraitPreset()
	{
		Settings preset{};
		preset.autoFocus = false;
		preset.transitionSpeed = 0.9F;
		preset.manualFocusMeters = 46.09F;
		preset.focalLength = 117.7F;
		preset.fNumber = 9.8F;
		preset.nearFocusRangeMeters = 0.0574862F;
		preset.farFocusRangeMeters = 2.1F;
		preset.farPlaneMaxBlur = 2.34F;
		preset.nearPlaneMaxBlur = 1.81F;
		preset.blurQuality = 8.0F;
		preset.nearFarDistanceCompensation = 2.5F;
		preset.bokehBusyFactor = 0.4F;
		preset.keepSkySharp = false;
		return preset;
	}

	Settings PhotoWidePreset()
	{
		Settings preset{};
		preset.autoFocus = false;
		preset.transitionSpeed = 0.9F;
		preset.manualFocusMeters = 46.09F;
		preset.focalLength = 98.6F;
		preset.fNumber = 10.6F;
		preset.nearFocusRangeMeters = 0.100821F;
		preset.farFocusRangeMeters = 2.7F;
		preset.farPlaneMaxBlur = 0.36F;
		preset.nearPlaneMaxBlur = 1.81F;
		preset.blurQuality = 8.0F;
		preset.nearFarDistanceCompensation = 2.5F;
		preset.bokehBusyFactor = 0.0F;
		preset.keepSkySharp = true;
		return preset;
	}

	Settings Custom1Preset()
	{
		Settings preset{};
		preset.autoFocus = false;
		preset.transitionSpeed = 0.9F;
		preset.manualFocusMeters = 46.09F;
		preset.focalLength = 110.9F;
		preset.fNumber = 20.1F;
		preset.nearFocusRangeMeters = 3.57F;
		preset.farPlaneMaxBlur = 4.18F;
		preset.nearPlaneMaxBlur = 2.6F;
		preset.blurQuality = 10.0F;
		preset.nearFarDistanceCompensation = 4.14F;
		preset.bokehBusyFactor = 0.0F;
		preset.highlightBoost = 0.11F;
		preset.keepSkySharp = false;
		return preset;
	}

	Settings Custom2Preset()
	{
		Settings preset{};
		preset.autoFocus = false;
		preset.transitionSpeed = 0.9F;
		preset.manualFocusMeters = 46.09F;
		preset.focalLength = 81.1F;
		preset.fNumber = 8.3F;
		preset.nearFocusRangeMeters = 0.0416233F;
		preset.farFocusRangeMeters = 55.0962F;
		preset.farPlaneMaxBlur = 0.90F;
		preset.nearPlaneMaxBlur = 0.5F;
		preset.blurQuality = 7.5F;
		preset.nearFarDistanceCompensation = 4.0F;
		preset.bokehBusyFactor = 0.0F;
		preset.highlightBoost = 0.08F;
		preset.starburstStrength = 6.66F;
		preset.starburstLength = 0.84F;
		preset.highlightExtractionThreshold = 0.84F;
		preset.starburstStrongLightSuppression = 0.64F;
		preset.keepSkySharp = false;
		return preset;
	}

	Settings ViewFocusPreset()
	{
		Settings preset{};
		preset.autoFocus = true;
		preset.focusX = 0.5F;
		preset.focusY = 0.39F;
		preset.transitionSpeed = 0.1F;
		preset.manualFocusMeters = 46.09F;
		preset.focalLength = 28.7F;
		preset.fNumber = 9.6F;
		preset.farPlaneMaxBlur = 6.16F;
		preset.nearPlaneMaxBlur = 0.0F;
		preset.blurQuality = 5.0F;
		preset.nearFarDistanceCompensation = 3.94F;
		preset.bokehBusyFactor = 0.0F;
		preset.keepSkySharp = true;
		return preset;
	}

	Settings FirstPersonPhotoPreset()
	{
		Settings preset{};
		preset.autoFocus = false;
		preset.focusX = 0.5F;
		preset.focusY = 0.5F;
		preset.transitionSpeed = 0.9F;
		preset.manualFocusMeters = 46.09F;
		preset.focalLength = 134.3F;
		preset.fNumber = 18.3F;
		preset.farFocusRangeMeters = 0.76F;
		preset.farPlaneMaxBlur = 2.25F;
		preset.nearPlaneMaxBlur = 1.08F;
		preset.enableFirstPersonNearBlur = true;
		preset.blurQuality = 7.5F;
		preset.nearFarDistanceCompensation = 2.5F;
		preset.bokehBusyFactor = 0.45F;
		preset.keepSkySharp = false;
		return preset;
	}

	Settings ApertureBokehPreset()
	{
		Settings preset{};
		preset.enabled = true;
		preset.autoFocus = false;
		preset.disableInMenus = true;
		preset.focusX = 0.5F;
		preset.focusY = 0.5F;
		preset.autoFocusOffsetMeters = 0.0F;
		preset.transitionSpeed = 0.9F;
		preset.manualFocusMeters = 46.09F;
		preset.focalLength = 84.2F;
		preset.fNumber = 3.4F;
		preset.farPlaneMaxBlur = 1.81F;
		preset.nearPlaneMaxBlur = 1.0F;
		preset.enableFirstPersonNearBlur = false;
		preset.blurQuality = 8.0F;
		preset.nearFarDistanceCompensation = 1.0F;
		preset.bokehBusyFactor = 0.0F;
		preset.highlightBoost = 0.45F;
		preset.postBlurSmoothing = 0.2F;
		preset.petzvalStrength = 0.0F;
		preset.apertureBokeh = true;
		preset.apertureFill = true;
		preset.apertureShape = 0;
		preset.apertureBlades = 5;
		preset.apertureRoundness = 0.1F;
		preset.apertureShapeStrength = 0.90F;
		preset.apertureRotationDegrees = 0.0F;
		preset.highlightExtractionThreshold = 0.36F;
		preset.bokehCoreFill = 0.50F;
		preset.starburstEnabled = false;
		preset.starburstRays = 6;
		preset.starburstStrength = 1.0F;
		preset.starburstLength = 0.65F;
		preset.starburstRotationDegrees = 0.0F;
		preset.nearFocusRangeMeters = 0.0F;
		preset.farFocusRangeMeters = 2.1F;
		preset.keepSkySharp = false;
		return preset;
	}

	Settings ShapeStarburstPreset()
	{
		auto preset = ApertureBokehPreset();
		preset.focalLength = 37.0F;
		preset.fNumber = 2.8F;
		preset.nearFocusRangeMeters = 0.0274115F;
		preset.farPlaneMaxBlur = 0.43F;
		preset.highlightBoost = 0.35F;
		preset.postBlurSmoothing = 0.0F;
		preset.apertureBlades = 6;
		preset.apertureShapeStrength = 0.19F;
		preset.highlightExtractionThreshold = 0.32F;
		preset.bokehCoreFill = 0.50F;
		preset.starburstEnabled = true;
		preset.starburstRays = 6;
		preset.starburstStrength = 7.0F;
		preset.starburstLength = 0.75F;
		preset.starburstRotationDegrees = 0.0F;
		preset.starburstWideFlareSuppression = 0.25F;
		preset.starburstStrongLightSuppression = 0.65F;
		preset.apertureFill = false;
		preset.starburstHighlightSensitivity = 1.75F;
		return preset;
	}

	TargetFocusSettings TargetDefaults(
		bool a_dialogueEnabled,
		std::uint32_t a_dialoguePreset,
		float a_dialogueDepthStrength,
		bool a_consoleEnabled,
		TargetFocusSource a_targetSource = TargetFocusSource::kPlayer)
	{
		TargetFocusSettings result{};
		result.dialogueEnabled = a_dialogueEnabled;
		result.dialoguePreset = a_dialoguePreset;
		result.dialogueDepthStrength = a_dialogueDepthStrength;
		result.consoleEnabled = a_consoleEnabled;
		result.targetSource = a_targetSource;
		return result;
	}

	// storageId is persisted as DialoguePreset. Keep existing IDs stable; array order alone controls UI slot order.
	std::array<PresetSlot, kPresetSlotCount> presetSlots{
		PresetSlot{ 0, "Gameplay", "通常プレイ", L"Preset.Gameplay", nullptr, GameplayPreset(), {},
			TargetDefaults(true, 0, 2.5F, false), {} },
		PresetSlot{ 1, "Cinematic", "シネマ", L"Preset.Cinematic", nullptr, CinematicPreset(), {},
			TargetDefaults(true, 7, 0.83F, true), {} },
		PresetSlot{ 6, "View Focus", "視点連動", L"Preset.ViewFocus", nullptr, ViewFocusPreset(), {},
			TargetDefaults(true, 0, 1.56F, false), {} },
		PresetSlot{ 2, "Photo Portrait", "人物撮影", L"Preset.PhotoPortrait", nullptr, PhotoPortraitPreset(), {},
			TargetDefaults(true, 7, 1.31F, true), {} },
		PresetSlot{ 3, "Photo Wide", "広角撮影", L"Preset.PhotoWide", nullptr, PhotoWidePreset(), {},
			TargetDefaults(true, 3, 1.6F, true), {} },
		PresetSlot{ 7, "First-Person Photo", "一人称撮影", L"Preset.FirstPersonPhoto", nullptr,
			FirstPersonPhotoPreset(), {}, TargetDefaults(true, 7, 0.49F, true, TargetFocusSource::kConsole), {} },
		PresetSlot{ 8, "Aperture Bokeh", "絞り形状", L"Preset.ApertureBokeh", nullptr,
			ApertureBokehPreset(), {}, TargetDefaults(true, 7, 1.31F, true), {} },
		PresetSlot{ 9, "Starburst", "光条", L"Preset.ShapeStarburst", nullptr,
			ShapeStarburstPreset(), {}, TargetDefaults(true, 7, 1.31F, true), {} },
		PresetSlot{ 4, "Custom 1", "カスタム1", L"Preset.Custom1", L"Preset.Video", Custom1Preset(), {},
			TargetDefaults(true, 7, 1.21F, true), {} },
		PresetSlot{ 5, "Custom 2", "カスタム2", L"Preset.Custom2", nullptr, Custom2Preset(), {},
			TargetDefaults(true, 0, 1.0F, true), {} }
	};

	const PresetSlot& PresetByStorageId(std::uint32_t a_storageId)
	{
		const auto found = std::ranges::find(presetSlots, a_storageId, &PresetSlot::storageId);
		return found != presetSlots.end() ? *found : presetSlots.front();
	}

	bool NearlyEqual(float a_left, float a_right)
	{
		return std::abs(a_left - a_right) <= 0.0005F;
	}

	bool Matches(const Settings& a_left, const Settings& a_right)
	{
		return a_left.enabled == a_right.enabled &&
			a_left.autoFocus == a_right.autoFocus &&
			a_left.disableInMenus == a_right.disableInMenus &&
			a_left.enableFirstPersonNearBlur == a_right.enableFirstPersonNearBlur &&
			a_left.keepSkySharp == a_right.keepSkySharp &&
			a_left.apertureBokeh == a_right.apertureBokeh &&
			a_left.apertureFill == a_right.apertureFill &&
			a_left.apertureShape == a_right.apertureShape &&
			a_left.apertureBlades == a_right.apertureBlades &&
			a_left.starburstEnabled == a_right.starburstEnabled &&
			a_left.starburstRays == a_right.starburstRays &&
			NearlyEqual(a_left.transitionSpeed, a_right.transitionSpeed) &&
			NearlyEqual(a_left.focusX, a_right.focusX) &&
			NearlyEqual(a_left.focusY, a_right.focusY) &&
			NearlyEqual(a_left.firstPersonFocusX, a_right.firstPersonFocusX) &&
			NearlyEqual(a_left.firstPersonFocusY, a_right.firstPersonFocusY) &&
			NearlyEqual(a_left.autoFocusOffsetMeters, a_right.autoFocusOffsetMeters) &&
			NearlyEqual(a_left.manualFocusMeters, a_right.manualFocusMeters) &&
			NearlyEqual(a_left.focalLength, a_right.focalLength) &&
			NearlyEqual(a_left.fNumber, a_right.fNumber) &&
			NearlyEqual(a_left.nearFocusRangeMeters, a_right.nearFocusRangeMeters) &&
			NearlyEqual(a_left.farFocusRangeMeters, a_right.farFocusRangeMeters) &&
			NearlyEqual(a_left.farPlaneMaxBlur, a_right.farPlaneMaxBlur) &&
			NearlyEqual(a_left.nearPlaneMaxBlur, a_right.nearPlaneMaxBlur) &&
			NearlyEqual(a_left.blurQuality, a_right.blurQuality) &&
			NearlyEqual(a_left.nearFarDistanceCompensation, a_right.nearFarDistanceCompensation) &&
			NearlyEqual(a_left.bokehBusyFactor, a_right.bokehBusyFactor) &&
			NearlyEqual(a_left.highlightBoost, a_right.highlightBoost) &&
			NearlyEqual(a_left.postBlurSmoothing, a_right.postBlurSmoothing) &&
			NearlyEqual(a_left.petzvalStrength, a_right.petzvalStrength) &&
			NearlyEqual(a_left.apertureRoundness, a_right.apertureRoundness) &&
			NearlyEqual(a_left.apertureShapeStrength, a_right.apertureShapeStrength) &&
			NearlyEqual(a_left.apertureRotationDegrees, a_right.apertureRotationDegrees) &&
			NearlyEqual(a_left.starburstStrength, a_right.starburstStrength) &&
			NearlyEqual(a_left.starburstLength, a_right.starburstLength) &&
			NearlyEqual(a_left.highlightExtractionThreshold, a_right.highlightExtractionThreshold) &&
			NearlyEqual(a_left.starburstRotationDegrees, a_right.starburstRotationDegrees) &&
			NearlyEqual(a_left.bokehCoreFill, a_right.bokehCoreFill) &&
			NearlyEqual(a_left.starburstWideFlareSuppression, a_right.starburstWideFlareSuppression) &&
			NearlyEqual(a_left.starburstHighlightSensitivity, a_right.starburstHighlightSensitivity) &&
			NearlyEqual(a_left.starburstStrongLightSuppression, a_right.starburstStrongLightSuppression);
	}

	bool Matches(const TargetFocusSettings& a_left, const TargetFocusSettings& a_right)
	{
		return a_left.dialogueEnabled == a_right.dialogueEnabled &&
			a_left.dialoguePreset == a_right.dialoguePreset &&
			NearlyEqual(a_left.dialogueDepthStrength, a_right.dialogueDepthStrength) &&
			a_left.consoleEnabled == a_right.consoleEnabled &&
			a_left.targetSource == a_right.targetSource &&
			NearlyEqual(a_left.targetFocusOffsetMeters, a_right.targetFocusOffsetMeters) &&
			a_left.useFirstPersonScreenAFFallback == a_right.useFirstPersonScreenAFFallback;
	}

	std::optional<std::size_t> FindMatchingPreset()
	{
		for (std::size_t index = 0; index < presetSlots.size(); ++index) {
			if (Matches(uiSettings, presetSlots[index].values) &&
				Matches(targetFocusSettings, presetSlots[index].targetValues)) {
				return index;
			}
		}
		return std::nullopt;
	}

	bool CurrentValuesMatchPreset(std::size_t a_index)
	{
		return Matches(uiSettings, presetSlots[a_index].values) &&
			Matches(targetFocusSettings, presetSlots[a_index].targetValues);
	}

	void ApplyTargetFocus()
	{
		DoFRenderer::GetSingleton().SetTargetFocus(
			targetFocusSettings,
			PresetByStorageId(targetFocusSettings.dialoguePreset).values);
	}

	void ApplyTDMFocus()
	{
		DoFRenderer::GetSingleton().SetTDMFocus(tdmFocusSettings);
	}

	void ApplySneakFocus()
	{
		DoFRenderer::GetSingleton().SetSneakFocus(sneakFocusSettings);
	}

	void ApplyExperimentalTransparency()
	{
		DoFRenderer::GetSingleton().SetExperimentalTransparency(experimentalTransparencySettings);
	}

	void LoadPresets()
	{
		for (auto& preset : presetSlots) {
			const wchar_t* sourceSection = nullptr;
			if (PresetSectionExists(preset.section)) {
				sourceSection = preset.section;
			} else if (preset.legacySection && PresetSectionExists(preset.legacySection)) {
				sourceSection = preset.legacySection;
				spdlog::info(
					"Loading legacy preset {} into {}",
					std::filesystem::path(preset.legacySection).string(),
					std::filesystem::path(preset.section).string());
			}

			if (sourceSection) {
				preset.values = LoadPresetSettings(sourceSection, preset.defaults);
				preset.targetValues = LoadPresetTargetFocusSettings(sourceSection, targetFocusSettings);
			} else {
				preset.values = preset.defaults;
				preset.targetValues = preset.targetDefaults;
			}
		}
	}

	void ApplyPreset(std::size_t a_index)
	{
		auto& preset = presetSlots[a_index];
		pendingPresetResetIndex.reset();
		uiSettings = preset.values;
		targetFocusSettings = preset.targetValues;
		DoFRenderer::GetSingleton().SetSettings(uiSettings);
		ApplyTargetFocus();
		editingPresetIndex = a_index;
		SetStatus(
			std::string(preset.name) + " applied (startup settings not changed).",
			std::string(preset.japaneseName) + "を適用しました（次回起動設定は変更していません）。");
	}

	void StorePreset(std::size_t a_index)
	{
		auto& preset = presetSlots[a_index];
		pendingPresetResetIndex.reset();
		preset.values = uiSettings;
		preset.targetValues = targetFocusSettings;
		ApplyTargetFocus();
		const auto presetSaved = SavePresetSettings(preset.section, preset.values) &&
			SavePresetTargetFocusSettings(preset.section, preset.targetValues);
		if (presetSaved) {
			editingPresetIndex = a_index;
			SetStatus(
				std::string("Stored the current DoF and focus values in ") + preset.name +
					" (startup settings not changed).",
				std::string(preset.japaneseName) +
					"へ現在のDoF・フォーカス設定を保存しました（次回起動設定は変更していません）。");
		} else {
			SetStatus(
				std::string("Could not fully store ") + preset.name + ". See the plugin log.",
				std::string(preset.japaneseName) + "を完全には保存できませんでした。ログを確認してください。");
		}
	}

	void RestorePresetDefaults(std::size_t a_index)
	{
		auto& preset = presetSlots[a_index];
		preset.values = preset.defaults;
		preset.targetValues = preset.targetDefaults;
		uiSettings = preset.values;
		targetFocusSettings = preset.targetValues;
		DoFRenderer::GetSingleton().SetSettings(uiSettings);
		ApplyTargetFocus();
		editingPresetIndex = a_index;
		SetStatus(
			std::string("Loaded the original values for ") + preset.name +
				" and applied them (preset not saved). Use Store Current to save this slot.",
			std::string(preset.japaneseName) +
				"の初期値を映像へ適用しました（プリセット未保存）。保存する場合は「現在値を保存」を押してください。");
		pendingPresetResetIndex.reset();
	}

	void RenderTargetPresetSelector(
		const char* a_modeNameEnglish,
		const char* a_modeNameJapanese,
		std::uint32_t& a_selectedPreset)
	{
		const auto selectedStorageId = std::min(a_selectedPreset, kMaxPresetIndex);
		const auto heading = interfaceSettings.japanese ?
			std::string(a_modeNameJapanese) + "レンズ" : std::string(a_modeNameEnglish) + " lens";
		MenuFramework::Text(heading.c_str());
		MenuFramework::ItemTooltip(Localized(
			"Selects the stored lens values used while this focus override is active.",
			"この自動ピント中に使用する、保存済みレンズ設定の番号です。"));
		MenuFramework::SameLine();
		for (std::size_t index = 0; index < presetSlots.size(); ++index) {
			const auto selected = presetSlots[index].storageId == selectedStorageId;
			const auto label = std::string(selected ? "*" : "") + std::to_string(index + 1) + "##" + a_modeNameEnglish +
				"TargetPreset";
			if (MenuFramework::Button(label.c_str()) && !selected) {
				a_selectedPreset = presetSlots[index].storageId;
				ApplyTargetFocus();
				SetStatus(
					std::string(a_modeNameEnglish) + " lens set to slot [" + std::to_string(index + 1) + "] (not saved).",
					std::string(a_modeNameJapanese) + "レンズ枠を[" + std::to_string(index + 1) + "]へ変更しました（INI未保存）。");
			}
			const auto tooltip = interfaceSettings.japanese ?
				std::to_string(index + 1) + "：" + presetSlots[index].japaneseName :
				std::to_string(index + 1) + ": " + presetSlots[index].name;
			MenuFramework::ItemTooltip(tooltip.c_str());
			if (index + 1 < presetSlots.size()) {
				MenuFramework::SameLine();
			}
		}
	}

	void RenderDialogueFocusControls()
	{
		MenuFramework::SeparatorText(Localized("Dialogue Target Focus", "会話中の自動ピント"));
		bool changed = CheckboxWithHelp(
			"Focus on conversation subject",
			"会話中の被写体にピントを合わせる",
			&targetFocusSettings.dialogueEnabled,
			"During dialogue, follows the NPC or player currently framed by the conversation camera.",
			"会話中、カメラに映っているNPCまたはプレイヤーの頭へ自動的にピントを合わせます。");
		if (targetFocusSettings.dialogueEnabled) {
			changed |= SliderWithHelp(
				"Dialogue Depth Strength",
				"ぼかし倍率",
				&targetFocusSettings.dialogueDepthStrength,
				0.0F,
				3.0F,
				"%.2f x",
				"Multiplies the near and far maximum blur only during dialogue focus.",
				"会話フォーカス中だけ、手前と背景の最大ぼかし量へ倍率を掛けます。");
			RenderTargetPresetSelector("Dialogue", "会話用", targetFocusSettings.dialoguePreset);
		}
		if (changed) {
			ApplyTargetFocus();
			SetStatus("Dialogue target focus changed (not saved).", "会話中の自動ピント設定を変更しました（INI未保存）。");
		}
	}

	void RenderTDMLockFocusControls()
	{
		MenuFramework::SeparatorText(Localized("TDM Lock-On Focus", "TDMロックオン時の自動ピント"));
		bool changed = CheckboxWithHelp(
			"Focus on TDM lock-on target",
			"TDMロックオン対象へピントを合わせる",
			&tdmFocusSettings.enabled,
			"While True Directional Movement target lock is active, focus follows its current actor target. Dialogue focus keeps priority.",
			"True Directional Movementでロックオン中、その対象へピントを追従します。会話中は会話フォーカスを優先します。");
		if (tdmFocusSettings.enabled) {
			MenuFramework::Text(Localized(
				TDMIntegration::IsAvailable() ? "TDM: Detected" : "TDM: Not detected",
				TDMIntegration::IsAvailable() ? "TDM：検出済み" : "TDM：未検出"));
			MenuFramework::ItemTooltip(Localized(
				"True Directional Movement detection status and lock-on depth profile.",
				"True Directional Movementの検出状態と、ロックオン時の深度プロファイルです。"));
			const auto profileButton = [&](const char* a_english, const char* a_japanese,
				const char* a_id, std::uint32_t a_profile, const char* a_englishHelp, const char* a_japaneseHelp) {
				MenuFramework::SameLine();
				const auto selected = tdmFocusSettings.profile == a_profile;
				const auto visible = std::string(selected ? "*" : "") + Localized(a_english, a_japanese);
				const auto label = visible + "##" + a_id;
				const auto pressed = MenuFramework::Button(label.c_str());
				MenuFramework::ItemTooltip(Localized(a_englishHelp, a_japaneseHelp));
				if (pressed && !selected) {
					tdmFocusSettings.profile = a_profile;
					return true;
				}
				return false;
			};
			changed |= profileButton(
				"Standard", "標準", "TDMProfileStandard", 0U,
				"Uses the balanced lock-on depth profile.",
				"バランス型のロックオン深度です。");
			changed |= profileButton(
				"Smooth", "なだらか", "TDMProfileSmooth", 1U,
				"Uses a gentler lens and widens the clear range as target distance increases.",
				"穏やかなレンズを使い、対象が遠いほど鮮明な範囲を広げます。");
			changed |= profileButton(
				"Emphasis", "強調", "TDMProfileEmphasis", 2U,
				"Narrows the clear range and strongly separates the target from its background.",
				"鮮明な範囲を狭め、対象と背景を強く分離します。");
			changed |= SliderWithHelp(
				"Lock-On Blur Strength",
				"強度",
				&tdmFocusSettings.blurStrength,
				0.0F,
				3.0F,
				"%.2f x",
				"1.00 is the selected profile's baseline. Lower values widen the clear range and soften the blur; higher values narrow the clear range and strengthen the blur.",
				"1.00が選択中タイプの基準です。小さいほど鮮明な範囲が広くぼかしが穏やかになり、大きいほど範囲が狭くぼかしが強くなります。");
			changed |= SliderWithHelp(
				"Foreground Blur",
				"手前ぼかし",
				&tdmFocusSettings.nearBlurStrength,
				0.0F,
				1.0F,
				"%.2f x",
				"Adds a small dedicated foreground blur during target lock. The default 0.00 keeps the camera-to-target foreground sharp; increasing it can make the focused ground plane visible as a horizontal band.",
				"ロックオン中の手前側へ専用のぼかしを加えます。初期値0.00ではカメラから対象まで鮮明です。上げると地面にピント面が横帯として見える場合があります。");
		}
		if (changed) {
			ApplyTDMFocus();
			SetStatus("TDM lock-on focus changed (not saved).", "TDMロックオン時の自動ピント設定を変更しました（INI未保存）。");
		}
	}

	void RenderSneakFocusControls()
	{
		MenuFramework::SeparatorText(Localized("Sneak Mode", "スニークモード"));
		bool changed = CheckboxWithHelp(
			"Enable Sneak Mode",
			"スニークモードを有効にする",
			&sneakFocusSettings.enabled,
			"While sneaking, focuses at the center of the game image with a dedicated depth profile. Dialogue and TDM lock-on focus keep priority; normal focus settings return after sneaking.",
			"スニーク中、専用の深度設定でゲーム画面中央へピントを合わせます。会話・TDMロックオンの自動ピントを優先し、スニーク終了後は通常設定へ戻ります。");
		if (sneakFocusSettings.enabled) {
			MenuFramework::Text(Localized("Type", "タイプ"));
			const auto profileButton = [&](const char* a_english, const char* a_japanese,
				const char* a_id, std::uint32_t a_profile, const char* a_englishHelp, const char* a_japaneseHelp) {
				MenuFramework::SameLine();
				const auto selected = sneakFocusSettings.profile == a_profile;
				const auto visible = std::string(selected ? "*" : "") + Localized(a_english, a_japanese);
				const auto label = visible + "##" + a_id;
				const auto pressed = MenuFramework::Button(label.c_str());
				MenuFramework::ItemTooltip(Localized(a_englishHelp, a_japaneseHelp));
				if (pressed && !selected) {
					sneakFocusSettings.profile = a_profile;
					return true;
				}
				return false;
			};
			changed |= profileButton(
				"Natural", "自然", "SneakProfileNatural", 0U,
				"Keeps a wider clear range for unobtrusive exploration and aiming.",
				"鮮明な範囲を広めに保ち、探索や照準を邪魔しにくい自然な見え方です。");
			changed |= profileButton(
				"Suspense", "緊張感", "SneakProfileSuspense", 1U,
				"Uses a narrower clear range and stronger separation for a tense stealth look.",
				"鮮明な範囲を狭め、背景を強く分離して緊張感を出します。");
			changed |= SliderWithHelp(
				"Sneak Blur Strength",
				"強度",
				&sneakFocusSettings.blurStrength,
				0.0F,
				3.0F,
				"%.2f x",
				"1.00 is the selected type's baseline. Lower values widen the clear range; higher values narrow it and strengthen background blur. Natural gains extra blur above 2.00 without further narrowing its clear range. Foreground blur remains disabled in Sneak Mode.",
				"1.00が選択中タイプの基準です。小さいほど鮮明な範囲が広く、大きいほど範囲が狭く背景ぼかしが強くなります。「自然」は2.00を超えると鮮明範囲をさらに狭めず、ぼかし量だけが強くなります。スニークモード中の手前ぼかしはOFFです。");
		}
		if (changed) {
			ApplySneakFocus();
			SetStatus("Sneak mode changed (not saved).", "スニークモード設定を変更しました（INI未保存）。");
		}
	}

	void RenderHairBlendingControls()
	{
		bool changed = CheckboxWithHelp(
			"Blend Hair",
			"髪を馴染ませる",
			&experimentalTransparencySettings.enabled,
			"Blurs the scenery behind hair so its edges blend naturally into the background. Transparent materials such as veils may become slightly softer. OFF uses the completed stable rendering path.",
			"髪の後ろをぼかして背景になじませます。ベールなどの透過素材は少しぼやけます。OFFでは完成版の安定描画を使用します。");

		const auto modeButton = [&](std::uint32_t a_mode, const char* a_id,
			const char* a_englishHelp, const char* a_japaneseHelp) {
			MenuFramework::SameLine();
			const auto selected = experimentalTransparencySettings.mode == a_mode;
			const auto visible = std::string(selected ? "*" : "") + std::to_string(a_mode);
			const auto label = visible + "##" + a_id;
			const auto pressed = MenuFramework::Button(label.c_str());
			MenuFramework::ItemTooltip(Localized(
				experimentalTransparencySettings.enabled ? a_englishHelp : "Enable Blend Hair before selecting a mode.",
				experimentalTransparencySettings.enabled ? a_japaneseHelp : "先に「髪を馴染ませる」をONにしてください。"));
			if (pressed && experimentalTransparencySettings.enabled && !selected) {
				experimentalTransparencySettings.mode = a_mode;
				return true;
			}
			return false;
		};
		changed |= modeButton(
			1U,
			"TransparencyMode1",
			"Mode 1 blends the background behind hair strongly and softly. It gives the smoothest transition, while transparent materials such as veils may become softer.",
			"モード1は髪の後ろの背景を強く柔らかく馴染ませます。最も滑らかな移行になりますが、ベールなどの透過素材も柔らかくなります。");
		changed |= modeButton(
			2U,
			"TransparencyMode2",
			"Mode 2 blends hair while preserving more transparency, shape, and interior detail. During strong blur it also keeps sky and foliage transitions even and protects moon boundaries.",
			"モード2は透過・形・内側の細部をより保ちながら髪を馴染ませます。強いぼかし時は空と枝葉の移行を均一にし、月の境界も保護します。");
		if (changed) {
			ApplyExperimentalTransparency();
			SetStatus(
				experimentalTransparencySettings.enabled ?
					"Hair blending mode " + std::to_string(experimentalTransparencySettings.mode) + " enabled (not saved)." :
					"Hair blending disabled; stable rendering restored (not saved).",
				experimentalTransparencySettings.enabled ?
					"髪を馴染ませるモード" + std::to_string(experimentalTransparencySettings.mode) + "を有効にしました（INI未保存）。" :
					"「髪を馴染ませる」を無効にし、完成版の安定描画へ戻しました（INI未保存）。");
		}
	}

    struct FrameCandidate
    {
        RE::ObjectRefHandle handle;
        std::string name;
        float distance{};
        RE::FormID formID{};
    };
    std::vector<FrameCandidate> frameCandidates;
    std::uint64_t frameScanRequest{};
    bool frameScanPending{};
    std::size_t frameCandidatePage{};
    bool showFrameCandidates{};

    void RefreshFrameCandidates()
    {
        const auto tasks = SKSE::GetTaskInterface();
        if (!tasks) return;
        const auto request = ++frameScanRequest;
        frameScanPending = true;
        tasks->AddTask([request] {
            std::vector<FrameCandidate> found;
            auto* player = RE::PlayerCharacter::GetSingleton();
            auto* tes = RE::TES::GetSingleton();
            auto* ui = RE::UI::GetSingleton();
            if (player && tes && ui && !ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME) &&
                !ui->IsMenuOpen(RE::MainMenu::MENU_NAME)) {
                constexpr float unitsPerMetre = 100.0f / 1.428f;
                tes->ForEachReferenceInRange(player, 30.0f * unitsPerMetre,
                    [&](RE::TESObjectREFR* ref) {
                        auto* cell = ref ? ref->GetParentCell() : nullptr;
                        auto* playerCell = player->GetParentCell();
                        if (!ref || ref == player || ref->IsDeleted() || ref->IsDisabled() ||
                            !cell || !playerCell || !cell->IsAttached() || !ref->Get3D(false)) {
                            return RE::BSContainer::ForEachResult::kContinue;
                        }
                        if (cell != playerCell && (!cell->IsExteriorCell() || !playerCell->IsExteriorCell() ||
                            ref->GetWorldspace() != player->GetWorldspace())) {
                            return RE::BSContainer::ForEachResult::kContinue;
                        }
                        const char* name = ref->GetDisplayFullName();
                        if (ref->As<RE::Actor>() || (name && *name)) {
                            const auto handle = ref->GetHandle();
                            if (handle) {
                                found.push_back({handle, name && *name ? name : "(Actor)",
                                    (ref->GetPosition() - player->GetPosition()).Length() / unitsPerMetre,
                                    ref->GetFormID()});
                            }
                        }
                        return RE::BSContainer::ForEachResult::kContinue;
                    });
            }
            std::sort(found.begin(), found.end(), [](const auto& left, const auto& right) {
                if (left.distance != right.distance) return left.distance < right.distance;
                return left.formID < right.formID;
            });
            if (found.size() > 64) found.resize(64);
            std::scoped_lock lock(uiMutex);
            if (request != frameScanRequest) return;
            frameCandidates = std::move(found);
            frameCandidatePage = 0;
            frameScanPending = false;
        });
    }

    void RenderFrameCandidates()
    {
        if (MenuFramework::Button(Localized("Player##FrameSubject", "プレイヤー##FrameSubject"))) {
            SubjectMask::UsePlayer();
            SetStatus("Frame subject set to player.", "枠の対象をプレイヤーにしました。");
        }
        MenuFramework::SameLine();
        if (MenuFramework::Button(Localized("Nearby Subjects", "近くの対象"))) {
            showFrameCandidates = !showFrameCandidates;
        }
        MenuFramework::SameLine();
        if (MenuFramework::Button(Localized("Console Subject##FrameSubject", "コンソール対象##FrameSubject"))) {
            if (SubjectMask::SetConsoleTarget()) {
                showFrameCandidates = false;
                SetStatus("Console selection registered as the frame subject.", "コンソール選択対象を枠の対象にしました。");
            } else {
                SetStatus("Select a loaded subject in the console first.", "先にコンソールで、画面にある対象を選択してください。");
            }
        }
        if (!showFrameCandidates) return;
        if (frameScanPending) MenuFramework::Text(Localized("Scanning...", "候補を検索中..."));
        if (frameCandidates.empty()) {
            MenuFramework::Text(Localized("No nearby candidates. Refresh near the subject.",
                "候補がありません。対象の近くで更新してください。"));
            return;
        }
        constexpr std::size_t pageSize = 4;
        const auto pageCount = (frameCandidates.size() + pageSize - 1) / pageSize;
        frameCandidatePage = std::min(frameCandidatePage, pageCount - 1);
        const auto end = std::min((frameCandidatePage + 1) * pageSize, frameCandidates.size());
        for (auto index = frameCandidatePage * pageSize; index < end; ++index) {
            const auto& candidate = frameCandidates[index];
            const auto label = std::format("{} / {:.1f} m [{:08X}]##FrameCandidate{}",
                candidate.name, candidate.distance, candidate.formID, index);
            if (MenuFramework::Button(label.c_str())) {
                const auto subject = candidate.handle.get();
                if (SubjectMask::SetTarget(subject.get())) {
                    showFrameCandidates = false;
                    SetStatus("Frame subject selected.", "枠の対象を選択しました。");
                } else {
                    SetStatus("Subject unavailable. Refresh the candidates.", "対象を取得できません。候補を更新してください。");
                }
            }
        }
        if (pageCount > 1) {
            if (MenuFramework::Button(Localized("Previous##FrameCandidates", "前へ##FrameCandidates")) && frameCandidatePage > 0) {
                --frameCandidatePage;
            }
            MenuFramework::SameLine();
            if (MenuFramework::Button(Localized("Next##FrameCandidates", "次へ##FrameCandidates")) && frameCandidatePage + 1 < pageCount) {
                ++frameCandidatePage;
            }
            const auto page = std::format("{} / {}", frameCandidatePage + 1, pageCount);
            MenuFramework::Text(page.c_str());
        }
    }

	void RenderPresetToolbar()
	{
		const auto activePreset = FindMatchingPreset();
		MenuFramework::Text(activePreset ? "Preset" : Localized("Preset: Modified", "Preset：変更中"));
		MenuFramework::ItemTooltip(Localized(
			"Applies every setting stored in the selected complete preset to the current image. It does not change the preset slot or startup settings.",
			"選択した完全プリセットの全設定を現在映像へ適用します。プリセット枠と次回起動設定は変更しません。"));
		for (std::size_t index = 0; index < presetSlots.size(); ++index) {
			MenuFramework::SameLine();
			const auto visible = activePreset && *activePreset == index ?
				std::string(interfaceSettings.japanese ? presetSlots[index].japaneseName : presetSlots[index].name) +
					" [" + std::to_string(index + 1) + "]" :
				std::to_string(index + 1);
			const auto label = visible + "##TopPreset" + std::to_string(index);
			if (MenuFramework::Button(label.c_str())) {
				ApplyPreset(index);
			}
			const auto tooltip = interfaceSettings.japanese ? presetSlots[index].japaneseName : presetSlots[index].name;
			MenuFramework::ItemTooltip(tooltip);
		}
	}

	bool RenderFocusModeControls()
	{
		bool changed{};
		const auto firstPerson = IsFirstPersonView();
		MenuFramework::SeparatorText(Localized("Focus Mode", "ピント方式"));
		const auto manualSelected = !targetFocusSettings.consoleEnabled && !uiSettings.autoFocus;
		const auto screenSelected = !targetFocusSettings.consoleEnabled && uiSettings.autoFocus;
		const auto targetSelected = targetFocusSettings.consoleEnabled;
		if (ChoiceButton(
				"Fixed Focus", "ピント固定", "ManualFocusMode", manualSelected,
				"Use the fixed focus-plane distance below.", "下のピント面までの距離を使用します。") &&
			!manualSelected) {
			targetFocusSettings.consoleEnabled = false;
			uiSettings.autoFocus = false;
			changed = true;
		}
		MenuFramework::SameLine();
		if (ChoiceButton(
				firstPerson ? "1st-Person AF" : "3rd-Person AF",
				firstPerson ? "一人称画面AF" : "三人称画面AF",
				"ScreenFocusMode",
				screenSelected,
				firstPerson ?
					"Read depth from the first-person X/Y screen position." :
					"Read depth from the third-person X/Y screen position.",
				firstPerson ?
					"一人称用の画面X・Y位置から距離を読み取ります。" :
					"三人称用の画面X・Y位置から距離を読み取ります。") &&
			!screenSelected) {
			targetFocusSettings.consoleEnabled = false;
			uiSettings.autoFocus = true;
			changed = true;
		}
		MenuFramework::SameLine();
		if (ChoiceButton(
				"Target Tracking", "対象追従", "TargetFocusMode", targetSelected,
				"Focus follows the player or a reference selected in the console.",
				"プレイヤー、またはコンソールで選択した対象を追従します。") &&
			!targetSelected) {
			targetFocusSettings.consoleEnabled = true;
			changed = true;
		}
		if (!targetFocusSettings.consoleEnabled) {
			return changed;
		}

		MenuFramework::Text(Localized("Target", "追従対象"));
		MenuFramework::SameLine();
		const auto playerSelected = targetFocusSettings.targetSource == TargetFocusSource::kPlayer;
		if (ChoiceButton(
				"Player (Automatic)", "プレイヤー（自動）", "PlayerTargetSource", playerSelected,
				"Tracks the player directly without console commands. Active in third person only.",
				"コンソール操作なしでプレイヤーを直接追従します。三人称視点でのみ有効です。") &&
			!playerSelected) {
			targetFocusSettings.targetSource = TargetFocusSource::kPlayer;
			changed = true;
		}
		MenuFramework::SameLine();
		if (ChoiceButton(
				"Console Selection", "コンソール選択", "ConsoleTargetSource", !playerSelected,
				"Tracks the reference last selected in the console.",
				"コンソールで最後に選択した対象を追従します。") &&
			playerSelected) {
			targetFocusSettings.targetSource = TargetFocusSource::kConsole;
			changed = true;
		}

		changed |= SliderWithHelp(
			"Target Focus Offset",
			"ピント位置補正",
			&targetFocusSettings.targetFocusOffsetMeters,
			-10.0F,
			10.0F,
			"%+.2f m",
			"Offsets the measured target distance. Negative moves focus toward the camera; positive moves it farther away.",
			"対象までの実測距離を補正します。マイナスでカメラ側、プラスで対象より奥へピント面を移動します。");
		if (playerSelected) {
			const auto camera = RE::PlayerCamera::GetSingleton();
			if (camera && camera->IsInFirstPerson()) {
				MenuFramework::Text(Localized(
					targetFocusSettings.useFirstPersonScreenAFFallback ?
						"Target tracking paused; 1st-Person AF is active." :
						"Target tracking is temporarily paused in first person.",
					targetFocusSettings.useFirstPersonScreenAFFallback ?
						"対象追従：一時停止中（一人称画面AFを使用）" :
						"対象追従：一人称のため一時停止中"));
			}
		}
		return changed;
	}

	void RenderPresetActions()
	{
		for (std::size_t index = 0; index < presetSlots.size(); ++index) {
			auto& preset = presetSlots[index];
			const auto isEditingSource = editingPresetIndex && *editingPresetIndex == index;
			const auto isUnmodified = isEditingSource && CurrentValuesMatchPreset(index);
			const auto prefix = interfaceSettings.japanese ?
				(isUnmodified ? "選択中：" : isEditingSource ? "編集元：" : "適用：") :
				(isUnmodified ? "Selected: " : isEditingSource ? "Editing From: " : "Apply ");
			const auto applyLabel = std::string(prefix) +
				(interfaceSettings.japanese ? preset.japaneseName : preset.name) +
				" [" + std::to_string(index + 1) + "]##ApplyPreset" + std::to_string(index);
			if (MenuFramework::Button(applyLabel.c_str())) {
				ApplyPreset(index);
			}
			MenuFramework::ItemTooltip(Localized(
				"Applies the stored DoF, dialogue-focus, and target-tracking values to the current image. It does not change the preset slot or startup settings.",
				"保存済みのDoF・会話フォーカス・対象追従設定を現在映像へ適用します。プリセット枠と次回起動設定は変更しません。"));
			MenuFramework::SameLine();
			const auto storeLabel = std::string(Localized("Store Current", "現在値を保存")) + "##" + preset.name;
			if (MenuFramework::Button(storeLabel.c_str())) {
				StorePreset(index);
			}
			MenuFramework::ItemTooltip(Localized(
				"Stores the DoF, dialogue-focus, and target-tracking values currently applied on screen in this preset slot. It does not change startup settings or the interface language.",
				"現在映像へ適用中のDoF・会話フォーカス・対象追従設定をこのプリセット枠へ保存します。次回起動設定とUI言語は変更しません。"));
			MenuFramework::SameLine();
			const auto resetPending = pendingPresetResetIndex && *pendingPresetResetIndex == index;
			const auto resetLabel = std::string(Localized(
				resetPending ? "Confirm Reset" : "Restore Defaults",
				resetPending ? "確認：初期値へ戻す" : "初期値へ戻す")) +
				"##ResetPreset" + std::to_string(index);
			if (MenuFramework::Button(resetLabel.c_str())) {
				if (resetPending) {
					RestorePresetDefaults(index);
				} else {
					pendingPresetResetIndex = index;
					SetStatus(
						std::string("Press Confirm Reset to restore ") + preset.name +
							" and apply it immediately.",
						std::string(preset.japaneseName) +
							"を初期値へ戻して映像へ適用するには、もう一度同じボタンを押してください。");
				}
			}
			MenuFramework::ItemTooltip(Localized(
				"Press twice to load this preset's bundled defaults and apply them immediately without saving. Use Store Current to save them in this preset slot.",
				"2回押すと、このプリセットの初期値を保存せず映像へ適用します。このプリセット枠へ保存する場合は「現在値を保存」を押します。"));
		}
	}

	void RenderActions()
	{
		RenderPresetActions();
		MenuFramework::SeparatorText(Localized("Startup Settings", "次回起動設定の保存・読み込み"));

		if (MenuFramework::Button(Localized("Save Startup Settings", "次回起動設定を保存"))) {
			if (SaveSettings(uiSettings) && SaveTargetFocusSettings(targetFocusSettings) &&
				SaveTDMFocusSettings(tdmFocusSettings) &&
				SaveSneakFocusSettings(sneakFocusSettings) &&
				SaveModeSettings(modeSettings) &&
				SaveSilhouetteSettings(silhouetteSettings) &&
				SaveDepthFrameSettings(depthFrameSettings) &&
				SaveExperimentalTransparencySettings(experimentalTransparencySettings) &&
				SaveInterfaceSettings(interfaceSettings)) {
				SetStatus("Saved startup settings to CinematicDoFStandalone.ini.", "次回起動設定をCinematicDoFStandalone.iniへ保存しました。");
			} else {
				SetStatus("Could not save INI. See the plugin log.", "INIを保存できませんでした。ログを確認してください。");
			}
		}
		MenuFramework::ItemTooltip(Localized(
			"Saves the master switch, normal-gameplay mode, DoF, silhouette, dialogue-focus, TDM lock-on focus, sneak mode, hair blending, target-tracking, and interface-language values currently applied on screen for the next launch. It does not overwrite any preset slot.",
			"現在映像へ適用中の主スイッチ・会話外DoF・DoF・シルエット・会話フォーカス・TDMロックオンフォーカス・スニークモード・髪の馴染ませ・対象追従設定とUI言語を、次回起動時の設定として保存します。プリセット枠は上書きしません。"));
		MenuFramework::SameLine();
		if (MenuFramework::Button(Localized("Reload INI", "INIを再読み込み"))) {
			uiSettings = LoadSettings();
			targetFocusSettings = LoadTargetFocusSettings();
			tdmFocusSettings = LoadTDMFocusSettings();
			sneakFocusSettings = LoadSneakFocusSettings();
			modeSettings = LoadModeSettings();
			silhouetteSettings = LoadSilhouetteSettings();
				depthFrameSettings = LoadDepthFrameSettings();
			experimentalTransparencySettings = LoadExperimentalTransparencySettings();
			interfaceSettings = LoadInterfaceSettings();
			hotkeySettings = LoadHotkeySettings();
			hotkeyCaptureTarget = HotkeyCaptureTarget::kNone;
			LoadPresets();
			editingPresetIndex = FindMatchingPreset();
			DoFRenderer::GetSingleton().SetSettings(uiSettings);
			DoFRenderer::GetSingleton().SetModeSettings(modeSettings);
			DoFRenderer::GetSingleton().SetSilhouetteSettings(silhouetteSettings);
				DoFRenderer::GetSingleton().SetDepthFrameSettings(depthFrameSettings);
			ApplyExperimentalTransparency();
			ApplyTargetFocus();
			ApplyTDMFocus();
			ApplySneakFocus();
			SetStatus(
				"Reloaded current settings and preset slots from INI.",
				"現在設定・自動ピント・プリセットをINIから再読み込みしました。");
		}
		MenuFramework::Text(interfaceSettings.japanese ? statusTextJapanese.c_str() : statusTextEnglish.c_str());
	}

	bool RenderCoreControls()
	{
		bool changed{};
		RenderPresetToolbar();

		changed |= RenderFocusModeControls();
		const auto firstPersonScreenAFFallback = IsFirstPersonScreenAFFallbackActive();
		if ((!targetFocusSettings.consoleEnabled && uiSettings.autoFocus) || firstPersonScreenAFFallback) {
			const auto firstPerson = IsFirstPersonView();
			auto& focusX = ScreenFocusX(uiSettings, firstPerson);
			auto& focusY = ScreenFocusY(uiSettings, firstPerson);
			changed |= SliderWithHelp(
				"Focus Point X",
				"自動ピント位置 X",
				&focusX,
				0.0F,
				1.0F,
				"%.2f",
				firstPerson ?
					"Horizontal position sampled by 1st-Person AF. This is stored separately from 3rd-Person AF; center is 0.50." :
					"Horizontal position sampled by 3rd-Person AF. This is stored separately from 1st-Person AF; center is 0.50.",
				firstPerson ?
					"一人称画面AFが距離を読み取る横位置です。三人称とは別に保存され、中央は0.50です。" :
					"三人称画面AFが距離を読み取る横位置です。一人称とは別に保存され、中央は0.50です。");
			changed |= SliderWithHelp(
				"Focus Point Y",
				"自動ピント位置 Y",
				&focusY,
				0.0F,
				1.0F,
				"%.2f",
				firstPerson ?
					"Vertical position sampled by 1st-Person AF. Move left for the top and right for the bottom; center is 0.50." :
					"Vertical position sampled by 3rd-Person AF. Move left for the top and right for the bottom; center is 0.50.",
				firstPerson ?
					"一人称画面AFが距離を読み取る縦位置です。左で画面上、右で画面下へ移動し、中央は0.50です。" :
					"三人称画面AFが距離を読み取る縦位置です。左で画面上、右で画面下へ移動し、中央は0.50です。");
			changed |= SliderWithHelp(
				"Screen AF Focus Offset",
				"画面AFのピント位置補正",
				&uiSettings.autoFocusOffsetMeters,
				-10.0F,
				10.0F,
				"%+.2f m",
				"Offsets the sampled screen depth. Negative moves focus toward the camera; positive moves it farther away.",
				"画面から取得した距離を補正します。マイナスでカメラ側、プラスで取得地点より奥へピント面を移動します。");
		} else if (!targetFocusSettings.consoleEnabled) {
			changed |= LogDistanceSliderWithHelp(
				"Focus Distance",
				"ピント距離",
				&uiSettings.manualFocusMeters,
				kManualFocusMinMeters,
				kManualFocusMaxMeters,
				"Fixed distance from the active camera to the focus plane. The logarithmic slider keeps short-distance control precise while extending to 2000 m for distant scenery.",
				"現在のカメラからピント面までの固定距離です。対数スライダーで近距離の細かな操作を保ちながら、遠景用に2000mまで指定できます。");
		}
		changed |= SliderWithHelp(
			"Transition Speed",
			"ピント移動速度",
			&uiSettings.transitionSpeed,
			0.01F,
			1.0F,
			"%.2f",
			"Controls how quickly focus catches up when its distance changes. Move left for slower, smoother transitions; move right for faster response. It also applies to target tracking.",
			"ピント距離が変化したときの追従速度です。左ほどゆっくり滑らかに、右ほど素早く追従します。対象追従中にも適用されます。");

		MenuFramework::SeparatorText(Localized("Lens", "レンズとぼかし"));
		MenuFramework::ItemTooltip(Localized(
			"Use focal length to set the overall depth separation, then use the F-number to refine the in-focus range. Finally, set the blur strength with the near and far maximum blur controls.",
			"焦点距離で前後の分離を大まかに決め、F値でピントの合う範囲を整えます。最後に手前・奥最大ぼかしでボケの強さを調整すると設定しやすくなります。"));
		changed |= SliderWithHelp(
			"Virtual Focal Length", "レンズ焦点距離", &uiSettings.focalLength, 1.0F, 300.0F, "%.1f mm",
			"Virtual lens focal length. Shorter values widen the in-focus range; longer values make depth separation stronger and the in-focus range narrower.",
			"仮想レンズの焦点距離です。短くするとピントの合う範囲が広がり、長くすると前後の分離が強くなって範囲が狭まります。");
		changed |= SliderWithHelp(
			"F-Number", "F値", &uiSettings.fNumber, 1.0F, 22.0F, "f/%.1f",
			"Virtual aperture. Lower values narrow the in-focus range; higher values widen it.",
			"仮想的な絞り値です。小さくするとピントの合う範囲が狭まり、大きくすると広がります。");
		changed |= SquaredRangeSliderWithHelp(
			"Far Focus Range",
			"奥ピント範囲",
			&uiSettings.farFocusRangeMeters,
			100.0F,
			"Adds up to 100 m of in-focus range behind the focus plane without reducing the configured maximum background blur outside that range. The squared slider response gives finer control at short distances; saved values remain actual meters.",
			"ピント面から奥側へ最大100mまで追加するピント範囲です。二乗カーブにより短距離を細かく調整でき、保存値は実際のメートル値のままです。範囲外の背景最大ぼかし量は弱めません。");
		changed |= SliderWithHelp(
			"Far Plane Max Blur",
			"奥最大ぼかし",
			&uiSettings.farPlaneMaxBlur,
			0.0F,
			8.0F,
			"%.2f",
			"Limits the maximum blur behind the focus plane.",
			"ピント面より奥側に掛かる最大ぼかし量です。遠い背景の最大ぼかしもこの値に従います。");
		changed |= SquaredRangeSliderWithHelp(
			"Near Focus Range",
			"手前ピント範囲",
			&uiSettings.nearFocusRangeMeters,
			10.0F,
			"Adds an in-focus range from the focus plane toward the camera without reducing the configured maximum blur outside that range. The squared slider response gives finer control near 0-2 m; saved values remain actual meters.",
			"ピント面からカメラ側へ追加するピント範囲です。二乗カーブにより0～2m付近を細かく調整できます。保存値は実際のメートル値のままです。");
		changed |= SliderWithHelp(
			"Near Plane Max Blur",
			"手前最大ぼかし",
			&uiSettings.nearPlaneMaxBlur,
			0.0F,
			4.0F,
			"%.2f",
			"Limits the maximum blur in front of the focus plane. In first person it follows the 1st-Person Near Blur option in Advanced Settings.",
			"ピント面より手前側に掛かる最大ぼかし量です。一人称視点では詳細設定内の「一人称の手前ぼかし」に従います。");

		if (MenuFramework::Button(Localized(
				showAdvanced ? "Hide Advanced Settings" : "Show Advanced Settings",
				showAdvanced ? "詳細設定を隠す" : "詳細設定を表示"))) {
			showAdvanced = !showAdvanced;
		}
		if (showAdvanced) {
			MenuFramework::SeparatorText(Localized("Bokeh and Quality", "画質・ボケの詳細"));
			changed |= SliderWithHelp(
					"Blur Quality", "ぼかし品質", &uiSettings.blurQuality, 2.0F, 30.0F, "%.1f",
					"Higher values improve blur sampling but increase GPU cost.",
					"高いほどボケのサンプリング品質が上がりますが、GPU負荷も増えます。");
				changed |= SliderWithHelp(
					"Background Boundary Compensation",
					"背景境界の補正",
					&uiSettings.nearFarDistanceCompensation,
					1.0F,
					5.0F,
					"%.2f",
					"Controls background-blur sampling near depth boundaries. Higher values reject more samples across large depth gaps. It does not affect near blur.",
					"背景ぼかしが深度境界をまたいでにじむ範囲を調整します。高いほど深度差の大きい背景サンプルを抑えます。手前ぼかしには作用しません。");
				changed |= SliderWithHelp(
					"Bokeh Busy Factor", "ボケの密度", &uiSettings.bokehBusyFactor, 0.0F, 1.0F, "%.2f",
					"Adjusts how busy or dense out-of-focus highlights appear.",
					"ピンぼけ部分の光がどれくらい密集して見えるかを調整します。");
				changed |= SliderWithHelp(
					"Petzval Strength", "周辺ボケの強さ", &uiSettings.petzvalStrength, 0.0F, 2.0F, "%.2f",
					"Stretches existing edge bokeh tangentially to reproduce swirling lens bokeh.",
					"画面周辺の既存ボケを接線方向へ引き延ばし、渦巻くレンズボケを再現します。");
				changed |= SliderWithHelp(
					"Highlight Boost", "明るいボケの強調", &uiSettings.highlightBoost, 0.0F, 1.0F, "%.2f",
					"Boosts bright highlights inside blurred regions. With aperture-shaped bokeh enabled, it blends the blur toward the brightest eligible shaped sample.",
					"ぼけた領域にある明るい光を強調します。絞り形状ボケが有効な場合は、ぼかし結果を形状付きの最も明るい有効サンプルへ近づけます。");
				changed |= SliderWithHelp(
					"Post Blur Smoothing",
					"仕上げの滑らかさ",
					&uiSettings.postBlurSmoothing,
					0.0F,
					2.0F,
					"%.2f",
					"Applies final smoothing to already blurred regions.",
					"ぼかし処理後の領域へ仕上げの平滑化を加えます。");
				changed |= CheckboxWithHelp(
					"Aperture Bokeh", "絞り形状ボケ", &uiSettings.apertureBokeh,
					"Shapes the blur kernel with a procedurally generated lens aperture. The shape is easiest to see on bright points that are strongly out of focus. No external image is used.",
					"数値から生成したレンズ絞りの形をボケへ反映します。大きくピンぼけした明るい点ほど形が見えやすくなります。外部画像は使用しません。");
				if (uiSettings.apertureBokeh) {
					changed |= CheckboxWithHelp(
						"Fill Aperture Interior", "内部を滑らかに埋める", &uiSettings.apertureFill,
						"Uses a denser, area-filled aperture gather so the centre of shaped bokeh remains smooth. Intended for screenshots; GPU cost increases substantially.",
						"形状ボケの内部全体を高密度で集め、中央まで滑らかに埋めます。撮影向けの機能で、GPU負荷が大きく増加します。");
					MenuFramework::Text(Localized("Bokeh Shape", "ボケ形状"));
					MenuFramework::SameLine();
					constexpr std::array<std::pair<const char*, const char*>, 3> shapeLabels{
						std::pair{ "Aperture", "絞り" },
						std::pair{ "Star", "星" },
						std::pair{ "Heart", "ハート" }
					};
					for (std::uint32_t shape = 0; shape < shapeLabels.size(); ++shape) {
						if (shape != 0)
							MenuFramework::SameLine();
						const auto& [english, japanese] = shapeLabels[shape];
						const auto label = std::string(uiSettings.apertureShape == shape ? "*" : "") +
							Localized(english, japanese);
						if (MenuFramework::Button(label.c_str())) {
							uiSettings.apertureShape = shape;
							changed = true;
						}
					}
					MenuFramework::ItemTooltip(Localized(
						"Selects the procedurally generated bokeh silhouette. Star uses the blade count as its number of points.",
						"数式から生成するボケの輪郭を選びます。星では絞り羽根枚数が頂点数になります。"));
					if (uiSettings.apertureShape != 2U)
						changed |= ApertureBladeSlider(uiSettings);
					changed |= SliderWithHelp(
						"Shape Roundness", "形状の丸み", &uiSettings.apertureRoundness, 0.0F, 1.0F, "%.2f",
						"Blends the selected silhouette toward a circle.",
						"選択した輪郭を円形へ近づけます。");
					changed |= SliderWithHelp(
						"Shape Strength", "形状の強さ", &uiSettings.apertureShapeStrength, 0.0F, 1.0F, "%.2f",
						"Controls how strongly the aperture shape affects the blur. Zero behaves like the standard circular blur; one uses the full generated shape.",
						"絞り形状をボケへ反映する強さです。0では通常の円形ボケ、1では生成した形状を最大限に反映します。");
					changed |= SliderWithHelp(
						"Shape Rotation", "形状の回転", &uiSettings.apertureRotationDegrees, 0.0F, 360.0F, "%.0f deg",
						"Rotates the aperture shape around the optical axis.",
						"光軸を中心に絞り形状を回転します。");
					changed |= SliderWithHelp(
						"Highlight Extraction Threshold", "明部抽出しきい値", &uiSettings.highlightExtractionThreshold, 0.20F, 1.50F, "%.2f",
						"Shared brightness threshold for diffraction rays and bokeh core fill. Lower values include softer bright regions.",
						"回折光条とボケ中心の補光で共用する明るさのしきい値です。下げるほど曖昧な明部も対象になります。");
					changed |= SliderWithHelp(
						"Bokeh Core Fill", "ボケ中心の補光", &uiSettings.bokehCoreFill, 0.0F, 2.0F, "%.2f",
						"Isotropically returns nearby highlight energy only where the shaped bokeh center is dimmer. It works independently of diffraction rays.",
						"周囲より薄い形状ボケの中心だけへ、近傍の明るさを方向性なしで戻します。回折光条とは独立して使用できます。");
				}
				changed |= CheckboxWithHelp(
					"Diffraction Starburst", "回折光条", &uiSettings.starburstEnabled,
					"Adds aperture-diffraction rays to strongly defocused bright points. The effect becomes longer and stronger as the F-number increases. This is an expensive screenshot-oriented effect.",
					"大きくピンぼけした明るい点へ絞り回折の光条を加えます。F値が大きいほど長く強くなります。GPU負荷の大きい撮影向け機能です。");
				if (uiSettings.starburstEnabled) {
					changed |= StarburstRaySelector(uiSettings);
					changed |= SliderWithHelp(
						"Starburst Highlight Sensitivity", "光条の明部感度", &uiSettings.starburstHighlightSensitivity, 1.0F, 4.0F, "%.2f x",
						"Higher values let dimmer highlights produce diffraction rays. 1.0 uses the original threshold.",
						"上げると弱い明部からも光条が出やすくなります。1.0は従来の抽出条件です。");
					changed |= SliderWithHelp(
						"Starburst Strength", "光条の強さ", &uiSettings.starburstStrength, 0.0F, 16.0F, "%.2f",
						"Controls the brightness of the diffraction rays after F-number scaling. The extended upper range is intended for strongly emphasized screenshot effects.",
						"F値による補正後の回折光条の明るさです。拡張した高い範囲は、撮影時に光条を強く強調する用途向けです。");
					changed |= SliderWithHelp(
						"Starburst Length", "光条の長さ", &uiSettings.starburstLength, 0.05F, 1.5F, "%.2f",
						"Controls the maximum ray length. The final length also follows the F-number.",
						"光条の最大長を調整します。最終的な長さはF値にも連動します。");
					changed |= SliderWithHelp(
						"Starburst Rotation", "光条の回転", &uiSettings.starburstRotationDegrees, 0.0F, 180.0F, "%.0f deg",
						"Rotates all diffraction-ray axes together.",
						"すべての回折光条の軸をまとめて回転します。");
					changed |= SliderWithHelp(
						"Wide-Flare Suppression", "広域フレア抑制", &uiSettings.starburstWideFlareSuppression, 0.0F, 1.0F, "%.2f",
						"Reduces diffraction rays generated from broad, nearly uniform lens flare while preserving compact bright sources. Zero disables the filter.",
						"周囲まで一様に明るい広域レンズフレアから発生する光条を抑え、局所的な明るい光源を残します。0で無効です。");
					changed |= SliderWithHelp(
						"Strong-Light Suppression", "強光源抑制", &uiSettings.starburstStrongLightSuppression, 0.0F, 1.0F, "%.2f",
						"Softly compresses diffraction rays from extremely bright sources while preserving moderate reflections. Zero disables the compression.",
						"極端に明るい光源から発生する光条だけを緩やかに圧縮し、中程度の反射光を残します。0で無効です。");
					if (!uiSettings.apertureBokeh) {
						changed |= SliderWithHelp(
							"Highlight Extraction Threshold", "明部抽出しきい値", &uiSettings.highlightExtractionThreshold, 0.20F, 1.50F, "%.2f",
							"Brightness threshold for diffraction rays. This control appears in the aperture-shape block when Aperture Bokeh is enabled.",
							"回折光条を発生させる明るさのしきい値です。絞り形状ボケが有効な場合は、絞り形状の項目内に表示されます。");
					}
				}
				changed |= CheckboxWithHelp(
					"Disable in Main/Loading/Map Menus",
					"タイトル・ロード・マップ画面ではDoFを停止",
					&uiSettings.disableInMenus,
					"Skip DoF in the main menu, loading screens, and world/local maps.",
					"タイトル・ロード・ワールドマップ・ローカルマップではDoFを停止します。");
			MenuFramework::SeparatorText(Localized("Focus Details", "ピントの詳細"));
			changed |= CheckboxWithHelp(
				"Use 1st-Person AF",
				"一人称時は一人称画面AFを使用",
				&targetFocusSettings.useFirstPersonScreenAFFallback,
				"Uses 1st-Person AF while automatic player tracking is paused in first person. Player tracking resumes when returning to third person. Console-selected target tracking is not affected.",
				"プレイヤー自動追従が一人称で停止している間、一人称画面AFを使用します。三人称へ戻ると対象追従を再開します。コンソール選択による追従には影響しません。");
			changed |= CheckboxWithHelp(
				"1st-Person Near Blur",
				"一人称の手前ぼかし",
				&uiSettings.enableFirstPersonNearBlur,
				"When enabled, the configured near blur is also used in first person. Leave this off for clear normal gameplay.",
				"ONでは設定した手前ぼかしを一人称視点にも適用します。通常プレイで手前をくっきり表示する場合はOFFにします。");
		}
		return changed;
	}

	bool RenderModeControls()
	{
		bool changed{};
		changed |= CheckboxWithHelp(
			"Enable DoF",
			"DoFを有効にする",
			&uiSettings.enabled,
			"Master switch for all DoF rendering, including dialogue. The hotkey controls this same switch.",
			"会話中を含むすべてのDoF描画の主スイッチです。ホットキーも同じスイッチを切り替えます。");
		RenderDoFHotkeyControl();
		MenuFramework::SameLine();
		if (MenuFramework::SilhouetteIconButton("##OpenSilhouettePhotoMode", silhouetteSettings.enabled)) {
			if (silhouetteWindow) {
				silhouetteWindow->isOpen = !silhouetteWindow->isOpen;
			}
		}
		const auto silhouetteWindowOpen = silhouetteWindow && silhouetteWindow->isOpen;
		MenuFramework::ItemTooltip(Localized(
			silhouetteWindowOpen ?
				"Close Silhouette Photo Mode settings." :
				"Open Silhouette Photo Mode settings.",
			silhouetteWindowOpen ?
				"シルエット撮影モードの小窓を閉じます。" :
				"シルエット撮影モードの小窓を開きます。"));
		MenuFramework::SameLine();
		if (MenuFramework::FocusPickerIconButton("##SelectScreenAFFocusPoint")) {
			BeginFocusPicker();
		}
		MenuFramework::ItemTooltip(Localized(
			uiSettings.enabled ?
				"Select a focus point on the game image. This temporarily switches to Screen AF; left-click confirms the new mode and position, while right-click/Esc restores the previous focus mode and position." :
				"Select a Screen AF point even while DoF is off. Live blur preview is unavailable until DoF is enabled. Left-click confirms; right-click/Esc restores the previous focus mode and position.",
			uiSettings.enabled ?
				"ゲーム画面上でピント位置を選びます。一時的に画面位置AFへ切り替わり、左クリックで方式と位置を確定、右クリック／Escで元の方式と位置へ戻します。" :
				"DoFがOFFでも画面位置AFの位置を選択できますが、ぼかしのプレビューは表示されません。左クリックで確定、右クリック／Escで元の方式と位置へ戻します。"));
		MenuFramework::SameLine();
		const auto tdmAvailable = TDMIntegration::IsAvailable();
		if (MenuFramework::TargetLockIconButton(
				"##ToggleTDMLockFocus", tdmFocusSettings.enabled, tdmAvailable)) {
			tdmFocusSettings.enabled = !tdmFocusSettings.enabled;
			ApplyTDMFocus();
			SetStatus(
				tdmFocusSettings.enabled ?
					"TDM lock-on focus enabled (not saved)." :
					"TDM lock-on focus disabled (not saved).",
				tdmFocusSettings.enabled ?
					"TDMロックオン時の自動ピントを有効にしました（INI未保存）。" :
					"TDMロックオン時の自動ピントを無効にしました（INI未保存）。");
		}
		MenuFramework::ItemTooltip(Localized(
			tdmAvailable ?
				(tdmFocusSettings.enabled ?
					"TDM lock-on focus is enabled. Click to disable it." :
					"TDM lock-on focus is disabled. Click to enable it.") :
				"True Directional Movement is not detected, so lock-on focus cannot be toggled.",
			tdmAvailable ?
				(tdmFocusSettings.enabled ?
					"TDMロックオン対象への自動ピント：有効。クリックして無効にします。" :
					"TDMロックオン対象への自動ピント：無効。クリックして有効にします。") :
				"True Directional Movementが検出されていないため、ロックオン自動ピントは切り替えられません。"));
		MenuFramework::SameLine();
		if (MenuFramework::SneakIconButton("##ToggleSneakFocus", sneakFocusSettings.enabled)) {
			sneakFocusSettings.enabled = !sneakFocusSettings.enabled;
			ApplySneakFocus();
			SetStatus(
				sneakFocusSettings.enabled ?
					"Sneak mode enabled (not saved)." :
					"Sneak mode disabled (not saved).",
				sneakFocusSettings.enabled ?
					"スニークモードを有効にしました（INI未保存）。" :
					"スニークモードを無効にしました（INI未保存）。");
		}
		MenuFramework::ItemTooltip(Localized(
			sneakFocusSettings.enabled ?
				"Sneak Mode is enabled. Click to disable it." :
				"Sneak Mode is disabled. Click to enable it.",
			sneakFocusSettings.enabled ?
				"スニークモード：有効。クリックして無効にします。" :
				"スニークモード：無効。クリックして有効にします。"));
        if (depthFrameWindow) {
            MenuFramework::SameLine();
            if (MenuFramework::DepthFrameIconButton("##OpenDepthFrame", depthFrameWindow->isOpen)) {
                depthFrameWindow->isOpen = !depthFrameWindow->isOpen;
                if (depthFrameWindow->isOpen) RefreshFrameCandidates();
            }
            MenuFramework::ItemTooltip(Localized("Open or close Depth Frame settings.", "深度フレーム設定を開閉します。"));
        }
		changed |= CheckboxWithHelp(
			"Use Normal Gameplay DoF",
			"通常時もDoFを使用",
			&modeSettings.normalGameplayEnabled,
			"Controls ordinary gameplay DoF. Enabled Dialogue, TDM Lock-On, and Sneak modes can still activate while this is off. Keep the DoF master switch enabled.",
			"通常時のDoFを切り替えます。オフでも、有効にした会話・TDMロックオン・スニークの各モードは発動します。DoFの主スイッチはオンにしてください。");
		RenderLanguageControls();
		changed |= CheckboxWithHelp(
			"Keep Sky Sharp",
			"空を鮮明に保つ",
			&uiSettings.keepSkySharp,
			"Excludes clear-depth sky pixels and far-depth moon pixels from DoF while preserving a soft boundary around geometry. Moon protection follows this same switch.",
			"深度が未描画の空と遠端深度の月をDoFから除外し、地形との境界は滑らかに保ちます。月の保護もこのスイッチに連動します。");
		MenuFramework::SameLine();
		RenderHairBlendingControls();
		return changed;
	}

	bool RenderDepthFrameControls()
	{
		bool changed{};
		MenuFramework::SeparatorText(Localized("Depth-Aware Frame", "深度フレーム"));
		changed |= CheckboxWithHelp(
			"Enable Depth Frame", "深度フレームを有効にする", &depthFrameSettings.enabled,
			"Adds a photographic matte after post-processing. A tracked actor can remain visible in front of the matte.",
			"ポスト処理後に撮影用の枠を重ねます。追跡中の人物だけを枠より手前へ残せます。");
        MenuFramework::SameLine();
        changed |= CheckboxWithHelp("Lock-on Only", "ロックオン時のみ", &depthFrameSettings.lockOnOnly,
            "Show the depth frame only while True Directional Movement has a target lock.",
            "True Directional Movement でロックオンしている間だけ深度フレームを表示します。");
        if (depthFrameSettings.lockOnOnly) {
            MenuFramework::SameLine();
            MenuFramework::Text(!TDMIntegration::IsAvailable() ? Localized("TDM: Not detected", "TDM：未検出") :
                SubjectMask::IsTargetLocked() ?
                Localized("Locked on", "ロックオン中") : Localized("Unlocked", "ロックオンなし"));
        }
		MenuFramework::Text(Localized("Frame Style", "フレーム形状"));
		MenuFramework::SameLine();
		if (ChoiceButton(
				"Letterbox", "上下", "DepthFrameLetterbox", depthFrameSettings.style == 0U,
				"Draws only top and bottom bars.", "上下だけに帯を描画します。")) {
			depthFrameSettings.style = 0U;
			changed = true;
		}
		MenuFramework::SameLine();
		if (ChoiceButton(
				"Four Sides", "四辺", "DepthFrameFourSides", depthFrameSettings.style == 1U,
				"Draws a complete four-sided photographic frame.", "参考画像のような四辺の撮影用フレームを描画します。")) {
			depthFrameSettings.style = 1U;
			changed = true;
		}
		MenuFramework::SameLine();
        if (ChoiceButton("Independent", "個別", "DepthFrameIndependent", depthFrameSettings.style == 2U,
                "Set each edge independently.", "上下左右を個別に設定します。")) {
            depthFrameSettings.style = 2U;
            changed = true;
        }
        changed |= CheckboxWithHelp(
            "Subject Breakout", "被写体を枠の手前に出す", &depthFrameSettings.subjectBreakout,
			"Uses the active tracked actor, or the player as a fallback, to cut the subject out of the frame with depth confirmation.",
			"追跡中の人物を優先し、対象がなければプレイヤーを使います。深度でも確認して人物だけを枠の手前へ残します。");
        if (depthFrameSettings.subjectBreakout) {
            const auto edgeButton = [&](const char* english, const char* japanese, const char* id, std::uint32_t bit) {
                MenuFramework::SameLine();
                if (ChoiceButton(english, japanese, id, (depthFrameSettings.breakoutEdges & bit) != 0U,
                        "Toggle subject breakout on this side of the rotated frame.",
                        "回転した枠のこの辺で、被写体を手前に出すか切り替えます。")) {
                    depthFrameSettings.breakoutEdges ^= bit;
                    changed = true;
                }
            };
            edgeButton("Top", "上", "BreakoutTop", 1U);
            edgeButton("Bottom", "下", "BreakoutBottom", 2U);
            edgeButton("Right", "右", "BreakoutRight", 8U);
            edgeButton("Left", "左", "BreakoutLeft", 4U);
        }
		if (depthFrameSettings.style == 2U) {
            changed |= SliderWithHelp("Top Size", "上の幅", &depthFrameSettings.frameTop, 0.0F, 0.45F, "%.3f", "Zero hides this edge.", "0で非表示。");
            changed |= SliderWithHelp("Bottom Size", "下の幅", &depthFrameSettings.frameBottom, 0.0F, 0.45F, "%.3f", "Zero hides this edge.", "0で非表示。");
            changed |= SliderWithHelp("Left Size", "左の幅", &depthFrameSettings.frameLeft, 0.0F, 0.45F, "%.3f", "Zero hides this edge.", "0で非表示。");
            changed |= SliderWithHelp("Right Size", "右の幅", &depthFrameSettings.frameRight, 0.0F, 0.45F, "%.3f", "Zero hides this edge.", "0で非表示。");
        } else
            changed |= SliderWithHelp(
                "Top / Bottom Size", "上下の幅", &depthFrameSettings.topBottomSize,
			0.0F, 0.45F, "%.3f",
			"Thickness of the top and bottom matte as a fraction of screen height.",
			"画面高に対する上下フレームの幅です。");
		if (depthFrameSettings.style == 1U) {
			changed |= SliderWithHelp(
				"Left / Right Size", "左右の幅", &depthFrameSettings.sideSize,
				0.0F, 0.45F, "%.3f",
				"Thickness of the left and right frame as a fraction of screen width.",
				"画面幅に対する左右フレームの幅です。");
		}
		if (depthFrameSettings.subjectBreakout) {
			changed |= SliderWithHelp("Subject Inset", "被写体の輪郭を内側へ", &depthFrameSettings.subjectInsetPixels, 0.0F, 4.0F, "%.1f px", "Inset the subject mask. Zero keeps the original edge.", "マスクを内側に詰めます。0で従来の輪郭。");
		}
		changed |= SliderWithHelp(
            "Frame Rotation", "枠の回転", &depthFrameSettings.frameRotationDegrees,
            -180.0F, 180.0F, "%.1f deg", "Positive values rotate the frame clockwise.", "正の値で枠を時計回りに回転します。");
		changed |= SliderWithHelp(
			"Frame Opacity", "フレームの不透明度", &depthFrameSettings.opacity,
			0.0F, 1.0F, "%.2f",
			"Blends between the game image and the selected frame color.",
			"ゲーム画面と選択したフレーム色を混ぜる強さです。");
		changed |= ColorEditWithHelp(
			"Frame Color", "フレーム色", depthFrameSettings.color,
			"Black gives a cinematic letterbox; white or pale colors create a photographic print frame.",
			"黒は映画風、白や淡色はプリント写真風のフレームになります。");
		if (depthFrameSettings.subjectBreakout) {
			changed |= SliderWithHelp(
				"Expand Subject Boundary", "被写体の境目を拡張", &depthFrameSettings.subjectExpansion,
				0.0F, kSubjectExpansionMax, "%.3f",
				"Adds up to 6 px of extra room at the subject boundary where it overlaps the frame, helping keep fine hair and clothing edges visible.",
				"枠と重なる被写体の境目に最大6 pxの余白を足し、毛先や衣装の端を残しやすくします。");
		}
		changed |= SliderWithHelp(
			"Frame Edge Softness", "フレーム境界の柔らかさ", &depthFrameSettings.edgeSoftness,
			0.0F, 0.03F, "%.4f",
			"Softens only the straight inner edge of the frame.",
			"フレーム内側の直線境界だけを柔らかくします。");
		return changed;
	}

	bool RenderSilhouetteControls()
	{
		bool changed{};
		changed |= CheckboxWithHelp(
			"Enable Silhouette Mode",
			"シルエット撮影モードを有効にする",
			&silhouetteSettings.enabled,
			"Renders an independent two-color depth silhouette. It works even when the DoF master switch is off and leaves all DoF and preset values unchanged.",
			"現在のメイン深度を使い、独立した2色のシルエットを描画します。DoF本体がOFFでも使用でき、DoF設定やプリセットは変更しません。");
		changed |= CheckboxWithHelp(
			"CoC Monochrome", "CoC モノクロ", &silhouetteSettings.cocMonochrome,
			"Uses active DoF focus to shade the scene. Enable DoF for this style.", "DoFを有効にすると、ピントとぼけ量に応じて画面全体を白黒で表示します。");
		changed |= CheckboxWithHelp("Smooth Silhouette Edges",
			"シルエット輪郭を滑らかにする",
			&silhouetteSettings.smoothEdges,
			"Softens only the binary silhouette boundary. Turn this off for the original crisp, unblurred edge.",
			"二値シルエットの境界だけを滑らかにします。従来のくっきりした、ぼかし無しの輪郭に戻す場合はOFFにします。");
		changed |= ColorEditWithHelp(
			"Silhouette Color",
			"シルエット色",
			silhouetteSettings.foregroundColor,
			"Color used for pixels where world geometry writes depth.",
			"地形や人物など、深度が書き込まれている領域の色です。");
		changed |= ColorEditWithHelp(
			"Background Color",
			"背景色",
			silhouetteSettings.backgroundColor,
			"Color used for clear-depth sky pixels. Water, moons, particles, and transparent objects follow whether they write to the current main depth.",
			"深度が未描画の空に使う色です。水面・月・パーティクル・半透明物は、現在のメイン深度へ書き込むかどうかで分類されます。");
		MenuFramework::ItemTooltip(Localized(
			"Silhouette mode always stops in the main menu, loading screens, and world/local maps, independently of the normal DoF menu setting.",
			"通常DoFのメニュー設定とは別に、タイトル・ロード・ワールドマップ・ローカルマップでは常に停止します。"));
		RenderSilhouetteHotkeyControl();
		return changed;
	}

	void __stdcall RenderSilhouetteWindow()
	{
		std::scoped_lock lock(uiMutex);
		const auto fontPushed = PushLocalizedFont();
		bool open = true;
		const auto visible = MenuFramework::Begin(
			Localized(
				"Silhouette Photo Mode##CinematicDoFStandaloneSilhouette",
				"シルエット撮影モード##CinematicDoFStandaloneSilhouette"),
			&open);
		if (visible) {
			if (RenderSilhouetteControls()) {
				ApplyLive();
			}
			if (MenuFramework::Button(Localized("Save Startup Settings", "次回起動設定を保存"))) {
				if (SaveSilhouetteSettings(silhouetteSettings) && SaveHotkeySettings(hotkeySettings)) {
					SetStatus(
						"Saved silhouette startup settings and hotkey.",
						"シルエットの次回起動設定とホットキーを保存しました。");
				} else {
					SetStatus(
						"Could not save silhouette settings. See the plugin log.",
						"シルエット設定を保存できませんでした。ログを確認してください。");
				}
			}
			MenuFramework::ItemTooltip(Localized(
				"Saves the current silhouette switch, edge smoothing, and colors for the next launch. Hotkey assignments are saved immediately when changed.",
				"現在のシルエットON/OFF、輪郭の滑らかさ、2色を次回起動用に保存します。ホットキーは変更時に自動保存されます。"));
			MenuFramework::Text(
				interfaceSettings.japanese ? statusTextJapanese.c_str() : statusTextEnglish.c_str());
		}
		MenuFramework::End();
		if (!open && silhouetteWindow) {
			silhouetteWindow->isOpen = false;
		}
		if (fontPushed) {
			MenuFramework::PopFont();
		}
	}

	void __stdcall RenderDepthFrameWindow()
    {
        std::scoped_lock lock(uiMutex);
        const auto fontPushed = PushLocalizedFont();
        bool open = true;
        if (MenuFramework::Begin(Localized("Depth Frame##CDoFFrame", "深度フレーム##CDoFFrame"), &open)) {
            if (RenderDepthFrameControls()) {
                ApplyLive();
            }
            MenuFramework::SeparatorText(Localized("Frame Subject", "枠の手前に出す対象"));
            if (frameScanRequest == 0) RefreshFrameCandidates();
            if (CheckboxWithHelp("Focus Follows Frame Subject", "対象選択時にピントも追従させる",
                &depthFrameSettings.linkSubjectFocus,
                "Temporarily focuses on the selected subject while frame breakout is active. Returning to the player or disabling the frame resumes your previous focus mode.",
                "深度フレームで選択した対象に一時的にピントを追従します。プレイヤーに戻すか深度フレームをオフにすると元のピント設定に戻ります。")) {
                ApplyLive();
            }
            MenuFramework::SameLine();
            if (MenuFramework::Button(Localized("Refresh Subjects", "対象を更新"))) {
                showFrameCandidates = true;
                RefreshFrameCandidates();
            }
            RenderFrameCandidates();
            const auto frameSubject = SubjectMask::GetTarget();
            if (frameSubject) {
                const char* name = frameSubject->GetDisplayFullName();
                const auto label = std::format("{} {} [{:08X}]",
                    Localized("Current subject", "現在の対象"),
                    name && *name ? name : Localized("(Unnamed)", "（名前なし）"), frameSubject->GetFormID());
                MenuFramework::Text(label.c_str());
            } else {
                MenuFramework::Text(Localized("Registered subject unavailable.", "登録した対象は現在利用できません。"));
            }
            MenuFramework::SeparatorText(Localized("Shared with Main Settings", "メイン設定と共通"));
            RenderHairBlendingControls();
            if (MenuFramework::Button(Localized("Save Startup Settings", "次回起動設定を保存"))) {
                const bool frameSaved = SaveDepthFrameSettings(depthFrameSettings);
                const bool hairSaved = SaveExperimentalTransparencySettings(experimentalTransparencySettings);
                if (frameSaved && hairSaved) {
                    SetStatus("Saved depth frame and hair blending settings.", "深度フレームと髪を馴染ませる設定を保存しました。");
                } else {
                    SetStatus("Could not save all depth frame and hair blending settings.", "深度フレームと髪を馴染ませる設定の一部を保存できませんでした。");
                }
            }
            MenuFramework::Text(interfaceSettings.japanese ? statusTextJapanese.c_str() : statusTextEnglish.c_str());
        }
        MenuFramework::End();
        if (!open && depthFrameWindow) {
            depthFrameWindow->isOpen = false;
        }
        if (fontPushed) {
            MenuFramework::PopFont();
        }
    }

	void __stdcall RenderMainPage()
	{
		std::scoped_lock lock(uiMutex);
		const auto fontPushed = PushLocalizedFont();
		if (RenderModeControls()) {
			ApplyLive();
		}
		if (!silhouetteWindow && RenderSilhouetteControls()) {
			ApplyLive();
		}
		if (!depthFrameWindow && RenderDepthFrameControls()) {
			ApplyLive();
		}
		if (RenderCoreControls()) {
			ApplyLive();
		}
		RenderTDMLockFocusControls();
		RenderSneakFocusControls();
		RenderDialogueFocusControls();
		MenuFramework::SeparatorText(Localized("Preset Management", "プリセット管理"));
		RenderActions();
		MenuFramework::Text("Cinematic DoF Standalone 1.0.11");
		if (fontPushed) {
			MenuFramework::PopFont();
		}
	}

	void __stdcall RenderFocusPickerWindow()
	{
		std::scoped_lock lock(uiMutex);
		if (!focusPickerActive) {
			if (focusPickerWindow) {
				focusPickerWindow->isOpen = false;
			}
			return;
		}

		auto& focusX = ScreenFocusX(uiSettings, focusPickerFirstPerson);
		auto& focusY = ScreenFocusY(uiSettings, focusPickerFirstPerson);
		const auto previousX = focusX;
		const auto previousY = focusY;
		const auto result = MenuFramework::RenderFocusPicker(focusX, focusY);
		if (result == MenuFramework::FocusPickerResult::kUnavailable) {
			FinishFocusPicker(false, true);
			SetStatus(
				"Screen AF visual selection could not start with the installed Menu Framework.",
				"導入中のMenu Frameworkでは画面位置AFの視覚選択を開始できませんでした。");
			return;
		}
		if (!focusPickerAcceptInput) {
			focusX = focusPickerOriginalX;
			focusY = focusPickerOriginalY;
			focusPickerAcceptInput = true;
			return;
		}
		if (!NearlyEqual(previousX, focusX) || !NearlyEqual(previousY, focusY)) {
			DoFRenderer::GetSingleton().SetSettings(uiSettings);
		}
		if (result == MenuFramework::FocusPickerResult::kConfirmed) {
			FinishFocusPicker(true, true);
		} else if (result == MenuFramework::FocusPickerResult::kCancelled) {
			FinishFocusPicker(false, true);
		}
	}

}

void CDoF::UI::Initialize(Settings a_settings)
{
	std::scoped_lock lock(uiMutex);
	uiSettings = a_settings;
	targetFocusSettings = LoadTargetFocusSettings();
	tdmFocusSettings = LoadTDMFocusSettings();
	sneakFocusSettings = LoadSneakFocusSettings();
	modeSettings = LoadModeSettings();
	silhouetteSettings = LoadSilhouetteSettings();
	depthFrameSettings = LoadDepthFrameSettings();
	experimentalTransparencySettings = LoadExperimentalTransparencySettings();
	interfaceSettings = LoadInterfaceSettings();
	hotkeySettings = LoadHotkeySettings();
	japaneseFontEnabled = IsJapaneseFontEnabled();
	LoadPresets();
	editingPresetIndex = FindMatchingPreset();
	DoFRenderer::GetSingleton().SetModeSettings(modeSettings);
	DoFRenderer::GetSingleton().SetSilhouetteSettings(silhouetteSettings);
	DoFRenderer::GetSingleton().SetDepthFrameSettings(depthFrameSettings);
	ApplyExperimentalTransparency();
	ApplyTargetFocus();
	ApplyTDMFocus();
	ApplySneakFocus();
	initialized = true;
}

void CDoF::UI::TryRegister()
{
	std::scoped_lock lock(uiMutex);
	if (registered || !initialized || !MenuFramework::IsLoaded()) {
		return;
	}
	if (!MenuFramework::AddSectionItem("Cinematic DoF Standalone", "Depth of Field", RenderMainPage)) {
		spdlog::warn("SKSE Menu Framework is loaded but AddSectionItem is unavailable");
		return;
	}
	silhouetteWindow = MenuFramework::AddWindow(RenderSilhouetteWindow, true);
	depthFrameWindow = MenuFramework::AddWindow(RenderDepthFrameWindow, true);
	if (!silhouetteWindow) {
		spdlog::warn("SKSE Menu Framework AddWindow is unavailable; using inline silhouette controls");
	}
	focusPickerWindow = MenuFramework::AddWindow(RenderFocusPickerWindow, true);
	if (!focusPickerWindow) {
		spdlog::warn("SKSE Menu Framework AddWindow is unavailable; Screen AF visual picker disabled");
	}
	registered = true;
	spdlog::info("Registered SKSE Menu Framework UI (framework API {:.1f})", MenuFramework::GetVersion());
}

void CDoF::UI::HandleKeyboardKey(std::uint32_t a_keyCode)
{
	std::scoped_lock lock(uiMutex);
	if (a_keyCode == 0 || a_keyCode > 0xFFu) {
		return;
	}
	if (focusPickerActive && a_keyCode == kEscapeKey) {
		// Menu Framework also consumes Esc after this hook. Do not reopen its main
		// window here, otherwise the framework closes it again in the same event.
		FinishFocusPicker(false, false);
		return;
	}

	if (hotkeyCaptureTarget != HotkeyCaptureTarget::kNone) {
		if (a_keyCode == kEscapeKey) {
			hotkeyCaptureTarget = HotkeyCaptureTarget::kNone;
			SetStatus("Hotkey assignment cancelled.", "ホットキーの登録を中止しました。");
			return;
		}

		const auto previousSettings = hotkeySettings;
		const auto assignedKey =
			(a_keyCode == kBackspaceKey || a_keyCode == kDeleteKey) ? 0u : a_keyCode;
		const auto target = hotkeyCaptureTarget;
		if (target == HotkeyCaptureTarget::kDoF) {
			hotkeySettings.toggleDoFKey = assignedKey;
			if (assignedKey != 0 && hotkeySettings.toggleSilhouetteKey == assignedKey) {
				hotkeySettings.toggleSilhouetteKey = 0;
			}
		} else {
			hotkeySettings.toggleSilhouetteKey = assignedKey;
			if (assignedKey != 0 && hotkeySettings.toggleDoFKey == assignedKey) {
				hotkeySettings.toggleDoFKey = 0;
			}
		}
		if (!SaveHotkeySettings(hotkeySettings)) {
			hotkeySettings = previousSettings;
			hotkeyCaptureTarget = HotkeyCaptureTarget::kNone;
			SetStatus(
				"Could not save the hotkey. See the plugin log.",
				"ホットキーを保存できませんでした。ログを確認してください。");
			return;
		}

		hotkeyCaptureTarget = HotkeyCaptureTarget::kNone;
		if (assignedKey == 0) {
			SetStatus(
				target == HotkeyCaptureTarget::kDoF ? "DoF hotkey cleared." : "Silhouette hotkey cleared.",
				target == HotkeyCaptureTarget::kDoF ? "DoFホットキーを解除しました。" : "シルエットホットキーを解除しました。");
		} else {
			const auto keyName = HotkeyName(assignedKey);
			SetStatus(
				target == HotkeyCaptureTarget::kDoF ?
					std::format("DoF hotkey assigned to {}.", keyName) :
					std::format("Silhouette hotkey assigned to {}.", keyName),
				target == HotkeyCaptureTarget::kDoF ?
					std::format("DoFホットキーを{}に登録しました。", keyName) :
					std::format("シルエットホットキーを{}に登録しました。", keyName));
		}
		return;
	}

	if (hotkeySettings.toggleSilhouetteKey != 0 && a_keyCode == hotkeySettings.toggleSilhouetteKey) {
		silhouetteSettings.enabled = !silhouetteSettings.enabled;
		DoFRenderer::GetSingleton().SetSilhouetteSettings(silhouetteSettings);
		SetStatus(
			silhouetteSettings.enabled ?
				"Silhouette mode enabled by hotkey (not saved)." :
				"Silhouette mode disabled by hotkey (not saved).",
			silhouetteSettings.enabled ?
				"ホットキーでシルエット撮影モードをONにしました（INI未保存）。" :
				"ホットキーでシルエット撮影モードをOFFにしました（INI未保存）。");
		spdlog::info(
			"Silhouette mode {} by keyboard hotkey 0x{:02X}",
			silhouetteSettings.enabled ? "enabled" : "disabled",
			a_keyCode);
		return;
	}

	if (hotkeySettings.toggleDoFKey == 0 || a_keyCode != hotkeySettings.toggleDoFKey) {
		return;
	}

	uiSettings.enabled = !uiSettings.enabled;
	DoFRenderer::GetSingleton().SetSettings(uiSettings);
	SetStatus(
		uiSettings.enabled ? "DoF enabled by hotkey (not saved)." : "DoF disabled by hotkey (not saved).",
		uiSettings.enabled ? "ホットキーでDoFをONにしました（INI未保存）。" : "ホットキーでDoFをOFFにしました（INI未保存）。");
	spdlog::info("DoF {} by keyboard hotkey 0x{:02X}", uiSettings.enabled ? "enabled" : "disabled", a_keyCode);
}
