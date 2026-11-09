#include "TDMIntegration.h"

#include "TrueDirectionalMovementAPI.h"

namespace
{
	TDM_API::IVTDM1* tdmAPI{};
}

void CDoF::TDMIntegration::Initialize()
{
	tdmAPI = TDM_API::RequestV1();
	if (tdmAPI) {
		spdlog::info("True Directional Movement API V1 detected; lock-on focus is available");
	} else {
		spdlog::info("True Directional Movement API not detected; lock-on focus remains inactive");
	}
}

bool CDoF::TDMIntegration::IsAvailable()
{
	return tdmAPI != nullptr;
}

bool CDoF::TDMIntegration::IsTargetLocked()
{
    return tdmAPI && tdmAPI->GetTargetLockState();
}

RE::NiPointer<RE::TESObjectREFR> CDoF::TDMIntegration::GetLockedTarget()
{
	if (!IsTargetLocked()) {
		return nullptr;
	}
	const auto actor = tdmAPI->GetCurrentTarget().get();
	return actor ? RE::NiPointer<RE::TESObjectREFR>{ actor.get() } : nullptr;
}
