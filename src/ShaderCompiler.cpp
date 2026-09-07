#include "ShaderCompiler.h"

namespace
{
	class IncludeHandler final : public ID3DInclude
	{
	public:
		explicit IncludeHandler(std::filesystem::path a_root) : root_(std::move(a_root)) {}

		HRESULT Open(
			D3D_INCLUDE_TYPE,
			LPCSTR a_fileName,
			LPCVOID,
			LPCVOID* a_data,
			UINT* a_size) override
		{
			const auto path = root_ / a_fileName;
			std::ifstream stream(path, std::ios::binary | std::ios::ate);
			if (!stream) {
				spdlog::error("Shader include not found: {}", path.string());
				*a_data = nullptr;
				*a_size = 0;
				return E_FAIL;
			}

			const auto end = stream.tellg();
			if (end <= 0 || end > static_cast<std::streamoff>(UINT_MAX)) {
				return E_FAIL;
			}
			const auto size = static_cast<UINT>(end);
			auto bytes = std::make_unique<char[]>(size);
			stream.seekg(0, std::ios::beg);
			stream.read(bytes.get(), size);
			if (!stream) {
				return E_FAIL;
			}
			*a_data = bytes.release();
			*a_size = size;
			return S_OK;
		}

		HRESULT Close(LPCVOID a_data) override
		{
			delete[] static_cast<const char*>(a_data);
			return S_OK;
		}

	private:
		std::filesystem::path root_;
	};
}

ComPtr<ID3D11ComputeShader> CDoF::CompileComputeShader(
	ID3D11Device* a_device,
	const std::filesystem::path& a_shaderPath,
	const char* a_entryPoint)
{
	ComPtr<ID3DBlob> byteCode;
	ComPtr<ID3DBlob> errors;
	IncludeHandler includes(a_shaderPath.parent_path().parent_path());
	const D3D_SHADER_MACRO defines[] = {
		{ "COMPUTESHADER", "" },
		{ "WINPC", "" },
		{ "DX11", "" },
		{ nullptr, nullptr }
	};

	const auto result = D3DCompileFromFile(
		a_shaderPath.c_str(),
		defines,
		&includes,
		a_entryPoint,
		"cs_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3,
		0,
		byteCode.GetAddressOf(),
		errors.GetAddressOf());

	if (FAILED(result)) {
		const auto message = errors ? static_cast<const char*>(errors->GetBufferPointer()) : "unknown compiler error";
		spdlog::error("Shader compilation failed for {}: {}", a_entryPoint, message);
		return {};
	}

	ComPtr<ID3D11ComputeShader> shader;
	const auto createResult = a_device->CreateComputeShader(
		byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, shader.GetAddressOf());
	if (FAILED(createResult)) {
		spdlog::error("CreateComputeShader failed for {} (HRESULT 0x{:08X})", a_entryPoint, static_cast<std::uint32_t>(createResult));
		return {};
	}
	return shader;
}
