#include "Settings.h"
#include <Windows.h>
#ifndef CP_UTF8
#define CP_UTF8 65001
#endif
#include <SimpleIni.h>

namespace
{
	constexpr auto kPath = L"Data\\SKSE\\Plugins\\CinematicDoFStandalone.ini";
	constexpr auto kSection = L"DepthOfField";
	constexpr auto kTargetFocusSection = L"TargetFocus";
	constexpr auto kTDMFocusSection = L"TDMLockFocus";
	constexpr auto kSneakFocusSection = L"SneakFocus";
	constexpr auto kModeSection = L"Modes";
	constexpr auto kSilhouetteSection = L"Silhouette";
	constexpr auto kDepthFrameSection = L"DepthFrame";
	constexpr auto kExperimentalTransparencySection = L"ExperimentalTransparency";
	constexpr auto kInterfaceSection = L"Interface";
	constexpr auto kHotkeySection = L"Hotkeys";

    std::mutex iniMutex;
    CSimpleIniW ini;
    bool iniReadable = false;

    struct IniAccess
    {
        std::scoped_lock<std::mutex> lock{ iniMutex };

        IniAccess()
        {
            ini.Reset();
            const auto attributes = GetFileAttributesW(kPath);
            const auto error = attributes == INVALID_FILE_ATTRIBUTES ? GetLastError() : ERROR_SUCCESS;
            iniReadable = error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND ||
                (error == ERROR_SUCCESS && ini.LoadFile(kPath) >= 0);
            if (!iniReadable) {
                spdlog::error("Could not read CinematicDoFStandalone.ini");
            }
        }
    };

