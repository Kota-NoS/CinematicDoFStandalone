#include "DoFRenderer.h"
#include "SubjectMask.h"

#include "TDMIntegration.h"

#include "ShaderCompiler.h"

namespace
{
	constexpr float kTDMLockNearPlaneMaxBlur{ 1.0F };
	constexpr float kSneakNearFarDistanceCompensation{ 3.0F };
	struct TDMLockProfile
	{
		float focalLength;
		float fNumber;
		float farFocusRangeMeters;
		float farPlaneMaxBlur;
		bool distanceAdaptive;
	};

	constexpr std::array kTDMLockProfiles{
		TDMLockProfile{ 85.0F, 8.0F, 0.75F, 1.75F, false },
		TDMLockProfile{ 70.0F, 10.0F, 1.00F, 1.35F, true },
		TDMLockProfile{ 105.0F, 5.6F, 0.35F, 2.25F, false }
	};

	struct SneakFocusProfile
	{
		float focalLength;
		float fNumber;
		float farFocusRangeMeters;
		float farPlaneMaxBlur;
	};

	constexpr std::array kSneakFocusProfiles{
		SneakFocusProfile{ 65.0F, 10.0F, 1.75F, 1.25F },
		SneakFocusProfile{ 95.0F, 5.6F, 0.55F, 1.85F }
	};

	[[nodiscard]] constexpr float GetFocusRangeScale(float a_strength)
	{
		const auto strength = std::clamp(a_strength, 0.0F, 3.0F);
		if (strength <= 1.0F) {
			return 2.0F - strength;
		}
		return 1.0F - (strength - 1.0F) * 0.30F;
	}

	[[nodiscard]] constexpr float GetSneakBlurStrength(std::size_t a_profileIndex, float a_strength)
	{
		const auto strength = std::clamp(a_strength, 0.0F, 3.0F);
		if (a_profileIndex == 0U && strength > 2.0F) {
			return strength + (strength - 2.0F);
		}
		return strength;
	}

	[[nodiscard]] constexpr float GetSneakFNumber(
		std::size_t a_profileIndex, float a_baseFNumber, float a_strength)
	{
		if (a_profileIndex != 0U) {
			return a_baseFNumber;
		}
		const auto highStrength = std::clamp(a_strength - 2.0F, 0.0F, 1.0F);
		return a_baseFNumber + (8.5F - a_baseFNumber) * highStrength;
	}
	struct Float2
	{
		float x;
		float y;
	};

	struct alignas(16) DoFConstants
	{
		float transitionSpeed;
		Float2 focusCoordinate;
		float manualFocusPlane;
		float focalLength;
		float fNumber;
		float farPlaneMaxBlur;
		float nearPlaneMaxBlur;
		float blurQuality;
		float nearFarDistanceCompensation;
		float bokehBusyFactor;
		float highlightBoost;
		float postBlurSmoothing;
		std::uint32_t highlightShape;
		float highlightShapeRotationAngle;
		float petzvalStrength;
		std::uint32_t autoFocus;
		float autoFocusOffsetPlane;
		std::uint32_t targetGuardEnabled;
		float apertureShapeStrength;
		Float2 targetGuardCenter;
		Float2 targetGuardRadius;
		float nearFocusRangeMeters;
		float farFocusRangeMeters;
		std::uint32_t apertureBlades;
		float apertureRoundness;
		Float2 headGuardCenter;
		float headGuardRadius;
		std::uint32_t keepSkySharp;
		Float2 targetGuardAxis;
		std::uint32_t experimentalTransparencyMode;
		std::uint32_t apertureOptions;
		std::uint32_t starburstEnabled;
		std::uint32_t starburstRays;
		float starburstStrength;
		float starburstLength;
		float highlightExtractionThreshold;
		float starburstRotationAngle;
		float bokehCoreFill;
		float starburstWideFlareSuppression;
		float starburstStrongLightSuppression;
		float starburstHighlightSensitivity;
		std::array<float, 2> starburstPadding;
	};
	static_assert(sizeof(DoFConstants) == 192);

	struct alignas(16) SharedConstants
	{
		std::array<float, 4> cameraData;
		std::array<float, 4> bufferDimensions;
		std::array<float, 4> inputRegion;
		std::uint32_t reverseZ;
		std::array<float, 2> depthScale{ 1.0F, 1.0F };
		std::uint32_t padding;
	};
	static_assert(sizeof(SharedConstants) == 64);

	struct alignas(16) DepthFrameMaskConstants
	{
		std::uint32_t expansionRadius;
		std::array<std::uint32_t, 3> padding;
	};
	static_assert(sizeof(DepthFrameMaskConstants) == 16);

	struct alignas(16) SilhouetteConstants
	{
		std::array<float, 4> foregroundColor;
		std::array<float, 4> backgroundColor;
		std::uint32_t smoothEdges;
		std::array<std::uint32_t, 3> padding;
	};
	static_assert(sizeof(SilhouetteConstants) == 48);

	struct alignas(16) DepthFrameConstants
	{
		std::array<float, 4> color;
		std::array<float, 4> frameParameters;
		std::array<float, 4> guardCenterRadius;
		std::array<float, 4> guardAxisExpansion;
		std::array<float, 4> headData;
		std::array<std::uint32_t, 4> options;
		std::array<std::array<float, 4>, 12> guardSegments;
		std::array<std::array<float, 4>, 3> guardSegmentRadii;
		std::array<float, 4> edgeAdjustment;
	};
	static_assert(sizeof(DepthFrameConstants) == 352);

	// CommonLibSSE-NG v7.1.0 moves the BSGraphics::State runtime data
	// from 0x60 to 0x70 on Skyrim 1.7.99 and later.
	const auto& GetGraphicsRuntimeData(const RE::BSGraphics::State& state)
	{
		if (REL::Module::get().version() >= REL::Version{ 1, 7, 99, 0 }) {
			return REL::RelocateMember<RE::BSGraphics::State::RUNTIME_DATA>(std::addressof(state), 0x70);
		}
		return state.GetRuntimeData();
	}

	struct RenderArea
	{
		std::uint32_t left{};
		std::uint32_t top{};
		std::uint32_t width{};
		std::uint32_t height{};
	};

	RenderArea GetActiveRenderArea(
		ID3D11DeviceContext* a_context,
		const D3D11_TEXTURE2D_DESC& a_inputDescription)
	{
		RenderArea area{ 0U, 0U, a_inputDescription.Width, a_inputDescription.Height };
		D3D11_VIEWPORT viewport{};
		UINT viewportCount = 1;
		a_context->RSGetViewports(&viewportCount, &viewport);
		if (viewportCount == 0 ||
			!std::isfinite(viewport.TopLeftX) || !std::isfinite(viewport.TopLeftY) ||
			!std::isfinite(viewport.Width) || !std::isfinite(viewport.Height) ||
			viewport.Width < 1.0F || viewport.Height < 1.0F) {
			return area;
		}

		const auto roundedLeft = static_cast<std::int64_t>(std::llround(viewport.TopLeftX));
		const auto roundedTop = static_cast<std::int64_t>(std::llround(viewport.TopLeftY));
		if (roundedLeft < 0 || roundedTop < 0 ||
			roundedLeft >= a_inputDescription.Width || roundedTop >= a_inputDescription.Height) {
			return area;
		}

		area.left = static_cast<std::uint32_t>(roundedLeft);
		area.top = static_cast<std::uint32_t>(roundedTop);
		area.width = std::clamp(
			static_cast<std::uint32_t>(std::llround(viewport.Width)),
			1U,
			a_inputDescription.Width - area.left);
		area.height = std::clamp(
			static_cast<std::uint32_t>(std::llround(viewport.Height)),
			1U,
			a_inputDescription.Height - area.top);
		return area;
	}

	std::array<float, 4> GetCameraData()
	{
		static REL::Relocation<std::uintptr_t> cameraClipData{ REL::RelocationID(517032, 403540) };
		const auto base = cameraClipData.address();
		const auto nearPlane = *reinterpret_cast<const float*>(base + 0x40);
		const auto farPlane = *reinterpret_cast<const float*>(base + 0x44);
		return { farPlane, nearPlane, farPlane - nearPlane, farPlane * nearPlane };
	}

	bool Failed(HRESULT a_result, std::string_view a_operation)
	{
		if (SUCCEEDED(a_result)) {
			return false;
		}
		spdlog::error("{} failed (HRESULT 0x{:08X})", a_operation, static_cast<std::uint32_t>(a_result));
		return true;
	}

	std::optional<bool> ReadDisplayBool(std::string_view a_name)
	{
		if (const auto setting = RE::GetINISetting(a_name.data());
			setting && setting->GetType() == RE::Setting::Type::kBool) {
			return setting->GetBool();
		}
		return std::nullopt;
	}

	const char* DescribeDisplayBool(const std::optional<bool>& a_value)
	{
		return a_value ? (*a_value ? "on" : "off") : "unknown";
	}

	std::optional<D3D11_TEXTURE2D_DESC> GetTextureDescription(ID3D11View* a_view)
	{
		if (!a_view) {
			return std::nullopt;
		}

		ComPtr<ID3D11Resource> resource;
		a_view->GetResource(resource.GetAddressOf());
		if (!resource) {
			return std::nullopt;
		}

		ComPtr<ID3D11Texture2D> texture;
		if (FAILED(resource.As(&texture)) || !texture) {
			return std::nullopt;
		}

		D3D11_TEXTURE2D_DESC description{};
		texture->GetDesc(&description);
		return description;
	}

	bool IsCompatibleDepth(
		ID3D11ShaderResourceView* a_depth,
		const D3D11_TEXTURE2D_DESC& a_colorDescription)
	{
		if (!a_depth) {
			return false;
		}

		const auto depthDescription = GetTextureDescription(a_depth);
		if (!depthDescription) {
			return false;
		}
		return depthDescription->Width == a_colorDescription.Width &&
		       depthDescription->Height == a_colorDescription.Height &&
		       depthDescription->SampleDesc.Count == 1;
	}

	bool IsReverseZDepth(ID3D11ShaderResourceView* a_depth)
	{
		if (!a_depth) {
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription{};
		a_depth->GetDesc(&viewDescription);
		if (viewDescription.Format == DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS) {
			return true;
		}

		const auto textureDescription = GetTextureDescription(a_depth);
		return textureDescription && textureDescription->Format == DXGI_FORMAT_R32G8X24_TYPELESS;
	}

	struct OutputMergerRestore
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* renderTarget;
		ID3D11DepthStencilView* depthStencil;

		~OutputMergerRestore()
		{
			context->OMSetRenderTargets(1, &renderTarget, depthStencil);
		}
	};

	constexpr float kGameUnitToMeters = 0.01428F;
	constexpr float kStandaloneNearFocusMinimumMeters = 0.15F;

	RE::NiCamera* FindActiveNiCamera(RE::NiAVObject* a_object)
	{
		if (!a_object) {
			return nullptr;
		}
		if (const auto camera = netimmerse_cast<RE::NiCamera*>(a_object)) {
			return camera;
		}
		const auto node = a_object->AsNode();
		if (!node) {
			return nullptr;
		}
		for (const auto& child : node->GetChildren()) {
			if (auto camera = FindActiveNiCamera(child.get())) {
				return camera;
			}
		}
		return nullptr;
	}

    bool IsDepthFrameVisible(const CDoF::DepthFrameSettings& settings)
    {
        if (!settings.enabled) return false;
        if (!settings.lockOnOnly && !settings.killMoveOnly && !settings.dialogueOnly) return true;
        const auto* player = RE::PlayerCharacter::GetSingleton();
        return (settings.lockOnOnly && CDoF::SubjectMask::IsTargetLocked()) ||
            (settings.killMoveOnly && player && player->IsInKillMove()) ||
            (settings.dialogueOnly && RE::UI::GetSingleton() && RE::UI::GetSingleton()->IsMenuOpen(RE::DialogueMenu::MENU_NAME));
    }

	std::optional<RE::NiPoint3> GetFocusPosition(const RE::NiPointer<RE::TESObjectREFR>& a_target)
	{
		if (!a_target || a_target->IsDeleted() || a_target->IsDisabled() || !a_target->Is3DLoaded()) {
			return std::nullopt;
		}
		auto targetPosition = a_target->GetLookingAtLocation();
		if (const auto head = a_target->GetNodeByName(RE::BSFixedString("NPC Head [Head]"))) {
			targetPosition = head->world.translate;
		}
		return targetPosition;
	}
}

CDoF::DoFRenderer& CDoF::DoFRenderer::GetSingleton()
{
	static DoFRenderer singleton;
	return singleton;
}

void CDoF::DoFRenderer::SetSettings(Settings a_settings)
{
	std::scoped_lock lock(mutex_);
	settings_ = a_settings;
}

void CDoF::DoFRenderer::SetModeSettings(ModeSettings a_settings)
{
	std::scoped_lock lock(mutex_);
	modeSettings_ = a_settings;
}

void CDoF::DoFRenderer::SetSilhouetteSettings(SilhouetteSettings a_settings)
{
	std::scoped_lock lock(mutex_);
	for (auto& component : a_settings.foregroundColor) {
		component = std::clamp(component, 0.0F, 1.0F);
	}
	for (auto& component : a_settings.backgroundColor) {
		component = std::clamp(component, 0.0F, 1.0F);
	}
	silhouetteSettings_ = a_settings;
}

void CDoF::DoFRenderer::SetDepthFrameSettings(DepthFrameSettings a_settings)
{
	subjectDrawAuditEnabled_.store(a_settings.enabled && a_settings.subjectBreakout, std::memory_order_relaxed);
	SubjectMask::SetEnabled(a_settings.enabled && a_settings.subjectBreakout && a_settings.breakoutEdges != 0U,
        a_settings.lockOnOnly && !a_settings.killMoveOnly && !a_settings.dialogueOnly);
	depthFrameStencilCaptureEnabled_.store(
		false, // Test18: projected depth mask requires no stencil capture.
		std::memory_order_relaxed);
	std::scoped_lock lock(mutex_);
	for (auto& component : a_settings.color) {
		component = std::clamp(component, 0.0F, 1.0F);
	}
	a_settings.style = std::min(a_settings.style, 2U);
	a_settings.topBottomSize = std::clamp(a_settings.topBottomSize, 0.0F, 0.45F);
	a_settings.sideSize = std::clamp(a_settings.sideSize, 0.0F, 0.45F);
	a_settings.opacity = std::clamp(a_settings.opacity, 0.0F, 1.0F);
	a_settings.subjectExpansion = std::clamp(a_settings.subjectExpansion, 0.0F, kSubjectExpansionMax);
	a_settings.edgeSoftness = std::clamp(a_settings.edgeSoftness, 0.0F, 0.03F);
	depthFrameSettings_ = a_settings;
}

bool CDoF::DoFRenderer::ShouldCaptureDepthFrameSubject() const noexcept
{
	return depthFrameStencilCaptureEnabled_.load(std::memory_order_relaxed);
}

bool CDoF::DoFRenderer::ShouldAuditSubjectDraws() const noexcept
{
	return subjectDrawAuditEnabled_.load(std::memory_order_relaxed);
}

void CDoF::DoFRenderer::SetTargetFocus(
	TargetFocusSettings a_settings,
	Settings a_dialogueLensSettings)
{
	std::scoped_lock lock(mutex_);
	targetFocusSettings_ = a_settings;
	dialogueLensSettings_ = a_dialogueLensSettings;
}

void CDoF::DoFRenderer::SetTDMFocus(TDMFocusSettings a_settings)
{
	std::scoped_lock lock(mutex_);
	tdmFocusSettings_ = std::move(a_settings);
}

void CDoF::DoFRenderer::SetSneakFocus(SneakFocusSettings a_settings)
{
	std::scoped_lock lock(mutex_);
	sneakFocusSettings_ = std::move(a_settings);
}

void CDoF::DoFRenderer::SetExperimentalTransparency(ExperimentalTransparencySettings a_settings)
{
	std::scoped_lock lock(mutex_);
	experimentalTransparencySettings_ = a_settings;
}

void CDoF::DoFRenderer::SetFocusPickerActive(bool a_active)
{
	std::scoped_lock lock(mutex_);
	focusPickerActive_ = a_active;
}

