#pragma once

#include "RenderPass.h"
#include "LightComponent.h"
#include "RendererComponent.h"
#include "SkyboxObject.h"

class SceneRenderPass : public RenderPass
{
public:
	SceneRenderPass();
	~SceneRenderPass();

	std::vector<LightComponent*> lights;
	std::vector<RendererComponent*> renderers;
	SkyboxObject* skyboxObj;

	virtual void Execute(Framebuffer* inputFB, CommandRecorder* recorder) override;
};