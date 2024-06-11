#pragma once

#include "RenderTexture.h"

class DepthTexture : public RenderTexture
{
public:

	DepthTexture(RenderTextureProfile profile);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsDesc;

	void CreateDSV(const D3D12_CPU_DESCRIPTOR_HANDLE& handle);
};