void CDoF::DoFRenderer::CapturePreOpaqueDepth()
{
	std::scoped_lock lock(mutex_);
	preOpaqueDepthValid_ = false;
	if (!experimentalTransparencySettings_.enabled || permanentlyDisabled_) {
		return;
	}

	const auto renderer = RE::BSGraphics::Renderer::GetSingleton();
	if (!renderer) {
		return;
	}
	auto& rendererData = renderer->GetRuntimeData();
	auto device = reinterpret_cast<ID3D11Device*>(rendererData.forwarder);
	auto context = reinterpret_cast<ID3D11DeviceContext*>(rendererData.context);
	auto& mainDepth = renderer->GetDepthStencilData().depthStencils[RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN];
	if (!device || !context || !mainDepth.depthSRV) {
		return;
	}

	// Follow the published SRV resource rather than DepthStencilData::texture.
	// Terrain Blending can temporarily publish its blended R32 depth through this
	// slot while the struct's original game texture remains R24/R32G8 typeless.
	ComPtr<ID3D11Resource> sourceResource;
	mainDepth.depthSRV->GetResource(sourceResource.GetAddressOf());
	ComPtr<ID3D11Texture2D> sourceTexture;
	if (!sourceResource || FAILED(sourceResource.As(&sourceTexture)) || !sourceTexture) {
		return;
	}
	D3D11_TEXTURE2D_DESC textureDescription{};
	sourceTexture->GetDesc(&textureDescription);
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription{};
	mainDepth.depthSRV->GetDesc(&viewDescription);
	if (textureDescription.SampleDesc.Count != 1 ||
		viewDescription.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) {
		return;
	}

	const auto resourceChanged =
		preOpaqueDepthDevice_ != device ||
		preOpaqueDepthTextureFormat_ != textureDescription.Format ||
		preOpaqueDepthViewFormat_ != viewDescription.Format ||
		preOpaqueDepthWidth_ != textureDescription.Width ||
		preOpaqueDepthHeight_ != textureDescription.Height ||
		preOpaqueDepthSampleCount_ != textureDescription.SampleDesc.Count;
	if (resourceChanged || !preOpaqueDepthTexture_ || !preOpaqueDepthSRV_) {
		preOpaqueDepthTexture_.Reset();
		preOpaqueDepthSRV_.Reset();

		auto copyDescription = textureDescription;
		copyDescription.Usage = D3D11_USAGE_DEFAULT;
		copyDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		copyDescription.CPUAccessFlags = 0;
		copyDescription.MiscFlags = 0;
		if (Failed(
				device->CreateTexture2D(&copyDescription, nullptr, preOpaqueDepthTexture_.GetAddressOf()),
				"Create pre-opaque depth texture") ||
			Failed(
				device->CreateShaderResourceView(
					preOpaqueDepthTexture_.Get(), &viewDescription, preOpaqueDepthSRV_.GetAddressOf()),
				"Create pre-opaque depth SRV")) {
			preOpaqueDepthTexture_.Reset();
			preOpaqueDepthSRV_.Reset();
			return;
		}

		preOpaqueDepthDevice_ = device;
		preOpaqueDepthTextureFormat_ = textureDescription.Format;
		preOpaqueDepthViewFormat_ = viewDescription.Format;
		preOpaqueDepthWidth_ = textureDescription.Width;
		preOpaqueDepthHeight_ = textureDescription.Height;
		preOpaqueDepthSampleCount_ = textureDescription.SampleDesc.Count;
	}

	context->CopyResource(preOpaqueDepthTexture_.Get(), sourceTexture.Get());
	// The bound viewport can belong to an earlier pass, even at Native AA.
	// Use the scene's resolution ratios when deciding whether depth coordinates need scaling.
	preOpaqueDepthScale_ = { 1.0F, 1.0F };
	if (const auto* graphicsState = RE::BSGraphics::State::GetSingleton()) {
		const auto& state = GetGraphicsRuntimeData(*graphicsState);
		const float widthRatio = state.dynamicResolutionWidthRatio;
		const float heightRatio = state.dynamicResolutionHeightRatio;
		if (!state.dynamicResolutionLock && widthRatio > 0.0F && widthRatio <= 1.0F &&
			heightRatio > 0.0F && heightRatio <= 1.0F) {
			preOpaqueDepthScale_ = { widthRatio, heightRatio };
		}
	}
	preOpaqueDepthValid_ = true;
	if (!loggedPreOpaqueDepthCapture_) {
		loggedPreOpaqueDepthCapture_ = true;
		spdlog::info(
			"Captured private pre-opaque depth for hair blending ({}x{}, texture format={}, SRV format={})",
			textureDescription.Width,
			textureDescription.Height,
			static_cast<std::uint32_t>(textureDescription.Format),
			static_cast<std::uint32_t>(viewDescription.Format));
	}
}

void CDoF::DoFRenderer::SnapshotDepthFrameSubjectStencil()
{
	if (!ShouldCaptureDepthFrameSubject()) {
		return;
	}
	const auto renderer = RE::BSGraphics::Renderer::GetSingleton();
	if (!renderer) {
		return;
	}
	auto& rendererData = renderer->GetRuntimeData();
	auto device = reinterpret_cast<ID3D11Device*>(rendererData.forwarder);
	auto context = reinterpret_cast<ID3D11DeviceContext*>(rendererData.context);
	auto& mainStencil = renderer->GetDepthStencilData().depthStencils[
		RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN];
	if (!device || !context) {
		return;
	}

	// The player draw is not guaranteed to use Bethesda's kMAIN depth target.
	// Capture the depth/stencil resource that is actually bound for this draw;
	// otherwise the marker can be written successfully while we copy an unrelated,
	// empty stencil surface.
	ComPtr<ID3D11DepthStencilView> boundDepthStencil;
	context->OMGetRenderTargets(0, nullptr, boundDepthStencil.GetAddressOf());
	ComPtr<ID3D11Resource> sourceResource;
	if (boundDepthStencil) {
		boundDepthStencil->GetResource(sourceResource.GetAddressOf());
	}
	if (!sourceResource && mainStencil.stencilSRV) {
		mainStencil.stencilSRV->GetResource(sourceResource.GetAddressOf());
	}
	ComPtr<ID3D11Texture2D> sourceTexture;
	if (!sourceResource || FAILED(sourceResource.As(&sourceTexture)) || !sourceTexture) {
		return;
	}
	D3D11_TEXTURE2D_DESC textureDescription{};
	sourceTexture->GetDesc(&textureDescription);
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription{};
	switch (textureDescription.Format) {
	case DXGI_FORMAT_R24G8_TYPELESS:
		viewDescription.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		break;
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		viewDescription.Format = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
		break;
	default:
		spdlog::warn(
			"Cannot snapshot the bound player stencil: unsupported texture format {}",
			static_cast<std::uint32_t>(textureDescription.Format));
		return;
	}
	viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDescription.Texture2D.MostDetailedMip = 0;
	viewDescription.Texture2D.MipLevels = 1;
	if (textureDescription.SampleDesc.Count != 1 ||
		viewDescription.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) {
		return;
	}

	bool sourceIsMainStencil = false;
	if (mainStencil.stencilSRV) {
		ComPtr<ID3D11Resource> mainStencilResource;
		mainStencil.stencilSRV->GetResource(mainStencilResource.GetAddressOf());
		sourceIsMainStencil = mainStencilResource.Get() == sourceResource.Get();
	}

	const auto resourceChanged =
		depthFrameSubjectStencilDevice_ != device ||
		depthFrameSubjectStencilTextureFormat_ != textureDescription.Format ||
		depthFrameSubjectStencilViewFormat_ != viewDescription.Format ||
		depthFrameSubjectStencilWidth_ != textureDescription.Width ||
		depthFrameSubjectStencilHeight_ != textureDescription.Height ||
		depthFrameSubjectStencilSampleCount_ != textureDescription.SampleDesc.Count;
	if (resourceChanged || !depthFrameSubjectStencilTexture_ ||
		!depthFrameSubjectStencilSRV_) {
		depthFrameSubjectStencilTexture_.Reset();
		depthFrameSubjectStencilSRV_.Reset();
		auto copyDescription = textureDescription;
		copyDescription.Usage = D3D11_USAGE_DEFAULT;
		copyDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		copyDescription.CPUAccessFlags = 0;
		copyDescription.MiscFlags = 0;
		if (Failed(
				device->CreateTexture2D(
					&copyDescription, nullptr,
					depthFrameSubjectStencilTexture_.GetAddressOf()),
				"Create depth-frame subject stencil snapshot") ||
			Failed(
				device->CreateShaderResourceView(
					depthFrameSubjectStencilTexture_.Get(), &viewDescription,
					depthFrameSubjectStencilSRV_.GetAddressOf()),
				"Create depth-frame subject stencil snapshot SRV")) {
			depthFrameSubjectStencilTexture_.Reset();
			depthFrameSubjectStencilSRV_.Reset();
			return;
		}
		depthFrameSubjectStencilDevice_ = device;
		depthFrameSubjectStencilTextureFormat_ = textureDescription.Format;
		depthFrameSubjectStencilViewFormat_ = viewDescription.Format;
		depthFrameSubjectStencilWidth_ = textureDescription.Width;
		depthFrameSubjectStencilHeight_ = textureDescription.Height;
		depthFrameSubjectStencilSampleCount_ = textureDescription.SampleDesc.Count;
	}

	// Copying the complete surface after every tagged player pass is deliberate:
	// the newest copy contains all player markers accumulated so far, while later
	// scene passes can no longer erase the private snapshot.
	context->CopyResource(depthFrameSubjectStencilTexture_.Get(), sourceTexture.Get());
	depthFrameSubjectStencilValid_ = true;
	if (!loggedDepthFrameSubjectStencilSnapshot_) {
		loggedDepthFrameSubjectStencilSnapshot_ = true;
		spdlog::info(
			"Snapshotted tagged player stencil immediately after draw ({}x{}, texture format={}, SRV format={}, bound DSV={}, same as kMAIN={})",
			textureDescription.Width,
			textureDescription.Height,
			static_cast<std::uint32_t>(textureDescription.Format),
			static_cast<std::uint32_t>(viewDescription.Format),
			boundDepthStencil != nullptr,
			sourceIsMainStencil);
	}
}

bool CDoF::DoFRenderer::CreateTexture(
	ID3D11Device* a_device,
	Texture& a_texture,
	DXGI_FORMAT a_format,
	std::uint32_t a_width,
	std::uint32_t a_height)
{
	D3D11_TEXTURE2D_DESC description{};
	description.Width = std::max(1U, a_width);
	description.Height = std::max(1U, a_height);
	description.MipLevels = 1;
	description.ArraySize = 1;
	description.Format = a_format;
	description.SampleDesc.Count = 1;
	description.Usage = D3D11_USAGE_DEFAULT;
	description.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	if (Failed(a_device->CreateTexture2D(&description, nullptr, a_texture.resource.GetAddressOf()), "CreateTexture2D")) {
		return false;
	}
	if (Failed(a_device->CreateShaderResourceView(a_texture.resource.Get(), nullptr, a_texture.srv.GetAddressOf()), "CreateShaderResourceView")) {
		return false;
	}
	if (Failed(a_device->CreateUnorderedAccessView(a_texture.resource.Get(), nullptr, a_texture.uav.GetAddressOf()), "CreateUnorderedAccessView")) {
		return false;
	}
	return true;
}

bool CDoF::DoFRenderer::CreateConstantBuffer(
	ID3D11Device* a_device,
	std::uint32_t a_size,
	ComPtr<ID3D11Buffer>& a_buffer)
{
	D3D11_BUFFER_DESC description{};
	description.ByteWidth = (a_size + 15U) & ~15U;
	description.Usage = D3D11_USAGE_DYNAMIC;
	description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	return !Failed(a_device->CreateBuffer(&description, nullptr, a_buffer.GetAddressOf()), "CreateBuffer");
}

bool CDoF::DoFRenderer::UpdateConstantBuffer(
	ID3D11DeviceContext* a_context,
	ID3D11Buffer* a_buffer,
	const void* a_data,
	std::uint32_t a_size)
{
	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (Failed(a_context->Map(a_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped), "Map constant buffer")) {
		return false;
	}
	std::memcpy(mapped.pData, a_data, a_size);
	a_context->Unmap(a_buffer, 0);
	return true;
}

bool CDoF::DoFRenderer::CompileShaders(ID3D11Device* a_device)
{
	const auto stableReady = CompileShaderSet(
		a_device,
		L"Data\\Shaders\\CinematicDoFStandalone\\DoF\\dof.cs.hlsl",
		shaders_,
		"stable");
	if (!stableReady) {
		return false;
	}

	test6ShadersReady_ = CompileShaderSet(
		a_device,
		L"Data\\Shaders\\CinematicDoFStandalone\\DoF\\dof_test6.cs.hlsl",
		test6Shaders_,
		"hair blending mode 1 (soft blend)");
	test8ShadersReady_ = CompileShaderSet(
		a_device,
		L"Data\\Shaders\\CinematicDoFStandalone\\DoF\\dof_test8.cs.hlsl",
		test8Shaders_,
		"hair blending mode 2 (detail-preserving blend)");
	if (!test6ShadersReady_ || !test8ShadersReady_) {
		spdlog::warn(
			"One or more optional hair-blending shader sets could not be prepared; unavailable modes will fall back to stable rendering");
	}
	return true;
}

bool CDoF::DoFRenderer::CompileShaderSet(
	ID3D11Device* a_device,
	const std::filesystem::path& a_path,
	Shaders& a_shaders,
	std::string_view a_name)
{
	a_shaders = {};
	const std::array entries{
		std::pair{ std::addressof(a_shaders.updateFocus), "CS_UpdateFocus" },
		std::pair{ std::addressof(a_shaders.calculateCoC), "CS_CalculateCoC" },
		std::pair{ std::addressof(a_shaders.cocTile1), "CS_CoCTile1" },
		std::pair{ std::addressof(a_shaders.cocTile2), "CS_CoCTile2" },
		std::pair{ std::addressof(a_shaders.cocTileNeighbor), "CS_CoCTileNeighbor" },
		std::pair{ std::addressof(a_shaders.cocGaussian1), "CS_CoCGaussian1" },
		std::pair{ std::addressof(a_shaders.cocGaussian2), "CS_CoCGaussian2" },
		std::pair{ std::addressof(a_shaders.blur), "CS_Blur" },
		std::pair{ std::addressof(a_shaders.reduceColor), "CS_ReduceColor" },
		std::pair{ std::addressof(a_shaders.farBlur), "CS_FarBlur" },
		std::pair{ std::addressof(a_shaders.nearBlur), "CS_NearBlur" },
		std::pair{ std::addressof(a_shaders.tentFilter), "CS_TentFilter" },
		std::pair{ std::addressof(a_shaders.combiner), "CS_Combiner" },
		std::pair{ std::addressof(a_shaders.postSmoothing1), "CS_PostSmoothing1" },
		std::pair{ std::addressof(a_shaders.postSmoothing2AndFocusing), "CS_PostSmoothing2AndFocusing" }
	};

	for (const auto& [destination, entry] : entries) {
		*destination = CompileComputeShader(a_device, a_path, entry);
		if (!destination->Get()) {
			a_shaders = {};
			spdlog::error("Could not compile {} shader set", a_name);
			return false;
		}
	}
	spdlog::info("Compiled all 15 depth-of-field compute passes for {}", a_name);
	return true;
}

bool CDoF::DoFRenderer::EnsureBaseResources(
	ID3D11Device* a_device,
	const D3D11_TEXTURE2D_DESC& a_inputDescription,
	std::uint32_t a_renderWidth,
	std::uint32_t a_renderHeight)
{
	const auto configurationMatches =
		resources_.device == a_device &&
		resources_.width == a_renderWidth &&
		resources_.height == a_renderHeight &&
		resources_.colorFormat == a_inputDescription.Format;
	if (!configurationMatches) {
		resources_ = {};
		resources_.device = a_device;
		resources_.width = a_renderWidth;
		resources_.height = a_renderHeight;
		resources_.colorFormat = a_inputDescription.Format;
	}

	if (shadersReady_ && shaderDevice_ != a_device) {
		shaders_ = {};
		test6Shaders_ = {};
		test8Shaders_ = {};
		shadersReady_ = false;
		test6ShadersReady_ = false;
		test8ShadersReady_ = false;
		shaderDevice_ = nullptr;
	}
	if (silhouetteShaderDevice_ && silhouetteShaderDevice_ != a_device) {
		silhouetteShader_.Reset();
		silhouetteShaderReady_ = false;
		silhouetteShaderDevice_ = nullptr;
		silhouetteDisabled_ = false;
	}
	if (depthFrameShaderDevice_ && depthFrameShaderDevice_ != a_device) {
		depthFrameShader_.Reset();
		depthFrameMaskCaptureShader_.Reset();
		depthFrameResources_ = {};
		depthFrameShaderReady_ = false;
		depthFrameShaderDevice_ = nullptr;
		depthFrameDisabled_ = false;
	}
	if (resources_.baseReady) {
		return true;
	}
	if (!CreateTexture(
			a_device,
			resources_.output,
			a_inputDescription.Format,
			a_renderWidth,
			a_renderHeight) ||
		!CreateTexture(
			a_device,
			resources_.overlayInput,
			a_inputDescription.Format,
			a_renderWidth,
			a_renderHeight) ||
		!CreateConstantBuffer(a_device, sizeof(SharedConstants), resources_.sharedConstants)) {
		resources_.output = {};
		resources_.overlayInput = {};
		resources_.sharedConstants.Reset();
		return false;
	}
	resources_.baseReady = true;
	return true;
}

