#include "stdafx.h"
#include "Shader.h"

#include <d3dcompiler.h>
#include <d3d12shader.h>

Shader::Shader(const std::filesystem::path& file, SHADER_TYPE shaderType)
{
	if (!std::filesystem::exists(file)) throw std::exception("File does not exist!");
	if (file.extension() != ".cso") throw std::exception("Unsupported file format, must be .cso!");

	filePath = file;
	type = shaderType;

	Utils::ThrowIfFailed(D3DReadFileToBlob(file.c_str(), &shaderCode));

	bytecode = D3D12_SHADER_BYTECODE();
	bytecode.BytecodeLength = shaderCode->GetBufferSize();
	bytecode.pShaderBytecode = shaderCode->GetBufferPointer();
}

Shader* Shader::Create(const std::filesystem::path& file, SHADER_TYPE shaderType)
{
	if (createdShaders.contains(file)) return createdShaders[file];
	return new Shader(file, shaderType);
}
