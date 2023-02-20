#pragma once

#include "Shader.h"

class ShaderProgram
{
	ShaderProgram(const std::map<SHADER_TYPE, Shader*>& shaderList, uint32_t samples, DXGI_FORMAT format);

public:
	static inline std::map<std::map<SHADER_TYPE, Shader*>, ShaderProgram*> createdPrograms{};

	std::map<SHADER_TYPE, Shader*> shaders;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pipeline;

	static ShaderProgram* Create(const std::map<SHADER_TYPE, Shader*>& shaderList, uint32_t samples, DXGI_FORMAT format);
	static ShaderProgram* Create(const std::filesystem::path& vertexShader,
		const std::filesystem::path& pixelShader, uint32_t samples, DXGI_FORMAT format);
};