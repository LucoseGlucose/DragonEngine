#pragma once

#include "RenderPass.h"
#include "LightComponent.h"
#include "RendererComponent.h"
#include "TextureCubemap.h"

class SceneRenderPass : public RenderPass
{
public:
	SceneRenderPass();
	~SceneRenderPass();

	std::vector<LightComponent*> lights;
	std::vector<RendererComponent*> renderers;

	TextureCubemap* skyboxTexture;
	TextureCubemap* diffuseSkybox;
	TextureCubemap* specularSkybox;

	Material* skyboxMat;
	Mesh* skyboxMesh;

	virtual void Execute(Framebuffer* inputFB, CommandRecorder* recorder) override;
};