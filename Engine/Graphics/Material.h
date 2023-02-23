#pragma once

#include "ShaderProgram.h"
#include "Texture.h"
#include "CommandRecorder.h"

#include <DirectXMath.h>

using namespace DirectX;

typedef D3D12_SAMPLER_DESC Sampler;

class Material
{
public:
	Material(ShaderProgram* shader);
	Material(const Material& mat) = default;

	ShaderProgram* shader;

	std::map<std::string, XMUINT2> cbParameters{};
	std::vector<ComPtr<ID3D12Resource>> parameterBuffers{};
	std::vector<UINT8*> cbGPUAddresses{};

	std::map<std::string, uint32_t> textureParameters{};
	ComPtr<ID3D12DescriptorHeap> textureDescHeap;

	std::map<std::string, uint32_t> samplerParameters{};
	ComPtr<ID3D12DescriptorHeap> samplerDescHeap;
	std::vector<Sampler> samplers{};

	void SetParameter(const std::string& name, void* data, size_t size);
	void SetTexture(const std::string& name, Texture* texture);
	void SetSampler(const std::string& name, Sampler sampler);

	void Bind(CommandRecorder* recorder);
};