bool CDoF::DoFRenderer::EnsureResources(
	ID3D11Device* a_device,
	const D3D11_TEXTURE2D_DESC& a_inputDescription,
	std::uint32_t a_renderWidth,
	std::uint32_t a_renderHeight)
{
	if (!EnsureBaseResources(
			a_device,
			a_inputDescription,
			a_renderWidth,
			a_renderHeight)) {
		return false;
	}
	if (resources_.dofReady) {
		return true;
	}

	// Preserve only the base resources shared with silhouette mode. Any partial
	// DoF allocation from an earlier failure is discarded before retrying.
	auto output = std::move(resources_.output);
	auto overlayInput = std::move(resources_.overlayInput);
	auto sharedConstants = std::move(resources_.sharedConstants);
	resources_ = {};
	resources_.device = a_device;
	resources_.width = a_renderWidth;
	resources_.height = a_renderHeight;
	resources_.colorFormat = a_inputDescription.Format;
	resources_.baseReady = true;
	resources_.output = std::move(output);
	resources_.overlayInput = std::move(overlayInput);
	resources_.sharedConstants = std::move(sharedConstants);

	if (!shadersReady_) {
		shadersReady_ = CompileShaders(a_device);
		if (!shadersReady_) {
			return false;
		}
		shaderDevice_ = a_device;
	}

	const auto width = a_renderWidth;
	const auto height = a_renderHeight;
	const auto halfWidth = std::max(1U, width / 2U);
	const auto halfHeight = std::max(1U, height / 2U);
	const auto quarterWidth = std::max(1U, (halfWidth + 1U) / 2U);
	const auto quarterHeight = std::max(1U, (halfHeight + 1U) / 2U);
	const auto eighthWidth = std::max(1U, (quarterWidth + 1U) / 2U);
	const auto eighthHeight = std::max(1U, (quarterHeight + 1U) / 2U);
	const auto sixteenthWidth = std::max(1U, (eighthWidth + 1U) / 2U);
	const auto sixteenthHeight = std::max(1U, (eighthHeight + 1U) / 2U);
	const auto color = a_inputDescription.Format;
	// R11G11B10_FLOAT has no alpha channel, while the near-blur pass stores its
	// coverage in alpha for the full-resolution combiner. Preserve that coverage
	// in a dedicated alpha-capable half-resolution texture on the HDR0 path.
	const auto nearBlurColor = color == DXGI_FORMAT_R11G11B10_FLOAT ?
		DXGI_FORMAT_R16G16B16A16_FLOAT :
		color;
	// The optional Test6 path carries a conservative transparency mask in alpha
	// through the far-gather pyramid. HDR0's R11G11B10 target has no alpha, so use
	// the same alpha-capable intermediate already proven by the near-blur path.
	// Keeping this allocation stable permits live ON/OFF switching without a
	// resource rebuild; the OFF shader branch preserves the completed RGB math.
	const auto farGatherSourceColor = color == DXGI_FORMAT_R11G11B10_FLOAT ?
		DXGI_FORMAT_R16G16B16A16_FLOAT :
		color;

	const auto createColor = [&](Texture& a_texture, bool a_half) {
		return CreateTexture(a_device, a_texture, color, a_half ? halfWidth : width, a_half ? halfHeight : height);
	};
	const auto createFloat = [&](Texture& a_texture, bool a_half) {
		return CreateTexture(a_device, a_texture, DXGI_FORMAT_R32_FLOAT, a_half ? halfWidth : width, a_half ? halfHeight : height);
	};

	if (!CreateTexture(a_device, resources_.preBlurred, farGatherSourceColor, halfWidth, halfHeight) ||
		!CreateTexture(a_device, resources_.farGatherColor1, farGatherSourceColor, quarterWidth, quarterHeight) ||
		!CreateTexture(a_device, resources_.farGatherColor2, farGatherSourceColor, eighthWidth, eighthHeight) ||
		!CreateTexture(a_device, resources_.farGatherColor3, farGatherSourceColor, sixteenthWidth, sixteenthHeight) ||
		!createColor(resources_.farBlurred, true) ||
		!CreateTexture(a_device, resources_.nearBlurred, nearBlurColor, halfWidth, halfHeight) ||
		!createColor(resources_.blurredFiltered, true) ||
		!createColor(resources_.postSmooth, false) ||
		!createColor(resources_.postSmooth2, false) ||
		!CreateTexture(a_device, resources_.focus, DXGI_FORMAT_R32_FLOAT, 1, 1) ||
		!CreateTexture(a_device, resources_.previousFocus, DXGI_FORMAT_R32_FLOAT, 1, 1) ||
		!createFloat(resources_.coc, false) ||
		!createFloat(resources_.cocTileTemporary, false) ||
		!createFloat(resources_.cocTileTemporary2, false) ||
		!createFloat(resources_.cocTileNeighbor, false) ||
		!createFloat(resources_.cocBlur1, true) ||
		!createFloat(resources_.cocBlur2, true) ||
		!CreateConstantBuffer(a_device, sizeof(DoFConstants), resources_.dofConstants)) {
		return false;
	}

	D3D11_SAMPLER_DESC sampler{};
	sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampler.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampler.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampler.MaxLOD = D3D11_FLOAT32_MAX;
	if (Failed(a_device->CreateSamplerState(&sampler, resources_.linearSampler.GetAddressOf()), "CreateSamplerState")) {
		return false;
	}
	resources_.dofReady = true;

	spdlog::info(
		"Created depth-of-field resources: {}x{}, color format {}, near-blur format {} (input allocation {}x{})",
		width,
		height,
		static_cast<std::uint32_t>(color),
		static_cast<std::uint32_t>(nearBlurColor),
		a_inputDescription.Width,
		a_inputDescription.Height);
	return true;
}

bool CDoF::DoFRenderer::EnsureSilhouetteResources(
	ID3D11Device* a_device,
	const D3D11_TEXTURE2D_DESC& a_inputDescription,
	std::uint32_t a_renderWidth,
	std::uint32_t a_renderHeight)
{
	if (!EnsureBaseResources(
			a_device,
			a_inputDescription,
			a_renderWidth,
			a_renderHeight)) {
		return false;
	}

	if (!silhouetteShaderReady_) {
		const std::filesystem::path path = L"Data\\Shaders\\CinematicDoFStandalone\\DoF\\dof.cs.hlsl";
		silhouetteShader_ = CompileComputeShader(a_device, path, "CS_Silhouette");
		if (!silhouetteShader_) {
			spdlog::error("Silhouette shader compilation failed; normal DoF remains available");
			return false;
		}
		silhouetteShaderReady_ = true;
		silhouetteShaderDevice_ = a_device;
		spdlog::info("Compiled independent silhouette compute pass");
	}

	if (!resources_.silhouetteConstants &&
		!CreateConstantBuffer(a_device, sizeof(SilhouetteConstants), resources_.silhouetteConstants)) {
		spdlog::error("Silhouette constant-buffer creation failed; normal DoF remains available");
		return false;
	}
	return true;
}

bool CDoF::DoFRenderer::EnsureDepthFrameResources(
	ID3D11Device* a_device,
	const D3D11_TEXTURE2D_DESC& a_inputDescription,
	std::uint32_t a_renderWidth,
	std::uint32_t a_renderHeight)
{
	if (!EnsureDepthFrameMaskResources(a_device, a_renderWidth, a_renderHeight)) {
		return false;
	}
	if (depthFrameResources_.colorReady &&
		depthFrameResources_.colorFormat == a_inputDescription.Format) {
		return true;
	}

	depthFrameResources_.output = {};
	depthFrameResources_.input = {};
	depthFrameResources_.constants.Reset();
	depthFrameResources_.colorFormat = a_inputDescription.Format;
	if (!CreateTexture(
			a_device,
			depthFrameResources_.output,
			a_inputDescription.Format,
			a_renderWidth,
			a_renderHeight) ||
		!CreateTexture(
			a_device,
			depthFrameResources_.input,
			a_inputDescription.Format,
			a_renderWidth,
			a_renderHeight) ||
		!CreateConstantBuffer(a_device, sizeof(DepthFrameConstants), depthFrameResources_.constants)) {
		spdlog::error("Depth-frame constant-buffer creation failed; normal DoF remains available");
		return false;
	}
	depthFrameResources_.colorReady = true;
	return true;
}

bool CDoF::DoFRenderer::EnsureDepthFrameMaskResources(
	ID3D11Device* a_device,
	std::uint32_t a_renderWidth,
	std::uint32_t a_renderHeight)
{
	const auto dimensionsMatch =
		depthFrameResources_.device == a_device &&
		depthFrameResources_.width == a_renderWidth &&
		depthFrameResources_.height == a_renderHeight;
	if (!dimensionsMatch) {
		depthFrameResources_ = {};
		depthFrameResources_.device = a_device;
		depthFrameResources_.width = a_renderWidth;
		depthFrameResources_.height = a_renderHeight;
	}

	if (depthFrameShaderDevice_ && depthFrameShaderDevice_ != a_device) {
		depthFrameShader_.Reset();
		depthFrameMaskCaptureShader_.Reset();
		depthFrameShaderReady_ = false;
		depthFrameShaderDevice_ = nullptr;
	}
	if (!depthFrameShaderReady_) {
		const std::filesystem::path path = L"Data\\Shaders\\CinematicDoFStandalone\\DoF\\dof.cs.hlsl";
		depthFrameShader_ = CompileComputeShader(a_device, path, "CS_DepthFrame");
		if (!depthFrameShader_) {
			depthFrameShader_.Reset();
			depthFrameMaskCaptureShader_.Reset();
			spdlog::error("Depth-frame shader compilation failed; normal DoF remains available");
			return false;
		}
		depthFrameShaderReady_ = true;
		depthFrameShaderDevice_ = a_device;
		spdlog::info("Compiled projected depth-aware frame compute pass");
	}
	if (depthFrameResources_.maskReady) {
		return true;
	}
	if (!CreateConstantBuffer(
			a_device,
			sizeof(SharedConstants),
			depthFrameResources_.sharedConstants)) {
		spdlog::error("Depth-frame subject-mask resources could not be created");
		return false;
	}
	depthFrameResources_.maskReady = true;
	return true;
}

void CDoF::DoFRenderer::CaptureDepthFrameSubjectMask(
	ID3D11Device* a_device,
	ID3D11DeviceContext* a_context,
	ID3D11ShaderResourceView* a_stencil,
	std::uint32_t a_inputWidth,
	std::uint32_t a_inputHeight,
	std::uint32_t a_renderLeft,
	std::uint32_t a_renderTop,
	std::uint32_t a_renderWidth,
	std::uint32_t a_renderHeight)
{
	if (!a_device || !a_context || !a_stencil ||
		!depthFrameSettings_.enabled || !depthFrameSettings_.subjectBreakout) {
		return;
	}
	if (!EnsureDepthFrameMaskResources(a_device, a_renderWidth, a_renderHeight)) {
		depthFrameDisabled_ = true;
		spdlog::error("Depth frame disabled because its private subject mask could not be created");
		return;
	}

	const SharedConstants sharedData{
		.cameraData = GetCameraData(),
		.bufferDimensions = {
			static_cast<float>(a_renderWidth), static_cast<float>(a_renderHeight),
			1.0F / static_cast<float>(a_renderWidth),
			1.0F / static_cast<float>(a_renderHeight) },
		.inputRegion = {
			static_cast<float>(a_renderLeft), static_cast<float>(a_renderTop),
			1.0F / static_cast<float>(a_inputWidth),
			1.0F / static_cast<float>(a_inputHeight) },
		.reverseZ = reverseZDepth_ ? 1U : 0U,
		.padding = {}
	};
	if (!UpdateConstantBuffer(
			a_context,
			depthFrameResources_.sharedConstants.Get(),
			&sharedData,
			sizeof(sharedData))) {
		depthFrameDisabled_ = true;
		spdlog::error("Depth frame disabled after its subject-mask constant-buffer update failed");
		return;
	}
	const DepthFrameMaskConstants maskData{
		.expansionRadius = static_cast<std::uint32_t>(std::lround(
			std::clamp(depthFrameSettings_.subjectExpansion / 0.15F, 0.0F, 1.0F) * 3.0F)),
		.padding = {}
	};
	if (!UpdateConstantBuffer(
			a_context,
			depthFrameResources_.maskConstants.Get(),
			&maskData,
			sizeof(maskData))) {
		depthFrameDisabled_ = true;
		spdlog::error("Depth frame disabled after its mask-expansion constant-buffer update failed");
		return;
	}

	ID3D11Buffer* sharedCB = depthFrameResources_.sharedConstants.Get();
	ID3D11Buffer* maskCB = depthFrameResources_.maskConstants.Get();
	ID3D11UnorderedAccessView* subjectMask = depthFrameResources_.subjectMask.uav.Get();
	a_context->CSSetConstantBuffers(4, 1, &maskCB);
	a_context->CSSetConstantBuffers(5, 1, &sharedCB);
	a_context->CSSetShaderResources(12, 1, &a_stencil);
	a_context->CSSetUnorderedAccessViews(1, 1, &subjectMask, nullptr);
	a_context->CSSetShader(depthFrameMaskCaptureShader_.Get(), nullptr, 0);
	a_context->Dispatch((a_renderWidth + 7U) >> 3U, (a_renderHeight + 7U) >> 3U, 1);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	ID3D11Buffer* nullBuffer = nullptr;
	a_context->CSSetShaderResources(12, 1, &nullSRV);
	a_context->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);
	a_context->CSSetConstantBuffers(4, 1, &nullBuffer);
	a_context->CSSetConstantBuffers(5, 1, &nullBuffer);
	a_context->CSSetShader(nullptr, nullptr, 0);

	if (!loggedFirstDepthFrameMaskContents_) {
		loggedFirstDepthFrameMaskContents_ = true;

		D3D11_SHADER_RESOURCE_VIEW_DESC stencilViewDescription{};
		a_stencil->GetDesc(&stencilViewDescription);

		D3D11_TEXTURE2D_DESC stagingDescription{};
		depthFrameResources_.subjectMask.resource->GetDesc(&stagingDescription);
		stagingDescription.Usage = D3D11_USAGE_STAGING;
		stagingDescription.BindFlags = 0;
		stagingDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		stagingDescription.MiscFlags = 0;

		ComPtr<ID3D11Texture2D> stagingMask;
		if (SUCCEEDED(a_device->CreateTexture2D(
				&stagingDescription,
				nullptr,
				stagingMask.GetAddressOf()))) {
			a_context->CopyResource(
				stagingMask.Get(),
				depthFrameResources_.subjectMask.resource.Get());

			D3D11_MAPPED_SUBRESOURCE mapped{};
			if (SUCCEEDED(a_context->Map(
					stagingMask.Get(),
					0,
					D3D11_MAP_READ,
					0,
					&mapped))) {
				std::uint64_t markedPixelCount = 0;
				std::uint32_t minimumX = a_renderWidth;
				std::uint32_t minimumY = a_renderHeight;
				std::uint32_t maximumX = 0;
				std::uint32_t maximumY = 0;
				std::uint8_t maximumValue = 0;
				for (std::uint32_t y = 0; y < a_renderHeight; ++y) {
					const auto* row = static_cast<const std::uint8_t*>(mapped.pData) +
						static_cast<std::size_t>(y) * mapped.RowPitch;
					for (std::uint32_t x = 0; x < a_renderWidth; ++x) {
						const auto value = row[x];
						maximumValue = std::max(maximumValue, value);
						if (value == 0) {
							continue;
						}
						++markedPixelCount;
						minimumX = std::min(minimumX, x);
						minimumY = std::min(minimumY, y);
						maximumX = std::max(maximumX, x);
						maximumY = std::max(maximumY, y);
					}
				}
				a_context->Unmap(stagingMask.Get(), 0);

				const auto totalPixelCount =
					static_cast<std::uint64_t>(a_renderWidth) * a_renderHeight;
				if (markedPixelCount != 0) {
					spdlog::info(
						"Depth-frame subject-mask diagnostic: {} of {} pixels marked ({:.4f}%), value max={}, bounds=({},{})->({},{}) stencil SRV format={} dimension={}",
						markedPixelCount,
						totalPixelCount,
						100.0 * static_cast<double>(markedPixelCount) /
							static_cast<double>(totalPixelCount),
						static_cast<std::uint32_t>(maximumValue),
						minimumX,
						minimumY,
						maximumX,
						maximumY,
						static_cast<std::uint32_t>(stencilViewDescription.Format),
						static_cast<std::uint32_t>(stencilViewDescription.ViewDimension));
				} else {
					spdlog::warn(
						"Depth-frame subject-mask diagnostic: 0 of {} pixels marked; the live stencil contained no player marker at capture time (value max={}, stencil SRV format={}, dimension={})",
						totalPixelCount,
						static_cast<std::uint32_t>(maximumValue),
						static_cast<std::uint32_t>(stencilViewDescription.Format),
						static_cast<std::uint32_t>(stencilViewDescription.ViewDimension));
				}
			} else {
				spdlog::warn("Depth-frame subject-mask diagnostic could not map its staging texture");
			}
		} else {
			spdlog::warn("Depth-frame subject-mask diagnostic could not create its staging texture");
		}
	}

	if (!loggedFirstDepthFrameMaskCapture_) {
		loggedFirstDepthFrameMaskCapture_ = true;
		spdlog::info(
			"Captured the first private depth-frame subject mask from the live player stencil ({}x{}, source origin {},{})",
			a_renderWidth,
			a_renderHeight,
			a_renderLeft,
			a_renderTop);
	}
}

