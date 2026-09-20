#pragma once

namespace CDoF
{
	inline constexpr std::uint32_t kPresetSlotCount{ 9 };
	inline constexpr std::uint32_t kMaxPresetIndex{ kPresetSlotCount - 1 };

	enum class TargetFocusSource : std::uint32_t
	{
		kPlayer = 0,
		kConsole = 1
	};

	struct Settings
	{
		bool enabled{ true };
		bool autoFocus{ false };
		bool disableInMenus{ true };
		float transitionSpeed{ 0.9F };
		float focusX{ 0.5F };
		float focusY{ 0.5F };
		float autoFocusOffsetMeters{ 0.0F };
		float manualFocusMeters{ 46.09F };
		float focalLength{ 20.6F };
		float fNumber{ 6.4F };
		float nearFocusRangeMeters{ 0.0F };
		float farFocusRangeMeters{ 0.0F };
		float farPlaneMaxBlur{ 1.91F };
		float nearPlaneMaxBlur{ 0.0F };
		bool enableFirstPersonNearBlur{ false };
		bool keepSkySharp{ false };
		float blurQuality{ 5.0F };
		float nearFarDistanceCompensation{ 3.98F };
		float bokehBusyFactor{ 0.25F };
		float highlightBoost{ 0.0F };
		float postBlurSmoothing{ 0.0F };
		float petzvalStrength{ 0.0F };
		bool apertureBokeh{ false };
		std::uint32_t apertureBlades{ 6 };
		float apertureRoundness{ 0.35F };
		float apertureShapeStrength{ 1.0F };
		float apertureRotationDegrees{ 0.0F };
	};

	struct TargetFocusSettings
	{
		bool dialogueEnabled{ true };
		std::uint32_t dialoguePreset{ 0 };
		float dialogueDepthStrength{ 2.5F };
		bool consoleEnabled{ false };
		TargetFocusSource targetSource{ TargetFocusSource::kPlayer };
		float targetFocusOffsetMeters{ 0.0F };
	};

	struct ModeSettings
	{
		// Keep this outside Settings so loading or storing a lens preset never
		// changes whether DoF is active during normal gameplay.
		bool normalGameplayEnabled{ true };
	};

	struct InterfaceSettings
	{
		bool japanese{ false };
	};

	struct HotkeySettings
	{
		// DirectInput keyboard scan code. 0 means unassigned.
		std::uint32_t toggleDoFKey{};
	};

	Settings LoadSettings();
	bool SaveSettings(const Settings& a_settings);
	Settings LoadPresetSettings(const wchar_t* a_section, Settings a_defaults);
	bool SavePresetSettings(const wchar_t* a_section, const Settings& a_settings);
	bool PresetSectionExists(const wchar_t* a_section);
	TargetFocusSettings LoadTargetFocusSettings();
	bool SaveTargetFocusSettings(const TargetFocusSettings& a_settings);
	ModeSettings LoadModeSettings();
	bool SaveModeSettings(const ModeSettings& a_settings);
	TargetFocusSettings LoadPresetTargetFocusSettings(
		const wchar_t* a_section,
		TargetFocusSettings a_defaults);
	bool SavePresetTargetFocusSettings(const wchar_t* a_section, const TargetFocusSettings& a_settings);
	InterfaceSettings LoadInterfaceSettings();
	bool SaveInterfaceSettings(const InterfaceSettings& a_settings);
	HotkeySettings LoadHotkeySettings();
	bool SaveHotkeySettings(const HotkeySettings& a_settings);
}
