#include "DoFUI.h"

#include "DoFRenderer.h"
#include "MenuFramework.h"

#include <format>

namespace
{
	using namespace CDoF;

	Settings uiSettings{};
	TargetFocusSettings targetFocusSettings{};
	ModeSettings modeSettings{};
	SilhouetteSettings silhouetteSettings{};
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
	bool japaneseFontEnabled{};
	std::optional<std::size_t> editingPresetIndex{};
	std::optional<std::size_t> pendingPresetResetIndex{};
	std::string statusTextEnglish{ "Values apply immediately. Use Save Startup Settings to keep them." };
	std::string statusTextJapanese{ "変更はすぐ反映されます。次回起動にも残す場合は「次回起動設定を保存」を押してください。" };

	const char* Localized(const char* a_english, const char* a_japanese)
	{
		return interfaceSettings.japanese ? a_japanese : a_english;
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
		ApplyTargetFocus();
		SetStatus("Live values changed (not saved).", "現在値を変更しました（INI未保存）。");
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
		preset.farFocusRangeMeters = 2.1F;
		preset.farPlaneMaxBlur = 2.34F;
		preset.nearPlaneMaxBlur = 1.81F;
		preset.blurQuality = 8.0F;
		preset.nearFarDistanceCompensation = 3.7F;
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
		preset.farFocusRangeMeters = 2.7F;
		preset.farPlaneMaxBlur = 0.36F;
		preset.nearPlaneMaxBlur = 1.81F;
		preset.blurQuality = 8.0F;
		preset.nearFarDistanceCompensation = 3.67F;
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
		preset.focalLength = 196.7F;
		preset.fNumber = 5.1F;
		preset.nearFocusRangeMeters = 0.0369995F;
		preset.farFocusRangeMeters = 32.8575F;
		preset.farPlaneMaxBlur = 0.05F;
		preset.nearPlaneMaxBlur = 1.21F;
		preset.blurQuality = 8.8F;
		preset.nearFarDistanceCompensation = 4.91F;
		preset.bokehBusyFactor = 0.0F;
		preset.highlightBoost = 0.08F;
		preset.keepSkySharp = true;
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
		preset.nearFarDistanceCompensation = 3.83F;
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
		preset.focalLength = 70.3F;
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
		preset.apertureBlades = 5;
		preset.apertureRoundness = 0.1F;
		preset.apertureShapeStrength = 0.65F;
		preset.apertureRotationDegrees = 0.0F;
		preset.nearFocusRangeMeters = 0.0F;
		preset.farFocusRangeMeters = 2.1F;
		preset.keepSkySharp = false;
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
			a_left.apertureBlades == a_right.apertureBlades &&
			NearlyEqual(a_left.transitionSpeed, a_right.transitionSpeed) &&
			NearlyEqual(a_left.focusX, a_right.focusX) &&
			NearlyEqual(a_left.focusY, a_right.focusY) &&
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
			NearlyEqual(a_left.apertureRotationDegrees, a_right.apertureRotationDegrees);
	}

