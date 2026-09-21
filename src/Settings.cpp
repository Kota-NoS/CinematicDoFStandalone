#include "Settings.h"

namespace
{
	constexpr auto kPath = L"Data\\SKSE\\Plugins\\CinematicDoFStandalone.ini";
	constexpr auto kSection = L"DepthOfField";
	constexpr auto kTargetFocusSection = L"TargetFocus";
	constexpr auto kModeSection = L"Modes";
	constexpr auto kInterfaceSection = L"Interface";
	constexpr auto kHotkeySection = L"Hotkeys";

	bool ReadBool(const wchar_t* a_section, const wchar_t* a_key, bool a_default)
	{
		wchar_t value[32]{};
		GetPrivateProfileStringW(
			a_section,
			a_key,
			a_default ? L"true" : L"false",
			value,
			static_cast<DWORD>(std::size(value)),
			kPath);
		std::wstring normalized(value);
		std::ranges::transform(normalized, normalized.begin(), [](wchar_t a_character) {
			return static_cast<wchar_t>(std::towlower(a_character));
		});
		if (normalized == L"true" || normalized == L"yes" || normalized == L"on" || normalized == L"1") {
			return true;
		}
		if (normalized == L"false" || normalized == L"no" || normalized == L"off" || normalized == L"0") {
			return false;
		}
		spdlog::warn("Invalid boolean INI value for {}.{}; using {}",
			std::filesystem::path(a_section).string(),
			std::filesystem::path(a_key).string(),
			a_default);
		return a_default;
	}

	float ReadFloat(const wchar_t* a_section, const wchar_t* a_key, float a_default)
	{
		wchar_t fallback[32]{};
		wchar_t value[64]{};
		swprintf_s(fallback, L"%.6g", static_cast<double>(a_default));
		GetPrivateProfileStringW(a_section, a_key, fallback, value, static_cast<DWORD>(std::size(value)), kPath);
		try {
			return std::stof(value);
		} catch (...) {
			spdlog::warn("Invalid INI value for {}.{}; using {}",
				std::filesystem::path(a_section).string(),
				std::filesystem::path(a_key).string(),
				a_default);
			return a_default;
		}
	}

	std::uint32_t ReadUInt(const wchar_t* a_section, const wchar_t* a_key, std::uint32_t a_default)
	{
		wchar_t fallback[32]{};
		wchar_t value[64]{};
		swprintf_s(fallback, L"%u", a_default);
		GetPrivateProfileStringW(a_section, a_key, fallback, value, static_cast<DWORD>(std::size(value)), kPath);
		try {
			return static_cast<std::uint32_t>(std::stoul(value));
		} catch (...) {
			spdlog::warn("Invalid INI value for {}.{}; using {}",
				std::filesystem::path(a_section).string(),
				std::filesystem::path(a_key).string(),
				a_default);
			return a_default;
		}
	}

	float Clamp(float a_value, float a_min, float a_max)
	{
		return std::clamp(a_value, a_min, a_max);
	}

	bool WriteValue(const wchar_t* a_section, const wchar_t* a_key, const wchar_t* a_value)
	{
		if (WritePrivateProfileStringW(a_section, a_key, a_value, kPath)) {
			return true;
		}
		spdlog::error("Failed to write INI key {}.{} (Win32 error {})",
			std::filesystem::path(a_section).string(),
			std::filesystem::path(a_key).string(),
			GetLastError());
		return false;
	}

	bool WriteBool(const wchar_t* a_section, const wchar_t* a_key, bool a_value)
	{
		return WriteValue(a_section, a_key, a_value ? L"true" : L"false");
	}

	bool WriteFloat(const wchar_t* a_section, const wchar_t* a_key, float a_value)
	{
		wchar_t value[32]{};
		swprintf_s(value, L"%.6g", static_cast<double>(a_value));
		return WriteValue(a_section, a_key, value);
	}

