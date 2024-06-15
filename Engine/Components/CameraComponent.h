#pragma once

#include "Component.h"
#include "Utils.h"

class CameraComponent : public Component
{
	Matrix projectionMat;
	Matrix viewMat;

	float fieldOfView = 60.f;
	float nearClip = .1f;
	float farClip = 75.f;
	Vector2 size = Application::GetFramebufferSize();

	Subscription<Matrix> viewMatCalcSub;

public:

	CameraComponent(SceneObject* owner);
	~CameraComponent();

	void CalculateProjection();
	void CalculateView();

	GETTER(GetFOV, fieldOfView);
	void SetFOV(float fov);

	GETTER(GetNear, nearClip);
	void SetNear(float nearVal);

	GETTER(GetFar, farClip);
	void SetFar(float farVal);

	GETTER(GetSize, size);
	void SetSize(Vector2 size);

	Matrix GetProjectionMat();
	Matrix GetViewMat();
};