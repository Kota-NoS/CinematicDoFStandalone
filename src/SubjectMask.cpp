#include "SubjectMask.h"
#include "TDMIntegration.h"

namespace
{
    std::mutex targetMutex;
    RE::ObjectRefHandle selectedTarget;
    bool consoleTarget{};
    std::atomic_bool enabled{false}, resetRequested{false};
    std::atomic_bool lockOnOnly{false};
    std::atomic_bool targetLocked{false};

    bool CaptureAllowed()
    {
        if (!enabled.load(std::memory_order_relaxed)) return false;
        if (!lockOnOnly.load(std::memory_order_relaxed)) return true;
        return CDoF::SubjectMask::IsTargetLocked();
    }

    bool ready{}, replaying{}, failed{}, nextFrame{};
    std::uint32_t completedFrames{};
    std::uint32_t captured{}, waitingFrames{};
    struct MaskLayer {
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11DepthStencilView> dsv;
        ComPtr<ID3D11ShaderResourceView> srv;
        ComPtr<ID3D11Resource> source;
        std::uint32_t draws{};
    };
    std::array<MaskLayer, 4> layers;
    std::uint32_t layerCount{}, activeLayer{}, maskWidth{}, maskHeight{};
    std::array<float, 4> viewport{};
    std::array<ComPtr<ID3D11RenderTargetView>, 8> savedRTVs;
    ComPtr<ID3D11DepthStencilView> savedDSV;
    ComPtr<ID3D11DepthStencilState> savedState;
    UINT savedReference{};
    struct CachedState { ComPtr<ID3D11DepthStencilState> source, replay; };
    std::vector<CachedState> states;

    enum class Decision : std::size_t { accepted, contextType, uav, streamOutput, noDSV, noRenderer,
        nonMainDepth, nonTexture, shape, format, viewportInvalid, sourceChanged, viewportChanged, resourceFailure, stateFailure, count };
    std::array<std::uint64_t, static_cast<std::size_t>(Decision::count)> decisions{};
    constexpr std::array<const char*, static_cast<std::size_t>(Decision::count)> decisionNames{
        "accepted", "deferred-context", "pixel-uav-bound", "stream-output-bound", "no-dsv", "no-renderer",
        "not-current-main-depth", "not-texture2d", "unsupported-view-or-samples", "unsupported-stencil-format",
        "invalid-viewport", "source-changed", "viewport-changed", "resource-creation-failed", "state-creation-failed" };
    std::uint32_t decisionLines{};
    void RecordDecision(Decision decision, ID3D11DeviceContext* context, const char* geometry, std::uint32_t pass, bool lighting)
    {
        ++decisions[static_cast<std::size_t>(decision)];
        if (decisionLines++ >= 96) return;
        ComPtr<ID3D11DepthStencilView> dsv;
        context->OMGetRenderTargets(0, nullptr, dsv.GetAddressOf());
        D3D11_DEPTH_STENCIL_VIEW_DESC view{}; D3D11_TEXTURE2D_DESC texture{};
        if (dsv) {
            dsv->GetDesc(&view);
            ComPtr<ID3D11Resource> resource; dsv->GetResource(resource.GetAddressOf());
            ComPtr<ID3D11Texture2D> tex;
            if (SUCCEEDED(resource.As(&tex))) tex->GetDesc(&texture);
        }
        UINT viewportCount = 1; D3D11_VIEWPORT vp{};
        context->RSGetViewports(&viewportCount, &vp);
        ComPtr<ID3D11DepthStencilState> ds;
        UINT reference{}; context->OMGetDepthStencilState(ds.GetAddressOf(), &reference);
        D3D11_DEPTH_STENCIL_DESC depth{};
        if (ds) ds->GetDesc(&depth);
        else { depth.DepthEnable = TRUE; depth.DepthFunc = D3D11_COMPARISON_LESS; depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; }
        ComPtr<ID3D11PixelShader> ps; context->PSGetShader(ps.GetAddressOf(), nullptr, nullptr);
        spdlog::info("MaskAudit26 draw {}: decision={} shader={} geometry={} pass=0x{:X} DSVformat={} texFormat={} allocation={}x{} samples={} vp=({},{}) {}x{} depthEnabled={} depthFunc={} depthWrite={} stencil={} PS={}",
            decisionLines, decisionNames[static_cast<std::size_t>(decision)], lighting ? "lighting" : "effect",
            geometry ? geometry : "(unnamed)", pass, static_cast<unsigned>(view.Format), static_cast<unsigned>(texture.Format),
            texture.Width, texture.Height, texture.SampleDesc.Count, vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height,
            depth.DepthEnable != FALSE, static_cast<unsigned>(depth.DepthFunc), static_cast<unsigned>(depth.DepthWriteMask),
            depth.StencilEnable != FALSE, ps != nullptr);
    }
    void ReportDecisions()
    {
        for (std::size_t i=0; i<decisions.size(); ++i)
            spdlog::info("MaskAudit26 totals: {}={}", decisionNames[i], decisions[i]);
        spdlog::info("MaskAudit26 note: accepted means replay submitted, not proof of stencil pixel coverage; names are diagnostic labels only");
    }
    void ProcessReset()
    {
        if (!resetRequested.exchange(false)) return;
        ready = replaying = failed = nextFrame = false; completedFrames = 0;
        captured = waitingFrames = 0;
        decisions.fill(0); decisionLines = 0;
        for (auto& layer : layers) layer = {};
        layerCount = activeLayer = maskWidth = maskHeight = 0; viewport = {};
        states.clear();
        spdlog::info("SubjectMask24 armed: capturing live player frames into private stencil; original pixel shader retained");
    }

