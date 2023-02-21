#pragma once

#include "Component.h"

class CameraControllerComponent : public Component
{
public:
	CameraControllerComponent(SceneObject* owner);

	float scrollSpeed = 1.5f;
	float panSpeed = 1.f;
	float rotateSpeed = 50.f;
	float flySpeed = 25.f;

	virtual void OnUpdate() override;
};