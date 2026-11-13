#include "DoFRenderer.h"
#include "SubjectMask.h"
#include "DoFUI.h"
#include "HotkeyInput.h"
#include "Settings.h"
#include "TDMIntegration.h"

namespace
{
	std::atomic_bool firstMainPathLogged{ false };
	std::atomic_bool firstPlayerStencilLogged{ false };
	std::atomic_bool directDrawHooksInstalled{ false };
	std::atomic_uint64_t playerGeometryChecks{ 0 };
	std::atomic_bool playerGeometryMissLogged{ false };
	// Skyrim and other render features already use individual stencil bits for
	// scene classification.  A single-bit player tag therefore aliases terrain,
	// foliage, and other geometry.  Use a full-byte marker and require an exact
	// match when the private subject mask is captured.
	constexpr UINT kPlayerStencilMarker = 0xFEU;

	struct TaggedDepthStencilState
	{
		ComPtr<ID3D11DepthStencilState> source;
		ComPtr<ID3D11DepthStencilState> tagged;
	};

	struct WritableStencilView
	{
		ComPtr<ID3D11DepthStencilView> source;
		ComPtr<ID3D11DepthStencilView> writable;
	};

	std::vector<TaggedDepthStencilState> taggedDepthStencilStates;
	std::vector<WritableStencilView> writableStencilViews;

	void InstallDirectDrawHooks(ID3D11DeviceContext* a_context);

	bool IsPlayerGeometry(RE::BSGeometry* a_geometry)
	{
		if (!a_geometry) {
			return false;
		}

		const auto player = CDoF::SubjectMask::GetTarget();
		if (!player) {
			return false;
		}
		const auto thirdPersonRoot = player->Get3D(false);
		const auto firstPersonRoot = player->IsPlayerRef() ? player->Get3D(true) : nullptr;

		// Render-pass geometry normally carries no actor user data of its own.
		// Bethesda attaches it to an ancestor (usually the actor's root node), so
		// inspect the whole parent chain.  Comparing the two player 3D roots also
		// covers equipment nodes whose user data is empty.
		for (auto object = static_cast<RE::NiAVObject*>(a_geometry); object; object = object->parent) {
			if (object == thirdPersonRoot || object == firstPersonRoot) {
				return true;
			}
			if (const auto reference = object->GetUserData(); reference && reference == player.get()) {
				return true;
			}
		}
		return false;
	}

	ID3D11DepthStencilState* GetPlayerTaggedDepthStencilState(
		ID3D11DeviceContext* a_context,
		ID3D11DepthStencilState* a_source)
	{
		for (auto& cached : taggedDepthStencilStates) {
			if (cached.source.Get() == a_source) {
				return cached.tagged.Get();
			}
		}

		D3D11_DEPTH_STENCIL_DESC description{};
		if (a_source) {
			a_source->GetDesc(&description);
		} else {
			description.DepthEnable = TRUE;
			description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			description.DepthFunc = D3D11_COMPARISON_LESS;
		}
		description.StencilEnable = TRUE;
		description.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		description.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		description.BackFace = description.FrontFace;

		ComPtr<ID3D11Device> device;
		a_context->GetDevice(device.GetAddressOf());
		if (!device) {
			return nullptr;
		}
		TaggedDepthStencilState cached;
		cached.source = a_source;
		if (FAILED(device->CreateDepthStencilState(&description, cached.tagged.GetAddressOf()))) {
			return nullptr;
		}
		taggedDepthStencilStates.push_back(std::move(cached));
		return taggedDepthStencilStates.back().tagged.Get();
	}

