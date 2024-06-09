#pragma once

#include "ColorTexture.h"
#include "DepthTexture.h"
#include "CommandRecorder.h"
#include "PipelineProfile.h"
#include "DescriptorHeap.h"

class Framebuffer
{
public:
	Framebuffer(RenderTextureProfile dsProfile, std::initializer_list<RenderTextureProfile> rtvProfiles);
	Framebuffer(std::initializer_list<RenderTextureProfile> rtvProfiles);
	Framebuffer(RenderTextureProfile dsProfile);
	~Framebuffer();

	std::vector<ColorTexture*> colorTextures;
	DepthTexture* depthTexture;

	DescriptorHeap colorDescHeap;
	DescriptorHeap depthDescHeap;

	PipelineProfile pipelineProfile;

	Vector2 size;
	UINT8 samples;

	void Resize(Vector2 newSize);
	void Setup(CommandRecorder* recorder, UINT8 colorClearMask = 0b11111111, bool clearDepth = true);
	void Clear(CommandRecorder* recorder, UINT8 colorMask = 0b11111111, bool depth = true);

	void BlitTo(CommandRecorder* recorder, Framebuffer* dest, UINT8 colorMask = 0xff, bool depthStencil = false);
	void CopyTo(CommandRecorder* recorder, Framebuffer* dest, UINT8 colorMask = 0xff, bool depthStencil = false);
	void ResolveTo(CommandRecorder* recorder, Framebuffer* dest, UINT8 colorMask = 0xff, bool depthStencil = false);
};