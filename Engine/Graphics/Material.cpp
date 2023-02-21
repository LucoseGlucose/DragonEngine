#include "stdafx.h"
#include "Material.h"

#include "Rendering.h"
#include <d3d12shader.h>
#include <d3dcompiler.h>

Material::Material(ShaderProgram* shader) : shader(shader)
{
	uint32_t numConstantBuffers = 0;
	uint32_t numTextures = 0;
	uint32_t numSamplers = 0;

	for (size_t i = 0; i < 2; i++)
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
				numTextures++;
			}
			if (bindDesc.Type == D3D_SIT_SAMPLER)
			{
				samplerParameters[bindDesc.Name] = numSamplers;
				samplers.push_back(Sampler());
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

	if (samplerParameters.size() > 0)
	{
		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc{};
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		samplerHeapDesc.NumDescriptors = numSamplers;

		Utils::ThrowIfFailed(Rendering::device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerDescHeap)));
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
	if (!textureParameters.contains(name)) return;

	uint32_t index = textureParameters[name];

	D3D12_CPU_DESCRIPTOR_HANDLE texDescStartHnd = textureDescHeap->GetCPUDescriptorHandleForHeapStart();
	UINT incrementSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE texDescHnd = CD3DX12_CPU_DESCRIPTOR_HANDLE(texDescStartHnd, index, incrementSize);

	Rendering::device->CreateShaderResourceView(texture->textureBuffer.Get(), &texture->srvDesc, texDescHnd);
}

void Material::SetSampler(const std::string& name, Sampler sampler)
{
	if (!samplerParameters.contains(name)) return;

	uint32_t index = samplerParameters[name];
	if (Utils::SamplersEqual(samplers[index], sampler)) return;

	D3D12_CPU_DESCRIPTOR_HANDLE samplerDescStartHnd = samplerDescHeap->GetCPUDescriptorHandleForHeapStart();
	UINT incrementSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE samplerDescHnd = CD3DX12_CPU_DESCRIPTOR_HANDLE(samplerDescStartHnd, index, incrementSize);

	Rendering::device->CreateSampler(&sampler, samplerDescHnd);
	samplers[index] = sampler;
}

void Material::Bind()
{
	Rendering::currentRecorder->list->SetPipelineState(shader->pipeline.Get());
	Rendering::currentRecorder->list->SetGraphicsRootSignature(shader->rootSignature.Get());

	std::vector<ID3D12DescriptorHeap*> descHeaps{};

	if (textureParameters.size() > 0) descHeaps.push_back(textureDescHeap.Get());
	if (samplerParameters.size() > 0) descHeaps.push_back(samplerDescHeap.Get());

	if (descHeaps.size() > 0) Rendering::currentRecorder->list->SetDescriptorHeaps(descHeaps.size(), descHeaps.data());

	for (size_t i = 0; i < parameterBuffers.size(); i++)
	{
		Rendering::currentRecorder->list->SetGraphicsRootConstantBufferView(i, parameterBuffers[i]->GetGPUVirtualAddress());
	}

	if (textureParameters.size() > 0)
		Rendering::currentRecorder->list->SetGraphicsRootDescriptorTable(
			parameterBuffers.size(), textureDescHeap->GetGPUDescriptorHandleForHeapStart());

	if (samplerParameters.size() > 0)
		Rendering::currentRecorder->list->SetGraphicsRootDescriptorTable(
			parameterBuffers.size() + 1, samplerDescHeap->GetGPUDescriptorHandleForHeapStart());
}
