#pragma once

typedef enum SHADER_TYPE
{
	SHADER_TYPE_VERTEX,
	SHADER_TYPE_PIXEL,
	SHADER_TYPE_MAX,
} ShaderType;

class Shader
{
	Shader(const std::filesystem::path& file, SHADER_TYPE shaderType);

public:
	Shader(const Shader& mat) = default;

	static inline std::map<std::filesystem::path, Shader*> createdShaders{};

	std::filesystem::path filePath;
	SHADER_TYPE type;
	D3D12_SHADER_BYTECODE bytecode;

	ComPtr<ID3DBlob> shaderCode;

	static Shader* Create(const std::filesystem::path& file, SHADER_TYPE shaderType);
};