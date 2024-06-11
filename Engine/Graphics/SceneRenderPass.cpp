#include "stdafx.h"
#include "SceneRenderPass.h"

#include "SceneManager.h"
#include "Rendering.h"

SceneRenderPass::SceneRenderPass()
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Magenta.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Vector2 size = Application::GetFramebufferSize();

	outputFB = new Framebuffer(RenderTextureProfile(size, DXGI_FORMAT_D32_FLOAT, dsClear, 4),
		{ RenderTextureProfile(size, DXGI_FORMAT_R16G16B16A16_FLOAT, rtClear, 4) });

	skyboxTexture = TextureCubemap::ImportHDR(Utils::GetPathFromProject(Settings::skyboxImagePathFromProject), true);
	diffuseSkybox = TextureCubemap::ComputeDiffuseIrradiance(skyboxTexture, Vector2(32, 32));
	specularSkybox = TextureCubemap::ComputeAmbientSpecular(skyboxTexture, Vector2(1024, 1024), 5, 4096);

	skyboxMat = new Material(ShaderProgram::Create("SkyboxV.cso", "SkyboxP.cso"));
	skyboxMat->SetTexture("t_texture", skyboxTexture);

	skyboxMesh = new Mesh(Utils::GetPathFromProject("Models/Inverted Cube.fbx"));
}

SceneRenderPass::~SceneRenderPass()
{
	delete skyboxTexture;
	delete diffuseSkybox;
	delete specularSkybox;

	delete skyboxMat;
	delete skyboxMesh;
}

void SceneRenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{
	renderers = SceneManager::GetActiveScene()->FindComponents<RendererComponent>();
	lights = SceneManager::GetActiveScene()->FindComponents<LightComponent>();

	Rendering::SetViewportSize(outputFB->colorTextures.front()->size);
	outputFB->Setup(recorder);

	for (size_t i = 0; i < renderers.size(); i++)
	{
		RendererComponent* renderer = renderers.at(i);
		renderer->Render(recorder, outputFB->pipelineProfile);
	}

	Matrix projMat = Rendering::outputCam->GetProjectionMat();
	Matrix viewMat = Rendering::outputCam->GetViewMat();

	XMFLOAT3X3 centerView;
	DirectX::XMStoreFloat3x3(&centerView, DirectX::XMLoadFloat4x4(&viewMat));

	XMMATRIX multiplied = DirectX::XMLoadFloat3x3(&centerView) * DirectX::XMLoadFloat4x4(&projMat);

	XMFLOAT4X4 mvpMat;
	DirectX::XMStoreFloat4x4(&mvpMat, DirectX::XMMatrixTranspose(multiplied));

	skyboxMat->SetParameter("p_mvpMat", &mvpMat);

	skyboxMat->Bind(recorder, outputFB->pipelineProfile);
	skyboxMesh->Draw(recorder);
}
