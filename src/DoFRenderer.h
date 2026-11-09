#pragma once

#include "Settings.h"

namespace CDoF
{
	class DoFRenderer
	{
	public:
		static DoFRenderer& GetSingleton();

		void SetSettings(Settings a_settings);
		void SetModeSettings(ModeSettings a_settings);
		void SetSilhouetteSettings(SilhouetteSettings a_settings);
		void SetDepthFrameSettings(DepthFrameSettings a_settings);
		[[nodiscard]] bool ShouldCaptureDepthFrameSubject() const noexcept;
		[[nodiscard]] bool ShouldAuditSubjectDraws() const noexcept;
		void SetTargetFocus(
			TargetFocusSettings a_settings,
			Settings a_dialogueLensSettings);
		void SetTDMFocus(TDMFocusSettings a_settings);
		void SetSneakFocus(SneakFocusSettings a_settings);
		void SetExperimentalTransparency(ExperimentalTransparencySettings a_settings);
		void SetFocusPickerActive(bool a_active);
		void CapturePreOpaqueDepth();
		void SnapshotDepthFrameSubjectStencil();
		void ApplyBeforePostProcessing();
		void ApplyAfterPostProcessing(RE::RENDER_TARGET a_outputTarget);

	private:
		struct TargetFocusSample
		{
			struct GuardSegment
			{
				std::array<float, 2> start{};
				std::array<float, 2> end{};
				float radius{};
			};
			static constexpr std::size_t kMaximumGuardSegments{ 12U };
			float distanceMeters{};
			std::array<float, 2> focusCoordinate{ 0.5F, 0.5F };
			std::array<float, 2> guardCenter{ 0.5F, 0.5F };
			std::array<float, 2> guardRadius{};
			std::array<float, 2> guardAxis{};
			std::array<float, 2> headGuardCenter{ 0.5F, 0.5F };
			float headGuardRadius{};
			bool guardValid{ false };
			bool headGuardValid{ false };
			bool surfaceFocusValid{ false };
			bool actor{ false };
			std::array<GuardSegment, kMaximumGuardSegments> guardSegments{};
			std::uint32_t guardSegmentCount{};
		};

		struct Texture
		{
			ComPtr<ID3D11Texture2D> resource;
			ComPtr<ID3D11ShaderResourceView> srv;
			ComPtr<ID3D11UnorderedAccessView> uav;
		};

		struct Resources
		{
			ID3D11Device* device{};
			std::uint32_t width{};
			std::uint32_t height{};
			DXGI_FORMAT colorFormat{ DXGI_FORMAT_UNKNOWN };
			bool baseReady{ false };
			bool dofReady{ false };
			bool focusInitialized{ false };

			Texture output;
			Texture overlayInput;
			Texture preBlurred;
			Texture farGatherColor1;
			Texture farGatherColor2;
			Texture farGatherColor3;
			Texture farBlurred;
			Texture nearBlurred;
			Texture blurredFiltered;
			Texture postSmooth;
			Texture postSmooth2;
			Texture focus;
			Texture previousFocus;
			Texture coc;
			Texture cocTileTemporary;
			Texture cocTileTemporary2;
			Texture cocTileNeighbor;
			Texture cocBlur1;
			Texture cocBlur2;
			ComPtr<ID3D11Buffer> dofConstants;
			ComPtr<ID3D11Buffer> silhouetteConstants;
			ComPtr<ID3D11Buffer> depthFrameConstants;
			ComPtr<ID3D11Buffer> sharedConstants;
			ComPtr<ID3D11SamplerState> linearSampler;
		};

		struct DepthFrameResources
		{
			ID3D11Device* device{};
			std::uint32_t width{};
			std::uint32_t height{};
			DXGI_FORMAT colorFormat{ DXGI_FORMAT_UNKNOWN };
			bool maskReady{ false };
			bool colorReady{ false };

			Texture output;
			Texture input;
			Texture subjectMask;
			ComPtr<ID3D11Buffer> constants;
			ComPtr<ID3D11Buffer> sharedConstants;
			ComPtr<ID3D11Buffer> maskConstants;
		};

		struct Shaders
		{
			ComPtr<ID3D11ComputeShader> updateFocus;
			ComPtr<ID3D11ComputeShader> calculateCoC;
			ComPtr<ID3D11ComputeShader> cocTile1;
			ComPtr<ID3D11ComputeShader> cocTile2;
			ComPtr<ID3D11ComputeShader> cocTileNeighbor;
			ComPtr<ID3D11ComputeShader> cocGaussian1;
			ComPtr<ID3D11ComputeShader> cocGaussian2;
			ComPtr<ID3D11ComputeShader> blur;
			ComPtr<ID3D11ComputeShader> reduceColor;
			ComPtr<ID3D11ComputeShader> farBlur;
			ComPtr<ID3D11ComputeShader> nearBlur;
			ComPtr<ID3D11ComputeShader> tentFilter;
			ComPtr<ID3D11ComputeShader> combiner;
			ComPtr<ID3D11ComputeShader> postSmoothing1;
			ComPtr<ID3D11ComputeShader> postSmoothing2AndFocusing;
		};

