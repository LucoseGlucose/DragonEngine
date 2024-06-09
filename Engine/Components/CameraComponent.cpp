#include "stdafx.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent(SceneObject* owner) : Component(owner)
{
	CalculateProjection();
	CalculateView();
}

void CameraComponent::CalculateProjection()
{
	projectionMat = Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(fieldOfView), size.x / size.y, nearClip, farClip);
}

void CameraComponent::CalculateView()
{
	Vector3 pos = GetTransform()->GetPosition();
	Vector3 fwd = GetTransform()->GetForward();
	Vector3 up = GetTransform()->GetUp();

	DirectX::XMStoreFloat4x4(&viewMat, DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&pos),
		DirectX::XMLoadFloat3(&fwd), DirectX::XMLoadFloat3(&up)));
}

void CameraComponent::SetFOV(float fov)
{
	if (fieldOfView == fov) return;

	fieldOfView = fov;
	CalculateProjection();
}

void CameraComponent::SetNear(float nearVal)
{
	if (nearClip == nearVal) return;

	nearClip = nearVal;
	CalculateProjection();
}

void CameraComponent::SetFar(float farVal)
{
	if (farClip == farVal) return;

	farClip = farVal;
	CalculateProjection();
}

void CameraComponent::SetSize(Vector2 size)
{
	this->size = size;
	CalculateProjection();
}

XMFLOAT4X4 CameraComponent::GetProjectionMat()
{
	return projectionMat;
}

XMFLOAT4X4 CameraComponent::GetViewMat()
{
	return viewMat;
}

void CameraComponent::OnUpdate()
{
	CalculateView();
}