    bool CommitIni(bool a_success)
    {
        if (!a_success || !iniReadable) {
            return false;
        }
        const std::wstring temporary = std::wstring(kPath) + L".tmp";
        if (ini.SaveFile(temporary.c_str(), false) >= 0 &&
            MoveFileExW(temporary.c_str(), kPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            return true;
        }
        DeleteFileW(temporary.c_str());
        spdlog::error("Could not replace CinematicDoFStandalone.ini");
        return false;
    }

    void ReadValue(const wchar_t* a_section, const wchar_t* a_key, const wchar_t* a_default,
        wchar_t* a_buffer, size_t a_capacity)
    {
        std::wstring_view value = ini.GetValue(a_section, a_key, a_default);
        if (value.size() >= 2 && (value.front() == L'"' || value.front() == L'\'') && value.back() == value.front()) {
            value.remove_prefix(1);
            value.remove_suffix(1);
        }
        wcsncpy_s(a_buffer, a_capacity, value.data(), std::min(value.size(), a_capacity - 1));
    }

	bool ReadBool(const wchar_t* a_section, const wchar_t* a_key, bool a_default)
	{
		wchar_t value[32]{};
		ReadValue(
			a_section,
			a_key,
			a_default ? L"true" : L"false",
			value,
			std::size(value));
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
		ReadValue(a_section, a_key, fallback, value, std::size(value));
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
		ReadValue(a_section, a_key, fallback, value, std::size(value));
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
		if (iniReadable && ini.SetValue(a_section, a_key, a_value) >= 0) {
			return true;
		}
		spdlog::error("Failed to write INI key {}.{}",
			std::filesystem::path(a_section).string(),
			std::filesystem::path(a_key).string());
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
		a_defaults.firstPersonFocusX = Clamp(
			ReadFloat(a_section, L"FirstPersonFocusX", a_defaults.firstPersonFocusX), 0.0F, 1.0F);
		a_defaults.firstPersonFocusY = Clamp(
			ReadFloat(a_section, L"FirstPersonFocusY", a_defaults.firstPersonFocusY), 0.0F, 1.0F);
		a_defaults.autoFocusOffsetMeters = Clamp(
			ReadFloat(a_section, L"AutoFocusOffsetMeters", a_defaults.autoFocusOffsetMeters), -10.0F, 10.0F);
		a_defaults.manualFocusMeters = Clamp(
			ReadFloat(a_section, L"ManualFocusMeters", a_defaults.manualFocusMeters),
			CDoF::kManualFocusMinMeters,
			CDoF::kManualFocusMaxMeters);
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
		a_defaults.apertureFill = ReadBool(a_section, L"ApertureFill", a_defaults.apertureFill);
		a_defaults.apertureShape = std::clamp(
			ReadUInt(a_section, L"ApertureShape", a_defaults.apertureShape), 0U, 2U);
		a_defaults.apertureBlades = std::clamp(
			ReadUInt(a_section, L"ApertureBlades", a_defaults.apertureBlades), 3U, 12U);
		a_defaults.apertureRoundness = Clamp(
			ReadFloat(a_section, L"ApertureRoundness", a_defaults.apertureRoundness), 0.0F, 1.0F);
		a_defaults.apertureShapeStrength = Clamp(
			ReadFloat(a_section, L"ApertureShapeStrength", a_defaults.apertureShapeStrength), 0.0F, 1.0F);
		a_defaults.apertureRotationDegrees = Clamp(
			ReadFloat(a_section, L"ApertureRotationDegrees", a_defaults.apertureRotationDegrees), 0.0F, 360.0F);
		a_defaults.starburstEnabled = ReadBool(a_section, L"StarburstEnabled", a_defaults.starburstEnabled);
		a_defaults.starburstRays = std::clamp(
			ReadUInt(a_section, L"StarburstRays", a_defaults.starburstRays), 4U, 12U);
		a_defaults.starburstStrength = Clamp(
			ReadFloat(a_section, L"StarburstStrength", a_defaults.starburstStrength), 0.0F, 16.0F);
		a_defaults.starburstLength = Clamp(
			ReadFloat(a_section, L"StarburstLength", a_defaults.starburstLength), 0.05F, 1.5F);
		const auto legacyStarburstThreshold =
			ReadFloat(a_section, L"StarburstThreshold", a_defaults.highlightExtractionThreshold);
		a_defaults.highlightExtractionThreshold = Clamp(
			ReadFloat(a_section, L"HighlightExtractionThreshold", legacyStarburstThreshold), 0.20F, 1.50F);
		a_defaults.starburstRotationDegrees = Clamp(
			ReadFloat(a_section, L"StarburstRotationDegrees", a_defaults.starburstRotationDegrees), 0.0F, 180.0F);
		const auto legacyStarburstCoreFill = ReadFloat(a_section, L"StarburstCoreFill", a_defaults.bokehCoreFill);
		a_defaults.bokehCoreFill = Clamp(
			ReadFloat(a_section, L"BokehCoreFill", legacyStarburstCoreFill), 0.0F, 2.0F);
		a_defaults.starburstWideFlareSuppression = Clamp(
			ReadFloat(a_section, L"StarburstWideFlareSuppression", a_defaults.starburstWideFlareSuppression), 0.0F, 1.0F);
		a_defaults.starburstHighlightSensitivity = Clamp(
			ReadFloat(a_section, L"StarburstHighlightSensitivity", a_defaults.starburstHighlightSensitivity), 1.0F, 4.0F);
		a_defaults.starburstStrongLightSuppression = Clamp(
			ReadFloat(a_section, L"StarburstStrongLightSuppression", a_defaults.starburstStrongLightSuppression), 0.0F, 1.0F);
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
		success = WriteFloat(a_section, L"FirstPersonFocusX", a_settings.firstPersonFocusX) && success;
		success = WriteFloat(a_section, L"FirstPersonFocusY", a_settings.firstPersonFocusY) && success;
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
		success = WriteBool(a_section, L"ApertureFill", a_settings.apertureFill) && success;
		success = WriteUInt(a_section, L"ApertureShape", std::clamp(a_settings.apertureShape, 0U, 2U)) && success;
		success = WriteUInt(a_section, L"ApertureBlades", std::clamp(a_settings.apertureBlades, 3U, 12U)) && success;
		success = WriteFloat(a_section, L"ApertureRoundness", a_settings.apertureRoundness) && success;
		success = WriteFloat(a_section, L"ApertureShapeStrength", a_settings.apertureShapeStrength) && success;
		success = WriteFloat(a_section, L"ApertureRotationDegrees", a_settings.apertureRotationDegrees) && success;
		success = WriteBool(a_section, L"StarburstEnabled", a_settings.starburstEnabled) && success;
		success = WriteUInt(a_section, L"StarburstRays", std::clamp(a_settings.starburstRays, 4U, 12U)) && success;
		success = WriteFloat(a_section, L"StarburstStrength", a_settings.starburstStrength) && success;
		success = WriteFloat(a_section, L"StarburstLength", a_settings.starburstLength) && success;
		success = WriteFloat(a_section, L"HighlightExtractionThreshold", a_settings.highlightExtractionThreshold) && success;
		success = WriteFloat(a_section, L"StarburstRotationDegrees", a_settings.starburstRotationDegrees) && success;
		success = WriteFloat(a_section, L"BokehCoreFill", a_settings.bokehCoreFill) && success;
		success = WriteFloat(a_section, L"StarburstWideFlareSuppression", a_settings.starburstWideFlareSuppression) && success;
		success = WriteFloat(a_section, L"StarburstHighlightSensitivity", a_settings.starburstHighlightSensitivity) && success;
		success = WriteFloat(a_section, L"StarburstStrongLightSuppression", a_settings.starburstStrongLightSuppression) && success;
		return CommitIni(success);
	}

	bool SectionExists(const wchar_t* a_section)
	{
		wchar_t value[16]{};
		ReadValue(a_section, L"FocalLength", L"", value, std::size(value));
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
		a_defaults.useFirstPersonScreenAFFallback = ReadBool(
			a_section,
			L"UseFirstPersonScreenAFFallback",
			a_defaults.useFirstPersonScreenAFFallback);
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
		success = WriteBool(
			a_section,
			L"UseFirstPersonScreenAFFallback",
			a_settings.useFirstPersonScreenAFFallback) && success;
		return CommitIni(success);
	}
}

CDoF::Settings CDoF::LoadSettings()
{
	const IniAccess iniAccess;
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
	const IniAccess iniAccess;
	const auto success = SaveSection(kSection, a_settings);
	if (success) {
		spdlog::info("Settings saved to CinematicDoFStandalone.ini");
	}
	return success;
}

CDoF::Settings CDoF::LoadPresetSettings(const wchar_t* a_section, Settings a_defaults)
{
	const IniAccess iniAccess;
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
	const IniAccess iniAccess;
	const auto success = SaveSection(a_section, a_settings);
	if (success) {
		spdlog::info("Preset saved to INI section {}", std::filesystem::path(a_section).string());
	}
	return success;
}

bool CDoF::PresetSectionExists(const wchar_t* a_section)
{
	const IniAccess iniAccess;
	return SectionExists(a_section);
}

CDoF::TargetFocusSettings CDoF::LoadTargetFocusSettings()
{
	const IniAccess iniAccess;
	const auto result = LoadTargetFocusSection(kTargetFocusSection, TargetFocusSettings{});
	spdlog::info(
		"Target focus loaded: dialogue enabled={}, preset={}, strength={}; target enabled={}, source={}, offset={} m, first-person Screen AF fallback={}",
		result.dialogueEnabled,
		result.dialoguePreset,
		result.dialogueDepthStrength,
		result.consoleEnabled,
		static_cast<std::uint32_t>(result.targetSource),
		result.targetFocusOffsetMeters,
		result.useFirstPersonScreenAFFallback);
	return result;
}

bool CDoF::SaveTargetFocusSettings(const TargetFocusSettings& a_settings)
{
	const IniAccess iniAccess;
	const auto success = SaveTargetFocusSection(kTargetFocusSection, a_settings);
	if (success) {
		spdlog::info("Target focus settings saved");
	}
	return success;
}

CDoF::TDMFocusSettings CDoF::LoadTDMFocusSettings()
{
	const IniAccess iniAccess;
	TDMFocusSettings result{};
	result.enabled = ReadBool(kTDMFocusSection, L"Enabled", result.enabled);
	result.profile = std::min(ReadUInt(kTDMFocusSection, L"Profile", result.profile), 2U);
	result.blurStrength = Clamp(ReadFloat(kTDMFocusSection, L"BlurStrength", result.blurStrength), 0.0F, 3.0F);
	result.nearBlurStrength = Clamp(ReadFloat(kTDMFocusSection, L"NearBlurStrength", result.nearBlurStrength), 0.0F, 1.0F);
	return result;
}

bool CDoF::SaveTDMFocusSettings(const TDMFocusSettings& a_settings)
{
	const IniAccess iniAccess;
	bool success = true;
	success = WriteBool(kTDMFocusSection, L"Enabled", a_settings.enabled) && success;
	success = WriteUInt(kTDMFocusSection, L"Profile", std::min(a_settings.profile, 2U)) && success;
	success = WriteFloat(kTDMFocusSection, L"BlurStrength", std::clamp(a_settings.blurStrength, 0.0F, 3.0F)) && success;
	success = WriteFloat(kTDMFocusSection, L"NearBlurStrength", std::clamp(a_settings.nearBlurStrength, 0.0F, 1.0F)) && success;
	return CommitIni(success);
}

CDoF::SneakFocusSettings CDoF::LoadSneakFocusSettings()
{
	const IniAccess iniAccess;
	SneakFocusSettings result{};
	result.enabled = ReadBool(kSneakFocusSection, L"Enabled", result.enabled);
	result.profile = std::min(ReadUInt(kSneakFocusSection, L"Profile", result.profile), 1U);
	result.blurStrength = Clamp(ReadFloat(kSneakFocusSection, L"BlurStrength", result.blurStrength), 0.0F, 3.0F);
	return result;
}

bool CDoF::SaveSneakFocusSettings(const SneakFocusSettings& a_settings)
{
	const IniAccess iniAccess;
	bool success = true;
	success = WriteBool(kSneakFocusSection, L"Enabled", a_settings.enabled) && success;
	success = WriteUInt(kSneakFocusSection, L"Profile", std::min(a_settings.profile, 1U)) && success;
	success = WriteFloat(kSneakFocusSection, L"BlurStrength", std::clamp(a_settings.blurStrength, 0.0F, 3.0F)) && success;
	return CommitIni(success);
}

CDoF::ModeSettings CDoF::LoadModeSettings()
{
	const IniAccess iniAccess;
	ModeSettings result{};
	result.normalGameplayEnabled = ReadBool(
		kModeSection, L"NormalGameplayEnabled", result.normalGameplayEnabled);
	spdlog::info("Mode settings loaded: normal gameplay DoF={}", result.normalGameplayEnabled);
	return result;
}

bool CDoF::SaveModeSettings(const ModeSettings& a_settings)
{
	const IniAccess iniAccess;
	auto success = WriteBool(
		kModeSection, L"NormalGameplayEnabled", a_settings.normalGameplayEnabled);
	success = CommitIni(success);
	if (success) {
		spdlog::info("Mode settings saved");
	}
	return success;
}

CDoF::SilhouetteSettings CDoF::LoadSilhouetteSettings()
{
	const IniAccess iniAccess;
	SilhouetteSettings result{};
	result.enabled = ReadBool(kSilhouetteSection, L"Enabled", result.enabled);
	result.cocMonochrome = ReadBool(kSilhouetteSection, L"CoCMonochrome", false);
	result.smoothEdges = ReadBool(kSilhouetteSection, L"SmoothEdges", result.smoothEdges);
	result.foregroundColor = {
		Clamp(ReadFloat(kSilhouetteSection, L"ForegroundR", result.foregroundColor[0]), 0.0F, 1.0F),
		Clamp(ReadFloat(kSilhouetteSection, L"ForegroundG", result.foregroundColor[1]), 0.0F, 1.0F),
		Clamp(ReadFloat(kSilhouetteSection, L"ForegroundB", result.foregroundColor[2]), 0.0F, 1.0F)
	};
	result.backgroundColor = {
		Clamp(ReadFloat(kSilhouetteSection, L"BackgroundR", result.backgroundColor[0]), 0.0F, 1.0F),
		Clamp(ReadFloat(kSilhouetteSection, L"BackgroundG", result.backgroundColor[1]), 0.0F, 1.0F),
		Clamp(ReadFloat(kSilhouetteSection, L"BackgroundB", result.backgroundColor[2]), 0.0F, 1.0F)
	};
	spdlog::info(
		"Silhouette settings loaded: enabled={}, smoothEdges={}, foreground=({:.3f}, {:.3f}, {:.3f}), background=({:.3f}, {:.3f}, {:.3f})",
		result.enabled,
		result.smoothEdges,
		result.foregroundColor[0], result.foregroundColor[1], result.foregroundColor[2],
		result.backgroundColor[0], result.backgroundColor[1], result.backgroundColor[2]);
	return result;
}

bool CDoF::SaveSilhouetteSettings(const SilhouetteSettings& a_settings)
{
	const IniAccess iniAccess;
	bool success = true;
	success = WriteBool(kSilhouetteSection, L"Enabled", a_settings.enabled) && success;
	success = WriteBool(kSilhouetteSection, L"CoCMonochrome", a_settings.cocMonochrome) && success;
	success = WriteBool(kSilhouetteSection, L"SmoothEdges", a_settings.smoothEdges) && success;
	success = WriteFloat(kSilhouetteSection, L"ForegroundR", std::clamp(a_settings.foregroundColor[0], 0.0F, 1.0F)) && success;
	success = WriteFloat(kSilhouetteSection, L"ForegroundG", std::clamp(a_settings.foregroundColor[1], 0.0F, 1.0F)) && success;
	success = WriteFloat(kSilhouetteSection, L"ForegroundB", std::clamp(a_settings.foregroundColor[2], 0.0F, 1.0F)) && success;
	success = WriteFloat(kSilhouetteSection, L"BackgroundR", std::clamp(a_settings.backgroundColor[0], 0.0F, 1.0F)) && success;
	success = WriteFloat(kSilhouetteSection, L"BackgroundG", std::clamp(a_settings.backgroundColor[1], 0.0F, 1.0F)) && success;
	success = WriteFloat(kSilhouetteSection, L"BackgroundB", std::clamp(a_settings.backgroundColor[2], 0.0F, 1.0F)) && success;
	success = CommitIni(success);
	if (success) {
		spdlog::info("Silhouette settings saved");
	}
	return success;
}

CDoF::DepthFrameSettings CDoF::LoadDepthFrameSettings(const wchar_t* a_section, DepthFrameSettings a_defaults)
{
	const IniAccess iniAccess;
	DepthFrameSettings result = a_defaults;
	result.enabled = ReadBool(a_section, L"Enabled", result.enabled);
	result.style = std::min(ReadUInt(a_section, L"Style", result.style), 2U);
	result.subjectBreakout = ReadBool(a_section, L"SubjectBreakout", result.subjectBreakout);
    result.lockOnOnly = ReadBool(a_section, L"LockOnOnly", result.lockOnOnly);
    result.dialogueOnly = ReadBool(a_section, L"DialogueOnly", result.dialogueOnly);
    result.photoMode = ReadBool(a_section, L"PhotoMode", result.photoMode);
    result.killMoveOnly = ReadBool(a_section, L"KillMoveOnly", result.killMoveOnly);
    result.breakoutEdges = std::min(ReadUInt(a_section, L"BreakoutEdges", result.breakoutEdges), 15U);
    result.linkSubjectFocus = ReadBool(a_section, L"LinkSubjectFocus", result.linkSubjectFocus);
	result.topBottomSize = Clamp(ReadFloat(a_section, L"TopBottomSize", result.topBottomSize), 0.0F, 0.45F);
	result.sideSize = Clamp(ReadFloat(a_section, L"SideSize", result.sideSize), 0.0F, 0.45F);
	result.opacity = Clamp(ReadFloat(a_section, L"Opacity", result.opacity), 0.0F, 1.0F);
	result.subjectExpansion = Clamp(ReadFloat(a_section, L"SubjectExpansion", result.subjectExpansion), 0.0F, kSubjectExpansionMax);
	result.frameTop = Clamp(ReadFloat(a_section, L"Top", result.frameTop), 0.0F, 0.45F);
	result.frameBottom = Clamp(ReadFloat(a_section, L"Bottom", result.frameBottom), 0.0F, 0.45F);
	result.frameLeft = Clamp(ReadFloat(a_section, L"Left", result.frameLeft), 0.0F, 0.45F);
	result.frameRight = Clamp(ReadFloat(a_section, L"Right", result.frameRight), 0.0F, 0.45F);
	result.subjectInsetPixels = Clamp(ReadFloat(a_section, L"SubjectInsetPixels", result.subjectInsetPixels), 0.0F, 4.0F);
	result.frameRotationDegrees = Clamp(ReadFloat(a_section, L"FrameRotationDegrees", result.frameRotationDegrees), -180.0F, 180.0F);
	result.edgeSoftness = Clamp(ReadFloat(a_section, L"EdgeSoftness", result.edgeSoftness), 0.0F, 0.03F);
	result.color = {
		Clamp(ReadFloat(a_section, L"ColorR", result.color[0]), 0.0F, 1.0F),
		Clamp(ReadFloat(a_section, L"ColorG", result.color[1]), 0.0F, 1.0F),
		Clamp(ReadFloat(a_section, L"ColorB", result.color[2]), 0.0F, 1.0F)
	};
	return result;
}

bool CDoF::SaveDepthFrameSettings(const DepthFrameSettings& a_settings, const wchar_t* a_section)
{
	const IniAccess iniAccess;
	bool success = true;
	success = WriteBool(a_section, L"Enabled", a_settings.enabled) && success;
	success = WriteUInt(a_section, L"Style", std::min(a_settings.style, 2U)) && success;
	success = WriteBool(a_section, L"SubjectBreakout", a_settings.subjectBreakout) && success;
    success = WriteBool(a_section, L"LockOnOnly", a_settings.lockOnOnly) && success;
    success = WriteBool(a_section, L"DialogueOnly", a_settings.dialogueOnly) && success;
    success = WriteBool(a_section, L"PhotoMode", a_settings.photoMode) && success;
    success = WriteBool(a_section, L"KillMoveOnly", a_settings.killMoveOnly) && success;
    success = WriteUInt(a_section, L"BreakoutEdges", std::min(a_settings.breakoutEdges, 15U)) && success;
    success = WriteBool(a_section, L"LinkSubjectFocus", a_settings.linkSubjectFocus) && success;
	success = WriteFloat(a_section, L"TopBottomSize", std::clamp(a_settings.topBottomSize, 0.0F, 0.45F)) && success;
	success = WriteFloat(a_section, L"SideSize", std::clamp(a_settings.sideSize, 0.0F, 0.45F)) && success;
	success = WriteFloat(a_section, L"Opacity", std::clamp(a_settings.opacity, 0.0F, 1.0F)) && success;
	success = WriteFloat(a_section, L"SubjectExpansion", std::clamp(a_settings.subjectExpansion, 0.0F, kSubjectExpansionMax)) && success;
	success = WriteFloat(a_section, L"Top", std::clamp(a_settings.frameTop, 0.0F, 0.45F)) && success;
	success = WriteFloat(a_section, L"Bottom", std::clamp(a_settings.frameBottom, 0.0F, 0.45F)) && success;
	success = WriteFloat(a_section, L"Left", std::clamp(a_settings.frameLeft, 0.0F, 0.45F)) && success;
	success = WriteFloat(a_section, L"Right", std::clamp(a_settings.frameRight, 0.0F, 0.45F)) && success;
	success = WriteFloat(a_section, L"SubjectInsetPixels", std::clamp(a_settings.subjectInsetPixels, 0.0F, 4.0F)) && success;
	success = WriteFloat(a_section, L"FrameRotationDegrees", std::clamp(a_settings.frameRotationDegrees, -180.0F, 180.0F)) && success;
	success = WriteFloat(a_section, L"EdgeSoftness", std::clamp(a_settings.edgeSoftness, 0.0F, 0.03F)) && success;
	success = WriteFloat(a_section, L"ColorR", std::clamp(a_settings.color[0], 0.0F, 1.0F)) && success;
	success = WriteFloat(a_section, L"ColorG", std::clamp(a_settings.color[1], 0.0F, 1.0F)) && success;
	success = WriteFloat(a_section, L"ColorB", std::clamp(a_settings.color[2], 0.0F, 1.0F)) && success;
	return CommitIni(success);
}

CDoF::ExperimentalTransparencySettings CDoF::LoadExperimentalTransparencySettings()
{
	const IniAccess iniAccess;
	ExperimentalTransparencySettings result{};
	result.enabled = ReadBool(kExperimentalTransparencySection, L"Enabled", result.enabled);
	result.mode = std::clamp(ReadUInt(kExperimentalTransparencySection, L"Mode", result.mode), 1U, 2U);
	spdlog::info("Experimental transparency path loaded: enabled={}, mode={}", result.enabled, result.mode);
	return result;
}

bool CDoF::SaveExperimentalTransparencySettings(const ExperimentalTransparencySettings& a_settings)
{
	const IniAccess iniAccess;
	auto success = WriteBool(kExperimentalTransparencySection, L"Enabled", a_settings.enabled);
	success = WriteUInt(kExperimentalTransparencySection, L"Mode", std::clamp(a_settings.mode, 1U, 2U)) && success;
	success = CommitIni(success);
	if (success) {
		spdlog::info("Experimental transparency path saved: enabled={}, mode={}", a_settings.enabled, a_settings.mode);
	}
	return success;
}

CDoF::TargetFocusSettings CDoF::LoadPresetTargetFocusSettings(
	const wchar_t* a_section,
	TargetFocusSettings a_defaults)
{
	const IniAccess iniAccess;
	return SectionExists(a_section) ? LoadTargetFocusSection(a_section, a_defaults) : a_defaults;
}

bool CDoF::SavePresetTargetFocusSettings(
	const wchar_t* a_section,
	const TargetFocusSettings& a_settings)
{
	const IniAccess iniAccess;
	const auto success = SaveTargetFocusSection(a_section, a_settings);
	if (success) {
		spdlog::info("Preset target focus saved to INI section {}", std::filesystem::path(a_section).string());
	}
	return success;
}

CDoF::InterfaceSettings CDoF::LoadInterfaceSettings()
{
	const IniAccess iniAccess;
	InterfaceSettings result{};
	result.japanese = ReadBool(kInterfaceSection, L"Japanese", result.japanese);
	return result;
}

bool CDoF::SaveInterfaceSettings(const InterfaceSettings& a_settings)
{
	const IniAccess iniAccess;
	auto success = WriteBool(kInterfaceSection, L"Japanese", a_settings.japanese);
	return CommitIni(success);
}

CDoF::HotkeySettings CDoF::LoadHotkeySettings()
{
	const IniAccess iniAccess;
	HotkeySettings result{};
	result.toggleDoFKey = std::min(ReadUInt(kHotkeySection, L"ToggleDoF", result.toggleDoFKey), 0xFFu);
	result.toggleSilhouetteKey = std::min(
		ReadUInt(kHotkeySection, L"ToggleSilhouette", result.toggleSilhouetteKey),
		0xFFu);
	return result;
}

bool CDoF::SaveHotkeySettings(const HotkeySettings& a_settings)
{
	const IniAccess iniAccess;
	bool success = true;
	success = WriteUInt(kHotkeySection, L"ToggleDoF", std::min(a_settings.toggleDoFKey, 0xFFu)) && success;
	success = WriteUInt(
		kHotkeySection,
		L"ToggleSilhouette",
		std::min(a_settings.toggleSilhouetteKey, 0xFFu)) && success;
	return CommitIni(success);
}
