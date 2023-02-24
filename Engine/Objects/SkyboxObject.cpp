#include "stdafx.h"
#include "SkyboxObject.h"

#include "RendererComponent.h"
#include "Rendering.h"

SkyboxObject::SkyboxObject(std::string name) : SceneObject(name)
{
	RendererComponent* renderer = AddComponent<RendererComponent>();
	renderer->mesh = new Mesh(Utils::GetPathFromProject("Models/Inverted Cube.fbx"));
	renderer->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("SkyboxVertex.cso"),
		Utils::GetPathFromExe("SkyboxPixel.cso"), 4, DXGI_FORMAT_R16G16B16A16_FLOAT));

	renderer->shaderParamFunc = [](RendererComponent* renderer)
	{
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
