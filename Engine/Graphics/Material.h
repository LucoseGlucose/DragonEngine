#pragma once

#include "ShaderProgram.h"
#include "Texture.h"
#include "CommandRecorder.h"
#include "DescriptorHeap.h"

typedef D3D12_SAMPLER_DESC Sampler;

class Material
{
public:

	Material(ShaderProgram* shader);
	~Material();

	ShaderProgram* shader;

	std::map<std::string, XMUINT2> cbParameters{};
	std::vector<ComPtr<ID3D12Resource>> parameterBuffers{};
	std::vector<UINT8*> cbGPUAddresses{};

	std::map<std::string, UINT32> textureParameters{};
	DescriptorHeap resourceDescHeap;

	std::map<std::string, UINT32> samplerParameters{};
	DescriptorHeap samplerDescHeap;

	std::map<std::string, UINT32> uavParameters{};

	std::map<std::string, Texture*> cachedTextures{};
	std::map<std::string, Sampler> cachedSamplers{};
	std::map<std::string, GraphicsResource*> cachedUAVs{};

	void SetParameter(const std::string& name, void* data, size_t size);
	void SetTexture(const std::string& name, Texture* texture);
	void SetSampler(const std::string& name, Sampler sampler);
	void SetUAV(const std::string& name, GraphicsResource* uav);

	void GetParameter(const std::string& name, void* data, size_t size);
	Texture* GetTexture(const std::string& name);
	Sampler GetSampler(const std::string& name);
	GraphicsResource* GetUAV(const std::string& name);

	void UpdateTexture(const std::string& name, Texture* texture);
	void UpdateUAV(const std::string& name, GraphicsResource* uav);

	void Bind(CommandRecorder* recorder, PipelineProfile profile);

	template<typename T>
	void SetParameter(const std::string& name, T data)
	{
		SetParameter(name, &data, sizeof(data));
	}

	template<typename T>
	void SetParameter(const std::string& name, T* data)
	{
		SetParameter(name, data, sizeof(T));
	}

	template<typename T>
	void GetParameter(const std::string& name, T* data)
	{
		T value{};
		GetParameter(name, &value, sizeof(data));
		*data = value;
	}

	template<typename T>
	T GetParameter(const std::string& name)
	{
		T value{};
		GetParameter(name, &value, sizeof(T));
		return value;
	}
};