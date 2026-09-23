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
		void SetTargetFocus(
			TargetFocusSettings a_settings,
			Settings a_dialogueLensSettings);
		void ApplyBeforePostProcessing();
		void ApplyAfterPostProcessing();

	private:
		struct TargetFocusSample
		{
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
			ComPtr<ID3D11Buffer> sharedConstants;
			ComPtr<ID3D11SamplerState> linearSampler;
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
		bool CompileShaders(ID3D11Device* a_device);
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
		bool IsMenuBlocked(const Settings& a_settings) const;
		bool IsSilhouetteMenuBlocked() const;
		void ApplySilhouette();
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
			kPlayer,
			kConsole
		};

		Settings settings_{};
		ModeSettings modeSettings_{};
		SilhouetteSettings silhouetteSettings_{};
		TargetFocusSettings targetFocusSettings_{};
		Settings dialogueLensSettings_{};
		Resources resources_{};
		Shaders shaders_{};
		ComPtr<ID3D11ComputeShader> silhouetteShader_;
		ID3D11Device* shaderDevice_{};
		ID3D11Device* silhouetteShaderDevice_{};
		bool shadersReady_{ false };
		bool silhouetteShaderReady_{ false };
		bool permanentlyDisabled_{ false };
		bool silhouetteDisabled_{ false };
		bool loggedFirstFrame_{ false };
		bool loggedFirstSilhouetteFrame_{ false };
		bool depthPathChecked_{ false };
		bool useLowSpecDepthFallback_{ false };
		bool useStandaloneNonActorTargetGuard_{ false };
		bool useCommunityShadersActorNearFocusAssist_{ false };
		bool useStandaloneNearFocusAssist_{ false };
		bool loggedDepthFallbackUnavailable_{ false };
		bool loggedTargetGuard_{ false };
		bool loggedTargetNearFocusAssist_{ false };
		bool loggedRenderAreaDiagnostics_{ false };
		bool dialogueOnlyPrewarmed_{ false };
		TargetFocusMode targetFocusMode_{ TargetFocusMode::kNone };
		std::mutex mutex_;
	};
}
