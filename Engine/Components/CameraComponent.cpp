#include "stdafx.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent(SceneObject* owner) : Component(owner)
{
	CalculateProjection();
	CalculateView();
}

void CameraComponent::CalculateProjection()
{
	XMUINT2 windowSize = Application::GetViewportSize();

	DirectX::XMStoreFloat4x4(&projectionMat, DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(fieldOfView), (float)windowSize.x / windowSize.y, nearClip, farClip));
}

void CameraComponent::CalculateView()
{
	XMFLOAT3 pos = GetTransform()->GetPosition();
	XMFLOAT3 fwd = GetTransform()->GetForward();
	XMFLOAT3 up = GetTransform()->GetUp();

	DirectX::XMStoreFloat4x4(&viewMat, DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&pos),
		DirectX::XMLoadFloat3(&fwd), DirectX::XMLoadFloat3(&up)));
}

float CameraComponent::GetFOV()
{
	return fieldOfView;
}

void CameraComponent::SetFOV(float fov)
{
	if (fieldOfView == fov) return;

	fieldOfView = fov;
	CalculateProjection();
}

float CameraComponent::GetNear()
{
	return nearClip;
}

void CameraComponent::SetNear(float nearVal)
{
	if (nearClip == nearVal) return;

	nearClip = nearVal;
	CalculateProjection();
}

float CameraComponent::GetFar()
{
	return farClip;
}

void CameraComponent::SetFar(float farVal)
{
	if (farClip == farVal) return;

	farClip = farVal;
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
