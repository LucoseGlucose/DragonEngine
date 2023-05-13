#pragma once

#include "RenderPass.h"

class ResolveRenderPass : public RenderPass
{
public:
	ResolveRenderPass();

	virtual void Execute(Framebuffer* inputFB, CommandRecorder* recorder) override;
};