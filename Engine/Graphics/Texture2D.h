#pragma once

#include "Texture.h"
#include "CommandRecorder.h"

class Texture2D : public Texture
{
public:
	Texture2D(void* data, size_t dataSize, XMUINT2 size, DXGI_FORMAT format, uint32_t bytesPerPixel, uint32_t mipCount);

	XMUINT2 size;
	ComPtr<ID3D12Resource> uploadBuffer;
	uint32_t mipCount;

	static Texture2D* Import(const std::filesystem::path& file, bool sRGB, bool generateMips);
	void GenerateMipMaps(CommandRecorder* recorder);
};