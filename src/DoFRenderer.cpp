#include "DoFRenderer.h"

#include "ShaderCompiler.h"

namespace
{
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
		std::uint32_t padding3;
		Float2 targetGuardAxis;
		std::uint32_t padding4[2];
	};
	static_assert(sizeof(DoFConstants) == 144);

	struct alignas(16) SharedConstants
	{
		std::array<float, 4> cameraData;
		std::array<float, 4> bufferDimensions;
		std::array<float, 4> inputRegion;
	};
	static_assert(sizeof(SharedConstants) == 48);

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

void CDoF::DoFRenderer::SetTargetFocus(
	TargetFocusSettings a_settings,
	Settings a_dialogueLensSettings)
{
	std::scoped_lock lock(mutex_);
	targetFocusSettings_ = a_settings;
	dialogueLensSettings_ = a_dialogueLensSettings;
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
	const std::filesystem::path path = L"Data\\Shaders\\CinematicDoFStandalone\\DoF\\dof.cs.hlsl";
	const std::array entries{
		std::pair{ std::addressof(shaders_.updateFocus), "CS_UpdateFocus" },
		std::pair{ std::addressof(shaders_.calculateCoC), "CS_CalculateCoC" },
		std::pair{ std::addressof(shaders_.cocTile1), "CS_CoCTile1" },
		std::pair{ std::addressof(shaders_.cocTile2), "CS_CoCTile2" },
		std::pair{ std::addressof(shaders_.cocTileNeighbor), "CS_CoCTileNeighbor" },
		std::pair{ std::addressof(shaders_.cocGaussian1), "CS_CoCGaussian1" },
		std::pair{ std::addressof(shaders_.cocGaussian2), "CS_CoCGaussian2" },
		std::pair{ std::addressof(shaders_.blur), "CS_Blur" },
		std::pair{ std::addressof(shaders_.reduceColor), "CS_ReduceColor" },
		std::pair{ std::addressof(shaders_.farBlur), "CS_FarBlur" },
		std::pair{ std::addressof(shaders_.nearBlur), "CS_NearBlur" },
		std::pair{ std::addressof(shaders_.tentFilter), "CS_TentFilter" },
		std::pair{ std::addressof(shaders_.combiner), "CS_Combiner" },
		std::pair{ std::addressof(shaders_.postSmoothing1), "CS_PostSmoothing1" },
		std::pair{ std::addressof(shaders_.postSmoothing2AndFocusing), "CS_PostSmoothing2AndFocusing" }
	};

	for (const auto& [destination, entry] : entries) {
		*destination = CompileComputeShader(a_device, path, entry);
		if (!destination->Get()) {
			return false;
		}
	}
	spdlog::info("Compiled all 15 depth-of-field compute passes (experimental near balanced-perimeter kernel v15)");
	return true;
}

