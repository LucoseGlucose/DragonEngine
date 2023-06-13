#pragma once

#include "RenderPass.h"

class ImGuiRenderPass : public RenderPass
{
public:
	ImGuiRenderPass();

	virtual void Execute(Framebuffer* inputFB, CommandRecorder* recorder) override;
	virtual void Resize(Framebuffer* inputFB, XMUINT2 newSize) override;
};
