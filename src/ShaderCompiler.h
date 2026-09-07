#pragma once

namespace CDoF
{
	ComPtr<ID3D11ComputeShader> CompileComputeShader(
		ID3D11Device* a_device,
		const std::filesystem::path& a_shaderPath,
		const char* a_entryPoint);
}
