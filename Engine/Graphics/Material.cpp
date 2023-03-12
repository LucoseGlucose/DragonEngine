#include "stdafx.h"
#include "Material.h"

#include "Rendering.h"
#include "Texture2D.h"

#include <d3d12shader.h>
#include <d3dcompiler.h>

Material::Material(ShaderProgram* shader) : shader(shader)
{
	uint32_t numConstantBuffers = 0;
	uint32_t numTextures = 0;
	uint32_t numSamplers = 0;

	std::vector<std::pair<std::string, char>> defaultTextures{};
	std::vector<std::string> defaultSamplers{};

	for (size_t i = 0; i < SHADER_TYPE_MAX; i++)
	{
		uint32_t constantBuffersPerShader = 0;

		SHADER_TYPE type = (SHADER_TYPE)i;
		if (!shader->shaders.contains(type)) continue;
		Shader* currentShader = shader->shaders[type];

		ComPtr<ID3D12ShaderReflection> shaderReflection;
		Utils::ThrowIfFailed(D3DReflect(currentShader->bytecode.pShaderBytecode,
			currentShader->bytecode.BytecodeLength, IID_PPV_ARGS(&shaderReflection)));

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
				textureParameters[bindDesc.Name] = numTextures;
				cachedTextures[bindDesc.Name] = nullptr;
				numTextures++;

				std::string varName = bindDesc.Name;
				defaultTextures.push_back(std::pair<std::string, char>(varName, varName.back()));
			}
			if (bindDesc.Type == D3D_SIT_SAMPLER)
			{
				samplerParameters[bindDesc.Name] = numSamplers;
				cachedSamplers[bindDesc.Name] = Utils::GetDefaultSampler();

				defaultSamplers.push_back(bindDesc.Name);
				numSamplers++;
			}
		}
	}

	if (textureParameters.size() > 0)
	{
		D3D12_DESCRIPTOR_HEAP_DESC textureHeapDesc{};
		textureHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		textureHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		textureHeapDesc.NumDescriptors = numTextures;

		Utils::ThrowIfFailed(Rendering::device->CreateDescriptorHeap(&textureHeapDesc, IID_PPV_ARGS(&textureDescHeap)));
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
		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc{};
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		samplerHeapDesc.NumDescriptors = numSamplers;

		Utils::ThrowIfFailed(Rendering::device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerDescHeap)));
	}

	for (size_t i = 0; i < defaultSamplers.size(); i++)
	{
		SetSampler(defaultSamplers[i], Utils::GetDefaultSampler());
	}
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

	uint32_t index = textureParameters[name];

	D3D12_CPU_DESCRIPTOR_HANDLE texDescStartHnd = textureDescHeap->GetCPUDescriptorHandleForHeapStart();
	UINT incrementSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE texDescHnd = CD3DX12_CPU_DESCRIPTOR_HANDLE(texDescStartHnd, index, incrementSize);

	Rendering::device->CreateShaderResourceView(texture->textureBuffer.Get(), &texture->srvDesc, texDescHnd);
	cachedTextures[name] = texture;
}

void Material::SetSampler(const std::string& name, Sampler sampler)
{
	if (!samplerParameters.contains(name)) return;

	uint32_t index = samplerParameters[name];

	D3D12_CPU_DESCRIPTOR_HANDLE samplerDescStartHnd = samplerDescHeap->GetCPUDescriptorHandleForHeapStart();
	UINT incrementSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE samplerDescHnd = CD3DX12_CPU_DESCRIPTOR_HANDLE(samplerDescStartHnd, index, incrementSize);

	Rendering::device->CreateSampler(&sampler, samplerDescHnd);
	cachedSamplers[name] = sampler;
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

void Material::UpdateTexture(const std::string& name, Texture* texture)
{
	if (!textureParameters.contains(name) || cachedTextures[name] != texture) return;

	uint32_t index = textureParameters[name];

	D3D12_CPU_DESCRIPTOR_HANDLE texDescStartHnd = textureDescHeap->GetCPUDescriptorHandleForHeapStart();
	UINT incrementSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE texDescHnd = CD3DX12_CPU_DESCRIPTOR_HANDLE(texDescStartHnd, index, incrementSize);

	Rendering::device->CreateShaderResourceView(texture->textureBuffer.Get(), &texture->srvDesc, texDescHnd);
}

void Material::Bind(CommandRecorder* recorder)
{
	recorder->list->SetPipelineState(shader->pipeline.Get());
	recorder->list->SetGraphicsRootSignature(shader->rootSignature.Get());

	std::vector<ID3D12DescriptorHeap*> descHeaps{};

	if (textureParameters.size() > 0) descHeaps.push_back(textureDescHeap.Get());
	if (samplerParameters.size() > 0) descHeaps.push_back(samplerDescHeap.Get());

	if (descHeaps.size() > 0) recorder->list->SetDescriptorHeaps(descHeaps.size(), descHeaps.data());

	for (size_t i = 0; i < parameterBuffers.size(); i++)
	{
		recorder->list->SetGraphicsRootConstantBufferView(i, parameterBuffers[i]->GetGPUVirtualAddress());
	}

	for (size_t i = 0; i < textureParameters.size(); i++)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE descHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(textureDescHeap->GetGPUDescriptorHandleForHeapStart(),
			i, Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		recorder->list->SetGraphicsRootDescriptorTable(parameterBuffers.size() + i, descHandle);
	}

	for (size_t i = 0; i < samplerParameters.size(); i++)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE descHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(samplerDescHeap->GetGPUDescriptorHandleForHeapStart(),
			i, Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));
		recorder->list->SetGraphicsRootDescriptorTable(parameterBuffers.size() + textureParameters.size() + i, descHandle);
	}
}