bool CDoF::DoFRenderer::IsMenuBlocked(const Settings& a_settings) const
{
	if (!a_settings.disableInMenus) {
		return false;
	}
	const auto ui = RE::UI::GetSingleton();
	return ui && (ui->IsMenuOpen(RE::MainMenu::MENU_NAME) ||
		ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME) ||
		ui->IsMenuOpen(RE::MapMenu::MENU_NAME));
}

bool CDoF::DoFRenderer::IsSilhouetteMenuBlocked() const
{
	const auto ui = RE::UI::GetSingleton();
	return ui && (ui->IsMenuOpen(RE::MainMenu::MENU_NAME) ||
		ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME) ||
		ui->IsMenuOpen(RE::MapMenu::MENU_NAME));
}

std::optional<CDoF::DoFRenderer::TargetFocusSample> CDoF::DoFRenderer::GetDialogueTargetFocus() const
{
	const auto topicManager = RE::MenuTopicManager::GetSingleton();
	if (!topicManager || !topicManager->menuOpen) {
		return std::nullopt;
	}

	auto target = topicManager->speaker.get();
	if (!target) {
		target = topicManager->lastSpeaker.get();
	}

	// Conversation-camera mods can keep MenuTopicManager::speaker on the NPC while
	// cutting the visible shot to the player. Pick whichever head is actually
	// framed closest to the screen center, then fall back to the logical speaker.
	const auto player = RE::PlayerCharacter::GetSingleton();
	const auto playerCamera = RE::PlayerCamera::GetSingleton();
	const auto niCamera = playerCamera ? FindActiveNiCamera(playerCamera->cameraRoot.get()) : nullptr;
	if (player && niCamera) {
		const RE::NiPointer<RE::TESObjectREFR> playerTarget{ player };
		RE::NiPointer<RE::TESObjectREFR> bestTarget;
		float bestScore = std::numeric_limits<float>::max();
		const std::array candidates{ target, playerTarget };
		for (const auto& candidate : candidates) {
			const auto position = GetFocusPosition(candidate);
			if (!position) {
				continue;
			}
			float screenX{};
			float screenY{};
			float screenZ{};
			if (!niCamera->WorldPtToScreenPt3(*position, screenX, screenY, screenZ, 1.0e-5F) ||
				screenX < -0.1F || screenX > 1.1F || screenY < -0.1F || screenY > 1.1F) {
				continue;
			}
			const auto x = screenX - 0.5F;
			const auto y = screenY - 0.5F;
			const auto score = x * x + y * y;
			if (score < bestScore) {
				bestScore = score;
				bestTarget = candidate;
			}
		}
		if (bestTarget) {
			return GetTargetFocus(bestTarget, true);
		}
	}
	return GetTargetFocus(target, false);
}

std::optional<CDoF::DoFRenderer::TargetFocusSample> CDoF::DoFRenderer::GetConsoleTargetFocus() const
{
	return GetTargetFocus(RE::Console::GetSelectedRef(), true);
}

std::optional<CDoF::DoFRenderer::TargetFocusSample> CDoF::DoFRenderer::GetPlayerTargetFocus() const
{
	const auto camera = RE::PlayerCamera::GetSingleton();
	const auto player = RE::PlayerCharacter::GetSingleton();
	if (!camera || camera->IsInFirstPerson() || !player) {
		return std::nullopt;
	}
	return GetTargetFocus(RE::NiPointer<RE::TESObjectREFR>{ player }, true);
}

std::optional<CDoF::DoFRenderer::TargetFocusSample> CDoF::DoFRenderer::GetTargetFocus(
	const RE::NiPointer<RE::TESObjectREFR>& a_target,
	bool a_allowPlayer) const
{
	if (!a_target || (!a_allowPlayer && a_target->IsPlayer()) || a_target->IsDeleted() || a_target->IsDisabled() ||
		!a_target->Is3DLoaded()) {
		return std::nullopt;
	}

	const auto targetPosition = GetFocusPosition(a_target);
	if (!targetPosition) {
		return std::nullopt;
	}
	const auto playerCamera = RE::PlayerCamera::GetSingleton();
	const auto niCamera = playerCamera ? FindActiveNiCamera(playerCamera->cameraRoot.get()) : nullptr;
	if (!niCamera) {
		return std::nullopt;
	}

	float screenX{};
	float screenY{};
	float screenDepth{};
	if (!niCamera->WorldPtToScreenPt3(*targetPosition, screenX, screenY, screenDepth, 1.0e-5F) ||
		!std::isfinite(screenX) || !std::isfinite(screenY) || !std::isfinite(screenDepth)) {
		return std::nullopt;
	}

	// Match the exact depth convention used by SharedData::GetScreenDepth in the
	// DoF shader.  A projected depth is camera-axis depth even when the target is
	// far from screen centre, unlike Euclidean camera-to-target distance.
	const auto cameraData = GetCameraData();
	const auto denominator = reverseZDepth_ ?
		screenDepth * cameraData[2] + cameraData[1] :
		-screenDepth * cameraData[2] + cameraData[0];
	if (!std::isfinite(denominator) || denominator <= 1.0e-5F) {
		return std::nullopt;
	}
	const auto distanceMeters = (cameraData[3] / denominator) * kGameUnitToMeters;
	if (!std::isfinite(distanceMeters) || distanceMeters <= 0.0F) {
		return std::nullopt;
	}
	TargetFocusSample result{};
	result.actor = a_target->GetFormType() == RE::FormType::ActorCharacter;
	result.distanceMeters = std::clamp(distanceMeters, 0.1F, 150.0F);
	const auto shaderScreenY = 1.0F - screenY;
	result.focusCoordinate = { screenX, shaderScreenY };
	result.surfaceFocusValid =
		screenX >= 0.0F && screenX <= 1.0F && shaderScreenY >= 0.0F && shaderScreenY <= 1.0F;
	result.guardCenter = { screenX, shaderScreenY };
	result.guardRadius = { 0.08F, 0.12F };
	result.headGuardCenter = { screenX, shaderScreenY };

	const auto project = [niCamera](const RE::NiPoint3& a_position) -> std::optional<std::array<float, 2>> {
		float x{};
		float y{};
		float z{};
		if (!niCamera->WorldPtToScreenPt3(a_position, x, y, z, 1.0e-5F) ||
			!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
			return std::nullopt;
		}
		// NiCamera screen Y grows upward; texture UV Y grows downward.
		return std::array{ x, 1.0F - y };
	};
	const auto projectNode = [&a_target, &project](const char* a_name) -> std::optional<std::array<float, 2>> {
		if (const auto node = a_target->GetNodeByName(RE::BSFixedString(a_name))) {
			return project(node->world.translate);
		}
		return std::nullopt;
	};

	if (result.actor) {
		// The head position is already the focus anchor.  Project a small sphere
		// around it so the screen-space guard grows naturally during close-ups and
		// shrinks with distance, independently of standing, crouching, or lying poses.
		result.headGuardValid = true;
		result.headGuardRadius = 0.065F;
		constexpr float headWorldRadius = 9.0F;
		const auto bufferAspect = resources_.height > 0U ?
			static_cast<float>(resources_.width) / static_cast<float>(resources_.height) : (16.0F / 9.0F);
		const std::array headOffsets{
			RE::NiPoint3{ headWorldRadius, 0.0F, 0.0F },
			RE::NiPoint3{ -headWorldRadius, 0.0F, 0.0F },
			RE::NiPoint3{ 0.0F, headWorldRadius, 0.0F },
			RE::NiPoint3{ 0.0F, -headWorldRadius, 0.0F },
			RE::NiPoint3{ 0.0F, 0.0F, headWorldRadius },
			RE::NiPoint3{ 0.0F, 0.0F, -headWorldRadius }
		};
		float projectedHeadRadius{};
		for (const auto& offset : headOffsets) {
			if (const auto point = project(*targetPosition + offset)) {
				const auto deltaX = (point->at(0) - screenX) * bufferAspect;
				const auto deltaY = point->at(1) - shaderScreenY;
				const auto radius = std::hypot(deltaX, deltaY);
				if (std::isfinite(radius)) {
					projectedHeadRadius = std::max(projectedHeadRadius, radius);
				}
			}
		}
		if (projectedHeadRadius > 0.001F) {
			result.headGuardRadius = std::clamp(projectedHeadRadius, 0.065F, 0.36F);
		}
		// An actor's root bound is close to spherical and greatly overestimates its
		// width.  Keep the projected head-to-feet axis so the shader can use a thin
		// pose-following capsule instead of a broad axis-aligned ellipse.
		if (const auto feet = project(a_target->GetPosition())) {
			// Keep an off-screen head or foot from pulling the entire protection
			// capsule outside the visible image in close portrait framing.
			const auto headX = std::clamp(screenX, -0.05F, 1.05F);
			const auto headY = std::clamp(shaderScreenY, -0.05F, 1.05F);
			const auto feetX = std::clamp(feet->at(0), -0.05F, 1.05F);
			const auto feetY = std::clamp(feet->at(1), -0.05F, 1.05F);
			const auto spanX = (headX - feetX) * bufferAspect;
			const auto spanY = headY - feetY;
			const auto screenSpan = std::hypot(spanX, spanY);
			if (std::isfinite(screenSpan) && screenSpan > 0.01F) {
				const auto centreX = (headX + feetX) * 0.5F;
				const auto centreY = (headY + feetY) * 0.5F;
				const auto capsuleRadius = std::clamp(screenSpan * 0.14F, 0.03F, 0.14F);
				result.guardCenter = { centreX, centreY };
				result.guardRadius = { capsuleRadius, std::clamp(screenSpan * 0.5F + capsuleRadius, 0.08F, 0.70F) };
				result.guardAxis = { headX - centreX, headY - centreY };

				const auto appendSegment = [&result](
					const std::optional<std::array<float, 2>>& a_start,
					const std::optional<std::array<float, 2>>& a_end,
					float a_radius) {
					if (!a_start || !a_end ||
						result.guardSegmentCount >= TargetFocusSample::kMaximumGuardSegments) {
						return;
					}
					auto& segment = result.guardSegments[result.guardSegmentCount++];
					segment.start = *a_start;
					segment.end = *a_end;
					segment.radius = a_radius;
				};
				const auto spine = projectNode("NPC Spine2 [Spn2]");
				const auto pelvis = projectNode("NPC Pelvis [Pelv]");
				const auto leftUpperArm = projectNode("NPC L UpperArm [LUar]");
				const auto leftForearm = projectNode("NPC L Forearm [LLar]");
				const auto leftHand = projectNode("NPC L Hand [LHnd]");
				const auto rightUpperArm = projectNode("NPC R UpperArm [RUar]");
				const auto rightForearm = projectNode("NPC R Forearm [RLar]");
				const auto rightHand = projectNode("NPC R Hand [RHnd]");
				const auto leftCalf = projectNode("NPC L Calf [LClf]");
				const auto leftFoot = projectNode("NPC L Foot [Lft ]");
				const auto rightCalf = projectNode("NPC R Calf [RClf]");
				const auto rightFoot = projectNode("NPC R Foot [Rft ]");
				const auto armRadius = std::clamp(screenSpan * 0.065F, 0.020F, 0.075F);
				const auto legRadius = std::clamp(screenSpan * 0.070F, 0.022F, 0.080F);
				const auto torsoRadius = std::clamp(screenSpan * 0.18F, 0.045F, 0.18F);
				appendSegment(spine, pelvis, torsoRadius);
				appendSegment(leftUpperArm ? leftUpperArm : spine, leftForearm, armRadius);
				appendSegment(leftForearm, leftHand, armRadius);
				appendSegment(rightUpperArm ? rightUpperArm : spine, rightForearm, armRadius);
				appendSegment(rightForearm, rightHand, armRadius);
				appendSegment(pelvis, leftCalf, legRadius);
				appendSegment(leftCalf, leftFoot, legRadius);
				appendSegment(pelvis, rightCalf, legRadius);
				appendSegment(rightCalf, rightFoot, legRadius);

				// Equipped weapons are separate geometry and can extend well beyond the
				// body skeleton. Preserve the first visible weapon attachment's projected
				// world bound as an additional compact capsule.
				constexpr std::array weaponNodes{
					"WeaponSword", "WeaponBack", "WeaponBow", "WeaponAxe",
					"WeaponMace", "WeaponDagger", "QUIVER", "SHIELD"
				};
				for (const auto* name : weaponNodes) {
					const auto node = a_target->GetNodeByName(RE::BSFixedString(name));
					if (!node || !std::isfinite(node->worldBound.radius) || node->worldBound.radius <= 1.0F) {
						continue;
					}
					const auto centre = project(node->worldBound.center);
					if (!centre) {
						continue;
					}
					float projectedRadius{};
					const auto worldRadius = node->worldBound.radius;
					const std::array offsets{
						RE::NiPoint3{ worldRadius, 0.0F, 0.0F },
						RE::NiPoint3{ 0.0F, worldRadius, 0.0F },
						RE::NiPoint3{ 0.0F, 0.0F, worldRadius }
					};
					for (const auto& offset : offsets) {
						if (const auto edge = project(node->worldBound.center + offset)) {
							projectedRadius = std::max(projectedRadius, std::hypot(
								(edge->at(0) - centre->at(0)) * bufferAspect,
								edge->at(1) - centre->at(1)));
						}
					}
					appendSegment(centre, centre, std::clamp(projectedRadius, armRadius, 0.16F));
					break;
				}
			}
		}
	} else if (const auto object3D = a_target->Get3D(); object3D &&
		std::isfinite(object3D->worldBound.radius) && object3D->worldBound.radius > 1.0F) {
		const auto& bound = object3D->worldBound;
		float minX = screenX;
		float maxX = screenX;
		float minY = shaderScreenY;
		float maxY = shaderScreenY;
		const std::array offsets{
			RE::NiPoint3{ bound.radius, 0.0F, 0.0F },
			RE::NiPoint3{ -bound.radius, 0.0F, 0.0F },
			RE::NiPoint3{ 0.0F, bound.radius, 0.0F },
			RE::NiPoint3{ 0.0F, -bound.radius, 0.0F },
			RE::NiPoint3{ 0.0F, 0.0F, bound.radius },
			RE::NiPoint3{ 0.0F, 0.0F, -bound.radius }
		};
		for (const auto& offset : offsets) {
			if (const auto point = project(bound.center + offset)) {
				minX = std::min(minX, point->at(0));
				maxX = std::max(maxX, point->at(0));
				minY = std::min(minY, point->at(1));
				maxY = std::max(maxY, point->at(1));
			}
		}
		result.guardCenter = { (minX + maxX) * 0.5F, (minY + maxY) * 0.5F };
		result.guardRadius = {
			std::clamp((maxX - minX) * 0.58F, 0.04F, 0.45F),
			std::clamp((maxY - minY) * 0.58F, 0.05F, 0.60F)
		};
	}
	result.guardValid = std::isfinite(result.guardCenter[0]) && std::isfinite(result.guardCenter[1]) &&
		std::isfinite(result.guardRadius[0]) && std::isfinite(result.guardRadius[1]) &&
		result.guardRadius[0] > 0.0F && result.guardRadius[1] > 0.0F;
	return result;
}

void CDoF::DoFRenderer::ApplyDepthStrength(Settings& a_settings, float a_strength)
{
	const auto strength = std::clamp(a_strength, 0.0F, 3.0F);
	a_settings.farPlaneMaxBlur = std::clamp(a_settings.farPlaneMaxBlur * strength, 0.0F, 8.0F);
	a_settings.nearPlaneMaxBlur = std::clamp(a_settings.nearPlaneMaxBlur * strength, 0.0F, 4.0F);
}

void CDoF::DoFRenderer::UpdateDepthConvention(ID3D11ShaderResourceView* a_depth)
{
	// A depth SRV can be briefly unavailable during renderer transitions. Keep the
	// last confirmed convention instead of treating that transient state as standard Z.
	if (!a_depth) {
		return;
	}

	const auto reverseZ = IsReverseZDepth(a_depth);
	if (!depthConventionInitialized_ || reverseZ != reverseZDepth_) {
		depthConventionInitialized_ = true;
		reverseZDepth_ = reverseZ;
		resources_.focusInitialized = false;

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription{};
		a_depth->GetDesc(&viewDescription);
		const auto textureDescription = GetTextureDescription(a_depth);
		spdlog::info(
			"Depth convention selected: {}; depth SRV format={}; texture format={}",
			reverseZDepth_ ? "Reverse Z (Bottled Shaders compatible)" : "standard Z",
			static_cast<std::uint32_t>(viewDescription.Format),
			textureDescription ? static_cast<std::uint32_t>(textureDescription->Format) : 0U);
	}
}

