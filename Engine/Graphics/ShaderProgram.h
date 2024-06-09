#pragma once

#include "Shader.h"
#include "Framebuffer.h"
#include "PipelineProfile.h"

class ShaderProgram
{
	ShaderProgram(const std::map<SHADER_TYPE, Shader*>& shaderList);

public:

	~ShaderProgram();

	static inline std::map<std::map<SHADER_TYPE, Shader*>, ShaderProgram*> createdPrograms{};

	std::map<SHADER_TYPE, Shader*> shaders;
	ComPtr<ID3D12RootSignature> rootSignature;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;

	std::map<PipelineProfile, ComPtr<ID3D12PipelineState>> compiledPipelines;

	void CompileShaderForProfile(const PipelineProfile& profile);

	static ShaderProgram* Create(const std::map<SHADER_TYPE, Shader*>& shaderList);
	static ShaderProgram* Create(const std::filesystem::path& vertexShader, const std::filesystem::path& pixelShader);
};