#pragma once

// Read-only subset of True Directional Movement's public modder API.
// Upstream explicitly permits mod authors to copy this header into their
// projects. Only the V1 calls needed for target-lock focus are retained.
// https://github.com/ersh1/TrueDirectionalMovement/blob/master/src/TrueDirectionalMovementAPI.h

namespace TDM_API
{
	using PluginHandle = SKSE::PluginHandle;
	using ActorHandle = RE::ActorHandle;

	enum class InterfaceVersion : std::uint8_t
	{
		V1,
		V2,
		V3,
		V4,
		V5
	};

	enum class APIResult : std::uint8_t
	{
		OK,
		NotOwner,
		MustKeep,
		AlreadyGiven,
		AlreadyTaken,
		BadThread
	};

	class IVTDM1
	{
	public:
		[[nodiscard]] virtual unsigned long GetTDMThreadId() const noexcept = 0;
		[[nodiscard]] virtual bool GetDirectionalMovementState() const noexcept = 0;
		[[nodiscard]] virtual bool GetTargetLockState() const noexcept = 0;
		[[nodiscard]] virtual ActorHandle GetCurrentTarget() const noexcept = 0;
		[[nodiscard]] virtual APIResult RequestDisableDirectionalMovement(PluginHandle a_pluginHandle) noexcept = 0;
		[[nodiscard]] virtual APIResult RequestDisableHeadtracking(PluginHandle a_pluginHandle) noexcept = 0;
		[[nodiscard]] virtual PluginHandle GetDisableDirectionalMovementOwner() const noexcept = 0;
		[[nodiscard]] virtual PluginHandle GetDisableHeadtrackingOwner() const noexcept = 0;
		virtual APIResult ReleaseDisableDirectionalMovement(PluginHandle a_pluginHandle) noexcept = 0;
		virtual APIResult ReleaseDisableHeadtracking(PluginHandle a_pluginHandle) noexcept = 0;
	};

	using RequestPluginAPIFunction = void* (*)(InterfaceVersion);

	[[nodiscard]] inline IVTDM1* RequestV1() noexcept
	{
		const auto module = GetModuleHandleA("TrueDirectionalMovement.dll");
		if (!module) {
			return nullptr;
		}
		const auto request = reinterpret_cast<RequestPluginAPIFunction>(GetProcAddress(module, "RequestPluginAPI"));
		return request ? static_cast<IVTDM1*>(request(InterfaceVersion::V1)) : nullptr;
	}
}