void CDoF::DoFRenderer::ApplySilhouette(RE::RENDER_TARGET a_outputTarget)
{
	try {
		const auto renderer = RE::BSGraphics::Renderer::GetSingleton();
		if (!renderer) {
			return;
		}

		auto& rendererData = renderer->GetRuntimeData();
		auto device = reinterpret_cast<ID3D11Device*>(rendererData.forwarder);
		auto context = reinterpret_cast<ID3D11DeviceContext*>(rendererData.context);
		if (!device || !context) {
			return;
		}

		const auto outputTargetIndex = static_cast<std::int32_t>(a_outputTarget);
		ComPtr<ID3D11Texture2D> framebufferTexture;
		ID3D11Texture2D* destinationTexture = nullptr;
		const char* destinationKind = "render target";
		if (a_outputTarget == RE::RENDER_TARGETS::kFRAMEBUFFER) {
			if (const auto window = RE::BSGraphics::Renderer::GetCurrentRenderWindow();
				window && window->renderView) {
				auto view = reinterpret_cast<ID3D11RenderTargetView*>(window->renderView);
				ComPtr<ID3D11Resource> framebufferResource;
				view->GetResource(framebufferResource.GetAddressOf());
				if (framebufferResource && SUCCEEDED(framebufferResource.As(&framebufferTexture))) {
					destinationTexture = framebufferTexture.Get();
					destinationKind = "swap-chain framebuffer";
				}
			}
		} else if (outputTargetIndex >= 0 && outputTargetIndex < RE::RENDER_TARGETS::kTOTAL) {
			destinationTexture = rendererData.renderTargets[outputTargetIndex].texture;
		}
		auto& depthStencils = renderer->GetDepthStencilData().depthStencils;
		auto mainDepth = depthStencils[RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN].depthSRV;
		if (!destinationTexture || !mainDepth) {
			if (!loggedSilhouetteDestinationFailure_) {
				loggedSilhouetteDestinationFailure_ = true;
				spdlog::error(
					"Silhouette could not resolve post-processing output target {}; destination={}, depth={}",
					outputTargetIndex,
					static_cast<const void*>(destinationTexture),
					static_cast<const void*>(mainDepth));
			}
			return;
		}
		UpdateDepthConvention(mainDepth);

		D3D11_TEXTURE2D_DESC inputDescription{};
		destinationTexture->GetDesc(&inputDescription);
		if (!IsCompatibleDepth(mainDepth, inputDescription)) {
			if (!loggedSilhouetteDestinationFailure_) {
				loggedSilhouetteDestinationFailure_ = true;
				spdlog::error(
					"Silhouette post-processing output target {} has incompatible dimensions {}x{}",
					outputTargetIndex,
					inputDescription.Width,
					inputDescription.Height);
			}
			return;
		}
		const auto renderArea = GetActiveRenderArea(context, inputDescription);
		if (inputDescription.SampleDesc.Count != 1 ||
			!EnsureSilhouetteResources(device, inputDescription, renderArea.width, renderArea.height)) {
			silhouetteDisabled_ = true;
			spdlog::error(
				"Silhouette mode disabled because its GPU resources could not be created; normal DoF remains available");
			return;
		}

		ComPtr<ID3D11RenderTargetView> currentRTV;
		ComPtr<ID3D11DepthStencilView> currentDSV;
		context->OMGetRenderTargets(1, currentRTV.GetAddressOf(), currentDSV.GetAddressOf());
		ID3D11RenderTargetView* restoreRTV = currentRTV.Get();
		OutputMergerRestore restore{ context, restoreRTV, currentDSV.Get() };
		context->OMSetRenderTargets(0, nullptr, nullptr);

		if (!DispatchSilhouette(
				context,
				mainDepth,
				silhouetteSettings_,
				inputDescription.Width,
				inputDescription.Height,
				renderArea.left,
				renderArea.top)) {
			silhouetteDisabled_ = true;
			spdlog::error(
				"Silhouette mode disabled after a constant-buffer update failure; normal DoF remains available");
			return;
		}

		const D3D11_BOX outputBox{
			0U,
			0U,
			0U,
			renderArea.width,
			renderArea.height,
			1U
		};
		context->CopySubresourceRegion(
			destinationTexture,
			0,
			renderArea.left,
			renderArea.top,
			0,
			resources_.output.resource.Get(),
			0,
			&outputBox);
		if (!loggedFirstSilhouetteFrame_) {
			loggedFirstSilhouetteFrame_ = true;
			spdlog::info(
				"First independent silhouette frame applied successfully to {} {} (format {})",
				destinationKind,
				outputTargetIndex,
				static_cast<std::uint32_t>(inputDescription.Format));
		}
	} catch (const std::exception& error) {
		silhouetteDisabled_ = true;
		spdlog::error(
			"Silhouette render exception; only silhouette mode was disabled: {}",
			error.what());
	} catch (...) {
		silhouetteDisabled_ = true;
		spdlog::error("Unknown silhouette render exception; only silhouette mode was disabled");
	}
}

void CDoF::DoFRenderer::ApplyDepthFrame(RE::RENDER_TARGET a_outputTarget, const DepthFrameSettings& a_frameSettings)
{
	try {
		const auto renderer = RE::BSGraphics::Renderer::GetSingleton();
		if (!renderer) {
			return;
		}

		auto& rendererData = renderer->GetRuntimeData();
		auto device = reinterpret_cast<ID3D11Device*>(rendererData.forwarder);
		auto context = reinterpret_cast<ID3D11DeviceContext*>(rendererData.context);
		if (!device || !context) {
			return;
		}

		const auto outputTargetIndex = static_cast<std::int32_t>(a_outputTarget);
		ComPtr<ID3D11Texture2D> framebufferTexture;
		ID3D11Texture2D* destinationTexture = nullptr;
		const char* destinationKind = "render target";
		if (a_outputTarget == RE::RENDER_TARGETS::kFRAMEBUFFER) {
			if (const auto window = RE::BSGraphics::Renderer::GetCurrentRenderWindow();
				window && window->renderView) {
				auto view = reinterpret_cast<ID3D11RenderTargetView*>(window->renderView);
				ComPtr<ID3D11Resource> framebufferResource;
				view->GetResource(framebufferResource.GetAddressOf());
				if (framebufferResource && SUCCEEDED(framebufferResource.As(&framebufferTexture))) {
					destinationTexture = framebufferTexture.Get();
					destinationKind = "swap-chain framebuffer";
				}
			}
			// CS temporarily replaces this slot with its linear HDR scene texture.
			// Draw there before CS composites it over the window back buffer.
			auto* framebufferTarget = rendererData.renderTargets[RE::RENDER_TARGETS::kFRAMEBUFFER].texture;
			if (framebufferTarget && framebufferTarget != destinationTexture) {
				D3D11_TEXTURE2D_DESC framebufferDesc{};
				framebufferTarget->GetDesc(&framebufferDesc);
				if (framebufferDesc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT) {
					destinationTexture = framebufferTarget;
					destinationKind = "redirected HDR framebuffer";
				}
			}
		} else if (outputTargetIndex >= 0 && outputTargetIndex < RE::RENDER_TARGETS::kTOTAL) {
			destinationTexture = rendererData.renderTargets[outputTargetIndex].texture;
		}
		auto& depthStencils = renderer->GetDepthStencilData().depthStencils;
		auto mainDepth = depthStencils[RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN].depthSRV;
		if (!destinationTexture || !mainDepth) {
			if (!loggedDepthFrameDestinationFailure_) {
				loggedDepthFrameDestinationFailure_ = true;
				spdlog::error(
					"Depth frame could not resolve post-processing output target {}; destination={}, depth={}",
					outputTargetIndex,
					static_cast<const void*>(destinationTexture),
					static_cast<const void*>(mainDepth));
			}
			return;
		}
		UpdateDepthConvention(mainDepth);

		D3D11_TEXTURE2D_DESC inputDescription{};
		destinationTexture->GetDesc(&inputDescription);
		if (!IsCompatibleDepth(mainDepth, inputDescription)) {
			return;
		}
		const auto renderArea = GetActiveRenderArea(context, inputDescription);
		if (inputDescription.SampleDesc.Count != 1 ||
			!EnsureDepthFrameResources(device, inputDescription, renderArea.width, renderArea.height)) {
			depthFrameDisabled_ = true;
			spdlog::error("Depth frame disabled because its GPU resources could not be created");
			return;
		}

		ComPtr<ID3D11RenderTargetView> currentRTV;
		ComPtr<ID3D11DepthStencilView> currentDSV;
		context->OMGetRenderTargets(1, currentRTV.GetAddressOf(), currentDSV.GetAddressOf());
		ID3D11RenderTargetView* restoreRTV = currentRTV.Get();
		OutputMergerRestore restore{ context, restoreRTV, currentDSV.Get() };
		context->OMSetRenderTargets(0, nullptr, nullptr);

		const D3D11_BOX sourceBox{
			renderArea.left,
			renderArea.top,
			0U,
			renderArea.left + renderArea.width,
			renderArea.top + renderArea.height,
			1U
		};
		context->CopySubresourceRegion(
			depthFrameResources_.input.resource.Get(), 0, 0, 0, 0,
			destinationTexture, 0, &sourceBox);

		auto target = activeDepthFrameTarget_;
		if ((!target || !target->guardValid) && a_frameSettings.subjectBreakout) {
			target = GetPlayerTargetFocus();
		}
		const auto* validTarget = target && target->guardValid && target->actor ?
			std::addressof(*target) : nullptr;
		auto hairDepth = experimentalTransparencySettings_.enabled && preOpaqueDepthValid_ ?
			preOpaqueDepthSRV_.Get() : nullptr;
		if (!IsCompatibleDepth(hairDepth, inputDescription)) {
			hairDepth = nullptr;
		}
		if (!DispatchDepthFrame(
				context,
				depthFrameResources_.input.srv.Get(),
				mainDepth,
				hairDepth,
				SubjectMask::GetSRV(), // Test24: private player stencil, never game stencil.
				a_frameSettings,
				validTarget,
				inputDescription.Width,
				inputDescription.Height,
				renderArea.left,
				renderArea.top)) {
			depthFrameDisabled_ = true;
			spdlog::error("Depth frame disabled after a constant-buffer update failure");
			return;
		}

		const D3D11_BOX outputBox{ 0U, 0U, 0U, renderArea.width, renderArea.height, 1U };
		context->CopySubresourceRegion(
			destinationTexture, 0, renderArea.left, renderArea.top, 0,
			depthFrameResources_.output.resource.Get(), 0, &outputBox);
		if (!loggedFirstDepthFrame_) {
			loggedFirstDepthFrame_ = true;
			spdlog::info(
				"First depth-aware frame applied successfully to {} {} (style {}, subject breakout {})",
				destinationKind,
				outputTargetIndex,
				a_frameSettings.style,
				validTarget != nullptr);
		}
	} catch (const std::exception& error) {
		depthFrameDisabled_ = true;
		spdlog::error("Depth-frame render exception; only depth frame was disabled: {}", error.what());
	} catch (...) {
		depthFrameDisabled_ = true;
		spdlog::error("Unknown depth-frame render exception; only depth frame was disabled");
	}
}

void CDoF::DoFRenderer::ApplyAfterPostProcessing(RE::RENDER_TARGET a_outputTarget)
{
	std::scoped_lock lock(mutex_);
	const auto now = std::chrono::steady_clock::now();
	const auto previousUpdate = std::exchange(lockOnFrameUpdatedAt_, now);
	if (IsSilhouetteMenuBlocked()) {
		return;
	}
	if (silhouetteSettings_.enabled) {
		if (silhouetteSettings_.cocMonochrome && settings_.enabled && !depthFrameDisabled_) {
			ApplyDepthFrame(a_outputTarget, depthFrameSettings_);
		} else if (!silhouetteDisabled_) {
			ApplySilhouette(a_outputTarget);
		}
		return;
	}
	if (!depthFrameSettings_.enabled || depthFrameDisabled_) {
		lockOnFrameProgress_ = 0.0F;
		return;
	}
	if (!depthFrameSettings_.lockOnOnly && !depthFrameSettings_.killMoveOnly && !depthFrameSettings_.dialogueOnly) {
		lockOnFrameProgress_ = 0.0F;
		ApplyDepthFrame(a_outputTarget, depthFrameSettings_);
		return;
	}

	constexpr std::chrono::duration<float> slideDuration{ 0.22F };
	const float step = previousUpdate == decltype(lockOnFrameUpdatedAt_){} ?
		0.0F : (now - previousUpdate) / slideDuration;
	const float direction = IsDepthFrameVisible(depthFrameSettings_) ? 1.0F : -1.0F;
	lockOnFrameProgress_ = std::clamp(lockOnFrameProgress_ + step * direction, 0.0F, 1.0F);
	if (lockOnFrameProgress_ <= 0.0F) {
		return;
	}

	const float progress = lockOnFrameProgress_;
	const float widthScale = progress * progress * (3.0F - 2.0F * progress);
	auto frame = depthFrameSettings_;
	frame.topBottomSize *= widthScale;
	frame.sideSize *= widthScale;
	frame.frameTop *= widthScale;
	frame.frameBottom *= widthScale;
	frame.frameLeft *= widthScale;
	frame.frameRight *= widthScale;
	// Rotated frame boundaries can cross the viewport even at zero width.
	if (frame.frameRotationDegrees != 0.0F) {
		frame.opacity *= widthScale;
	}
	ApplyDepthFrame(a_outputTarget, frame);
}

