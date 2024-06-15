#include "stdafx.h"
#include "Material.h"

#include "Rendering.h"
#include "Texture2D.h"

#include <d3d12shader.h>
#include <dxcapi.h>

Material::Material(ShaderProgram* shader) : shader(shader)
{
	UINT32 numConstantBuffers = 0;
	UINT32 numTextures = 0;
	UINT32 numSamplers = 0;
	UINT32 numUAVs = 0;

	std::vector<std::pair<std::string, char>> defaultTextures{};
	std::vector<std::pair<std::string, char>> defaultSamplers{};

	for (size_t i = 0; i < SHADER_TYPE_MAX; i++)
	{
		UINT32 constantBuffersPerShader = 0;

		SHADER_TYPE type = (SHADER_TYPE)i;
		if (!shader->shaders.contains(type)) continue;
		Shader* currentShader = shader->shaders[type];

		ComPtr<IDxcUtils> utils;
		Utils::ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));

		DxcBuffer shaderBuffer{};
		shaderBuffer.Ptr = currentShader->bytecode.pShaderBytecode;
		shaderBuffer.Size = currentShader->bytecode.BytecodeLength;
		shaderBuffer.Encoding = 0u;

		ComPtr<ID3D12ShaderReflection> shaderReflection;
		Utils::ThrowIfFailed(utils->CreateReflection(&shaderBuffer, IID_PPV_ARGS(&shaderReflection)));

		D3D12_SHADER_DESC shaderDesc{};
		Utils::ThrowIfFailed(shaderReflection->GetDesc(&shaderDesc));

		for (size_t res = 0; res < shaderDesc.BoundResources; res++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			Utils::ThrowIfFailed(shaderReflection->GetResourceBindingDesc(res, &bindDesc));

			if (bindDesc.Type == D3D_SIT_CBUFFER)
			{
				ID3D12ShaderReflectionConstantBuffer* cb = shaderReflection->GetConstantBufferByIndex(constantBuffersPerShader);

				D3D12_SHADER_BUFFER_DESC cbDesc;
				Utils::ThrowIfFailed(cb->GetDesc(&cbDesc));

				ComPtr<ID3D12Resource> currentCB = ComPtr<ID3D12Resource>();
				UINT8* cbGPUAddy = nullptr;

				CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(cbDesc.Size);
				CD3DX12_RANGE mapRange{};

				Rendering::device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
					D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&currentCB));
				parameterBuffers.push_back(currentCB);

				currentCB->Map(0, &mapRange, reinterpret_cast<void**>(&cbGPUAddy));
				cbGPUAddresses.push_back(cbGPUAddy);

				for (size_t vars = 0; vars < cbDesc.Variables; vars++)
				{
					ID3D12ShaderReflectionVariable* var = cb->GetVariableByIndex(vars);

					D3D12_SHADER_VARIABLE_DESC varDesc;
					Utils::ThrowIfFailed(var->GetDesc(&varDesc));

					if (varDesc.DefaultValue != nullptr) memcpy(cbGPUAddy + varDesc.StartOffset, varDesc.DefaultValue, varDesc.Size);
					cbParameters[varDesc.Name] = XMUINT2(numConstantBuffers, varDesc.StartOffset);
				}

				constantBuffersPerShader++;
				numConstantBuffers++;
			}
			if (bindDesc.Type == D3D_SIT_TEXTURE)
			{
				textureParameters[bindDesc.Name] = numUAVs + numTextures;
				cachedTextures[bindDesc.Name] = nullptr;

				std::string varName = bindDesc.Name;
				defaultTextures.push_back(std::pair<std::string, char>(varName, varName.back()));
				numTextures++;
			}
			if (bindDesc.Type == D3D_SIT_SAMPLER)
			{
				samplerParameters[bindDesc.Name] = numSamplers;
				cachedSamplers[bindDesc.Name] = Rendering::GetDefaultSampler();

				std::string varName = bindDesc.Name;
				defaultSamplers.push_back(std::pair<std::string, char>(varName, varName.back()));
				numSamplers++;
			}
			if (bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED || bindDesc.Type == D3D_SIT_UAV_RWTYPED)
			{
				uavParameters[bindDesc.Name] = numUAVs + numTextures;
				cachedUAVs[bindDesc.Name] = nullptr;

				numUAVs++;
			}
		}
	}

	if (textureParameters.size() > 0 || uavParameters.size() > 0)
	{
		resourceDescHeap = DescriptorHeap(numUAVs + numTextures, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	}

	for (size_t i = 0; i < defaultTextures.size(); i++)
	{
		char code = defaultTextures[i].second;

		if (code == 'W') SetTexture(defaultTextures[i].first, Texture2D::GetWhiteTexture());
		if (code == 'N') SetTexture(defaultTextures[i].first, Texture2D::GetNormalTexture());
		if (code == 'L') SetTexture(defaultTextures[i].first, Texture2D::GetBRDFTexture());
	}

	if (samplerParameters.size() > 0)
	{
		samplerDescHeap = DescriptorHeap(numSamplers, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	}

	for (size_t i = 0; i < defaultSamplers.size(); i++)
	{
		char code = defaultSamplers[i].second;

		if (code == 'L') SetSampler(defaultSamplers[i].first, Rendering::GetBRDFSampler());
		else SetSampler(defaultSamplers[i].first, Rendering::GetDefaultSampler());
	}
}

Material::~Material()
{
	delete shader;
}

void Material::SetParameter(const std::string& name, void* data, size_t size)
{
	if (!cbParameters.contains(name)) return;

	XMUINT2& param = cbParameters[name];
	memcpy(cbGPUAddresses[param.x] + param.y, data, size);
}

void Material::SetTexture(const std::string& name, Texture* texture)
{
	if (!textureParameters.contains(name) || cachedTextures[name] == texture) return;

	UINT32 index = textureParameters[name];
	Rendering::device->CreateShaderResourceView(texture->resourceBuffer.Get(), &texture->srvDesc, resourceDescHeap.GetCPUHandleForIndex(index));

	cachedTextures[name] = texture;
}

void Material::SetSampler(const std::string& name, Sampler sampler)
{
	if (!samplerParameters.contains(name)) return;

	UINT32 index = samplerParameters[name];
	Rendering::device->CreateSampler(&sampler, samplerDescHeap.GetCPUHandleForIndex(index));

	cachedSamplers[name] = sampler;
}

void Material::SetUAV(const std::string& name, GraphicsResource* uav)
{
	if (!uavParameters.contains(name) || cachedUAVs[name] == uav) return;

	UINT32 index = uavParameters[name];
	Rendering::device->CreateUnorderedAccessView(uav->resourceBuffer.Get(), nullptr, &uav->uavDesc, resourceDescHeap.GetCPUHandleForIndex(index));

	cachedUAVs[name] = uav;
}

void Material::GetParameter(const std::string& name, void* data, size_t size)
{
	if (!cbParameters.contains(name)) return;

	XMUINT2& param = cbParameters[name];
	void* gpuPtr = cbGPUAddresses[param.x] + param.y;
	memcpy(data, gpuPtr, size);
}

Texture* Material::GetTexture(const std::string& name)
{
	if (!cachedTextures.contains(name)) return nullptr;
	return cachedTextures[name];
}

Sampler Material::GetSampler(const std::string& name)
{
	if (!cachedSamplers.contains(name)) return Sampler{};
	return cachedSamplers[name];
}

GraphicsResource* Material::GetUAV(const std::string& name)
{
	return nullptr;
}

void Material::UpdateTexture(const std::string& name, Texture* texture)
{
	if (!textureParameters.contains(name) || cachedTextures[name] != texture) return;

	UINT32 index = textureParameters[name];
	Rendering::device->CreateShaderResourceView(texture->resourceBuffer.Get(), &texture->srvDesc, resourceDescHeap.GetCPUHandleForIndex(index));
}

void Material::UpdateUAV(const std::string& name, GraphicsResource* uav)
{
	if (!uavParameters.contains(name) || cachedUAVs[name] != uav) return;

	UINT32 index = uavParameters[name];
	Rendering::device->CreateUnorderedAccessView(uav->resourceBuffer.Get(), nullptr, &uav->uavDesc, resourceDescHeap.GetCPUHandleForIndex(index));
}

void Material::Bind(CommandRecorder* recorder, PipelineProfile profile)
{
	if (!shader->compiledPipelines.contains(profile)) shader->CompileShaderForProfile(profile);

	recorder->list->SetPipelineState(shader->compiledPipelines[profile].Get());
	recorder->list->SetGraphicsRootSignature(shader->rootSignature.Get());

	std::vector<ID3D12DescriptorHeap*> descHeaps{};

	if (textureParameters.size() > 0 || uavParameters.size() > 0) descHeaps.push_back(resourceDescHeap.heap.Get());
	if (samplerParameters.size() > 0) descHeaps.push_back(samplerDescHeap.heap.Get());

	if (descHeaps.size() > 0) recorder->list->SetDescriptorHeaps(descHeaps.size(), descHeaps.data());

	for (size_t i = 0; i < parameterBuffers.size(); i++)
	{
		recorder->list->SetGraphicsRootConstantBufferView(i, parameterBuffers[i]->GetGPUVirtualAddress());
	}

	for (size_t i = 0; i < textureParameters.size(); i++)
	{
		recorder->list->SetGraphicsRootDescriptorTable(parameterBuffers.size() + i, resourceDescHeap.GetGPUHandleForIndex(i));
	}

	for (size_t i = 0; i < samplerParameters.size(); i++)
	{
		recorder->list->SetGraphicsRootDescriptorTable(
			parameterBuffers.size() + textureParameters.size() + i, samplerDescHeap.GetGPUHandleForIndex(i));
	}

	for (size_t i = 0; i < uavParameters.size(); i++)
	{
		recorder->list->SetGraphicsRootDescriptorTable(
			parameterBuffers.size() + textureParameters.size() + samplerParameters.size() + i, resourceDescHeap.GetGPUHandleForIndex(i));
	}
}
