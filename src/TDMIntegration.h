#pragma once

namespace CDoF::TDMIntegration
{
	void Initialize();
	[[nodiscard]] bool IsAvailable();
	[[nodiscard]] bool IsTargetLocked();
	[[nodiscard]] RE::NiPointer<RE::TESObjectREFR> GetLockedTarget();
}
