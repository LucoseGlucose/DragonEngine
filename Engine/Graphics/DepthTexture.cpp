#include "stdafx.h"
#include "DepthTexture.h"

#include "Rendering.h"

DepthTexture::DepthTexture(RenderTextureProfile profile) :
	RenderTexture(profile, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE)
{
	dsDesc.Format = format;
	dsDesc.ViewDimension = samples < 2 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS;
}

void DepthTexture::CreateDSV(const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
	Rendering::device->CreateDepthStencilView(resourceBuffer.Get(), &dsDesc, handle);
}
