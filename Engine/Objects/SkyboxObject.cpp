#include "stdafx.h"
#include "SkyboxObject.h"

#include "Rendering.h"

SkyboxObject::SkyboxObject(std::string name) : SceneObject(name)
{
	renderer = AddComponent<RendererComponent>();
	if (skyboxMesh == nullptr) skyboxMesh = new Mesh(Utils::GetPathFromProject("Models/Inverted Cube.fbx"));

	renderer->mesh = skyboxMesh;

	renderer->shaderParamFunc = [](RendererComponent* renderer)
	{
		renderer->material->SetTexture("t_texture", renderer->GetOwner<SkyboxObject>()->skybox);

		XMFLOAT4X4 projMat = Rendering::outputCam->GetProjectionMat();
		XMFLOAT4X4 viewMat = Rendering::outputCam->GetViewMat();

		XMFLOAT3X3 centerView;
		DirectX::XMStoreFloat3x3(&centerView, DirectX::XMLoadFloat4x4(&viewMat));

		XMMATRIX multiplied = DirectX::XMLoadFloat3x3(&centerView) * DirectX::XMLoadFloat4x4(&projMat);

		XMFLOAT4X4 mvpMat;
		DirectX::XMStoreFloat4x4(&mvpMat, DirectX::XMMatrixTranspose(multiplied));

		renderer->material->SetParameter("p_mvpMat", &mvpMat, sizeof(mvpMat));
	};
}

RendererComponent* SkyboxObject::GetRenderer()
{
	return renderer;
}