    void RestoreOM(ID3D11DeviceContext* context)
    {
        std::array<ID3D11RenderTargetView*, 8> rtvs{};
        for (std::size_t i=0; i<rtvs.size(); ++i) rtvs[i] = savedRTVs[i].Get();
        context->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(), savedDSV.Get());
        context->OMSetDepthStencilState(savedState.Get(), savedReference);
        for (auto& rtv : savedRTVs) rtv.Reset();
        savedDSV.Reset(); savedState.Reset();
    }
}

RE::NiPointer<RE::TESObjectREFR> CDoF::SubjectMask::GetTarget()
{
    std::scoped_lock lock(targetMutex);
    if (consoleTarget) return selectedTarget.get();
    return RE::NiPointer<RE::TESObjectREFR>{ RE::PlayerCharacter::GetSingleton() };
}

bool CDoF::SubjectMask::SetConsoleTarget()
{
    const auto target = RE::Console::GetSelectedRef();
    return SetTarget(target.get());
}

bool CDoF::SubjectMask::SetTarget(RE::TESObjectREFR* target)
{
    if (!target || target->IsDeleted() || target->IsDisabled() ||
        !target->Get3D(false)) return false;
    const auto handle = target->GetHandle();
    if (!handle) return false;
    std::scoped_lock lock(targetMutex);
    selectedTarget = handle;
    consoleTarget = true;
    resetRequested.store(true);
    return true;
}

void CDoF::SubjectMask::UsePlayer()
{
    std::scoped_lock lock(targetMutex);
    selectedTarget.reset();
    consoleTarget = false;
    resetRequested.store(true);
}

void CDoF::SubjectMask::ResetLockOnState()
{
    targetLocked.store(false, std::memory_order_relaxed);
}

void CDoF::SubjectMask::UpdateLockOnState()
{
    // Frame visibility needs the current lock state even without subject breakout.
    targetLocked.store(TDMIntegration::IsTargetLocked(), std::memory_order_relaxed);
}

bool CDoF::SubjectMask::IsTargetLocked()
{
    return targetLocked.load(std::memory_order_relaxed);
}

void CDoF::SubjectMask::SetEnabled(bool value, bool onlyWhileLockedOn)
{
    lockOnOnly.store(onlyWhileLockedOn, std::memory_order_relaxed);
    const bool old = enabled.exchange(value, std::memory_order_relaxed);
    if (value && !old) resetRequested.store(true);
}