void CDoF::DoFRenderer::ApplyBeforePostProcessing()
{
	std::scoped_lock lock(mutex_);
	activeDepthFrameTarget_.reset();
	// Silhouette colors are display-space choices.  Apply them only after the
	// game's image-space pass so exposure, tonemapping, and color grading cannot
	// tint an exact white/black palette.  Normal DoF remains on this HDR-side path.
	if (silhouetteSettings_.enabled && !silhouetteSettings_.cocMonochrome) {
		return;
	}
	const auto frameSubject = IsDepthFrameVisible(depthFrameSettings_) && depthFrameSettings_.subjectBreakout &&
        depthFrameSettings_.linkSubjectFocus ? SubjectMask::GetTarget() : nullptr;
    const bool followFrameSubject = frameSubject && !frameSubject->IsPlayerRef();
    if ((!settings_.enabled && !followFrameSubject) || permanentlyDisabled_) {
		return;
	}
	if (const auto renderer = RE::BSGraphics::Renderer::GetSingleton()) {
		auto& depthStencils = renderer->GetDepthStencilData().depthStencils;
		UpdateDepthConvention(depthStencils[RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN].depthSRV);
	}
	auto effectiveSettings = settings_;
	auto targetFocusMode = TargetFocusMode::kNone;
	std::optional<TargetFocusSample> activeTargetFocus;
	if (targetFocusSettings_.dialogueEnabled) {
		if (const auto focus = GetDialogueTargetFocus()) {
			effectiveSettings = dialogueLensSettings_;
			effectiveSettings.enabled = true;
			// Focus on the visible surface at the projected dialogue anchor.  Using the
			// camera-space anchor distance can disagree with the depth texture supplied
			// by Community Shaders, which leaves the entire subject on the near side.
			effectiveSettings.autoFocus = focus->surfaceFocusValid;
			if (focus->surfaceFocusValid) {
				effectiveSettings.focusX = focus->focusCoordinate[0];
				effectiveSettings.focusY = focus->focusCoordinate[1];
			} else {
				effectiveSettings.manualFocusMeters = focus->distanceMeters;
			}
			ApplyDepthStrength(effectiveSettings, targetFocusSettings_.dialogueDepthStrength);
			activeTargetFocus = focus;
			targetFocusMode = TargetFocusMode::kDialogue;
		}
	}
	if (targetFocusMode == TargetFocusMode::kNone && tdmFocusSettings_.enabled) {
		if (const auto target = TDMIntegration::GetLockedTarget()) {
			if (const auto focus = GetTargetFocus(target, false)) {
				effectiveSettings = settings_;
				effectiveSettings.enabled = true;
				effectiveSettings.autoFocus = focus->surfaceFocusValid;
				if (focus->surfaceFocusValid) {
					effectiveSettings.focusX = focus->focusCoordinate[0];
					effectiveSettings.focusY = focus->focusCoordinate[1];
					effectiveSettings.autoFocusOffsetMeters = 0.0F;
				} else {
					effectiveSettings.manualFocusMeters = std::clamp(focus->distanceMeters, 0.1F, 150.0F);
				}
				const auto profileIndex = std::min<std::size_t>(
					tdmFocusSettings_.profile, kTDMLockProfiles.size() - 1U);
				const auto& profile = kTDMLockProfiles[profileIndex];
				const auto focusRangeScale = GetFocusRangeScale(tdmFocusSettings_.blurStrength);
				effectiveSettings.focalLength = profile.focalLength;
				effectiveSettings.fNumber = profile.fNumber;
				effectiveSettings.nearFocusRangeMeters = 0.0F;
				const auto profileFocusRange = profile.distanceAdaptive ?
					std::clamp(profile.farFocusRangeMeters + focus->distanceMeters * 0.08F, 1.0F, 4.0F) :
					profile.farFocusRangeMeters;
				effectiveSettings.farFocusRangeMeters = std::clamp(
					profileFocusRange * focusRangeScale, 0.1F, 8.0F);
				effectiveSettings.farPlaneMaxBlur = std::clamp(
					profile.farPlaneMaxBlur * tdmFocusSettings_.blurStrength, 0.0F, 8.0F);
				effectiveSettings.nearPlaneMaxBlur = std::clamp(
					kTDMLockNearPlaneMaxBlur * tdmFocusSettings_.nearBlurStrength, 0.0F, 4.0F);
				activeTargetFocus = focus;
				targetFocusMode = TargetFocusMode::kTDMLock;
			}
		}
	}
	if (targetFocusMode == TargetFocusMode::kNone &&
		sneakFocusSettings_.enabled && !focusPickerActive_) {
		if (const auto* player = RE::PlayerCharacter::GetSingleton(); player && player->IsSneaking()) {
			effectiveSettings = settings_;
			effectiveSettings.enabled = true;
			effectiveSettings.autoFocus = true;
			effectiveSettings.focusX = 0.5F;
			effectiveSettings.focusY = 0.5F;
			effectiveSettings.autoFocusOffsetMeters = 0.0F;

			const auto profileIndex = std::min<std::size_t>(
				sneakFocusSettings_.profile, kSneakFocusProfiles.size() - 1U);
			const auto& profile = kSneakFocusProfiles[profileIndex];
			const auto focusRangeScale = GetFocusRangeScale(sneakFocusSettings_.blurStrength);
			effectiveSettings.focalLength = profile.focalLength;
			effectiveSettings.fNumber = GetSneakFNumber(
				profileIndex, profile.fNumber, sneakFocusSettings_.blurStrength);
			effectiveSettings.nearFocusRangeMeters = 0.0F;
			effectiveSettings.farFocusRangeMeters = std::clamp(
				profile.farFocusRangeMeters * focusRangeScale, 0.1F, 8.0F);
			const auto effectiveBlurStrength = GetSneakBlurStrength(
				profileIndex, sneakFocusSettings_.blurStrength);
			effectiveSettings.farPlaneMaxBlur = std::clamp(
				profile.farPlaneMaxBlur * effectiveBlurStrength, 0.0F, 8.0F);
			effectiveSettings.nearPlaneMaxBlur = 0.0F;
			effectiveSettings.nearFarDistanceCompensation = kSneakNearFarDistanceCompensation;
			targetFocusMode = TargetFocusMode::kSneak;
		}
	}
	const auto normalDoFIdle =
		targetFocusMode == TargetFocusMode::kNone && !modeSettings_.normalGameplayEnabled;
	if (normalDoFIdle) {
		if (targetFocusMode_ != TargetFocusMode::kNone) {
			targetFocusMode_ = TargetFocusMode::kNone;
			spdlog::info("Special focus mode ended; normal gameplay DoF is disabled");
		}
		if (dialogueOnlyPrewarmed_) {
			return;
		}
	}
    const auto frameFocus = followFrameSubject && !normalDoFIdle && targetFocusMode == TargetFocusMode::kNone ?
        GetTargetFocus(frameSubject, false) : std::nullopt;
	if (!normalDoFIdle &&
		targetFocusMode == TargetFocusMode::kNone && (frameFocus || targetFocusSettings_.consoleEnabled)) {
		const auto playerSource = !frameFocus && targetFocusSettings_.targetSource == TargetFocusSource::kPlayer;
		const auto camera = RE::PlayerCamera::GetSingleton();
		const auto useFirstPersonScreenAFFallback =
			playerSource &&
			camera && camera->IsInFirstPerson() &&
			targetFocusSettings_.useFirstPersonScreenAFFallback;
		if (useFirstPersonScreenAFFallback) {
			// Treat the first-person fallback as a real renderer mode instead of
			// patching the normal settings after target resolution. This keeps the
			// live first-person X/Y controls connected to the constants uploaded to
			// the autofocus shader while player tracking is intentionally paused.
			effectiveSettings = settings_;
			effectiveSettings.enabled = true;
			effectiveSettings.autoFocus = true;
			effectiveSettings.focusX = effectiveSettings.firstPersonFocusX;
			effectiveSettings.focusY = effectiveSettings.firstPersonFocusY;
			targetFocusMode = TargetFocusMode::kFirstPersonScreenAF;
		} else if (const auto focus = frameFocus ? frameFocus : (playerSource ? GetPlayerTargetFocus() : GetConsoleTargetFocus())) {
			effectiveSettings = settings_;
			effectiveSettings.enabled = true;
			// Match Screen AF: sample the same depth texture used by the CoC pass at
			// the projected target anchor.  This avoids mixing a camera-space distance
			// with a Community Shaders depth surface that can use a different mapping.
			effectiveSettings.autoFocus = focus->surfaceFocusValid;
			if (focus->surfaceFocusValid) {
				effectiveSettings.focusX = focus->focusCoordinate[0];
				effectiveSettings.focusY = focus->focusCoordinate[1];
				effectiveSettings.autoFocusOffsetMeters = targetFocusSettings_.targetFocusOffsetMeters;
			} else {
				effectiveSettings.manualFocusMeters = std::clamp(
					focus->distanceMeters + targetFocusSettings_.targetFocusOffsetMeters, 0.1F, 150.0F);
			}
			activeTargetFocus = focus;
			targetFocusMode = playerSource ? TargetFocusMode::kPlayer : TargetFocusMode::kConsole;
		}
	}
	if (targetFocusMode != targetFocusMode_) {
		const auto previousTargetFocusMode = targetFocusMode_;
		targetFocusMode_ = targetFocusMode;
		// Entering or leaving the first-person fallback replaces a tracked target
		// distance with a sampled screen position. Seed only that transition
		// immediately; preserve the established smoothing between all other modes.
		if (previousTargetFocusMode == TargetFocusMode::kFirstPersonScreenAF ||
			targetFocusMode_ == TargetFocusMode::kFirstPersonScreenAF) {
			resources_.focusInitialized = false;
		}
		switch (targetFocusMode_) {
		case TargetFocusMode::kDialogue:
			if (effectiveSettings.autoFocus) {
				spdlog::info("Dialogue target focus activated using visible-surface depth at ({:.3f}, {:.3f})",
					effectiveSettings.focusX, effectiveSettings.focusY);
			} else {
				spdlog::info("Dialogue target focus activated using projected anchor distance at {:.2f} m",
					effectiveSettings.manualFocusMeters);
			}
			break;
		case TargetFocusMode::kTDMLock:
			if (effectiveSettings.autoFocus) {
				spdlog::info(
					"TDM lock-on focus activated using visible-surface depth at ({:.3f}, {:.3f}); profile {}; dedicated background blur {:.2f}x; foreground blur {:.2f}x",
					effectiveSettings.focusX,
					effectiveSettings.focusY,
					tdmFocusSettings_.profile + 1U,
					tdmFocusSettings_.blurStrength,
					tdmFocusSettings_.nearBlurStrength);
			} else {
				spdlog::info(
					"TDM lock-on focus activated using projected anchor distance at {:.2f} m; profile {}; dedicated background blur {:.2f}x; foreground blur {:.2f}x",
					effectiveSettings.manualFocusMeters,
					tdmFocusSettings_.profile + 1U,
					tdmFocusSettings_.blurStrength,
					tdmFocusSettings_.nearBlurStrength);
			}
			break;
		case TargetFocusMode::kSneak:
			spdlog::info(
				"Sneak mode activated using center-screen depth; profile {}; strength {:.2f}x",
				sneakFocusSettings_.profile + 1U,
				sneakFocusSettings_.blurStrength);
			break;
		case TargetFocusMode::kPlayer:
			if (effectiveSettings.autoFocus) {
				spdlog::info("Player target focus activated using visible-surface depth at ({:.3f}, {:.3f}); offset {:+.2f} m",
					effectiveSettings.focusX, effectiveSettings.focusY, effectiveSettings.autoFocusOffsetMeters);
			} else {
				spdlog::info("Player target focus activated using projected anchor distance at {:.2f} m",
					effectiveSettings.manualFocusMeters);
			}
			break;
		case TargetFocusMode::kFirstPersonScreenAF:
			spdlog::info(
				"First-person Screen AF fallback activated at ({:.3f}, {:.3f}); offset {:+.2f} m",
				effectiveSettings.focusX,
				effectiveSettings.focusY,
				effectiveSettings.autoFocusOffsetMeters);
			break;
		case TargetFocusMode::kConsole:
			if (effectiveSettings.autoFocus) {
				spdlog::info("Console target focus activated using visible-surface depth at ({:.3f}, {:.3f}); offset {:+.2f} m",
					effectiveSettings.focusX, effectiveSettings.focusY, effectiveSettings.autoFocusOffsetMeters);
			} else {
				spdlog::info("Console target focus activated using projected anchor distance at {:.2f} m",
					effectiveSettings.manualFocusMeters);
			}
			break;
		case TargetFocusMode::kNone:
			spdlog::info("Target focus ended; restored normal settings");
			break;
		}
	}
	if (const auto camera = RE::PlayerCamera::GetSingleton(); camera && camera->IsInFirstPerson()) {
		// Ordinary Screen AF keeps independent framing coordinates for first and
		// third person. Target tracking's optional first-person fallback has
		// already selected these coordinates as its own renderer mode above.
		if (targetFocusMode == TargetFocusMode::kNone && effectiveSettings.autoFocus) {
			effectiveSettings.focusX = effectiveSettings.firstPersonFocusX;
			effectiveSettings.focusY = effectiveSettings.firstPersonFocusY;
		}
		if (!effectiveSettings.enableFirstPersonNearBlur) {
			effectiveSettings.nearPlaneMaxBlur = 0.0F;
		}
	}
	// With normal gameplay DoF disabled, idle frames normally skip the renderer. Force the
	// existing menu guard during its one-time preparation so shader compilation
	// and texture creation happen on the first safe gameplay frame, not in the
	// main/loading/map menu and before the first special-focus frame.
	auto preparationSettings = effectiveSettings;
	if (normalDoFIdle) {
		preparationSettings.disableInMenus = true;
	}
	if (!effectiveSettings.enabled || IsMenuBlocked(preparationSettings)) {
		return;
	}

	try {
		const auto renderer = RE::BSGraphics::Renderer::GetSingleton();
		if (!renderer) {
			return;
		}
		auto& rendererData = renderer->GetRuntimeData();
		auto device = reinterpret_cast<ID3D11Device*>(rendererData.forwarder);
		auto context = reinterpret_cast<ID3D11DeviceContext*>(rendererData.context);
		if (!device || !context) {
			return;
		}

		auto& mainTarget = rendererData.renderTargets[RE::RENDER_TARGETS::kMAIN];
		ComPtr<ID3D11RenderTargetView> currentRTV;
		ComPtr<ID3D11DepthStencilView> currentDSV;
		context->OMGetRenderTargets(1, currentRTV.GetAddressOf(), currentDSV.GetAddressOf());

		auto& depthStencils = renderer->GetDepthStencilData().depthStencils;
		auto mainDepth = depthStencils[RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN].depthSRV;
		auto depth = mainDepth;
		if (!mainTarget.texture || !mainTarget.SRV || !mainDepth) {
			return;
		}
		D3D11_TEXTURE2D_DESC inputDescription{};
		mainTarget.texture->GetDesc(&inputDescription);
		auto renderArea = GetActiveRenderArea(context, inputDescription);
		const auto* graphicsState = RE::BSGraphics::State::GetSingleton();
		// CS 1.9.1 leaves the half-resolution underwater-mask viewport bound
		// after upscaling the full-resolution scene color and main depth.
		// The DLSS resolve unbinds the RTV but leaves that viewport intact.
		const bool afterUpscale = graphicsState && GetGraphicsRuntimeData(*graphicsState).dynamicResolutionLock &&
			(!currentRTV || currentRTV.Get() == rendererData.renderTargets[RE::RENDER_TARGETS::kUNDERWATER_MASK].RTV) &&
			inputDescription.Width == graphicsState->screenWidth &&
			inputDescription.Height == graphicsState->screenHeight;
		if (afterUpscale) {
			renderArea = { 0U, 0U, inputDescription.Width, inputDescription.Height };
		}
		// The optional transparency modes are dual-depth experiments: the private
		// pre-opaque snapshot supplies the farther background depth used by the ordinary DoF path, while the current
		// main depth supplies the nearer transparent-subject surface used by the
		// protection mask. Bottled Shaders and newer Community Shaders overwrite the
		// shared post-Z-prepass target with completed opaque depth, which makes it
		// identical to mainDepth by the time this pass runs. Passing mainDepth for both inputs makes their difference
		// identically zero, so the live checkbox cannot have any visible effect.
		// Capture our own snapshot before opaque rendering instead of depending on
		// another renderer's later use of that shared target.
		if (experimentalTransparencySettings_.enabled) {
			auto experimentalDepth = preOpaqueDepthValid_ ? preOpaqueDepthSRV_.Get() : nullptr;
			const bool hairDepthActive = IsCompatibleDepth(experimentalDepth, inputDescription);
			if (hairDepthActive) {
				depth = experimentalDepth;
			}
			if (lastHairDepthActive_ != hairDepthActive && hairDepthDiagnosticCount_ < 32) {
				lastHairDepthActive_ = hairDepthActive;
				++hairDepthDiagnosticCount_;
				spdlog::info(
					"Hair depth diagnostic: active={}, captured={}, capture scale={:.4f}x{:.4f}, after upscale={}, render={}x{}",
					hairDepthActive, preOpaqueDepthValid_, preOpaqueDepthScale_[0], preOpaqueDepthScale_[1], afterUpscale,
					renderArea.width, renderArea.height);
			}
		} else {
			lastHairDepthActive_.reset();
		}
		if (!depthPathChecked_) {
			const auto runtimeSaoEnabled = ReadDisplayBool("bSAOEnable:Display");
			const auto runtimeReflectionsEnabled = ReadDisplayBool("bScreenSpaceReflectionEnabled:Display");
			const auto runtimeHdr64Enabled = ReadDisplayBool("bUse64bitsHDRRenderTarget:Display");
			const auto communityShadersLoaded =
				GetModuleHandleW(L"CommunityShaders.dll") != nullptr;
			const auto actualHdr64Target =
				inputDescription.Format == DXGI_FORMAT_R16G16B16A16_FLOAT;
			// The actual target format remains useful diagnostics, but Community Shaders
			// normally supplies its own non-64-bit target even when the game setting is on.
			// Do not treat that normal format as a reason to enable a visible subject mask.
			const auto standaloneHdrTargetGuardSetting =
				runtimeHdr64Enabled && !*runtimeHdr64Enabled;
			const auto standaloneTargetGuardSetting =
				(runtimeReflectionsEnabled && !*runtimeReflectionsEnabled) ||
				standaloneHdrTargetGuardSetting;
			useStandaloneNonActorTargetGuard_ =
				!communityShadersLoaded && standaloneTargetGuardSetting;
			useStandaloneNearFocusAssist_ =
				!communityShadersLoaded && standaloneHdrTargetGuardSetting;
			depthPathChecked_ = true;
			spdlog::info(
				"Display depth settings at first frame: SAO={}, SSR={}, 64-bit HDR={}; main target format={} (actual 64-bit HDR target={}); Community Shaders={}; selected main depth path; tracked-actor depth guard=unified; standalone non-actor guard={}; standalone HDR-off near-focus assist={}",
				DescribeDisplayBool(runtimeSaoEnabled),
				DescribeDisplayBool(runtimeReflectionsEnabled),
				DescribeDisplayBool(runtimeHdr64Enabled),
				static_cast<std::uint32_t>(inputDescription.Format),
				actualHdr64Target ? "yes" : "no",
				communityShadersLoaded ? "loaded" : "not loaded",
				useStandaloneNonActorTargetGuard_ ? "enabled" : "disabled",
				useStandaloneNearFocusAssist_ ? "enabled" : "disabled");
		}

		if (targetFocusMode == TargetFocusMode::kDialogue && !loggedRenderAreaDiagnostics_) {
			loggedRenderAreaDiagnostics_ = true;
			constexpr UINT kMaximumViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			std::array<D3D11_VIEWPORT, kMaximumViewports> viewports{};
			UINT viewportCount = kMaximumViewports;
			context->RSGetViewports(&viewportCount, viewports.data());

			std::array<D3D11_RECT, kMaximumViewports> scissorRects{};
			UINT scissorCount = kMaximumViewports;
			context->RSGetScissorRects(&scissorCount, scissorRects.data());

			const auto omDescription = GetTextureDescription(currentRTV.Get());
			const auto depthDescription = GetTextureDescription(depth);
			const auto viewport = viewportCount > 0 ? viewports[0] : D3D11_VIEWPORT{};
			const auto scissor = scissorCount > 0 ? scissorRects[0] : D3D11_RECT{};
			spdlog::info(
				"Conversation render diagnostics: main={}x{} fmt={}; OM={}x{} fmt={}; depth={}x{} fmt={}; viewportCount={} first=({:.1f},{:.1f}) {:.1f}x{:.1f} depth={:.3f}..{:.3f}; scissorCount={} first=({},{})->({},{})",
				inputDescription.Width,
				inputDescription.Height,
				static_cast<std::uint32_t>(inputDescription.Format),
				omDescription ? omDescription->Width : 0U,
				omDescription ? omDescription->Height : 0U,
				omDescription ? static_cast<std::uint32_t>(omDescription->Format) : 0U,
				depthDescription ? depthDescription->Width : 0U,
				depthDescription ? depthDescription->Height : 0U,
				depthDescription ? static_cast<std::uint32_t>(depthDescription->Format) : 0U,
				viewportCount,
				viewport.TopLeftX,
				viewport.TopLeftY,
				viewport.Width,
				viewport.Height,
				viewport.MinDepth,
				viewport.MaxDepth,
				scissorCount,
				scissor.left,
				scissor.top,
				scissor.right,
				scissor.bottom);
		}

		const auto preparationStarted = std::chrono::steady_clock::now();
		if (inputDescription.SampleDesc.Count != 1 ||
			!EnsureResources(device, inputDescription, renderArea.width, renderArea.height)) {
			permanentlyDisabled_ = true;
			spdlog::critical("Depth of field disabled because GPU resources could not be created");
			return;
		}
		if (normalDoFIdle) {
			dialogueOnlyPrewarmed_ = true;
			const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - preparationStarted);
			spdlog::info(
				"DoF renderer prewarm completed in {} ms; no DoF frame was rendered",
				elapsed.count());
			return;
		}
		ID3D11RenderTargetView* restoreRTV = currentRTV.Get();
		OutputMergerRestore restore{ context, restoreRTV, currentDSV.Get() };
		context->OMSetRenderTargets(0, nullptr, nullptr);
		const auto* validTargetFocus = activeTargetFocus ?
			std::addressof(*activeTargetFocus) : nullptr;
		if (validTargetFocus && validTargetFocus->actor && validTargetFocus->guardValid) {
			activeDepthFrameTarget_ = *validTargetFocus;
		}
		// Actor protection now follows one depth-confirmed path in every display and
		// Community Shaders configuration.  Preserve the legacy standalone guard for
		// non-actor console targets only in the low-spec configurations that required it.
		const auto actorTargetGuard = validTargetFocus && validTargetFocus->actor;
		const auto standaloneNonActorTargetGuard =
			validTargetFocus && !validTargetFocus->actor && useStandaloneNonActorTargetGuard_;
		const auto* targetGuard =
			validTargetFocus && validTargetFocus->guardValid &&
				(actorTargetGuard || standaloneNonActorTargetGuard) ?
				validTargetFocus : nullptr;
		float targetNearFocusMinimumMeters{};
		const char* targetNearFocusAssistName = nullptr;
		if (validTargetFocus && useStandaloneNearFocusAssist_) {
			targetNearFocusMinimumMeters = kStandaloneNearFocusMinimumMeters;
			targetNearFocusAssistName = "Standalone HDR-off";
		}
		if (targetGuard && !loggedTargetGuard_) {
			loggedTargetGuard_ = true;
			spdlog::info(
				"{} depth-confirmed target protection activated (body centre {:.3f}, {:.3f}; radius {:.3f}, {:.3f}; head centre {:.3f}, {:.3f}; radius {:.3f})",
				targetGuard->actor ? "Tracked actor" : "Standalone non-actor",
				targetGuard->guardCenter[0], targetGuard->guardCenter[1],
				targetGuard->guardRadius[0], targetGuard->guardRadius[1],
				targetGuard->headGuardCenter[0], targetGuard->headGuardCenter[1],
				targetGuard->headGuardValid ? targetGuard->headGuardRadius : 0.0F);
		}
		if (targetNearFocusMinimumMeters > 0.0F && !loggedTargetNearFocusAssist_) {
			loggedTargetNearFocusAssist_ = true;
			spdlog::info(
				"{} near-focus assist activated (saved {:.2f} m; minimum {:.2f} m; effective {:.2f} m)",
				targetNearFocusAssistName,
				effectiveSettings.nearFocusRangeMeters,
				targetNearFocusMinimumMeters,
				std::max(effectiveSettings.nearFocusRangeMeters, targetNearFocusMinimumMeters));
		}
		Dispatch(
			context,
			mainTarget.SRV,
			depth,
			mainDepth,
			effectiveSettings,
			targetGuard,
			targetNearFocusMinimumMeters,
			afterUpscale && depth != mainDepth ? preOpaqueDepthScale_ : std::array<float, 2>{ 1.0F, 1.0F },
			inputDescription.Width,
			inputDescription.Height,
			renderArea.left,
			renderArea.top);
		const D3D11_BOX outputBox{
			0U,
			0U,
			0U,
			renderArea.width,
			renderArea.height,
			1U
		};
		context->CopySubresourceRegion(
			mainTarget.texture,
			0,
			renderArea.left,
			renderArea.top,
			0,
			resources_.output.resource.Get(),
			0,
			&outputBox);
		if (!loggedFirstFrame_) {
			loggedFirstFrame_ = true;
			spdlog::info("First depth-of-field frame applied successfully");
		}
	} catch (const std::exception& error) {
		permanentlyDisabled_ = true;
		spdlog::critical("Depth-of-field render exception; effect disabled: {}", error.what());
	} catch (...) {
		permanentlyDisabled_ = true;
		spdlog::critical("Unknown depth-of-field render exception; effect disabled");
	}
}