	bool WriteUInt(const wchar_t* a_section, const wchar_t* a_key, std::uint32_t a_value)
	{
		wchar_t value[32]{};
		swprintf_s(value, L"%u", a_value);
		return WriteValue(a_section, a_key, value);
	}

	CDoF::Settings LoadSection(const wchar_t* a_section, CDoF::Settings a_defaults)
	{
		a_defaults.enabled = ReadBool(a_section, L"Enabled", a_defaults.enabled);
		a_defaults.autoFocus = ReadBool(a_section, L"AutoFocus", a_defaults.autoFocus);
		a_defaults.disableInMenus = ReadBool(a_section, L"DisableInMainAndLoadingMenus", a_defaults.disableInMenus);
		a_defaults.transitionSpeed = Clamp(ReadFloat(a_section, L"TransitionSpeed", a_defaults.transitionSpeed), 0.01F, 1.0F);
		a_defaults.focusX = Clamp(ReadFloat(a_section, L"FocusX", a_defaults.focusX), 0.0F, 1.0F);
		a_defaults.focusY = Clamp(ReadFloat(a_section, L"FocusY", a_defaults.focusY), 0.0F, 1.0F);
		a_defaults.autoFocusOffsetMeters = Clamp(
			ReadFloat(a_section, L"AutoFocusOffsetMeters", a_defaults.autoFocusOffsetMeters), -10.0F, 10.0F);
		a_defaults.manualFocusMeters = Clamp(ReadFloat(a_section, L"ManualFocusMeters", a_defaults.manualFocusMeters), 0.1F, 150.0F);
		a_defaults.focalLength = Clamp(ReadFloat(a_section, L"FocalLength", a_defaults.focalLength), 1.0F, 300.0F);
		a_defaults.fNumber = Clamp(ReadFloat(a_section, L"FNumber", a_defaults.fNumber), 1.0F, 22.0F);
		a_defaults.nearFocusRangeMeters = Clamp(
			ReadFloat(a_section, L"NearFocusRangeMeters", a_defaults.nearFocusRangeMeters), 0.0F, 10.0F);
		a_defaults.farFocusRangeMeters = Clamp(
			ReadFloat(a_section, L"FarFocusRangeMeters", a_defaults.farFocusRangeMeters), 0.0F, 100.0F);
		a_defaults.farPlaneMaxBlur = Clamp(ReadFloat(a_section, L"FarPlaneMaxBlur", a_defaults.farPlaneMaxBlur), 0.0F, 8.0F);
		a_defaults.nearPlaneMaxBlur = Clamp(ReadFloat(a_section, L"NearPlaneMaxBlur", a_defaults.nearPlaneMaxBlur), 0.0F, 4.0F);
		a_defaults.enableFirstPersonNearBlur = ReadBool(
			a_section, L"EnableFirstPersonNearBlur", a_defaults.enableFirstPersonNearBlur);
		a_defaults.keepSkySharp = ReadBool(a_section, L"KeepSkySharp", a_defaults.keepSkySharp);
		a_defaults.blurQuality = Clamp(ReadFloat(a_section, L"BlurQuality", a_defaults.blurQuality), 2.0F, 30.0F);
		a_defaults.nearFarDistanceCompensation = Clamp(ReadFloat(a_section, L"NearFarDistanceCompensation", a_defaults.nearFarDistanceCompensation), 1.0F, 5.0F);
		a_defaults.bokehBusyFactor = Clamp(ReadFloat(a_section, L"BokehBusyFactor", a_defaults.bokehBusyFactor), 0.0F, 1.0F);
		a_defaults.highlightBoost = Clamp(ReadFloat(a_section, L"HighlightBoost", a_defaults.highlightBoost), 0.0F, 1.0F);
		a_defaults.postBlurSmoothing = Clamp(ReadFloat(a_section, L"PostBlurSmoothing", a_defaults.postBlurSmoothing), 0.0F, 2.0F);
		a_defaults.petzvalStrength = Clamp(ReadFloat(a_section, L"PetzvalStrength", a_defaults.petzvalStrength), 0.0F, 2.0F);
		a_defaults.apertureBokeh = ReadBool(a_section, L"ApertureBokeh", a_defaults.apertureBokeh);
		a_defaults.apertureBlades = std::clamp(
			ReadUInt(a_section, L"ApertureBlades", a_defaults.apertureBlades), 3U, 12U);
		a_defaults.apertureRoundness = Clamp(
			ReadFloat(a_section, L"ApertureRoundness", a_defaults.apertureRoundness), 0.0F, 1.0F);
		a_defaults.apertureShapeStrength = Clamp(
			ReadFloat(a_section, L"ApertureShapeStrength", a_defaults.apertureShapeStrength), 0.0F, 1.0F);
		a_defaults.apertureRotationDegrees = Clamp(
			ReadFloat(a_section, L"ApertureRotationDegrees", a_defaults.apertureRotationDegrees), 0.0F, 360.0F);
		return a_defaults;
	}

