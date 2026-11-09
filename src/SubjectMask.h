#pragma once
namespace CDoF::SubjectMask
{
    void ResetLockOnState();
    void UpdateLockOnState();
    [[nodiscard]] bool IsTargetLocked();
    void SetEnabled(bool enabled, bool lockOnOnly);
    RE::NiPointer<RE::TESObjectREFR> GetTarget();
    bool SetConsoleTarget();
    bool SetTarget(RE::TESObjectREFR* target);
    void UsePlayer();
    bool BeginReplay(ID3D11DeviceContext* context, const char* geometry, std::uint32_t pass, bool lighting);
    void EndReplay(ID3D11DeviceContext* context);
    void CompleteFrame();
    void EndFrame();
    bool WantsCapture();
    ID3D11ShaderResourceView* GetSRV(std::uint32_t index = 0);
    std::uint32_t GetLayerCount();
    std::array<float, 4> GetViewport();
}
