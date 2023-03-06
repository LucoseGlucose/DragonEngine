#pragma once

#include "RenderTexture.h"
#include "CommandRecorder.h"

class Framebuffer
{
public:
	Framebuffer(XMUINT2 size, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsFormat,
		D3D12_CLEAR_VALUE rtvClear, D3D12_CLEAR_VALUE dsClear, uint32_t samples);
	~Framebuffer();

	RenderTexture* colorTexture;
	RenderTexture* depthStencilTexture;

	ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
	ComPtr<ID3D12DescriptorHeap> dsDescHeap;

	XMFLOAT4 clearColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	float clearDepth = 1.0f;

	void Resize(XMUINT2 newSize);
	void Setup(CommandRecorder* recorder, bool clearTargets);
	void Blit(CommandRecorder* recorder, Framebuffer* fb, bool color, DXGI_FORMAT colorFormat, bool depthStencil, DXGI_FORMAT dsFormat);
};