	bool SaveSection(const wchar_t* a_section, const CDoF::Settings& a_settings)
	{
		bool success = true;
		success = WriteBool(a_section, L"Enabled", a_settings.enabled) && success;
		success = WriteBool(a_section, L"AutoFocus", a_settings.autoFocus) && success;
		success = WriteBool(a_section, L"DisableInMainAndLoadingMenus", a_settings.disableInMenus) && success;
		success = WriteFloat(a_section, L"FocusX", a_settings.focusX) && success;
		success = WriteFloat(a_section, L"FocusY", a_settings.focusY) && success;
		success = WriteFloat(a_section, L"AutoFocusOffsetMeters", a_settings.autoFocusOffsetMeters) && success;
		success = WriteFloat(a_section, L"TransitionSpeed", a_settings.transitionSpeed) && success;
		success = WriteFloat(a_section, L"ManualFocusMeters", a_settings.manualFocusMeters) && success;
		success = WriteFloat(a_section, L"FocalLength", a_settings.focalLength) && success;
		success = WriteFloat(a_section, L"FNumber", a_settings.fNumber) && success;
		success = WriteFloat(a_section, L"NearFocusRangeMeters", a_settings.nearFocusRangeMeters) && success;
		success = WriteFloat(a_section, L"FarFocusRangeMeters", a_settings.farFocusRangeMeters) && success;
		success = WriteFloat(a_section, L"FarPlaneMaxBlur", a_settings.farPlaneMaxBlur) && success;
		success = WriteFloat(a_section, L"NearPlaneMaxBlur", a_settings.nearPlaneMaxBlur) && success;
		success = WriteBool(a_section, L"EnableFirstPersonNearBlur", a_settings.enableFirstPersonNearBlur) && success;
		success = WriteBool(a_section, L"KeepSkySharp", a_settings.keepSkySharp) && success;
		success = WriteFloat(a_section, L"BlurQuality", a_settings.blurQuality) && success;
		success = WriteFloat(a_section, L"NearFarDistanceCompensation", a_settings.nearFarDistanceCompensation) && success;
		success = WriteFloat(a_section, L"BokehBusyFactor", a_settings.bokehBusyFactor) && success;
		success = WriteFloat(a_section, L"HighlightBoost", a_settings.highlightBoost) && success;
		success = WriteFloat(a_section, L"PostBlurSmoothing", a_settings.postBlurSmoothing) && success;
		success = WriteFloat(a_section, L"PetzvalStrength", a_settings.petzvalStrength) && success;
		success = WriteBool(a_section, L"ApertureBokeh", a_settings.apertureBokeh) && success;
		success = WriteUInt(a_section, L"ApertureBlades", std::clamp(a_settings.apertureBlades, 3U, 12U)) && success;
		success = WriteFloat(a_section, L"ApertureRoundness", a_settings.apertureRoundness) && success;
		success = WriteFloat(a_section, L"ApertureShapeStrength", a_settings.apertureShapeStrength) && success;
		success = WriteFloat(a_section, L"ApertureRotationDegrees", a_settings.apertureRotationDegrees) && success;
		WritePrivateProfileStringW(nullptr, nullptr, nullptr, kPath);
		return success;
	}

