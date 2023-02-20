#pragma once

#include "Texture.h"

union View
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsDesc;
};

class RenderTexture : public Texture
{
public:
	RenderTexture(XMUINT2 size, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags,
		D3D12_CLEAR_VALUE clearValue, View view, D3D12_RESOURCE_STATES startingState, uint32_t samples);
	RenderTexture(ID3D12Resource** existingResource, View view);

	XMUINT2 size;
	View descriptor;
	DXGI_FORMAT format;
	D3D12_RESOURCE_FLAGS flags;
	D3D12_CLEAR_VALUE clearValue;
	uint32_t samples;

	void Resize(XMUINT2 size, D3D12_RESOURCE_STATES startingState);
	void Resize(XMUINT2 size, ID3D12Resource** existingResource);
};