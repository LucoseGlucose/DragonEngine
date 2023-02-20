#pragma once

#include "Texture.h"
#include <DirectXMath.h>

using namespace DirectX;

class Texture2D : public Texture
{
public:
	Texture2D(void* data, size_t dataSize, XMUINT2 size, DXGI_FORMAT format, uint32_t bytesPerPixel);

	XMUINT2 size;
	ComPtr<ID3D12Resource> uploadBuffer;

	static Texture2D* Import(const std::filesystem::path& file, bool sRGB);
};