void CDoF::DoFRenderer::Dispatch(
	ID3D11DeviceContext* a_context,
	ID3D11ShaderResourceView* a_color,
	ID3D11ShaderResourceView* a_depth,
	ID3D11ShaderResourceView* a_skyMaskDepth,
	const Settings& a_settings,
	const TargetFocusSample* a_targetGuard,
	float a_targetNearFocusMinimumMeters,
	const std::array<float, 2>& a_depthScale,
	std::uint32_t a_inputWidth,
	std::uint32_t a_inputHeight,
	std::uint32_t a_renderLeft,
	std::uint32_t a_renderTop)
{
	const auto requestedTransparencyMode = experimentalTransparencySettings_.enabled ?
		std::clamp(experimentalTransparencySettings_.mode, 1U, 2U) : 0U;
	const Shaders* activeShaders = std::addressof(shaders_);
	std::uint32_t activeTransparencyMode{};
	if (requestedTransparencyMode == 1U && test6ShadersReady_) {
		activeShaders = std::addressof(test6Shaders_);
		activeTransparencyMode = 1U;
	} else if (requestedTransparencyMode == 2U && test8ShadersReady_) {
		activeShaders = std::addressof(test8Shaders_);
		activeTransparencyMode = 2U;
	}
	const auto effectiveNearFocusRangeMeters =
		a_targetNearFocusMinimumMeters > 0.0F ?
			std::max(a_settings.nearFocusRangeMeters, a_targetNearFocusMinimumMeters) :
			a_settings.nearFocusRangeMeters;
	const DoFConstants dofData{
		.transitionSpeed = resources_.focusInitialized ? a_settings.transitionSpeed : 1.0F,
		.focusCoordinate = { a_settings.focusX, a_settings.focusY },
		.manualFocusPlane = a_settings.manualFocusMeters / 1000.0F,
		.focalLength = a_settings.focalLength,
		.fNumber = a_settings.fNumber,
		.farPlaneMaxBlur = a_settings.farPlaneMaxBlur,
		.nearPlaneMaxBlur = a_settings.nearPlaneMaxBlur,
		.blurQuality = a_settings.blurQuality,
		.nearFarDistanceCompensation = a_settings.nearFarDistanceCompensation,
		.bokehBusyFactor = a_settings.bokehBusyFactor,
		.highlightBoost = a_settings.highlightBoost,
		.postBlurSmoothing = a_settings.postBlurSmoothing,
		.highlightShape = a_settings.apertureBokeh ? 1U : 0U,
		.highlightShapeRotationAngle = a_settings.apertureRotationDegrees / 360.0F,
		.petzvalStrength = a_settings.petzvalStrength,
		.autoFocus = a_settings.autoFocus ? 1U : 0U,
		.autoFocusOffsetPlane = a_settings.autoFocusOffsetMeters / 1000.0F,
		.targetGuardEnabled = a_targetGuard ? 1U : 0U,
		.apertureShapeStrength = a_settings.apertureShapeStrength,
		.targetGuardCenter = a_targetGuard ?
			Float2{ a_targetGuard->guardCenter[0], a_targetGuard->guardCenter[1] } : Float2{ 0.5F, 0.5F },
		.targetGuardRadius = a_targetGuard ?
			Float2{ a_targetGuard->guardRadius[0], a_targetGuard->guardRadius[1] } : Float2{ 1.0F, 1.0F },
		.nearFocusRangeMeters = effectiveNearFocusRangeMeters,
		.farFocusRangeMeters = a_settings.farFocusRangeMeters,
		.apertureBlades = std::clamp(a_settings.apertureBlades, 3U, 12U),
		.apertureRoundness = std::clamp(a_settings.apertureRoundness, 0.0F, 1.0F),
		.headGuardCenter = a_targetGuard && a_targetGuard->headGuardValid ?
			Float2{ a_targetGuard->headGuardCenter[0], a_targetGuard->headGuardCenter[1] } : Float2{ 0.5F, 0.5F },
		.headGuardRadius = a_targetGuard && a_targetGuard->headGuardValid ?
			a_targetGuard->headGuardRadius : 0.0F,
		.keepSkySharp = a_settings.keepSkySharp ? 1U : 0U,
		.targetGuardAxis = a_targetGuard ?
			Float2{ a_targetGuard->guardAxis[0], a_targetGuard->guardAxis[1] } : Float2{},
		.experimentalTransparencyMode = activeTransparencyMode,
		// Bit 0 enables the dense filled gather; bits 1-2 select the procedural shape.
		.apertureOptions = (a_settings.apertureBokeh && a_settings.apertureFill ? 1U : 0U) |
			(std::clamp(a_settings.apertureShape, 0U, 2U) << 1U),
		.starburstEnabled = a_settings.starburstEnabled ? 1U : 0U,
		.starburstRays = std::clamp(a_settings.starburstRays, 4U, 12U),
		.starburstStrength = std::clamp(a_settings.starburstStrength, 0.0F, 16.0F),
		.starburstLength = std::clamp(a_settings.starburstLength, 0.05F, 1.5F),
		.highlightExtractionThreshold = std::clamp(a_settings.highlightExtractionThreshold, 0.20F, 1.50F),
		.starburstRotationAngle = std::clamp(a_settings.starburstRotationDegrees, 0.0F, 180.0F) / 360.0F,
		.bokehCoreFill = std::clamp(a_settings.bokehCoreFill, 0.0F, 2.0F),
		.starburstWideFlareSuppression = std::clamp(a_settings.starburstWideFlareSuppression, 0.0F, 1.0F),
		.starburstStrongLightSuppression = std::clamp(a_settings.starburstStrongLightSuppression, 0.0F, 1.0F),
		.starburstHighlightSensitivity = std::clamp(a_settings.starburstHighlightSensitivity, 1.0F, 4.0F),
		.starburstPadding = {}
	};
	const SharedConstants sharedData{
		.cameraData = GetCameraData(),
		.bufferDimensions = {
			static_cast<float>(resources_.width),
			static_cast<float>(resources_.height),
			1.0F / static_cast<float>(resources_.width),
			1.0F / static_cast<float>(resources_.height) },
		.inputRegion = {
			static_cast<float>(a_renderLeft),
			static_cast<float>(a_renderTop),
			1.0F / static_cast<float>(a_inputWidth),
			1.0F / static_cast<float>(a_inputHeight) },
		.reverseZ = reverseZDepth_ ? 1U : 0U,
		.depthScale = a_depthScale,
		.padding = {}
	};

	if (!UpdateConstantBuffer(a_context, resources_.dofConstants.Get(), &dofData, sizeof(dofData)) ||
		!UpdateConstantBuffer(a_context, resources_.sharedConstants.Get(), &sharedData, sizeof(sharedData))) {
		return;
	}

	ID3D11Buffer* dofCB = resources_.dofConstants.Get();
	ID3D11Buffer* sharedCB = resources_.sharedConstants.Get();
	ID3D11SamplerState* sampler = resources_.linearSampler.Get();
	a_context->CSSetConstantBuffers(1, 1, &dofCB);
	a_context->CSSetConstantBuffers(5, 1, &sharedCB);
	a_context->CSSetSamplers(0, 1, &sampler);

	std::array<ID3D11ShaderResourceView*, 12> srvs{};
	std::array<ID3D11UnorderedAccessView*, 3> uavs{};
	const auto resetViews = [&]() {
		srvs.fill(nullptr);
		uavs.fill(nullptr);
		a_context->CSSetShaderResources(0, static_cast<UINT>(srvs.size()), srvs.data());
		a_context->CSSetUnorderedAccessViews(0, static_cast<UINT>(uavs.size()), uavs.data(), nullptr);
	};
	const auto bindAndDispatch = [&](ID3D11ComputeShader* a_shader, UINT a_width, UINT a_height) {
		a_context->CSSetShaderResources(0, static_cast<UINT>(srvs.size()), srvs.data());
		a_context->CSSetUnorderedAccessViews(0, static_cast<UINT>(uavs.size()), uavs.data(), nullptr);
		a_context->CSSetShader(a_shader, nullptr, 0);
		a_context->Dispatch(a_width, a_height, 1);
	};

	const UINT fullWidth = (resources_.width + 7U) >> 3U;
	const UINT fullHeight = (resources_.height + 7U) >> 3U;
	const UINT halfPixelsX = std::max(1U, resources_.width / 2U);
	const UINT halfPixelsY = std::max(1U, resources_.height / 2U);
	const UINT quarterPixelsX = std::max(1U, (halfPixelsX + 1U) / 2U);
	const UINT quarterPixelsY = std::max(1U, (halfPixelsY + 1U) / 2U);
	const UINT eighthPixelsX = std::max(1U, (quarterPixelsX + 1U) / 2U);
	const UINT eighthPixelsY = std::max(1U, (quarterPixelsY + 1U) / 2U);
	const UINT sixteenthPixelsX = std::max(1U, (eighthPixelsX + 1U) / 2U);
	const UINT sixteenthPixelsY = std::max(1U, (eighthPixelsY + 1U) / 2U);
	const UINT halfWidth = (halfPixelsX + 7U) >> 3U;
	const UINT halfHeight = (halfPixelsY + 7U) >> 3U;
	const UINT quarterWidth = (quarterPixelsX + 7U) >> 3U;
	const UINT quarterHeight = (quarterPixelsY + 7U) >> 3U;
	const UINT eighthWidth = (eighthPixelsX + 7U) >> 3U;
	const UINT eighthHeight = (eighthPixelsY + 7U) >> 3U;
	const UINT sixteenthWidth = (sixteenthPixelsX + 7U) >> 3U;
	const UINT sixteenthHeight = (sixteenthPixelsY + 7U) >> 3U;

	// Update focus.
	srvs[0] = a_color;
	srvs[1] = resources_.previousFocus.srv.Get();
	srvs[2] = a_depth;
	uavs[1] = resources_.focus.uav.Get();
	bindAndDispatch(activeShaders->updateFocus.Get(), 1, 1);
	resetViews();
	a_context->CopyResource(resources_.previousFocus.resource.Get(), resources_.focus.resource.Get());
	resources_.focusInitialized = true;

	// Calculate circle of confusion.
	srvs[0] = a_color;
	srvs[1] = resources_.previousFocus.srv.Get();
	srvs[2] = a_depth;
	// Keep autofocus, ordinary DoF calculations, and sky classification on the
	// full-resolution main depth so transparent subjects retain the same depth
	// behavior across Community Shaders and display-quality configurations.
	srvs[11] = a_skyMaskDepth;
	uavs[2] = resources_.coc.uav.Get();
	bindAndDispatch(activeShaders->calculateCoC.Get(), fullWidth, fullHeight);
	resetViews();

	// Tile and expand CoC.
	srvs[3] = resources_.coc.srv.Get();
	uavs[2] = resources_.cocTileTemporary.uav.Get();
	bindAndDispatch(activeShaders->cocTile1.Get(), fullWidth, fullHeight);
	resetViews();
	srvs[3] = resources_.cocTileTemporary.srv.Get();
	uavs[2] = resources_.cocTileTemporary2.uav.Get();
	bindAndDispatch(activeShaders->cocTile2.Get(), fullWidth, fullHeight);
	resetViews();
	srvs[3] = resources_.cocTileTemporary2.srv.Get();
	uavs[2] = resources_.cocTileNeighbor.uav.Get();
	bindAndDispatch(activeShaders->cocTileNeighbor.Get(), fullWidth, fullHeight);
	resetViews();

	// Half-resolution CoC Gaussian blur.
	srvs[3] = resources_.cocTileNeighbor.srv.Get();
	uavs[2] = resources_.cocBlur1.uav.Get();
	bindAndDispatch(activeShaders->cocGaussian1.Get(), halfWidth, halfHeight);
	resetViews();
	srvs[3] = resources_.cocBlur1.srv.Get();
	uavs[2] = resources_.cocBlur2.uav.Get();
	bindAndDispatch(activeShaders->cocGaussian2.Get(), halfWidth, halfHeight);
	resetViews();

	// Pre-, far-, and near-plane blur. Aperture shaping is calculated directly
	// from the saved blade count, roundness, strength, and rotation constants.
	srvs[0] = a_color;
	srvs[3] = resources_.coc.srv.Get();
	srvs[4] = resources_.cocBlur2.srv.Get();
	// Pre-blur performs the general bright-bokeh amplification. Expose main depth
	// so clear sky can bypass that amplification when Keep Sky Sharp is enabled.
	srvs[1] = resources_.previousFocus.srv.Get();
	srvs[2] = a_depth;
	srvs[11] = a_skyMaskDepth;
	uavs[0] = resources_.preBlurred.uav.Get();
	bindAndDispatch(activeShaders->blur.Get(), halfWidth, halfHeight);
	resetViews();
	// Build a compact color pyramid only for the far-plane gather. Large blur
	// kernels select a coarser source so each fixed gather tap represents a
	// wider, contiguous footprint instead of leaving visible gaps.
	srvs[0] = resources_.preBlurred.srv.Get();
	uavs[0] = resources_.farGatherColor1.uav.Get();
	bindAndDispatch(activeShaders->reduceColor.Get(), quarterWidth, quarterHeight);
	resetViews();
	srvs[0] = resources_.farGatherColor1.srv.Get();
	uavs[0] = resources_.farGatherColor2.uav.Get();
	bindAndDispatch(activeShaders->reduceColor.Get(), eighthWidth, eighthHeight);
	resetViews();
	srvs[0] = resources_.farGatherColor2.srv.Get();
	uavs[0] = resources_.farGatherColor3.uav.Get();
	bindAndDispatch(activeShaders->reduceColor.Get(), sixteenthWidth, sixteenthHeight);
	resetViews();
	srvs[0] = resources_.preBlurred.srv.Get();
	srvs[8] = resources_.farGatherColor1.srv.Get();
	srvs[9] = resources_.farGatherColor2.srv.Get();
	srvs[10] = resources_.farGatherColor3.srv.Get();
	// Far gather performs a generic one-sided depth discontinuity test so that
	// nearer surface colour is not spread farther into background pixels.
	srvs[2] = a_depth;
	srvs[3] = resources_.coc.srv.Get();
	srvs[4] = resources_.cocBlur2.srv.Get();
	// Keep ordinary far-blur sampling unchanged, but expose the existing main
	// depth so the shader can reject clear-sky samples from highlight extraction.
	srvs[11] = a_skyMaskDepth;
	uavs[0] = resources_.farBlurred.uav.Get();
	bindAndDispatch(activeShaders->farBlur.Get(), halfWidth, halfHeight);
	resetViews();
	srvs[0] = resources_.farBlurred.srv.Get();
	srvs[3] = resources_.cocTileNeighbor.srv.Get();
	srvs[4] = resources_.cocBlur2.srv.Get();
	// Near highlights use the same sky classification as the far gather.
	srvs[11] = a_skyMaskDepth;
	uavs[0] = resources_.nearBlurred.uav.Get();
	bindAndDispatch(activeShaders->nearBlur.Get(), halfWidth, halfHeight);
	resetViews();

	// Tent filter and combine.
	srvs[0] = resources_.farBlurred.srv.Get();
	uavs[0] = resources_.blurredFiltered.uav.Get();
	bindAndDispatch(activeShaders->tentFilter.Get(), halfWidth, halfHeight);
	resetViews();
	srvs[0] = a_color;
	// CS_Combiner re-evaluates the target/focus-range depth protections.  Bind
	// the same focus and depth inputs used by CS_CalculateCoC; leaving t1/t2
	// unbound makes both sample as zero and turns the spatial target ellipse
	// into an unconditional sharp window on the Low depth path.
	srvs[1] = resources_.previousFocus.srv.Get();
	srvs[2] = a_depth;
	srvs[3] = resources_.coc.srv.Get();
	srvs[5] = resources_.blurredFiltered.srv.Get();
	srvs[6] = resources_.nearBlurred.srv.Get();
	srvs[11] = a_skyMaskDepth;
	uavs[0] = resources_.postSmooth.uav.Get();
	bindAndDispatch(activeShaders->combiner.Get(), fullWidth, fullHeight);
	resetViews();

	// Final smoothing and focusing.
	srvs[0] = resources_.postSmooth.srv.Get();
	srvs[3] = resources_.coc.srv.Get();
	uavs[0] = resources_.postSmooth2.uav.Get();
	bindAndDispatch(activeShaders->postSmoothing1.Get(), fullWidth, fullHeight);
	resetViews();
	srvs[0] = resources_.postSmooth.srv.Get();
	// Mode 2 derives its final detail-recovery confidence from the previous focus,
	// selected post-Z depth, and current main depth. Mode 1 deliberately omits
	// that recovery in HLSL to reproduce the useful soft Test2 appearance without
	// relying on unbound resources. The stable shader ignores these bindings.
	srvs[1] = resources_.previousFocus.srv.Get();
	srvs[2] = a_depth;
	srvs[3] = resources_.coc.srv.Get();
	srvs[7] = resources_.postSmooth2.srv.Get();
	// The experimental final pass reads the untouched scene for detail bands and
	// the aligned half-resolution transparency mask. OFF ignores both bindings.
	srvs[8] = a_color;
	srvs[9] = resources_.preBlurred.srv.Get();
	srvs[11] = a_skyMaskDepth;
	uavs[0] = resources_.output.uav.Get();
	bindAndDispatch(activeShaders->postSmoothing2AndFocusing.Get(), fullWidth, fullHeight);
	resetViews();

	ID3D11Buffer* nullBuffer = nullptr;
	ID3D11SamplerState* nullSampler = nullptr;
	a_context->CSSetConstantBuffers(1, 1, &nullBuffer);
	a_context->CSSetConstantBuffers(5, 1, &nullBuffer);
	a_context->CSSetSamplers(0, 1, &nullSampler);
	a_context->CSSetShader(nullptr, nullptr, 0);
}

