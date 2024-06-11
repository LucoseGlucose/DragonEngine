#include "stdafx.h"
#include "ColorTexture.h"

#include "Rendering.h"

ColorTexture::ColorTexture(RenderTextureProfile profile) :
	RenderTexture(profile, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET)
{
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = samples < 2 ? D3D12_RTV_DIMENSION_TEXTURE2D : D3D12_RTV_DIMENSION_TEXTURE2DMS;
}

ColorTexture::ColorTexture(ID3D12Resource** existingResource, D3D12_CLEAR_VALUE clearValue) :
	RenderTexture(existingResource, clearValue, D3D12_RESOURCE_STATE_RENDER_TARGET)
{
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = samples < 2 ? D3D12_RTV_DIMENSION_TEXTURE2D : D3D12_RTV_DIMENSION_TEXTURE2DMS;
}

void ColorTexture::CreateRTV(const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
	Rendering::device->CreateRenderTargetView(resourceBuffer.Get(), &rtvDesc, handle);
}
