#include "stdafx.h"
#include "RenderTexture.h"

#include "Rendering.h"

RenderTexture::RenderTexture(ID3D12Resource** existingResource, D3D12_CLEAR_VALUE clearValue, D3D12_RESOURCE_STATES startingState) :
	clearValue(clearValue), startingState(startingState)
{
	resourceBuffer = *existingResource;
	NAME_D3D_OBJECT(resourceBuffer);

	D3D12_RESOURCE_DESC desc = (*existingResource)->GetDesc();
	size = Vector2(desc.Width, desc.Height);
	format = desc.Format;
	samples = desc.SampleDesc.Count;
	flags = desc.Flags;

	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = samples < 2 ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MipLevels = 1;

	currentState = startingState;
}

RenderTexture::RenderTexture(RenderTextureProfile profile, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES startingState) : size(profile.size),
format(profile.format), clearValue(profile.clearValue), samples(profile.samples), flags(flags), startingState(startingState)
{
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 1, 1, samples, 0, flags);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		startingState, &clearValue, IID_PPV_ARGS(&resourceBuffer)));
	NAME_D3D_OBJECT(resourceBuffer);

	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = samples < 2 ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MipLevels = 1;

	currentState = startingState;
}

void RenderTexture::Resize(Vector2 size)
{
	if (size.x == 0 || size.y == 0) return;
	resourceBuffer.Reset();

	this->size = size;

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 1, 1, samples, 0, flags);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		startingState, &clearValue, IID_PPV_ARGS(&resourceBuffer)));
	NAME_D3D_OBJECT(resourceBuffer);
}

void RenderTexture::Resize(Vector2 size, ID3D12Resource** existingResource)
{
	if (size.x == 0 || size.y == 0) return;

	resourceBuffer = *existingResource;
	NAME_D3D_OBJECT(resourceBuffer);

	this->size = size;
}
