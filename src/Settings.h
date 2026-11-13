#pragma once

namespace CDoF
{
	inline constexpr std::uint32_t kPresetSlotCount{ 10 };
	inline constexpr std::uint32_t kMaxPresetIndex{ kPresetSlotCount - 1 };
	inline constexpr float kManualFocusMinMeters{ 0.1F };
	inline constexpr float kManualFocusMaxMeters{ 2000.0F };

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
		// Existing FocusX/FocusY remain the third-person coordinates for INI and
		// preset compatibility. First person has its own center-screen defaults.
		float focusX{ 0.5F };
		float focusY{ 0.5F };
		float firstPersonFocusX{ 0.5F };
		float firstPersonFocusY{ 0.5F };
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
		bool apertureFill{ false };
		// 0 = regular aperture, 1 = star, 2 = heart.
		std::uint32_t apertureShape{ 0 };
		std::uint32_t apertureBlades{ 6 };
		float apertureRoundness{ 0.35F };
		float apertureShapeStrength{ 1.0F };
		float apertureRotationDegrees{ 0.0F };
		bool starburstEnabled{ false };
		std::uint32_t starburstRays{ 6 };
		float starburstStrength{ 1.00F };
		float starburstLength{ 0.65F };
		float highlightExtractionThreshold{ 0.65F };
		float starburstRotationDegrees{ 0.0F };
		float bokehCoreFill{ 0.50F };
		float starburstWideFlareSuppression{ 0.0F };
		float starburstStrongLightSuppression{ 0.0F };
		float starburstHighlightSensitivity{ 1.0F };
	};

	struct TargetFocusSettings
	{
		bool dialogueEnabled{ true };
		std::uint32_t dialoguePreset{ 0 };
		float dialogueDepthStrength{ 2.5F };
		bool consoleEnabled{ false };
		TargetFocusSource targetSource{ TargetFocusSource::kPlayer };
		float targetFocusOffsetMeters{ 0.0F };
		bool useFirstPersonScreenAFFallback{ false };
	};

	struct TDMFocusSettings
	{
		bool enabled{ false };
		std::uint32_t profile{};
		float blurStrength{ 1.0F };
		float nearBlurStrength{ 0.0F };
	};

	struct SneakFocusSettings
	{
		bool enabled{ false };
		std::uint32_t profile{};
		float blurStrength{ 1.0F };
	};

	struct ModeSettings
	{
		// Keep this outside Settings so loading or storing a lens preset never
		// changes whether DoF is active during normal gameplay.
		bool normalGameplayEnabled{ true };
	};

	struct SilhouetteSettings
	{
		bool cocMonochrome{ false };
		bool enabled{ false };
		bool smoothEdges{ true };
		std::array<float, 3> foregroundColor{ 0.0F, 0.0F, 0.0F };
		std::array<float, 3> backgroundColor{ 1.0F, 1.0F, 1.0F };
	};

	inline constexpr float kSubjectExpansionMax = 0.30F;

	struct DepthFrameSettings
	{
		bool enabled{ false };
		// 0 = horizontal letterbox, 1 = four-sided frame.
		std::uint32_t style{ 0U };
		bool subjectBreakout{ false };
		bool lockOnOnly{ false };
		bool killMoveOnly{ false };
		bool dialogueOnly{ false };
		bool photoMode{ false };
		std::uint32_t breakoutEdges{ 15U }; // Top, bottom, left, right.
        bool linkSubjectFocus{ false };
		float topBottomSize{ 0.13F };
		float sideSize{ 0.08F };
		float opacity{ 1.0F };
		float subjectExpansion{ 0.025F };
		float frameTop{ 0.13F };
		float frameBottom{ 0.13F };
		float frameLeft{ 0.13F };
		float frameRight{ 0.13F };
		float subjectInsetPixels{ 0.0F };
		float frameRotationDegrees{ 0.0F };
		float edgeSoftness{ 0.002F };
		std::array<float, 3> color{ 0.0F, 0.0F, 0.0F };
	};

	struct ExperimentalTransparencySettings
	{
		// Public hair-blending option. The legacy type and INI section names remain
		// unchanged for compatibility. Disabled uses the standard rendering path;
		// mode 1 is the soft blend and mode 2 preserves more transparency detail.
		bool enabled{ false };
		std::uint32_t mode{ 1U };
	};

	struct InterfaceSettings
	{
		bool japanese{ false };
	};

	struct HotkeySettings
	{
		// DirectInput keyboard scan code. 0 means unassigned.
		std::uint32_t toggleDoFKey{};
		std::uint32_t toggleSilhouetteKey{};
	};

	Settings LoadSettings();
	bool SaveSettings(const Settings& a_settings);
	Settings LoadPresetSettings(const wchar_t* a_section, Settings a_defaults);
	bool SavePresetSettings(const wchar_t* a_section, const Settings& a_settings);
	bool PresetSectionExists(const wchar_t* a_section);
	TargetFocusSettings LoadTargetFocusSettings();
	bool SaveTargetFocusSettings(const TargetFocusSettings& a_settings);
	TDMFocusSettings LoadTDMFocusSettings();
	bool SaveTDMFocusSettings(const TDMFocusSettings& a_settings);
	SneakFocusSettings LoadSneakFocusSettings();
	bool SaveSneakFocusSettings(const SneakFocusSettings& a_settings);
	ModeSettings LoadModeSettings();
	bool SaveModeSettings(const ModeSettings& a_settings);
	SilhouetteSettings LoadSilhouetteSettings();
	bool SaveSilhouetteSettings(const SilhouetteSettings& a_settings);
	DepthFrameSettings LoadDepthFrameSettings(const wchar_t* a_section = L"DepthFrame", DepthFrameSettings a_defaults = {});
	bool SaveDepthFrameSettings(const DepthFrameSettings& a_settings, const wchar_t* a_section = L"DepthFrame");
	ExperimentalTransparencySettings LoadExperimentalTransparencySettings();
	bool SaveExperimentalTransparencySettings(const ExperimentalTransparencySettings& a_settings);
	TargetFocusSettings LoadPresetTargetFocusSettings(
		const wchar_t* a_section,
		TargetFocusSettings a_defaults);
	bool SavePresetTargetFocusSettings(const wchar_t* a_section, const TargetFocusSettings& a_settings);
	InterfaceSettings LoadInterfaceSettings();
	bool SaveInterfaceSettings(const InterfaceSettings& a_settings);
	HotkeySettings LoadHotkeySettings();
	bool SaveHotkeySettings(const HotkeySettings& a_settings);
}
