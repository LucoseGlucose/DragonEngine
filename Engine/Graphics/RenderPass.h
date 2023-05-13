#pragma once

#include "Framebuffer.h"

class RenderPass
{
public:
	~RenderPass();

	Framebuffer* outputFB;

	virtual void Render(Framebuffer* inputFB);
	virtual void Execute(Framebuffer* inputFB, CommandRecorder* recorder);

	virtual void Resize(Framebuffer* inputFB, XMUINT2 newSize);
};