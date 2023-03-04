#pragma once

#include "Texture.h"
#include "CommandRecorder.h"

class Texture2D : public Texture
{
	static inline Texture2D* whiteTexture{};
	static inline Texture2D* normalTexture{};
	static inline Texture2D* brdfTexture{};

public:
	Texture2D(void* data, XMUINT2 size, uint32_t bytesPerPixel, uint32_t mipCount, DXGI_FORMAT format);

	XMUINT2 size;
	ComPtr<ID3D12Resource> uploadBuffer;
	uint32_t mipCount;
	DXGI_FORMAT format;

	static Texture2D* Import(const std::filesystem::path& file, bool sRGB, bool generateMips);
	static Texture2D* ImportHDR(const std::filesystem::path& file);
	void GenerateMipMaps(CommandRecorder* recorder);

	static Texture2D* GetWhiteTexture();
	static Texture2D* GetNormalTexture();
	static Texture2D* GetBRDFTexture();
};