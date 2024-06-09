#pragma once

#include "ColorTexture.h"
#include "DepthTexture.h"
#include "CommandRecorder.h"
#include "EngineSettings.h"
#include "DescriptorHeap.h"
#include "PipelineProfile.h"

class PresentationBuffer
{
public:
	PresentationBuffer();
	~PresentationBuffer();

	ComPtr<IDXGISwapChain3> swapchain;
	std::array<ColorTexture*, Settings::numPresentationFrames> colorTextures;

	DescriptorHeap rtvDescHeap;

	Vector4 clearColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	PipelineProfile pipelineProfile;

	void Resize(Vector2 newSize);
	void Setup(CommandRecorder* recorder);
	void Present(CommandRecorder* recorder);
};