	bool SectionExists(const wchar_t* a_section)
	{
		wchar_t value[16]{};
		GetPrivateProfileStringW(a_section, L"FocalLength", L"", value, static_cast<DWORD>(std::size(value)), kPath);
		return value[0] != L'\0';
	}

	CDoF::TargetFocusSettings LoadTargetFocusSection(
		const wchar_t* a_section,
		CDoF::TargetFocusSettings a_defaults)
	{
		a_defaults.dialogueEnabled = ReadBool(a_section, L"DialogueEnabled", a_defaults.dialogueEnabled);
		a_defaults.dialoguePreset = std::min(
			ReadUInt(a_section, L"DialoguePreset", a_defaults.dialoguePreset), CDoF::kMaxPresetIndex);
		a_defaults.dialogueDepthStrength = Clamp(
			ReadFloat(a_section, L"DialogueDepthStrength", a_defaults.dialogueDepthStrength), 0.0F, 3.0F);
		a_defaults.consoleEnabled = ReadBool(a_section, L"ConsoleEnabled", a_defaults.consoleEnabled);
		a_defaults.targetSource = static_cast<CDoF::TargetFocusSource>(std::min(
			ReadUInt(
				a_section,
				L"TargetSource",
				static_cast<std::uint32_t>(a_defaults.targetSource)),
			1U));
		a_defaults.targetFocusOffsetMeters = Clamp(
			ReadFloat(a_section, L"TargetFocusOffsetMeters", a_defaults.targetFocusOffsetMeters), -10.0F, 10.0F);
		return a_defaults;
	}

	bool SaveTargetFocusSection(const wchar_t* a_section, const CDoF::TargetFocusSettings& a_settings)
	{
		bool success = true;
		success = WriteBool(a_section, L"DialogueEnabled", a_settings.dialogueEnabled) && success;
		success = WriteUInt(
			a_section, L"DialoguePreset", std::min(a_settings.dialoguePreset, CDoF::kMaxPresetIndex)) && success;
		success = WriteFloat(
			a_section,
			L"DialogueDepthStrength",
			std::clamp(a_settings.dialogueDepthStrength, 0.0F, 3.0F)) && success;
		success = WriteBool(a_section, L"ConsoleEnabled", a_settings.consoleEnabled) && success;
		success = WriteUInt(
			a_section,
			L"TargetSource",
			std::min(static_cast<std::uint32_t>(a_settings.targetSource), 1U)) && success;
		success = WriteFloat(
			a_section,
			L"TargetFocusOffsetMeters",
			std::clamp(a_settings.targetFocusOffsetMeters, -10.0F, 10.0F)) && success;
		WritePrivateProfileStringW(nullptr, nullptr, nullptr, kPath);
		return success;
	}
}

CDoF::Settings CDoF::LoadSettings()
{
	const auto result = LoadSection(kSection, Settings{});

	spdlog::info("Settings loaded: enabled={}, autofocus={}, focalLength={}, fNumber={}",
		result.enabled,
		result.autoFocus,
		result.focalLength,
		result.fNumber);
	return result;
}

bool CDoF::SaveSettings(const Settings& a_settings)
{
	const auto success = SaveSection(kSection, a_settings);
	if (success) {
		spdlog::info("Settings saved to CinematicDoFStandalone.ini");
	}
	return success;
}

CDoF::Settings CDoF::LoadPresetSettings(const wchar_t* a_section, Settings a_defaults)
{
	if (!SectionExists(a_section)) {
		return a_defaults;
	}

	// Presets saved before KeepSkySharp existed must retain their original sky
	// blur. New built-in presets can still opt in by setting their own default,
	// while an explicitly stored KeepSkySharp value continues to win here.
	a_defaults.keepSkySharp = false;
	return LoadSection(a_section, a_defaults);
}

