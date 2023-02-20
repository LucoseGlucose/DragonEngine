#include "stdafx.h"
#include "RendererComponent.h"

#include "Rendering.h"

RendererComponent::RendererComponent(SceneObject* owner) : Component(owner)
{

}

RendererComponent::~RendererComponent()
{
	delete mesh;
	delete material;
}

std::function<void(RendererComponent*)> RendererComponent::GetSetParamFunc()
{
	return [](RendererComponent* renderer)
	{
		XMFLOAT4X4 projMat = Rendering::outputCam->GetProjectionMat();
		XMFLOAT4X4 viewMat = Rendering::outputCam->GetViewMat();
		XMFLOAT4X4 modelMat = renderer->GetOwner()->GetTransform()->GetMatrix();

		XMMATRIX multiplied = DirectX::XMLoadFloat4x4(&modelMat) * DirectX::XMLoadFloat4x4(&viewMat) * DirectX::XMLoadFloat4x4(&projMat);

		XMFLOAT4X4 mvpMat;
		DirectX::XMStoreFloat4x4(&mvpMat, DirectX::XMMatrixTranspose(multiplied));

		renderer->material->SetParameter("mvpMat", &mvpMat, sizeof(mvpMat));
	};
}

void RendererComponent::Render()
{
	if (mesh == nullptr || material == nullptr) return;

	GetSetParamFunc()(this);

	material->Bind();
	mesh->Draw();
}