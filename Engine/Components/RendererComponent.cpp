#include "stdafx.h"
#include "RendererComponent.h"

#include "Rendering.h"

RendererComponent::RendererComponent(SceneObject* owner) : Component(owner)
{
	shaderParamFunc = [](RendererComponent* renderer)
	{
		XMFLOAT4X4 projMat = Rendering::outputCam->GetProjectionMat();
		XMFLOAT4X4 viewMat = Rendering::outputCam->GetViewMat();
		XMFLOAT4X4 modelMat = renderer->GetTransform()->GetMatrix();

		XMMATRIX multiplied = DirectX::XMLoadFloat4x4(&modelMat) * DirectX::XMLoadFloat4x4(&viewMat) * DirectX::XMLoadFloat4x4(&projMat);

		XMFLOAT4X4 mvpMat;
		DirectX::XMStoreFloat4x4(&mvpMat, DirectX::XMMatrixTranspose(multiplied));

		renderer->material->SetParameter("p_mvpMat", &mvpMat, sizeof(mvpMat));
	};
}

RendererComponent::~RendererComponent()
{
	delete mesh;
	delete material;
}

void RendererComponent::Render(CommandRecorder* recorder)
{
	if (mesh == nullptr || material == nullptr) return;

	shaderParamFunc(this);

	material->Bind(recorder);
	mesh->Draw(recorder);
}