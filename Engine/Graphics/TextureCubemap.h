#pragma once

#include "Texture.h"

class TextureCubemap : public Texture
{
public:
	TextureCubemap(std::array<void*, 6> data, XMUINT2 size, DXGI_FORMAT format, uint32_t bytesPerPixel);

	XMUINT2 size;
	ComPtr<ID3D12Resource> uploadBuffer;

	static TextureCubemap* Import(const std::array<std::filesystem::path, 6>& files, bool sRGB);
	static TextureCubemap* ImportHDR(const std::filesystem::path& file);
};