bool CDoF::DoFRenderer::EnsureResources(
	ID3D11Device* a_device,
	const D3D11_TEXTURE2D_DESC& a_inputDescription,
	std::uint32_t a_renderWidth,
	std::uint32_t a_renderHeight)
{
	if (resources_.device == a_device &&
		resources_.width == a_renderWidth &&
		resources_.height == a_renderHeight &&
		resources_.colorFormat == a_inputDescription.Format) {
		return true;
	}

	resources_ = {};
	resources_.device = a_device;
	resources_.width = a_renderWidth;
	resources_.height = a_renderHeight;
	resources_.colorFormat = a_inputDescription.Format;

	if (shadersReady_ && shaderDevice_ != a_device) {
		shaders_ = {};
		shadersReady_ = false;
	}
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

	const auto createColor = [&](Texture& a_texture, bool a_half) {
		return CreateTexture(a_device, a_texture, color, a_half ? halfWidth : width, a_half ? halfHeight : height);
	};
	const auto createFloat = [&](Texture& a_texture, bool a_half) {
		return CreateTexture(a_device, a_texture, DXGI_FORMAT_R32_FLOAT, a_half ? halfWidth : width, a_half ? halfHeight : height);
	};

	if (!createColor(resources_.output, false) ||
		!createColor(resources_.preBlurred, true) ||
		!CreateTexture(a_device, resources_.farGatherColor1, color, quarterWidth, quarterHeight) ||
		!CreateTexture(a_device, resources_.farGatherColor2, color, eighthWidth, eighthHeight) ||
		!CreateTexture(a_device, resources_.farGatherColor3, color, sixteenthWidth, sixteenthHeight) ||
		!createColor(resources_.farBlurred, true) ||
		!createColor(resources_.nearBlurred, true) ||
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
		!CreateConstantBuffer(a_device, sizeof(DoFConstants), resources_.dofConstants) ||
		!CreateConstantBuffer(a_device, sizeof(SharedConstants), resources_.sharedConstants)) {
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

	spdlog::info(
		"Created depth-of-field resources: {}x{}, format {} (input allocation {}x{})",
		width,
		height,
		static_cast<std::uint32_t>(color),
		a_inputDescription.Width,
		a_inputDescription.Height);
	return true;
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
	const auto denominator = -screenDepth * cameraData[2] + cameraData[0];
	if (!std::isfinite(denominator) || denominator <= 1.0e-5F) {
		return std::nullopt;
	}
	const auto distanceMeters = (cameraData[3] / denominator) * kGameUnitToMeters;
	if (!std::isfinite(distanceMeters) || distanceMeters <= 0.0F) {
		return std::nullopt;
	}
	TargetFocusSample result{};
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

	if (a_target->GetFormType() == RE::FormType::ActorCharacter) {
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

void CDoF::DoFRenderer::Apply()
{
	std::scoped_lock lock(mutex_);
	if (!settings_.enabled || permanentlyDisabled_) {
		return;
	}
	auto effectiveSettings = settings_;
	auto targetFocusMode = TargetFocusMode::kNone;
	std::optional<TargetFocusSample> activeTargetFocus;
	if (targetFocusSettings_.dialogueEnabled) {
		if (const auto focus = GetDialogueTargetFocus()) {
			effectiveSettings = dialogueLensSettings_;
			effectiveSettings.enabled = true;
			effectiveSettings.autoFocus = false;
			effectiveSettings.manualFocusMeters = focus->distanceMeters;
			ApplyDepthStrength(effectiveSettings, targetFocusSettings_.dialogueDepthStrength);
			activeTargetFocus = focus;
			targetFocusMode = TargetFocusMode::kDialogue;
		}
	}
	if (targetFocusMode == TargetFocusMode::kNone && !modeSettings_.normalGameplayEnabled) {
		if (targetFocusMode_ != TargetFocusMode::kNone) {
			targetFocusMode_ = TargetFocusMode::kNone;
			spdlog::info("Dialogue target focus ended; normal gameplay DoF is disabled");
		}
		return;
	}
	if (targetFocusMode == TargetFocusMode::kNone && targetFocusSettings_.consoleEnabled) {
		const auto playerSource = targetFocusSettings_.targetSource == TargetFocusSource::kPlayer;
		const auto focus = playerSource ? GetPlayerTargetFocus() : GetConsoleTargetFocus();
		if (focus) {
			effectiveSettings = settings_;
			effectiveSettings.enabled = true;
			// Use the projected head/target anchor distance, matching the restored behaviour.
			// GetTargetFocus continues
			// to populate the visible-surface coordinate so a future advanced UI
			// option can select it without changing the target sampling code again.
			effectiveSettings.autoFocus = false;
			effectiveSettings.manualFocusMeters = std::clamp(
				focus->distanceMeters + targetFocusSettings_.targetFocusOffsetMeters, 0.1F, 150.0F);
			activeTargetFocus = focus;
			targetFocusMode = playerSource ? TargetFocusMode::kPlayer : TargetFocusMode::kConsole;
		}
	}
	if (targetFocusMode != targetFocusMode_) {
		targetFocusMode_ = targetFocusMode;
		switch (targetFocusMode_) {
		case TargetFocusMode::kDialogue:
			spdlog::info("Dialogue target focus activated at {:.2f} m", effectiveSettings.manualFocusMeters);
			break;
		case TargetFocusMode::kPlayer:
			spdlog::info("Player target focus activated using projected anchor distance at {:.2f} m",
				effectiveSettings.manualFocusMeters);
			break;
		case TargetFocusMode::kConsole:
			spdlog::info("Console target focus activated using projected anchor distance at {:.2f} m",
				effectiveSettings.manualFocusMeters);
			break;
		case TargetFocusMode::kNone:
			spdlog::info("Target focus ended; restored normal settings");
			break;
		}
	}
	if (const auto camera = RE::PlayerCamera::GetSingleton();
		camera && camera->IsInFirstPerson() && !effectiveSettings.enableFirstPersonNearBlur) {
		effectiveSettings.nearPlaneMaxBlur = 0.0F;
	}
	if (!effectiveSettings.enabled || IsMenuBlocked(effectiveSettings)) {
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
		auto depth = depthStencils[RE::RENDER_TARGETS_DEPTHSTENCIL::kMAIN].depthSRV;
		if (!mainTarget.texture || !mainTarget.SRV || !depth) {
			return;
		}

		D3D11_TEXTURE2D_DESC inputDescription{};
		mainTarget.texture->GetDesc(&inputDescription);
		const auto renderArea = GetActiveRenderArea(context, inputDescription);
		if (!depthPathChecked_) {
			const auto saoEnabled = ReadDisplayBool("bSAOEnable:Display");
			const auto reflectionsEnabled = ReadDisplayBool("bScreenSpaceReflectionEnabled:Display");
			const auto hdr64Enabled = ReadDisplayBool("bUse64bitsHDRRenderTarget:Display");
			const auto communityShadersLoaded =
				GetModuleHandleW(L"CommunityShaders.dll") != nullptr;
			const auto lowSpecDepthSettings =
				(saoEnabled && !*saoEnabled) ||
				(reflectionsEnabled && !*reflectionsEnabled) ||
				(hdr64Enabled && !*hdr64Enabled);
			useLowSpecDepthFallback_ = communityShadersLoaded && lowSpecDepthSettings;
			depthPathChecked_ = true;
			spdlog::info(
				"Display depth settings: SAO={}, SSR={}, 64-bit HDR={}; Community Shaders={}; selected {} depth path",
				saoEnabled ? (*saoEnabled ? "on" : "off") : "unknown",
				reflectionsEnabled ? (*reflectionsEnabled ? "on" : "off") : "unknown",
				hdr64Enabled ? (*hdr64Enabled ? "on" : "off") : "unknown",
				communityShadersLoaded ? "loaded" : "not loaded",
				useLowSpecDepthFallback_ ? "low-spec fallback" : "standard");
			if (lowSpecDepthSettings && !communityShadersLoaded) {
				spdlog::warn(
					"Low-spec depth settings were detected, but Community Shaders was not loaded; preserving the standard depth path");
			}
		}

		if (useLowSpecDepthFallback_) {
			auto fallbackDepth =
				depthStencils[RE::RENDER_TARGETS_DEPTHSTENCIL::kPOST_ZPREPASS_COPY].depthSRV;
			if (IsCompatibleDepth(fallbackDepth, inputDescription)) {
				depth = fallbackDepth;
			} else if (!loggedDepthFallbackUnavailable_) {
				loggedDepthFallbackUnavailable_ = true;
				spdlog::warn(
					"Low-spec depth fallback was requested but its depth texture was unavailable or incompatible; using standard depth");
			}
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

		if (inputDescription.SampleDesc.Count != 1 ||
			!EnsureResources(device, inputDescription, renderArea.width, renderArea.height)) {
			permanentlyDisabled_ = true;
			spdlog::critical("Depth of field disabled because GPU resources could not be created");
			return;
		}
		ID3D11RenderTargetView* restoreRTV = currentRTV.Get();
		OutputMergerRestore restore{ context, restoreRTV, currentDSV.Get() };
		context->OMSetRenderTargets(0, nullptr, nullptr);
		const auto* lowSpecTargetGuard = useLowSpecDepthFallback_ && activeTargetFocus && activeTargetFocus->guardValid ?
			std::addressof(*activeTargetFocus) : nullptr;
		if (lowSpecTargetGuard && !loggedLowSpecTargetGuard_) {
			loggedLowSpecTargetGuard_ = true;
			spdlog::info(
				"Low-spec target near-blur protection activated (body centre {:.3f}, {:.3f}; radius {:.3f}, {:.3f}; head centre {:.3f}, {:.3f}; radius {:.3f})",
				lowSpecTargetGuard->guardCenter[0], lowSpecTargetGuard->guardCenter[1],
				lowSpecTargetGuard->guardRadius[0], lowSpecTargetGuard->guardRadius[1],
				lowSpecTargetGuard->headGuardCenter[0], lowSpecTargetGuard->headGuardCenter[1],
				lowSpecTargetGuard->headGuardValid ? lowSpecTargetGuard->headGuardRadius : 0.0F);
		}
		Dispatch(
			context,
			mainTarget.SRV,
			depth,
			effectiveSettings,
			lowSpecTargetGuard,
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
	const Settings& a_settings,
	const TargetFocusSample* a_lowSpecTargetGuard,
	std::uint32_t a_inputWidth,
	std::uint32_t a_inputHeight,
	std::uint32_t a_renderLeft,
	std::uint32_t a_renderTop)
{
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
		.targetGuardEnabled = a_lowSpecTargetGuard ? 1U : 0U,
		.apertureShapeStrength = a_settings.apertureShapeStrength,
		.targetGuardCenter = a_lowSpecTargetGuard ?
			Float2{ a_lowSpecTargetGuard->guardCenter[0], a_lowSpecTargetGuard->guardCenter[1] } : Float2{ 0.5F, 0.5F },
		.targetGuardRadius = a_lowSpecTargetGuard ?
			Float2{ a_lowSpecTargetGuard->guardRadius[0], a_lowSpecTargetGuard->guardRadius[1] } : Float2{ 1.0F, 1.0F },
		.nearFocusRangeMeters = a_settings.nearFocusRangeMeters,
		.farFocusRangeMeters = a_settings.farFocusRangeMeters,
		.apertureBlades = std::clamp(a_settings.apertureBlades, 3U, 12U),
		.apertureRoundness = std::clamp(a_settings.apertureRoundness, 0.0F, 1.0F),
		.headGuardCenter = a_lowSpecTargetGuard && a_lowSpecTargetGuard->headGuardValid ?
			Float2{ a_lowSpecTargetGuard->headGuardCenter[0], a_lowSpecTargetGuard->headGuardCenter[1] } : Float2{ 0.5F, 0.5F },
		.headGuardRadius = a_lowSpecTargetGuard && a_lowSpecTargetGuard->headGuardValid ?
			a_lowSpecTargetGuard->headGuardRadius : 0.0F,
		.padding3 = 0U,
		.targetGuardAxis = a_lowSpecTargetGuard ?
			Float2{ a_lowSpecTargetGuard->guardAxis[0], a_lowSpecTargetGuard->guardAxis[1] } : Float2{},
		.padding4 = {}
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
			1.0F / static_cast<float>(a_inputHeight) }
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

	std::array<ID3D11ShaderResourceView*, 11> srvs{};
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
	bindAndDispatch(shaders_.updateFocus.Get(), 1, 1);
	resetViews();
	a_context->CopyResource(resources_.previousFocus.resource.Get(), resources_.focus.resource.Get());
	resources_.focusInitialized = true;

	// Calculate circle of confusion.
	srvs[0] = a_color;
	srvs[1] = resources_.previousFocus.srv.Get();
	srvs[2] = a_depth;
	uavs[2] = resources_.coc.uav.Get();
	bindAndDispatch(shaders_.calculateCoC.Get(), fullWidth, fullHeight);
	resetViews();

	// Tile and expand CoC.
	srvs[3] = resources_.coc.srv.Get();
	uavs[2] = resources_.cocTileTemporary.uav.Get();
	bindAndDispatch(shaders_.cocTile1.Get(), fullWidth, fullHeight);
	resetViews();
	srvs[3] = resources_.cocTileTemporary.srv.Get();
	uavs[2] = resources_.cocTileTemporary2.uav.Get();
	bindAndDispatch(shaders_.cocTile2.Get(), fullWidth, fullHeight);
	resetViews();
	srvs[3] = resources_.cocTileTemporary2.srv.Get();
	uavs[2] = resources_.cocTileNeighbor.uav.Get();
	bindAndDispatch(shaders_.cocTileNeighbor.Get(), fullWidth, fullHeight);
	resetViews();

	// Half-resolution CoC Gaussian blur.
	srvs[3] = resources_.cocTileNeighbor.srv.Get();
	uavs[2] = resources_.cocBlur1.uav.Get();
	bindAndDispatch(shaders_.cocGaussian1.Get(), halfWidth, halfHeight);
	resetViews();
	srvs[3] = resources_.cocBlur1.srv.Get();
	uavs[2] = resources_.cocBlur2.uav.Get();
	bindAndDispatch(shaders_.cocGaussian2.Get(), halfWidth, halfHeight);
	resetViews();

	// Pre-, far-, and near-plane blur. Aperture shaping is calculated directly
	// from the saved blade count, roundness, strength, and rotation constants.
	srvs[0] = a_color;
	srvs[3] = resources_.coc.srv.Get();
	srvs[4] = resources_.cocBlur2.srv.Get();
	uavs[0] = resources_.preBlurred.uav.Get();
	bindAndDispatch(shaders_.blur.Get(), halfWidth, halfHeight);
	resetViews();
	// Build a compact color pyramid only for the far-plane gather. Large blur
	// kernels select a coarser source so each fixed gather tap represents a
	// wider, contiguous footprint instead of leaving visible gaps.
	srvs[0] = resources_.preBlurred.srv.Get();
	uavs[0] = resources_.farGatherColor1.uav.Get();
	bindAndDispatch(shaders_.reduceColor.Get(), quarterWidth, quarterHeight);
	resetViews();
	srvs[0] = resources_.farGatherColor1.srv.Get();
	uavs[0] = resources_.farGatherColor2.uav.Get();
	bindAndDispatch(shaders_.reduceColor.Get(), eighthWidth, eighthHeight);
	resetViews();
	srvs[0] = resources_.farGatherColor2.srv.Get();
	uavs[0] = resources_.farGatherColor3.uav.Get();
	bindAndDispatch(shaders_.reduceColor.Get(), sixteenthWidth, sixteenthHeight);
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
	uavs[0] = resources_.farBlurred.uav.Get();
	bindAndDispatch(shaders_.farBlur.Get(), halfWidth, halfHeight);
	resetViews();
	srvs[0] = resources_.farBlurred.srv.Get();
	srvs[3] = resources_.cocTileNeighbor.srv.Get();
	srvs[4] = resources_.cocBlur2.srv.Get();
	uavs[0] = resources_.nearBlurred.uav.Get();
	bindAndDispatch(shaders_.nearBlur.Get(), halfWidth, halfHeight);
	resetViews();

	// Tent filter and combine.
	srvs[0] = resources_.farBlurred.srv.Get();
	uavs[0] = resources_.blurredFiltered.uav.Get();
	bindAndDispatch(shaders_.tentFilter.Get(), halfWidth, halfHeight);
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
	uavs[0] = resources_.postSmooth.uav.Get();
	bindAndDispatch(shaders_.combiner.Get(), fullWidth, fullHeight);
	resetViews();

	// Final smoothing and focusing.
	srvs[0] = resources_.postSmooth.srv.Get();
	srvs[3] = resources_.coc.srv.Get();
	uavs[0] = resources_.postSmooth2.uav.Get();
	bindAndDispatch(shaders_.postSmoothing1.Get(), fullWidth, fullHeight);
	resetViews();
	srvs[0] = resources_.postSmooth.srv.Get();
	srvs[3] = resources_.coc.srv.Get();
	srvs[7] = resources_.postSmooth2.srv.Get();
	uavs[0] = resources_.output.uav.Get();
	bindAndDispatch(shaders_.postSmoothing2AndFocusing.Get(), fullWidth, fullHeight);
	resetViews();

	ID3D11Buffer* nullBuffer = nullptr;
	ID3D11SamplerState* nullSampler = nullptr;
	a_context->CSSetConstantBuffers(1, 1, &nullBuffer);
	a_context->CSSetConstantBuffers(5, 1, &nullBuffer);
	a_context->CSSetSamplers(0, 1, &nullSampler);
	a_context->CSSetShader(nullptr, nullptr, 0);
}
