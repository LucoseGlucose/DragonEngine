#pragma once

#include "Texture.h"
#include "CommandRecorder.h"

class Texture2D : public Texture
{
public:
	Texture2D(void* data, XMUINT2 size, uint32_t bytesPerPixel, uint32_t mipCount, bool sRGB);

	XMUINT2 size;
	ComPtr<ID3D12Resource> uploadBuffer;
	uint32_t mipCount;
	bool sRGB;

	static Texture2D* Import(const std::filesystem::path& file, bool sRGB, bool generateMips);
	void GenerateMipMaps(CommandRecorder* recorder);
};