#include "stdafx.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent(SceneObject* owner) : Component(owner), viewMatCalcSub([this](Matrix m) { CalculateView(); })
{
	CalculateProjection();
	CalculateView();

	GetTransform()->onMatrixChanged.Subscribe(&viewMatCalcSub);
}

CameraComponent::~CameraComponent()
{
	GetTransform()->onMatrixChanged.Unsubscribe(&viewMatCalcSub);
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

	viewMat = Matrix::CreateLookAt(pos, pos + fwd, up);
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

Matrix CameraComponent::GetProjectionMat()
{
	return projectionMat;
}

Matrix CameraComponent::GetViewMat()
{
	return viewMat;
}