bool CDoF::SavePresetSettings(const wchar_t* a_section, const Settings& a_settings)
{
	const auto success = SaveSection(a_section, a_settings);
	if (success) {
		spdlog::info("Preset saved to INI section {}", std::filesystem::path(a_section).string());
	}
	return success;
}

bool CDoF::PresetSectionExists(const wchar_t* a_section)
{
	return SectionExists(a_section);
}

CDoF::TargetFocusSettings CDoF::LoadTargetFocusSettings()
{
	const auto result = LoadTargetFocusSection(kTargetFocusSection, TargetFocusSettings{});
	spdlog::info(
		"Target focus loaded: dialogue enabled={}, preset={}, strength={}; target enabled={}, source={}, offset={} m",
		result.dialogueEnabled,
		result.dialoguePreset,
		result.dialogueDepthStrength,
		result.consoleEnabled,
		static_cast<std::uint32_t>(result.targetSource),
		result.targetFocusOffsetMeters);
	return result;
}

bool CDoF::SaveTargetFocusSettings(const TargetFocusSettings& a_settings)
{
	const auto success = SaveTargetFocusSection(kTargetFocusSection, a_settings);
	if (success) {
		spdlog::info("Target focus settings saved");
	}
	return success;
}

CDoF::ModeSettings CDoF::LoadModeSettings()
{
	ModeSettings result{};
	result.normalGameplayEnabled = ReadBool(
		kModeSection, L"NormalGameplayEnabled", result.normalGameplayEnabled);
	spdlog::info("Mode settings loaded: normal gameplay DoF={}", result.normalGameplayEnabled);
	return result;
}

bool CDoF::SaveModeSettings(const ModeSettings& a_settings)
{
	const auto success = WriteBool(
		kModeSection, L"NormalGameplayEnabled", a_settings.normalGameplayEnabled);
	WritePrivateProfileStringW(nullptr, nullptr, nullptr, kPath);
	if (success) {
		spdlog::info("Mode settings saved");
	}
	return success;
}

CDoF::TargetFocusSettings CDoF::LoadPresetTargetFocusSettings(
	const wchar_t* a_section,
	TargetFocusSettings a_defaults)
{
	return SectionExists(a_section) ? LoadTargetFocusSection(a_section, a_defaults) : a_defaults;
}

bool CDoF::SavePresetTargetFocusSettings(
	const wchar_t* a_section,
	const TargetFocusSettings& a_settings)
{
	const auto success = SaveTargetFocusSection(a_section, a_settings);
	if (success) {
		spdlog::info("Preset target focus saved to INI section {}", std::filesystem::path(a_section).string());
	}
	return success;
}

CDoF::InterfaceSettings CDoF::LoadInterfaceSettings()
{
	InterfaceSettings result{};
	result.japanese = ReadBool(kInterfaceSection, L"Japanese", result.japanese);
	return result;
}

bool CDoF::SaveInterfaceSettings(const InterfaceSettings& a_settings)
{
	const auto success = WriteBool(kInterfaceSection, L"Japanese", a_settings.japanese);
	WritePrivateProfileStringW(nullptr, nullptr, nullptr, kPath);
	return success;
}

CDoF::HotkeySettings CDoF::LoadHotkeySettings()
{
	HotkeySettings result{};
	result.toggleDoFKey = std::min(ReadUInt(kHotkeySection, L"ToggleDoF", result.toggleDoFKey), 0xFFu);
	return result;
}

bool CDoF::SaveHotkeySettings(const HotkeySettings& a_settings)
{
	const auto success = WriteUInt(kHotkeySection, L"ToggleDoF", std::min(a_settings.toggleDoFKey, 0xFFu));
	WritePrivateProfileStringW(nullptr, nullptr, nullptr, kPath);
	return success;
}
