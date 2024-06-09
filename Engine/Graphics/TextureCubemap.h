#pragma once

#include "Texture.h"
#include "CommandRecorder.h"

class TextureCubemap : public Texture
{
public:
	TextureCubemap(std::array<void*, 6> data, Vector2 size, DXGI_FORMAT format, UINT32 bytesPerPixel, UINT32 mipCount);

	Vector2 size;
	ComPtr<ID3D12Resource> uploadBuffer;
	UINT32 mipCount;
	DXGI_FORMAT format;

	void GenerateMipMaps();
	static std::array<Matrix, 6> GetCubemapMatrices();

	static TextureCubemap* Import(const std::array<std::filesystem::path, 6>& files, bool sRGB, bool generateMips);
	static TextureCubemap* ImportHDR(const std::filesystem::path& file, bool generateMips);
	static TextureCubemap* ComputeDiffuseIrradiance(TextureCubemap* skybox, Vector2 size);
	static TextureCubemap* ComputeAmbientSpecular(TextureCubemap* skybox, Vector2 size, UINT32 mipCount, float sampleCount);
};