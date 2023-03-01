#pragma once

#include "Component.h"

struct LightData
{
	XMFLOAT3 color;
	float type;
	XMFLOAT3 position;
	float pad0;
	XMFLOAT3 direction;
	float pad1;
	XMFLOAT2 falloff;
	XMFLOAT2 radius;
};

class LightComponent : public Component
{
public:
	LightComponent(SceneObject* owner);

	XMFLOAT3 color = XMFLOAT3(1.f, 1.f, 1.f);
	float strength = 1.f;

	virtual float GetLightType();
	virtual XMFLOAT3 GetOutput();
	virtual void GetLightData(LightData* data);
};