bool CDoF::SubjectMask::BeginReplay(ID3D11DeviceContext* context, const char* geometry, std::uint32_t pass, bool lighting)
{
    if (replaying || !context || !CaptureAllowed()) return false;
    ProcessReset();
    if (ready || failed) return false;
    // Effect passes can cover transparent areas outside the subject silhouette.
    if (!lighting) return false;
    const auto skip = [&](Decision reason) { RecordDecision(reason, context, geometry, pass, lighting); return false; };
    if (context->GetType() != D3D11_DEVICE_CONTEXT_IMMEDIATE) return skip(Decision::contextType);
    if (nextFrame) {
        for (auto& layer : layers) {
            if (layer.dsv) context->ClearDepthStencilView(layer.dsv.Get(), D3D11_CLEAR_STENCIL, 1.0f, 0);
            layer.draws = 0;
        }
        nextFrame = false;
    }
    // Replaying shaders with output UAVs or stream-output could repeat unrelated writes.
    std::array<ID3D11UnorderedAccessView*, 8> uavs{};
    context->OMGetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, 8, uavs.data());
    bool sideEffects = false, hasUAV = false;
    for (auto* uav : uavs) { if (uav) { sideEffects = true; hasUAV = true; uav->Release(); } }
    std::array<ID3D11Buffer*, 4> so{};
    context->SOGetTargets(4, so.data());
    for (auto* buffer : so) { if (buffer) { sideEffects = true; buffer->Release(); } }
    if (sideEffects) return skip(hasUAV ? Decision::uav : Decision::streamOutput);

    ComPtr<ID3D11DepthStencilView> liveDSV;
    context->OMGetRenderTargets(0, nullptr, liveDSV.GetAddressOf());
    if (!liveDSV) return skip(Decision::noDSV);
    ComPtr<ID3D11Resource> source;
    liveDSV->GetResource(source.GetAddressOf());
    const auto renderer = RE::BSGraphics::Renderer::GetSingleton();
    if (!renderer) return skip(Decision::noRenderer);
    auto* sceneDepth = renderer->GetDepthStencilData().depthStencils[RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN].depthSRV;
    ComPtr<ID3D11Resource> mainResource;
    if (sceneDepth) sceneDepth->GetResource(mainResource.GetAddressOf());
    // Exclude shadows, reflections and other non-main camera passes.
    // Test27: recognize actual main-camera depth storage, not only its current SRV.
    const auto& depthTable = renderer->GetDepthStencilData().depthStencils;
    const char* sourceKind = nullptr;
    if (mainResource && source.Get() == mainResource.Get()) sourceKind = "main-current-srv";
    for (auto slot : {RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN, RE::RENDER_TARGETS_DEPTHSTENCIL::kPOST_ZPREPASS_COPY}) {
        const auto& entry = depthTable[slot];
        const bool primary = slot == RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN;
        if (source.Get() == static_cast<ID3D11Resource*>(entry.texture)) sourceKind = primary ? "main-texture" : "post-z-texture";
        for (std::size_t i=0; i<8 && !sourceKind; ++i) {
            for (auto* candidate : {entry.views[i], entry.readOnlyViews[i]}) {
                ComPtr<ID3D11Resource> known;
                if (candidate) candidate->GetResource(known.GetAddressOf());
                if (known && source.Get() == known.Get()) { sourceKind = primary ? "main-view" : "post-z-view"; break; }
            }
        }
    }
    if (!sourceKind) return skip(Decision::nonMainDepth);
    ComPtr<ID3D11Texture2D> texture;
    if (FAILED(source.As(&texture))) return skip(Decision::nonTexture);
    D3D11_TEXTURE2D_DESC desc{}; texture->GetDesc(&desc);
    D3D11_DEPTH_STENCIL_VIEW_DESC view{}; liveDSV->GetDesc(&view);
    if (desc.SampleDesc.Count != 1 || desc.ArraySize != 1 || desc.MipLevels != 1 || view.ViewDimension != D3D11_DSV_DIMENSION_TEXTURE2D) return skip(Decision::shape);
    DXGI_FORMAT stencilFormat = DXGI_FORMAT_UNKNOWN;
    if (desc.Format == DXGI_FORMAT_R24G8_TYPELESS) stencilFormat = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
    if (desc.Format == DXGI_FORMAT_R32G8X24_TYPELESS) stencilFormat = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
    if (stencilFormat == DXGI_FORMAT_UNKNOWN) {
        failed = true; spdlog::warn("SubjectMask24 unsupported depth format {}; private stencil capture skipped", static_cast<unsigned>(desc.Format)); return skip(Decision::format);
    }
    UINT count = 1; D3D11_VIEWPORT currentViewport{};
    context->RSGetViewports(&count, &currentViewport);
    if (count != 1 || currentViewport.Width <= 0 || currentViewport.Height <= 0) return skip(Decision::viewportInvalid);

    const std::array<float,4> drawViewport{currentViewport.TopLeftX,currentViewport.TopLeftY,currentViewport.Width,currentViewport.Height};
    if (captured == 0) {
        if (layerCount && (desc.Width != maskWidth || desc.Height != maskHeight)) {
            for (auto& layer : layers) layer = {};
            layerCount = 0;
        }
        if (viewport != drawViewport) {
            spdlog::debug("SubjectMask36 viewport: ({},{}) {}x{}", drawViewport[0], drawViewport[1], drawViewport[2], drawViewport[3]);
        }
        viewport = drawViewport;
    } else {
        if (desc.Width != maskWidth || desc.Height != maskHeight) return skip(Decision::sourceChanged);
        if (viewport != drawViewport) return skip(Decision::viewportChanged);
    }
    std::uint32_t index = 0;
    for (; index < layerCount; ++index) if (layers[index].source.Get() == source.Get()) break;
    if (index == layers.size()) return skip(Decision::sourceChanged);
    activeLayer = index;
    auto& layer = layers[index];
    auto& privateTexture = layer.texture;
    auto& privateDSV = layer.dsv;
    auto& privateSRV = layer.srv;
    ComPtr<ID3D11Device> device; context->GetDevice(device.GetAddressOf());
    if (!privateTexture) {
        auto copy = desc; copy.Usage = D3D11_USAGE_DEFAULT;
        copy.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        copy.CPUAccessFlags = 0; copy.MiscFlags = 0;
        view.Flags = 0;
        D3D11_SHADER_RESOURCE_VIEW_DESC srv{};
        srv.Format = stencilFormat; srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; srv.Texture2D.MipLevels = 1;
        if (FAILED(device->CreateTexture2D(&copy, nullptr, privateTexture.GetAddressOf())) ||
            FAILED(device->CreateDepthStencilView(privateTexture.Get(), &view, privateDSV.GetAddressOf())) ||
            FAILED(device->CreateShaderResourceView(privateTexture.Get(), &srv, privateSRV.GetAddressOf()))) {
            failed = true; spdlog::error("SubjectMask24 private resource creation failed"); return skip(Decision::resourceFailure);
        }
        // Keep the depth as it existed at the first captured draw. Only private stencil is cleared.
        // Initial copy is performed below, once per layer per frame.
        context->ClearDepthStencilView(privateDSV.Get(), D3D11_CLEAR_STENCIL, 1.0f, 0);
        layer.source = source;
        maskWidth = desc.Width; maskHeight = desc.Height;
        if (index == layerCount) ++layerCount;
        spdlog::info("SubjectMask27 layer {}: source={}", index, sourceKind);
        viewport = {currentViewport.TopLeftX, currentViewport.TopLeftY, currentViewport.Width, currentViewport.Height};
        spdlog::info("SubjectMask24 resources: allocation={}x{} viewport=({},{}) {}x{} format={}",
            desc.Width, desc.Height, viewport[0], viewport[1], viewport[2], viewport[3], static_cast<unsigned>(desc.Format));
    }
    if (layer.draws == 0) {
        context->CopyResource(privateTexture.Get(), source.Get());
        context->ClearDepthStencilView(privateDSV.Get(), D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
    // Do not mix differently sized viewports in this frame.
    if (viewport != std::array<float,4>{currentViewport.TopLeftX,currentViewport.TopLeftY,currentViewport.Width,currentViewport.Height}) return skip(Decision::viewportChanged);
    ComPtr<ID3D11DepthStencilState> original;
    UINT reference{}; context->OMGetDepthStencilState(original.GetAddressOf(), &reference);
    ID3D11DepthStencilState* replayState = nullptr;
    for (auto& state : states) if (state.source.Get() == original.Get()) { replayState = state.replay.Get(); break; }
    if (!replayState) {
        D3D11_DEPTH_STENCIL_DESC state{};
        if (original) original->GetDesc(&state);
        else {state.DepthEnable=TRUE; state.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL; state.DepthFunc=D3D11_COMPARISON_LESS;}
        state.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        if (state.DepthFunc == D3D11_COMPARISON_LESS) state.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        if (state.DepthFunc == D3D11_COMPARISON_GREATER) state.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
        state.StencilEnable = TRUE; state.StencilReadMask = 0xFF; state.StencilWriteMask = 0xFF;
        state.FrontFace = {D3D11_STENCIL_OP_KEEP,D3D11_STENCIL_OP_KEEP,D3D11_STENCIL_OP_REPLACE,D3D11_COMPARISON_ALWAYS};
        state.BackFace = state.FrontFace;
        CachedState cached; cached.source = original;
        if (FAILED(device->CreateDepthStencilState(&state, cached.replay.GetAddressOf()))) {failed=true; return skip(Decision::stateFailure);}
        states.push_back(std::move(cached)); replayState = states.back().replay.Get();
    }
    std::array<ID3D11RenderTargetView*,8> rtvs{};
    context->OMGetRenderTargets(8, rtvs.data(), savedDSV.ReleaseAndGetAddressOf());
    for (std::size_t i=0;i<rtvs.size();++i) savedRTVs[i].Attach(rtvs[i]);
    savedState = original; savedReference = reference;
    RecordDecision(Decision::accepted, context, geometry, pass, lighting);
    replaying = true;
    context->OMSetRenderTargets(0, nullptr, privateDSV.Get());
    context->OMSetDepthStencilState(replayState, 1);
    return true;
}

void CDoF::SubjectMask::EndReplay(ID3D11DeviceContext* context)
{
    if (!replaying) return;
    RestoreOM(context); replaying = false; ++captured; ++layers[activeLayer].draws;
}

void CDoF::SubjectMask::CompleteFrame()
{
    if (!CaptureAllowed()) return;
    ProcessReset();
    if (ready || failed) return;
    if (captured > 0) {
        ready = true;
        if (completedFrames++ == 0) { for (std::uint32_t i=0;i<layerCount;++i) spdlog::info("SubjectMask29 first live layer {}: draws={} viewport=({},{}) {}x{}", i,layers[i].draws,viewport[0],viewport[1],viewport[2],viewport[3]);
        ReportDecisions();
        spdlog::info("SubjectMask29 first live frame: {} player draws replayed, private mask ready; screen alignment/coverage/occlusion require visual verification", captured); }
        if (completedFrames % 300 == 0) spdlog::info("SubjectMask29 live: frames={} draws={} layers={}", completedFrames, captured, layerCount);
    } else if (++waitingFrames == 120) {
        ReportDecisions();
        spdlog::warn("SubjectMask24 no eligible main-camera player draws after 120 frames");
    }
}
ID3D11ShaderResourceView* CDoF::SubjectMask::GetSRV(std::uint32_t index)
{
    return enabled.load(std::memory_order_relaxed) && ready && !resetRequested.load() && index < layerCount ? layers[index].srv.Get() : nullptr;
}
std::array<float,4> CDoF::SubjectMask::GetViewport() { return viewport; }
bool CDoF::SubjectMask::WantsCapture() { return (resetRequested.load() || (!ready && !failed)) && CaptureAllowed(); }

std::uint32_t CDoF::SubjectMask::GetLayerCount() { return GetSRV() ? layerCount : 0; }

void CDoF::SubjectMask::EndFrame()
{
    if (!enabled.load(std::memory_order_relaxed)) return;
    ready = false;
    captured = 0;
    nextFrame = true;
}