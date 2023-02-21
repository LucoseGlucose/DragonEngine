#pragma once

#include "Component.h"

class CameraComponent : public Component
{
	XMFLOAT4X4 projectionMat;
	XMFLOAT4X4 viewMat;

	float fieldOfView = 60.f;
	float nearClip = .1f;
	float farClip = 75.f;

public:
	CameraComponent(SceneObject* owner);

	void CalculateProjection();
	void CalculateView();

	float GetFOV();
	void SetFOV(float fov);

	float GetNear();
	void SetNear(float nearVal);

	float GetFar();
	void SetFar(float farVal);

	XMFLOAT4X4 GetProjectionMat();
	XMFLOAT4X4 GetViewMat();

	virtual void OnUpdate() override;
};