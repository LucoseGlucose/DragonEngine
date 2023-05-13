#pragma once

#include "RenderPass.h"
#include "Material.h"

class ProcessRenderPass : public RenderPass
{
public:
	ProcessRenderPass(Material* material);
	~ProcessRenderPass();

	Material* material;

	virtual void Execute(Framebuffer* inputFB, CommandRecorder* recorder) override;
	virtual void Resize(Framebuffer* inputFB, XMUINT2 newSize) override;
};