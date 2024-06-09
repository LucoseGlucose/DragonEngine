#pragma once

#include <d3d12.h>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct RenderTextureProfile
{
	Vector2 size;
	DXGI_FORMAT format;
	D3D12_CLEAR_VALUE clearValue;
	uint8_t samples;

	RenderTextureProfile() = default;

	RenderTextureProfile(const Vector2& size, const DXGI_FORMAT& format, const D3D12_CLEAR_VALUE& clearValue, const uint8_t& samples)
		: size(size), format(format), clearValue(clearValue), samples(samples)
	{

	}
};