		bool EnsureResources(
			ID3D11Device* a_device,
			const D3D11_TEXTURE2D_DESC& a_inputDescription,
			std::uint32_t a_renderWidth,
			std::uint32_t a_renderHeight);
		bool EnsureBaseResources(
			ID3D11Device* a_device,
			const D3D11_TEXTURE2D_DESC& a_inputDescription,
			std::uint32_t a_renderWidth,
			std::uint32_t a_renderHeight);
		bool EnsureSilhouetteResources(
			ID3D11Device* a_device,
			const D3D11_TEXTURE2D_DESC& a_inputDescription,
			std::uint32_t a_renderWidth,
			std::uint32_t a_renderHeight);
		bool EnsureDepthFrameResources(
			ID3D11Device* a_device,
			const D3D11_TEXTURE2D_DESC& a_inputDescription,
			std::uint32_t a_renderWidth,
			std::uint32_t a_renderHeight);
		bool EnsureDepthFrameMaskResources(
			ID3D11Device* a_device,
			std::uint32_t a_renderWidth,
			std::uint32_t a_renderHeight);
		bool CompileShaders(ID3D11Device* a_device);
		bool CompileShaderSet(
			ID3D11Device* a_device,
			const std::filesystem::path& a_path,
			Shaders& a_shaders,
			std::string_view a_name);
		bool CreateTexture(ID3D11Device* a_device, Texture& a_texture, DXGI_FORMAT a_format, std::uint32_t a_width, std::uint32_t a_height);
		bool CreateConstantBuffer(ID3D11Device* a_device, std::uint32_t a_size, ComPtr<ID3D11Buffer>& a_buffer);
		bool UpdateConstantBuffer(ID3D11DeviceContext* a_context, ID3D11Buffer* a_buffer, const void* a_data, std::uint32_t a_size);
		void Dispatch(
			ID3D11DeviceContext* a_context,
			ID3D11ShaderResourceView* a_color,
			ID3D11ShaderResourceView* a_depth,
			ID3D11ShaderResourceView* a_skyMaskDepth,
			const Settings& a_settings,
			const TargetFocusSample* a_targetGuard,
			float a_targetNearFocusMinimumMeters,
			std::uint32_t a_inputWidth,
			std::uint32_t a_inputHeight,
			std::uint32_t a_renderLeft,
			std::uint32_t a_renderTop);
		bool DispatchSilhouette(
			ID3D11DeviceContext* a_context,
			ID3D11ShaderResourceView* a_skyMaskDepth,
			const SilhouetteSettings& a_settings,
			std::uint32_t a_inputWidth,
			std::uint32_t a_inputHeight,
			std::uint32_t a_renderLeft,
			std::uint32_t a_renderTop);
		bool DispatchDepthFrame(
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
			std::uint32_t a_renderTop);
		bool IsMenuBlocked(const Settings& a_settings) const;
		bool IsSilhouetteMenuBlocked() const;
		void UpdateDepthConvention(ID3D11ShaderResourceView* a_depth);
		void ApplySilhouette(RE::RENDER_TARGET a_outputTarget);
		void ApplyDepthFrame(RE::RENDER_TARGET a_outputTarget, const DepthFrameSettings& a_frameSettings);
		void CaptureDepthFrameSubjectMask(
			ID3D11Device* a_device,
			ID3D11DeviceContext* a_context,
			ID3D11ShaderResourceView* a_stencil,
			std::uint32_t a_inputWidth,
			std::uint32_t a_inputHeight,
			std::uint32_t a_renderLeft,
			std::uint32_t a_renderTop,
			std::uint32_t a_renderWidth,
			std::uint32_t a_renderHeight);
		std::optional<TargetFocusSample> GetDialogueTargetFocus() const;
		std::optional<TargetFocusSample> GetPlayerTargetFocus() const;
		std::optional<TargetFocusSample> GetConsoleTargetFocus() const;
		std::optional<TargetFocusSample> GetTargetFocus(
			const RE::NiPointer<RE::TESObjectREFR>& a_target,
			bool a_allowPlayer) const;
		static void ApplyDepthStrength(Settings& a_settings, float a_strength);

