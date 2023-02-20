#include "stdafx.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent(SceneObject* owner) : Component(owner)
{
	XMINT2 windowSize = Application::GetFramebufferSize();

	DirectX::XMStoreFloat4x4(&projectionMat, XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(fieldOfView), (float)windowSize.x / windowSize.y, nearClip, farClip));

	XMFLOAT3 pos = owner->GetTransform()->GetPosition();
	XMFLOAT3 fwd = owner->GetTransform()->GetForward();
	XMFLOAT3 up = owner->GetTransform()->GetUp();

	DirectX::XMStoreFloat4x4(&viewMat, XMMatrixLookToLH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&fwd), DirectX::XMLoadFloat3(&up)));
}

XMFLOAT4X4 CameraComponent::GetProjectionMat()
{
	return projectionMat;
}

XMFLOAT4X4 CameraComponent::GetViewMat()
{
	return viewMat;
}