	bool Matches(const TargetFocusSettings& a_left, const TargetFocusSettings& a_right)
	{
		return a_left.dialogueEnabled == a_right.dialogueEnabled &&
			a_left.dialoguePreset == a_right.dialoguePreset &&
			NearlyEqual(a_left.dialogueDepthStrength, a_right.dialogueDepthStrength) &&
			a_left.consoleEnabled == a_right.consoleEnabled &&
			a_left.targetSource == a_right.targetSource &&
			NearlyEqual(a_left.targetFocusOffsetMeters, a_right.targetFocusOffsetMeters);
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
				"Screen AF", "画面位置AF", "ScreenFocusMode", screenSelected,
				"Read depth from the selected X/Y screen position.", "画面上のX・Y位置から距離を読み取ります。") &&
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
				"Tracks the reference last selected in the console. With no valid target, normal focus continues.",
				"コンソールで最後に選択した対象を追従します。有効な対象が無い場合は通常ピントを継続します。") &&
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
					"Target tracking is temporarily paused in first person.",
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
				SaveModeSettings(modeSettings) &&
				SaveSilhouetteSettings(silhouetteSettings) &&
				SaveInterfaceSettings(interfaceSettings)) {
				SetStatus("Saved startup settings to CinematicDoFStandalone.ini.", "次回起動設定をCinematicDoFStandalone.iniへ保存しました。");
			} else {
				SetStatus("Could not save INI. See the plugin log.", "INIを保存できませんでした。ログを確認してください。");
			}
		}
		MenuFramework::ItemTooltip(Localized(
			"Saves the master switch, normal-gameplay mode, DoF, silhouette, dialogue-focus, target-tracking, and interface-language values currently applied on screen for the next launch. It does not overwrite any preset slot.",
			"現在映像へ適用中の主スイッチ・会話外DoF・DoF・シルエット・会話フォーカス・対象追従設定とUI言語を、次回起動時の設定として保存します。プリセット枠は上書きしません。"));
		MenuFramework::SameLine();
		if (MenuFramework::Button(Localized("Reload INI", "INIを再読み込み"))) {
			uiSettings = LoadSettings();
			targetFocusSettings = LoadTargetFocusSettings();
			modeSettings = LoadModeSettings();
			silhouetteSettings = LoadSilhouetteSettings();
			interfaceSettings = LoadInterfaceSettings();
			hotkeySettings = LoadHotkeySettings();
			hotkeyCaptureTarget = HotkeyCaptureTarget::kNone;
			LoadPresets();
			editingPresetIndex = FindMatchingPreset();
			DoFRenderer::GetSingleton().SetSettings(uiSettings);
			DoFRenderer::GetSingleton().SetModeSettings(modeSettings);
			DoFRenderer::GetSingleton().SetSilhouetteSettings(silhouetteSettings);
			ApplyTargetFocus();
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
		if (!targetFocusSettings.consoleEnabled && uiSettings.autoFocus) {
			changed |= SliderWithHelp(
				"Focus Point X",
				"自動ピント位置 X",
				&uiSettings.focusX,
				0.0F,
				1.0F,
				"%.2f",
				"Horizontal screen position sampled by Screen AF. Move left to aim left and right to aim right; center is 0.50.",
				"画面位置AFが距離を読み取る横位置です。スライダーを左へ動かすと照準も左、右へ動かすと右へ移動します。中央は0.50です。");
			changed |= SliderWithHelp(
				"Focus Point Y",
				"自動ピント位置 Y",
				&uiSettings.focusY,
				0.0F,
				1.0F,
				"%.2f",
				"Vertical screen position sampled by Screen AF. Move left for the top of the screen and right for the bottom; center is 0.50.",
				"画面位置AFが距離を読み取る縦位置です。スライダーを左へ動かすと照準は画面上、右へ動かすと画面下へ移動します。中央は0.50です。");
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
			changed |= SliderWithHelp(
				"Focus Distance",
				"ピント距離",
				&uiSettings.manualFocusMeters,
				0.1F,
				150.0F,
				"%.2f m",
				"Fixed distance from the active camera to the focus plane.",
				"現在のカメラからピント面までの固定距離です。");
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
		MenuFramework::SameLine();
		changed |= CheckboxWithHelp(
			"Keep Sky Sharp",
			"空を鮮明に保つ",
			&uiSettings.keepSkySharp,
			"Excludes clear-depth sky pixels from DoF while preserving a soft boundary around geometry. Moons and other sky objects that write depth can still be blurred.",
			"深度が未描画の空をDoFから除外し、地形との境界は滑らかに保ちます。月など深度を書き込む天体はぼける場合があります。");
		if (showAdvanced) {
			MenuFramework::SeparatorText(Localized("Bokeh and Quality", "画質・ボケの詳細"));
			changed |= CheckboxWithHelp(
				"1st-Person Near Blur",
				"一人称の手前ぼかし",
				&uiSettings.enableFirstPersonNearBlur,
				"When enabled, the configured near blur is also used in first person. Leave this off for clear normal gameplay.",
				"ONでは設定した手前ぼかしを一人称視点にも適用します。通常プレイで手前をくっきり表示する場合はOFFにします。");
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
				changed |= CheckboxWithHelp(
					"Aperture Bokeh", "絞り形状ボケ", &uiSettings.apertureBokeh,
					"Shapes the blur kernel with a procedurally generated lens aperture. The shape is easiest to see on bright points that are strongly out of focus. No external image is used.",
					"数値から生成したレンズ絞りの形をボケへ反映します。大きくピンぼけした明るい点ほど形が見えやすくなります。外部画像は使用しません。");
				if (uiSettings.apertureBokeh) {
					changed |= ApertureBladeSlider(uiSettings);
					changed |= SliderWithHelp(
						"Blade Roundness", "羽根の丸み", &uiSettings.apertureRoundness, 0.0F, 1.0F, "%.2f",
						"Blends the aperture from a straight-edged polygon toward a circle.",
						"直線的な多角形から円形へ、絞り形状の丸みを調整します。");
					changed |= SliderWithHelp(
						"Shape Strength", "形状の強さ", &uiSettings.apertureShapeStrength, 0.0F, 1.0F, "%.2f",
						"Controls how strongly the aperture shape affects the blur. Zero behaves like the standard circular blur; one uses the full generated shape.",
						"絞り形状をボケへ反映する強さです。0では通常の円形ボケ、1では生成した形状を最大限に反映します。");
					changed |= SliderWithHelp(
						"Shape Rotation", "形状の回転", &uiSettings.apertureRotationDegrees, 0.0F, 360.0F, "%.0f deg",
						"Rotates the aperture shape around the optical axis.",
						"光軸を中心に絞り形状を回転します。");
				}
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
					"Disable in Main/Loading/Map Menus",
					"タイトル・ロード・マップ画面ではDoFを停止",
					&uiSettings.disableInMenus,
					"Skip DoF in the main menu, loading screens, and world/local maps.",
					"タイトル・ロード・ワールドマップ・ローカルマップではDoFを停止します。");
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
		changed |= CheckboxWithHelp(
			"Use DoF outside dialogue",
			"会話外でもDoFを使用",
			&modeSettings.normalGameplayEnabled,
			"Turn this off for dialogue-only DoF. The master switch and Dialogue Focus must remain enabled.",
			"OFFにすると会話中だけDoFを使用します。主スイッチと「会話中の被写体にピントを合わせる」はONのままにしてください。");
		RenderLanguageControls();
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
				"Saves the current silhouette switch and colors for the next launch. Hotkey assignments are saved immediately when changed.",
				"現在のシルエットON/OFFと2色を次回起動用に保存します。ホットキーは変更時に自動保存されます。"));
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
		if (RenderCoreControls()) {
			ApplyLive();
		}
		RenderDialogueFocusControls();
		MenuFramework::SeparatorText(Localized("Preset Management", "プリセット管理"));
		RenderActions();
		MenuFramework::Text("Cinematic DoF Standalone 1.0.1 - Silhouette UI Test 9");
		if (fontPushed) {
			MenuFramework::PopFont();
		}
	}

}

void CDoF::UI::Initialize(Settings a_settings)
{
	std::scoped_lock lock(uiMutex);
	uiSettings = a_settings;
	targetFocusSettings = LoadTargetFocusSettings();
	modeSettings = LoadModeSettings();
	silhouetteSettings = LoadSilhouetteSettings();
	interfaceSettings = LoadInterfaceSettings();
	hotkeySettings = LoadHotkeySettings();
	japaneseFontEnabled = IsJapaneseFontEnabled();
	LoadPresets();
	editingPresetIndex = FindMatchingPreset();
	DoFRenderer::GetSingleton().SetModeSettings(modeSettings);
	DoFRenderer::GetSingleton().SetSilhouetteSettings(silhouetteSettings);
	ApplyTargetFocus();
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
	if (!silhouetteWindow) {
		spdlog::warn("SKSE Menu Framework AddWindow is unavailable; using inline silhouette controls");
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
