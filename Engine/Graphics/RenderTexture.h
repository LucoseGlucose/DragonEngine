#pragma once

#include "Texture.h"
#include "RenderTextureProfile.h"

class RenderTexture : public Texture
{
public:

	Vector2 size;
	DXGI_FORMAT format;
	D3D12_CLEAR_VALUE clearValue;
	uint8_t samples;

	virtual void Resize(Vector2 size);
	virtual void Resize(Vector2 size, ID3D12Resource** existingResource);

protected:

	RenderTexture(ID3D12Resource** existingResource, D3D12_CLEAR_VALUE clearValue, D3D12_RESOURCE_STATES startingState);
	RenderTexture(RenderTextureProfile profile, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES startingState);

	D3D12_RESOURCE_FLAGS flags;
	D3D12_RESOURCE_STATES startingState;
};