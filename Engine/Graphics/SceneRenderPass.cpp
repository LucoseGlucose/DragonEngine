#include "stdafx.h"
#include "SceneRenderPass.h"

#include "SceneManager.h"
#include "Rendering.h"

SceneRenderPass::SceneRenderPass()
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	outputFB = new Framebuffer(Application::GetUnsignedFramebufferSize(), DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 4);

	skyboxObj = new SkyboxObject("Skybox");
	skyboxObj->skybox = TextureCubemap::ImportHDR(Utils::GetPathFromProject("Images/limpopo_golf_course_4k.hdr"), true);
	skyboxObj->irradiance = TextureCubemap::ComputeDiffuseIrradiance(skyboxObj->skybox, XMUINT2(32, 32));
	skyboxObj->specular = TextureCubemap::ComputeAmbientSpecular(skyboxObj->skybox, XMUINT2(256, 256), 5);

	skyboxObj->GetRendererComponent()->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("SkyboxV.cso"),
		Utils::GetPathFromExe("SkyboxP.cso"), outputFB));
}

SceneRenderPass::~SceneRenderPass()
{
	delete skyboxObj;
}

void SceneRenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{
	renderers = SceneManager::GetActiveScene()->FindComponents<RendererComponent>();
	lights = SceneManager::GetActiveScene()->FindComponents<LightComponent>();

	Rendering::SetViewportSize(outputFB->colorTexture->size);
	outputFB->Setup(recorder, true);

	for (size_t i = 0; i < renderers.size(); i++)
	{
		RendererComponent* renderer = renderers.at(i);
		renderer->Render(recorder);
	}

	skyboxObj->GetRendererComponent()->Render(recorder);
}