bool CDoF::DoFRenderer::DispatchSilhouette(
	ID3D11DeviceContext* a_context,
	ID3D11ShaderResourceView* a_skyMaskDepth,
	const SilhouetteSettings& a_settings,
	std::uint32_t a_inputWidth,
	std::uint32_t a_inputHeight,
	std::uint32_t a_renderLeft,
	std::uint32_t a_renderTop)
{
	const SilhouetteConstants silhouetteData{
		.foregroundColor = {
			std::clamp(a_settings.foregroundColor[0], 0.0F, 1.0F),
			std::clamp(a_settings.foregroundColor[1], 0.0F, 1.0F),
			std::clamp(a_settings.foregroundColor[2], 0.0F, 1.0F),
			1.0F },
		.backgroundColor = {
			std::clamp(a_settings.backgroundColor[0], 0.0F, 1.0F),
			std::clamp(a_settings.backgroundColor[1], 0.0F, 1.0F),
			std::clamp(a_settings.backgroundColor[2], 0.0F, 1.0F),
			1.0F },
		.smoothEdges = a_settings.smoothEdges ? 1U : 0U,
		.padding = {}
	};
	const SharedConstants sharedData{
		.cameraData = GetCameraData(),
		.bufferDimensions = {
			static_cast<float>(resources_.width),
			static_cast<float>(resources_.height),
			1.0F / static_cast<float>(resources_.width),
			1.0F / static_cast<float>(resources_.height) },
		.inputRegion = {
			static_cast<float>(a_renderLeft),
			static_cast<float>(a_renderTop),
			1.0F / static_cast<float>(a_inputWidth),
			1.0F / static_cast<float>(a_inputHeight) },
		.reverseZ = reverseZDepth_ ? 1U : 0U,
		.padding = {}
	};

	if (!UpdateConstantBuffer(
			a_context,
			resources_.silhouetteConstants.Get(),
			&silhouetteData,
			sizeof(silhouetteData)) ||
		!UpdateConstantBuffer(
			a_context,
			resources_.sharedConstants.Get(),
			&sharedData,
			sizeof(sharedData))) {
		return false;
	}

	ID3D11Buffer* silhouetteCB = resources_.silhouetteConstants.Get();
	ID3D11Buffer* sharedCB = resources_.sharedConstants.Get();
	ID3D11UnorderedAccessView* output = resources_.output.uav.Get();
	a_context->CSSetConstantBuffers(2, 1, &silhouetteCB);
	a_context->CSSetConstantBuffers(5, 1, &sharedCB);
	a_context->CSSetShaderResources(11, 1, &a_skyMaskDepth);
	a_context->CSSetUnorderedAccessViews(0, 1, &output, nullptr);
	a_context->CSSetShader(silhouetteShader_.Get(), nullptr, 0);
	a_context->Dispatch((resources_.width + 7U) >> 3U, (resources_.height + 7U) >> 3U, 1);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	ID3D11Buffer* nullBuffer = nullptr;
	a_context->CSSetShaderResources(11, 1, &nullSRV);
	a_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	a_context->CSSetConstantBuffers(2, 1, &nullBuffer);
	a_context->CSSetConstantBuffers(5, 1, &nullBuffer);
	a_context->CSSetShader(nullptr, nullptr, 0);
	return true;
}

bool CDoF::DoFRenderer::DispatchDepthFrame(
	ID3D11DeviceContext* a_context,
	ID3D11ShaderResourceView* a_color,
	ID3D11ShaderResourceView* a_depth,
	ID3D11ShaderResourceView* a_hairDepth,
	ID3D11ShaderResourceView* a_stencil,
	const DepthFrameSettings& a_settings,
	const TargetFocusSample* a_target,
	std::uint32_t a_inputWidth,
	std::uint32_t a_inputHeight,
	std::uint32_t a_renderLeft,
	std::uint32_t a_renderTop)
{
	DepthFrameConstants frameData{
		.color = {
			std::clamp(a_settings.color[0], 0.0F, 1.0F),
			std::clamp(a_settings.color[1], 0.0F, 1.0F),
			std::clamp(a_settings.color[2], 0.0F, 1.0F),
			1.0F },
		.frameParameters = {
			std::clamp(a_settings.sideSize, 0.0F, 0.45F),
			std::clamp(a_settings.topBottomSize, 0.0F, 0.45F),
			std::clamp(a_settings.opacity, 0.0F, 1.0F),
			std::clamp(a_settings.edgeSoftness, 0.0F, 0.03F) },
		.guardCenterRadius = a_target ?
			std::array<float, 4>{
				a_target->guardCenter[0], a_target->guardCenter[1],
				a_target->guardRadius[0], a_target->guardRadius[1] } :
			std::array<float, 4>{ 0.5F, 0.5F, 0.0F, 0.0F },
		.guardAxisExpansion = a_target ?
			std::array<float, 4>{
				a_target->guardAxis[0], a_target->guardAxis[1],
				std::clamp(a_settings.subjectExpansion, 0.0F, kSubjectExpansionMax),
				a_target->distanceMeters } :
			std::array<float, 4>{},
		.headData = a_target && a_target->headGuardValid ?
			std::array<float, 4>{
				a_target->headGuardCenter[0], a_target->headGuardCenter[1],
				a_target->headGuardRadius, 0.0F } :
			std::array<float, 4>{ 0.5F, 0.5F, 0.0F, 0.0F },
			.options = {
			std::min(a_settings.style, 2U),
			a_settings.subjectBreakout ? 1U : 0U,
			// Test22: CoC silhouette inside a generous actor envelope.
			silhouetteSettings_.enabled && silhouetteSettings_.cocMonochrome ? 7U : (SubjectMask::GetSRV() ? 6U : 0U),
			a_target ? std::min<std::uint32_t>(
				a_target->guardSegmentCount,
				static_cast<std::uint32_t>(TargetFocusSample::kMaximumGuardSegments)) : 0U },
		.guardSegments = {},
		.guardSegmentRadii = {},
		.edgeAdjustment = {
            std::clamp(a_settings.subjectInsetPixels, 0.0F, 4.0F),
            std::cos(a_settings.frameRotationDegrees * 0.01745329252F),
            std::sin(a_settings.frameRotationDegrees * 0.01745329252F),
            static_cast<float>(15U ^ (a_settings.breakoutEdges & 15U)) }
	};
	if (frameData.options[0] == 2U) {
        frameData.guardCenterRadius = {
            std::clamp(a_settings.frameTop, 0.0F, 0.45F),
            std::clamp(a_settings.frameBottom, 0.0F, 0.45F),
            std::clamp(a_settings.frameLeft, 0.0F, 0.45F),
            std::clamp(a_settings.frameRight, 0.0F, 0.45F) };
    }
    if (SubjectMask::GetSRV()) {
        frameData.headData = SubjectMask::GetViewport();
        frameData.guardAxisExpansion[2] = std::clamp(a_settings.subjectExpansion, 0.0F, kSubjectExpansionMax);
    }
	if (a_target) {
		for (std::uint32_t index = 0; index < frameData.options[3]; ++index) {
			const auto& segment = a_target->guardSegments[index];
			frameData.guardSegments[index] = {
				segment.start[0], segment.start[1], segment.end[0], segment.end[1] };
			frameData.guardSegmentRadii[index / 4U][index % 4U] = segment.radius;
		}
	}
	if (SubjectMask::GetSRV()) frameData.options[3] = SubjectMask::GetLayerCount();
	const SharedConstants sharedData{
		.cameraData = GetCameraData(),
		.bufferDimensions = {
			static_cast<float>(depthFrameResources_.width), static_cast<float>(depthFrameResources_.height),
			1.0F / static_cast<float>(depthFrameResources_.width),
			1.0F / static_cast<float>(depthFrameResources_.height) },
		.inputRegion = {
			static_cast<float>(a_renderLeft), static_cast<float>(a_renderTop),
			1.0F / static_cast<float>(a_inputWidth),
			1.0F / static_cast<float>(a_inputHeight) },
		.reverseZ = reverseZDepth_ ? 1U : 0U,
		.padding = {}
	};
	if (!UpdateConstantBuffer(a_context, depthFrameResources_.constants.Get(), &frameData, sizeof(frameData)) ||
		!UpdateConstantBuffer(a_context, depthFrameResources_.sharedConstants.Get(), &sharedData, sizeof(sharedData))) {
		return false;
	}

	ID3D11Buffer* frameCB = depthFrameResources_.constants.Get();
	ID3D11Buffer* sharedCB = depthFrameResources_.sharedConstants.Get();
	ID3D11UnorderedAccessView* output = depthFrameResources_.output.uav.Get();
	a_context->CSSetConstantBuffers(3, 1, &frameCB);
	a_context->CSSetConstantBuffers(5, 1, &sharedCB);
	a_context->CSSetShaderResources(0, 1, &a_color);
	// CoC monochrome uses main depth. The expanded edge can compare both depth inputs.
	ID3D11Buffer* dofCB = resources_.dofConstants.Get();
	ID3D11SamplerState* sampler = resources_.linearSampler.Get();
	ID3D11ShaderResourceView* focusInputs[3]{
		resources_.previousFocus.srv.Get(),
		a_hairDepth && frameData.options[2] == 6U ? a_hairDepth : a_depth,
		resources_.coc.srv.Get() };
	a_context->CSSetConstantBuffers(1, 1, &dofCB);
	a_context->CSSetSamplers(0, 1, &sampler);
	a_context->CSSetShaderResources(1, 3, focusInputs);
	a_context->CSSetShaderResources(11, 1, &a_depth);
	ID3D11ShaderResourceView* maskLayers[4]{ a_stencil, SubjectMask::GetSRV(1), SubjectMask::GetSRV(2), SubjectMask::GetSRV(3) };
    a_context->CSSetShaderResources(12, 4, maskLayers);
	a_context->CSSetUnorderedAccessViews(0, 1, &output, nullptr);
	a_context->CSSetShader(depthFrameShader_.Get(), nullptr, 0);
	a_context->Dispatch(
		(depthFrameResources_.width + 7U) >> 3U,
		(depthFrameResources_.height + 7U) >> 3U,
		1);

	ID3D11ShaderResourceView* nullSRVs[3]{};
 a_context->CSSetShaderResources(1, 3, nullSRVs);
 ID3D11SamplerState* nullSampler = nullptr;
 a_context->CSSetSamplers(0, 1, &nullSampler);
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	ID3D11Buffer* nullBuffer = nullptr;
	a_context->CSSetShaderResources(0, 1, &nullSRVs[0]);
	a_context->CSSetShaderResources(11, 1, &nullSRVs[1]);
	ID3D11ShaderResourceView* nullMaskLayers[4]{};
    a_context->CSSetShaderResources(12, 4, nullMaskLayers);
	a_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	a_context->CSSetConstantBuffers(1, 1, &nullBuffer);
	a_context->CSSetConstantBuffers(3, 1, &nullBuffer);
	a_context->CSSetConstantBuffers(5, 1, &nullBuffer);
	a_context->CSSetShader(nullptr, nullptr, 0);
	return true;
}
