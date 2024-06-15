#pragma once

#include "ProcessRenderPass.h"

#include "Buffer.h"

class RayTracingRenderPass : public ProcessRenderPass
{
public:

	RayTracingRenderPass();
	~RayTracingRenderPass();

	Buffer* meshBuffer;
	Buffer* indicesBuffer;
	Buffer* verticesBuffer;

	Buffer* meshBufferUpload;
	Buffer* indicesBufferUpload;
	Buffer* verticesBufferUpload;

	void CreateBuffers();

	virtual void Execute(Framebuffer* inputFB, CommandRecorder* recorder) override;
	virtual void Resize(Framebuffer* inputFB, Vector2 newSize) override;
};