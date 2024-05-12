#pragma once

#include "Texture.h"
#include "CommandRecorder.h"

class TextureCubemap : public Texture
{
public:
	TextureCubemap(std::array<void*, 6> data, XMUINT2 size, DXGI_FORMAT format, uint32_t bytesPerPixel, uint32_t mipCount);

	XMUINT2 size;
	ComPtr<ID3D12Resource> uploadBuffer;
	uint32_t mipCount;
	DXGI_FORMAT format;

	void GenerateMipMaps();
	static XMFLOAT4X4* GetCubemapMatrices();

	static TextureCubemap* Import(const std::array<std::filesystem::path, 6>& files, bool sRGB, bool generateMips);
	static TextureCubemap* ImportHDR(const std::filesystem::path& file, bool generateMips);
	static TextureCubemap* ComputeDiffuseIrradiance(TextureCubemap* skybox, XMUINT2 size);
	static TextureCubemap* ComputeAmbientSpecular(TextureCubemap* skybox, XMUINT2 size, uint32_t mipCount, float sampleCount);
};