#pragma once

#include "RenderTexture.h"

using namespace Microsoft::WRL;

class PresentationBuffer
{
public:
	PresentationBuffer();
	~PresentationBuffer();

	ComPtr<IDXGISwapChain3> swapchain;
	std::array<RenderTexture*, 2> colorTextures;
	RenderTexture* depthStencilTexture;

	ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
	ComPtr<ID3D12DescriptorHeap> dsDescHeap;

	XMFLOAT4 clearColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	float clearDepth = 1.0f;

	void Resize(XMUINT2 newSize);
	void Setup();
	void Present();
};