	ID3D11DepthStencilView* GetWritableStencilView(
		ID3D11DeviceContext* a_context,
		ID3D11DepthStencilView* a_source,
		bool& a_rebound)
	{
		a_rebound = false;
		if (!a_source) {
			return nullptr;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC description{};
		a_source->GetDesc(&description);
		if ((description.Flags & D3D11_DSV_READ_ONLY_STENCIL) == 0) {
			return a_source;
		}

		for (auto& cached : writableStencilViews) {
			if (cached.source.Get() == a_source) {
				a_rebound = true;
				return cached.writable.Get();
			}
		}

		ComPtr<ID3D11Resource> resource;
		a_source->GetResource(resource.GetAddressOf());
		ComPtr<ID3D11Device> device;
		a_context->GetDevice(device.GetAddressOf());
		if (!resource || !device) {
			return nullptr;
		}

		description.Flags &= ~D3D11_DSV_READ_ONLY_STENCIL;
		WritableStencilView cached;
		cached.source = a_source;
		if (FAILED(device->CreateDepthStencilView(
				resource.Get(),
				&description,
				cached.writable.GetAddressOf()))) {
			return nullptr;
		}
		writableStencilViews.push_back(std::move(cached));
		a_rebound = true;
		return writableStencilViews.back().writable.Get();
	}

	// Test23 observes existing player draw scopes. No OM/RS/shader/resource writes.
	struct SubjectDrawAudit
	{
		static inline std::atomic_bool complete{ false };
		static inline std::atomic_uint64_t eligible{ 0 }, matched{ 0 }, draws{ 0 }, scopesWithDraws{ 0 }, multiDrawScopes{ 0 };
		static inline std::atomic_uint64_t lightingMatches{ 0 }, effectMatches{ 0 }, indexed{ 0 }, instanced{ 0 };
		static inline std::atomic_uint32_t samples{ 0 };
		struct Scope
		{
			ID3D11DeviceContext* context{};
			std::uint32_t drawCount{};
            const char* geometry{};
            std::uint32_t pass{};
		};
		static inline thread_local Scope lightingScope{ nullptr, 0 };
		static inline thread_local Scope effectScope{ nullptr, 0 };

		static void End(bool a_lighting)
		{
			auto& scope = a_lighting ? lightingScope : effectScope;
			if (scope.context && scope.drawCount > 0) {
				scopesWithDraws.fetch_add(1, std::memory_order_relaxed);
				if (scope.drawCount > 1) multiDrawScopes.fetch_add(1, std::memory_order_relaxed);
			}
			scope = { nullptr, 0, nullptr, 0 };
		}

		static void Begin(RE::BSRenderPass* a_pass, bool a_lighting)
		{
			End(a_lighting);
			if ((complete.load(std::memory_order_relaxed) && !CDoF::SubjectMask::WantsCapture()) ||
				!CDoF::DoFRenderer::GetSingleton().ShouldAuditSubjectDraws()) return;
			eligible.fetch_add(1, std::memory_order_relaxed);
			if (!a_pass || !IsPlayerGeometry(a_pass->geometry)) return;
			matched.fetch_add(1, std::memory_order_relaxed);
			(a_lighting ? lightingMatches : effectMatches).fetch_add(1, std::memory_order_relaxed);
			const auto renderer = RE::BSGraphics::Renderer::GetSingleton();
			if (!renderer) return;
			auto context = reinterpret_cast<ID3D11DeviceContext*>(renderer->GetRuntimeData().context);
			if (!context) return;
			InstallDirectDrawHooks(context);
			auto& scope = a_lighting ? lightingScope : effectScope;
			scope = { context, 0, a_pass->geometry->name.c_str(), a_pass->passEnum };
			const auto sample = samples.fetch_add(1, std::memory_order_relaxed);
			if (sample < 6) {
				spdlog::info("SubjectAudit sample {}: shader={} geometry={} pass=0x{:X} contextType={}",
					sample + 1, a_lighting ? "lighting" : "effect",
					a_pass->geometry->name.c_str() ? a_pass->geometry->name.c_str() : "(unnamed)",
					a_pass->passEnum, static_cast<unsigned>(context->GetType()));
			}
		}

		static void ObserveDraw(ID3D11DeviceContext* a_context, const char* a_name)
		{
			auto* scope = lightingScope.context == a_context ? &lightingScope :
				(effectScope.context == a_context ? &effectScope : nullptr);
			if (!scope || complete.load(std::memory_order_relaxed)) return;
			++scope->drawCount;
			const auto count = draws.fetch_add(1, std::memory_order_relaxed);
			if (std::string_view(a_name).find("Indexed") != std::string_view::npos) indexed.fetch_add(1, std::memory_order_relaxed);
			if (std::string_view(a_name).find("Instanced") != std::string_view::npos) instanced.fetch_add(1, std::memory_order_relaxed);
			if (count == 0) {
				ComPtr<ID3D11DepthStencilView> dsv;
				a_context->OMGetRenderTargets(0, nullptr, dsv.GetAddressOf());
				D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
				if (dsv) dsv->GetDesc(&desc);
				spdlog::info("SubjectAudit first draw: {} DSV={} format={} flags=0x{:X}; observation only, no stencil writes",
					a_name, dsv != nullptr, static_cast<unsigned>(desc.Format), desc.Flags);
			}
		}

		static void ReportFrame()
		{
			if (complete.load(std::memory_order_relaxed) ||
				!CDoF::DoFRenderer::GetSingleton().ShouldAuditSubjectDraws()) return;
			static std::uint32_t frames = 0;
			++frames;
			if (frames % 120 != 0) return;
			spdlog::info("SubjectAudit summary frame={}: eligible={} matched={} lighting={} effect={} draws={} indexed={} instanced={} scopesWithDraws={} multiDrawScopes={}",
				frames, eligible.load(), matched.load(), lightingMatches.load(), effectMatches.load(), draws.load(),
				indexed.load(), instanced.load(), scopesWithDraws.load(), multiDrawScopes.load());
			if (frames >= 360) {
				complete.store(true, std::memory_order_relaxed);
				spdlog::info("SubjectAudit complete: observation stopped after 360 enabled frames; player-only mask feasibility still requires a separate GPU capture test");
			}
		}
	};
	template <RE::BSShader::Type ShaderType>
	struct PlayerStencilScope
	{
		static void Begin(RE::BSRenderPass* a_pass)
		{
			Reset();
			if (!CDoF::DoFRenderer::GetSingleton().ShouldCaptureDepthFrameSubject()) {
				return;
			}
			if (!a_pass || !a_pass->geometry) {
				return;
			}
			const auto checked = playerGeometryChecks.fetch_add(1, std::memory_order_relaxed) + 1;
			if (!IsPlayerGeometry(a_pass->geometry)) {
				if (checked == 100000 &&
					!firstPlayerStencilLogged.load(std::memory_order_relaxed) &&
					!playerGeometryMissLogged.exchange(true)) {
					spdlog::warn(
						"No player render geometry matched after {} eligible lighting/effect passes",
						checked);
				}
				return;
			}

			const auto renderer = RE::BSGraphics::Renderer::GetSingleton();
			if (!renderer) {
				return;
			}
			auto context = reinterpret_cast<ID3D11DeviceContext*>(renderer->GetRuntimeData().context);
			if (!context) {
				return;
			}
			InstallDirectDrawHooks(context);

			std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> renderTargets{};
			ID3D11DepthStencilView* depthStencilView = nullptr;
			context->OMGetRenderTargets(
				static_cast<UINT>(renderTargets.size()),
				renderTargets.data(),
				&depthStencilView);
			previousRenderTargetCount = 0;
			for (std::size_t index = 0; index < renderTargets.size(); ++index) {
				previousRenderTargets[index].Attach(renderTargets[index]);
				if (renderTargets[index]) {
					previousRenderTargetCount = static_cast<UINT>(index + 1);
				}
			}
			previousDepthStencilView.Attach(depthStencilView);

			context->OMGetDepthStencilState(previousState.ReleaseAndGetAddressOf(), &previousReference);
			writableDepthStencilView = GetWritableStencilView(
				context,
				previousDepthStencilView.Get(),
				needsWritableRebind);
			taggedState = GetPlayerTaggedDepthStencilState(context, previousState.Get());
			if (taggedState) {
				capturedContext = context;
				checkedPass = checked;
				active = true;
			}
		}

		static bool BeforeDraw(ID3D11DeviceContext* a_context, const char* a_drawName)
		{
			if (!active || a_context != capturedContext || !taggedState) {
				return false;
			}

			if (needsWritableRebind && writableDepthStencilView) {
				std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> renderTargets{};
				for (std::size_t index = 0; index < renderTargets.size(); ++index) {
					renderTargets[index] = previousRenderTargets[index].Get();
				}
				a_context->OMSetRenderTargets(
					previousRenderTargetCount,
					previousRenderTargetCount ? renderTargets.data() : nullptr,
					writableDepthStencilView);
				viewRebound = true;
			}
			a_context->OMSetDepthStencilState(taggedState, kPlayerStencilMarker);
			drawArmed = true;

			if (!firstPlayerStencilLogged.exchange(true)) {
				D3D11_DEPTH_STENCIL_VIEW_DESC viewDescription{};
				if (previousDepthStencilView) {
					previousDepthStencilView->GetDesc(&viewDescription);
				}
				spdlog::info(
					"Tagged first player geometry immediately before {} after {} checked passes "
					"(DSV flags=0x{:X}, writable rebind={})",
					a_drawName,
					checkedPass,
					viewDescription.Flags,
					viewRebound);
			}
			return true;
		}

		static void AfterDraw(ID3D11DeviceContext* a_context)
		{
			if (!drawArmed || a_context != capturedContext) {
				return;
			}
			// Copy the stencil before any following engine state transition can clear
			// or recycle it, then restore the exact OM state seen before this draw.
			CDoF::DoFRenderer::GetSingleton().SnapshotDepthFrameSubjectStencil();
			a_context->OMSetDepthStencilState(previousState.Get(), previousReference);
			if (viewRebound) {
				std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> renderTargets{};
				for (std::size_t index = 0; index < renderTargets.size(); ++index) {
					renderTargets[index] = previousRenderTargets[index].Get();
				}
				a_context->OMSetRenderTargets(
					previousRenderTargetCount,
					previousRenderTargetCount ? renderTargets.data() : nullptr,
					previousDepthStencilView.Get());
			}
			Reset();
		}

		static void End()
		{
			// If the setup/restore pair contained no ID3D11 draw, no GPU state was
			// changed.  A successful direct draw already restored and reset the scope.
			Reset();
		}

		static void Reset()
		{
			for (auto& renderTarget : previousRenderTargets) {
				renderTarget.Reset();
			}
			previousDepthStencilView.Reset();
			previousState.Reset();
			writableDepthStencilView = nullptr;
			taggedState = nullptr;
			capturedContext = nullptr;
			previousRenderTargetCount = 0;
			previousReference = 0;
			checkedPass = 0;
			needsWritableRebind = false;
			viewRebound = false;
			drawArmed = false;
			active = false;
		}

		static inline thread_local std::array<ComPtr<ID3D11RenderTargetView>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> previousRenderTargets;
		static inline thread_local ComPtr<ID3D11DepthStencilView> previousDepthStencilView;
		static inline thread_local ComPtr<ID3D11DepthStencilState> previousState;
		static inline thread_local ID3D11DepthStencilView* writableDepthStencilView{};
		static inline thread_local ID3D11DepthStencilState* taggedState{};
		static inline thread_local ID3D11DeviceContext* capturedContext{};
		static inline thread_local UINT previousRenderTargetCount{};
		static inline thread_local UINT previousReference{};
		static inline thread_local std::uint64_t checkedPass{};
		static inline thread_local bool needsWritableRebind{};
		static inline thread_local bool viewRebound{};
		static inline thread_local bool drawArmed{};
		static inline thread_local bool active{};
	};

	template <class DrawCall>
	void InvokeDirectDraw(ID3D11DeviceContext* a_context, const char* a_drawName, DrawCall&& a_drawCall)
	{
		SubjectDrawAudit::ObserveDraw(a_context, a_drawName);
		const bool lighting = PlayerStencilScope<RE::BSShader::Type::Lighting>::BeforeDraw(a_context, a_drawName);
		const bool effect = !lighting && PlayerStencilScope<RE::BSShader::Type::Effect>::BeforeDraw(a_context, a_drawName);
		a_drawCall();
		auto* maskScope = SubjectDrawAudit::lightingScope.context == a_context ? &SubjectDrawAudit::lightingScope :
            (SubjectDrawAudit::effectScope.context == a_context ? &SubjectDrawAudit::effectScope : nullptr);
        if (maskScope && CDoF::SubjectMask::BeginReplay(a_context, maskScope->geometry, maskScope->pass,
            maskScope == &SubjectDrawAudit::lightingScope)) {
			struct ReplayRestore { ID3D11DeviceContext* context; ~ReplayRestore() { CDoF::SubjectMask::EndReplay(context); } } restore{a_context};
			a_drawCall(); // Saved original draw function: no recursive entry through our vtable hook.
		}
		if (lighting) {
			PlayerStencilScope<RE::BSShader::Type::Lighting>::AfterDraw(a_context);
		} else if (effect) {
			PlayerStencilScope<RE::BSShader::Type::Effect>::AfterDraw(a_context);
		}
	}

	struct DrawIndexedHook
	{
		using Function = void(STDMETHODCALLTYPE*)(ID3D11DeviceContext*, UINT, UINT, INT);
		static void STDMETHODCALLTYPE Thunk(ID3D11DeviceContext* a_context, UINT a_indexCount, UINT a_startIndex, INT a_baseVertex)
		{
			InvokeDirectDraw(a_context, "DrawIndexed", [&] { function(a_context, a_indexCount, a_startIndex, a_baseVertex); });
		}
		static inline Function function{};
	};

	struct DrawHook
	{
		using Function = void(STDMETHODCALLTYPE*)(ID3D11DeviceContext*, UINT, UINT);
		static void STDMETHODCALLTYPE Thunk(ID3D11DeviceContext* a_context, UINT a_vertexCount, UINT a_startVertex)
		{
			InvokeDirectDraw(a_context, "Draw", [&] { function(a_context, a_vertexCount, a_startVertex); });
		}
		static inline Function function{};
	};

	struct DrawIndexedInstancedHook
	{
		using Function = void(STDMETHODCALLTYPE*)(ID3D11DeviceContext*, UINT, UINT, UINT, INT, UINT);
		static void STDMETHODCALLTYPE Thunk(
			ID3D11DeviceContext* a_context,
			UINT a_indexCountPerInstance,
			UINT a_instanceCount,
			UINT a_startIndex,
			INT a_baseVertex,
			UINT a_startInstance)
		{
			InvokeDirectDraw(a_context, "DrawIndexedInstanced", [&] {
				function(a_context, a_indexCountPerInstance, a_instanceCount, a_startIndex, a_baseVertex, a_startInstance);
			});
		}
		static inline Function function{};
	};

	struct DrawInstancedHook
	{
		using Function = void(STDMETHODCALLTYPE*)(ID3D11DeviceContext*, UINT, UINT, UINT, UINT);
		static void STDMETHODCALLTYPE Thunk(
			ID3D11DeviceContext* a_context,
			UINT a_vertexCountPerInstance,
			UINT a_instanceCount,
			UINT a_startVertex,
			UINT a_startInstance)
		{
			InvokeDirectDraw(a_context, "DrawInstanced", [&] {
				function(a_context, a_vertexCountPerInstance, a_instanceCount, a_startVertex, a_startInstance);
			});
		}
		static inline Function function{};
	};

	void InstallDirectDrawHooks(ID3D11DeviceContext* a_context)
	{
		if (!a_context || directDrawHooksInstalled.load(std::memory_order_acquire)) {
			return;
		}
		static std::mutex installMutex;
		std::scoped_lock lock(installMutex);
		if (directDrawHooksInstalled.load(std::memory_order_relaxed)) {
			return;
		}

		auto vtable = *reinterpret_cast<std::uintptr_t**>(a_context);
		if (!vtable) {
			return;
		}
		DrawIndexedHook::function = reinterpret_cast<DrawIndexedHook::Function>(vtable[12]);
		DrawHook::function = reinterpret_cast<DrawHook::Function>(vtable[13]);
		DrawIndexedInstancedHook::function = reinterpret_cast<DrawIndexedInstancedHook::Function>(vtable[20]);
		DrawInstancedHook::function = reinterpret_cast<DrawInstancedHook::Function>(vtable[21]);

		REL::safe_write(reinterpret_cast<std::uintptr_t>(vtable + 12), reinterpret_cast<std::uintptr_t>(&DrawIndexedHook::Thunk));
		REL::safe_write(reinterpret_cast<std::uintptr_t>(vtable + 13), reinterpret_cast<std::uintptr_t>(&DrawHook::Thunk));
		REL::safe_write(reinterpret_cast<std::uintptr_t>(vtable + 20), reinterpret_cast<std::uintptr_t>(&DrawIndexedInstancedHook::Thunk));
		REL::safe_write(reinterpret_cast<std::uintptr_t>(vtable + 21), reinterpret_cast<std::uintptr_t>(&DrawInstancedHook::Thunk));
		directDrawHooksInstalled.store(true, std::memory_order_release);
		spdlog::info("Installed ID3D11 direct draw hooks on context vtable 0x{:X}", reinterpret_cast<std::uintptr_t>(vtable));
	}

	template <RE::BSShader::Type ShaderType>
	struct PlayerStencilSetupHook
	{
		static void Thunk(RE::BSShader* a_shader, RE::BSRenderPass* a_pass, std::uint32_t a_renderFlags)
		{
			function(a_shader, a_pass, a_renderFlags);
			PlayerStencilScope<ShaderType>::Begin(a_pass);
			SubjectDrawAudit::Begin(a_pass, ShaderType == RE::BSShader::Type::Lighting);
		}
		static inline REL::Relocation<decltype(Thunk)> function;
	};

	template <RE::BSShader::Type ShaderType>
	struct PlayerStencilRestoreHook
	{
		static void Thunk(RE::BSShader* a_shader, RE::BSRenderPass* a_pass, std::uint32_t a_renderFlags)
		{
			SubjectDrawAudit::End(ShaderType == RE::BSShader::Type::Lighting);
			PlayerStencilScope<ShaderType>::End();
			function(a_shader, a_pass, a_renderFlags);
		}
		static inline REL::Relocation<decltype(Thunk)> function;
	};

	void SetupLogging()
	{
		auto directory = SKSE::log::log_directory();
		if (!directory) {
			return;
		}
		*directory /= "CinematicDoFStandalone.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(directory->string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
	}

	struct MainPostProcessingHook
	{
		static void Thunk(
			RE::ImageSpaceManager* a_imageSpaceManager,
			std::uint32_t a_unk1,
			RE::RENDER_TARGET a_target,
			void* a_unk2,
			bool a_unk3)
		{
			if (!firstMainPathLogged.exchange(true)) {
				spdlog::info("Entered main post-processing path");
			}
			auto& renderer = CDoF::DoFRenderer::GetSingleton();
			CDoF::SubjectMask::UpdateLockOnState();
			CDoF::SubjectMask::CompleteFrame();
			renderer.ApplyBeforePostProcessing();
			function(a_imageSpaceManager, a_unk1, a_target, a_unk2, a_unk3);
			renderer.ApplyAfterPostProcessing(a_target);
            CDoF::SubjectMask::EndFrame();
			SubjectDrawAudit::ReportFrame();
		}
		static inline REL::Relocation<decltype(Thunk)> function;
	};

	struct PreOpaqueDepthHook
	{
		static void Thunk(
			RE::BSBatchRenderer* a_batchRenderer,
			std::uint32_t a_startRange,
			std::uint32_t a_endRanges,
			std::uint32_t a_renderFlags,
			int a_geometryGroup)
		{
			// This call site is reached after the depth prepass and immediately before
			// the first opaque batches. Install after Community/Bottled Shaders so our
			// saved original continues through their deferred-rendering thunk.
			CDoF::DoFRenderer::GetSingleton().CapturePreOpaqueDepth();
			function(a_batchRenderer, a_startRange, a_endRanges, a_renderFlags, a_geometryGroup);
		}
		static inline REL::Relocation<decltype(Thunk)> function;
	};

	void InstallMainPostProcessingHook()
	{
		static REL::Relocation<std::uintptr_t> mainPostProcessing{
			REL::RelocationID(100430, 107148),
			static_cast<std::ptrdiff_t>(REL::VariantOffset(0x1F0, 0x1E7, 0x206).offset())
		};
		MainPostProcessingHook::function = SKSE::GetTrampoline().write_call<5>(
			mainPostProcessing.address(),
			MainPostProcessingHook::Thunk);
		spdlog::info("Installed main post-processing hook at 0x{:X}", mainPostProcessing.address());
	}

	void InstallHooks()
	{
		static std::atomic_bool installed{ false };
		if (installed.exchange(true)) {
			return;
		}

		static REL::Relocation<std::uintptr_t> firstOpaqueBatches{
			REL::RelocationID(99938, 106583),
			static_cast<std::ptrdiff_t>(REL::Relocate(0x8E, 0x84))
		};
		PreOpaqueDepthHook::function = SKSE::GetTrampoline().write_call<5>(
			firstOpaqueBatches.address(),
			PreOpaqueDepthHook::Thunk);

		static REL::Relocation<std::uintptr_t> lightingShaderVTable{ RE::VTABLE_BSLightingShader[0] };
		PlayerStencilSetupHook<RE::BSShader::Type::Lighting>::function = lightingShaderVTable.write_vfunc(
			0x6, PlayerStencilSetupHook<RE::BSShader::Type::Lighting>::Thunk);
		PlayerStencilRestoreHook<RE::BSShader::Type::Lighting>::function = lightingShaderVTable.write_vfunc(
			0x7, PlayerStencilRestoreHook<RE::BSShader::Type::Lighting>::Thunk);

		static REL::Relocation<std::uintptr_t> effectShaderVTable{ RE::VTABLE_BSEffectShader[0] };
		PlayerStencilSetupHook<RE::BSShader::Type::Effect>::function = effectShaderVTable.write_vfunc(
			0x6, PlayerStencilSetupHook<RE::BSShader::Type::Effect>::Thunk);
		PlayerStencilRestoreHook<RE::BSShader::Type::Effect>::function = effectShaderVTable.write_vfunc(
			0x7, PlayerStencilRestoreHook<RE::BSShader::Type::Effect>::Thunk);

		spdlog::info(
			"Installed render hooks: player stencil, pre-opaque depth at 0x{:X}",
			firstOpaqueBatches.address());
	}

	void MessageHandler(SKSE::MessagingInterface::Message* a_message)
	{
		if (!a_message) {
			return;
		}
		if (a_message->type == SKSE::MessagingInterface::kPreLoadGame ||
            a_message->type == SKSE::MessagingInterface::kNewGame) {
            CDoF::SubjectMask::ResetLockOnState();
            CDoF::SubjectMask::UsePlayer();
        } else if (a_message->type == SKSE::MessagingInterface::kPostLoad) {
			// CS installs its HDR wrapper at PostPostLoad. Chain inside it so
			// the depth frame runs before CS restores the framebuffer slot.
			InstallMainPostProcessingHook();
			CDoF::TDMIntegration::Initialize();
			CDoF::UI::TryRegister();
		} else if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
			CDoF::HotkeyInput::Register();
			InstallHooks();
			CDoF::UI::TryRegister();
		}
	}
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	SetupLogging();
	spdlog::info("CinematicDoFStandalone loading - 1.0.13");

	try {
		SKSE::Init(a_skse, false);
		SKSE::AllocTrampoline(64);
		const auto settings = CDoF::LoadSettings();
		CDoF::DoFRenderer::GetSingleton().SetSettings(settings);
		CDoF::UI::Initialize(settings);
		if (!SKSE::GetMessagingInterface()->RegisterListener(MessageHandler)) {
			spdlog::critical("Failed to register SKSE message listener");
			return false;
		}
		spdlog::info("Runtime {} accepted; waiting for post-load", REL::Module::get().version().string());
		return true;
	} catch (const std::exception& error) {
		spdlog::critical("Plugin load failed: {}", error.what());
		return false;
	} catch (...) {
		spdlog::critical("Plugin load failed with an unknown exception");
		return false;
	}
}
