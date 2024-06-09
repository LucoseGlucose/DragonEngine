#pragma once

#include "RenderTexture.h"

class ColorTexture : public RenderTexture
{
public:

	ColorTexture(RenderTextureProfile profile);
	ColorTexture(ID3D12Resource** existingResource, D3D12_CLEAR_VALUE clearValue);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
};