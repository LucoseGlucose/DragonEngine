#pragma once

#include "Component.h"

struct LightData
{
	Vector3 color;
	float type;
	Vector3 position;
	float pad0;
	Vector3 direction;
	float pad1;
	Vector2 falloff;
	Vector2 radius;
};

class LightComponent : public Component
{
public:
	LightComponent(SceneObject* owner);

	Vector3 color = Vector3(1.f, 1.f, 1.f);
	float strength = 1.f;

	virtual float GetLightType();
	virtual Vector3 GetOutput();
	virtual void GetLightData(LightData* data);
};