		enum class TargetFocusMode
		{
			kNone,
			kDialogue,
			kTDMLock,
			kSneak,
			kPlayer,
			kFirstPersonScreenAF,
			kConsole
		};

		Settings settings_{};
		ModeSettings modeSettings_{};
		SilhouetteSettings silhouetteSettings_{};
		DepthFrameSettings depthFrameSettings_{};
		float lockOnFrameProgress_{};
		std::chrono::steady_clock::time_point lockOnFrameUpdatedAt_{};
		std::atomic_bool depthFrameStencilCaptureEnabled_{ false };
		std::atomic_bool subjectDrawAuditEnabled_{ false };
		TargetFocusSettings targetFocusSettings_{};
		TDMFocusSettings tdmFocusSettings_{};
		SneakFocusSettings sneakFocusSettings_{};
		ExperimentalTransparencySettings experimentalTransparencySettings_{};
		Settings dialogueLensSettings_{};
		Resources resources_{};
		DepthFrameResources depthFrameResources_{};
		Shaders shaders_{};
		Shaders test6Shaders_{};
		Shaders test8Shaders_{};
		ComPtr<ID3D11ComputeShader> silhouetteShader_;
		ComPtr<ID3D11ComputeShader> depthFrameShader_;
		ComPtr<ID3D11ComputeShader> depthFrameMaskCaptureShader_;
		ComPtr<ID3D11Texture2D> preOpaqueDepthTexture_;
		ComPtr<ID3D11ShaderResourceView> preOpaqueDepthSRV_;
		ID3D11Device* preOpaqueDepthDevice_{};
		DXGI_FORMAT preOpaqueDepthTextureFormat_{ DXGI_FORMAT_UNKNOWN };
		DXGI_FORMAT preOpaqueDepthViewFormat_{ DXGI_FORMAT_UNKNOWN };
		std::uint32_t preOpaqueDepthWidth_{};
		std::uint32_t preOpaqueDepthHeight_{};
		std::uint32_t preOpaqueDepthSampleCount_{};
		ComPtr<ID3D11Texture2D> depthFrameSubjectStencilTexture_;
		ComPtr<ID3D11ShaderResourceView> depthFrameSubjectStencilSRV_;
		ID3D11Device* depthFrameSubjectStencilDevice_{};
		DXGI_FORMAT depthFrameSubjectStencilTextureFormat_{ DXGI_FORMAT_UNKNOWN };
		DXGI_FORMAT depthFrameSubjectStencilViewFormat_{ DXGI_FORMAT_UNKNOWN };
		std::uint32_t depthFrameSubjectStencilWidth_{};
		std::uint32_t depthFrameSubjectStencilHeight_{};
		std::uint32_t depthFrameSubjectStencilSampleCount_{};
		ID3D11Device* shaderDevice_{};
		ID3D11Device* silhouetteShaderDevice_{};
		ID3D11Device* depthFrameShaderDevice_{};
		bool shadersReady_{ false };
		bool test6ShadersReady_{ false };
		bool test8ShadersReady_{ false };
		bool silhouetteShaderReady_{ false };
		bool depthFrameShaderReady_{ false };
		bool permanentlyDisabled_{ false };
		bool silhouetteDisabled_{ false };
		bool depthFrameDisabled_{ false };
		bool loggedFirstFrame_{ false };
		bool loggedFirstSilhouetteFrame_{ false };
		bool loggedFirstDepthFrame_{ false };
		bool loggedFirstDepthFrameMaskCapture_{ false };
		bool loggedFirstDepthFrameMaskContents_{ false };
		bool loggedSilhouetteDestinationFailure_{ false };
		bool loggedDepthFrameDestinationFailure_{ false };
		bool depthPathChecked_{ false };
		bool useStandaloneNonActorTargetGuard_{ false };
		bool useStandaloneNearFocusAssist_{ false };
		bool loggedTargetGuard_{ false };
		bool loggedTargetNearFocusAssist_{ false };
		bool loggedExperimentalTransparencyDepth_{ false };
		bool loggedExperimentalTransparencyDepthUnavailable_{ false };
		bool loggedPreOpaqueDepthCapture_{ false };
		bool loggedDepthFrameSubjectStencilSnapshot_{ false };
		bool preOpaqueDepthValid_{ false };
		bool depthFrameSubjectStencilValid_{ false };
		bool loggedRenderAreaDiagnostics_{ false };
		bool depthConventionInitialized_{ false };
		bool reverseZDepth_{ false };
		bool dialogueOnlyPrewarmed_{ false };
		bool focusPickerActive_{ false };
		TargetFocusMode targetFocusMode_{ TargetFocusMode::kNone };
		std::optional<TargetFocusSample> activeDepthFrameTarget_{};
		std::mutex mutex_;
	};
}
