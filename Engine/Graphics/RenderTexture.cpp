#include "stdafx.h"
#include "RenderTexture.h"

#include "Rendering.h"

RenderTexture::RenderTexture(XMUINT2 size, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_CLEAR_VALUE clearValue,
	View view, D3D12_RESOURCE_STATES startingState, uint32_t samples) : Texture(), format(format), flags(flags),
	clearValue(clearValue), samples(samples), size(size), descriptor(view)
{
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 1, 1, samples, 0, flags);
	textureDesc.SampleDesc.Count = samples;

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc,
		startingState, &clearValue, IID_PPV_ARGS(&textureBuffer)));

	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = samples < 2 ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MipLevels = 1;
}

RenderTexture::RenderTexture(ID3D12Resource** existingResource, View view) : Texture(), descriptor(view)
{
	textureBuffer = *existingResource;

	D3D12_RESOURCE_DESC desc = textureBuffer->GetDesc();
	format = desc.Format;
	size = XMUINT2(desc.Width, desc.Height);
	flags = desc.Flags;
	samples = desc.SampleDesc.Count;

	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = samples < 2 ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MipLevels = 1;
}

void RenderTexture::Resize(XMUINT2 size, D3D12_RESOURCE_STATES startingState)
{
	if (size.x == 0 || size.y == 0) return;
	textureBuffer.Reset();

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 1, 0, samples, 0, flags);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc,
		startingState, &clearValue, IID_PPV_ARGS(&textureBuffer)));
}

void RenderTexture::Resize(XMUINT2 size, ID3D12Resource** existingResource)
{
	if (size.x == 0 || size.y == 0) return;
	textureBuffer = *existingResource;
}
