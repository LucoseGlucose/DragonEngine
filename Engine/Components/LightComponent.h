#pragma once

#include "Component.h"

struct LightData
{
	XMFLOAT3 color;
	float pad0;
	XMFLOAT3 position;
	float pad1;
	XMFLOAT3 direction;
	float pad2;
	XMFLOAT2 falloff;
	XMFLOAT2 radius;
};

class LightComponent : public Component
{
public:
	LightComponent(SceneObject* owner);

	XMFLOAT3 color = XMFLOAT3(1.f, 1.f, 1.f);
	float strength = 1.f;
	XMFLOAT2 falloff = XMFLOAT2(.09f, .032f);
	XMFLOAT2 radius = XMFLOAT2(1.f, 1.f);

	XMFLOAT3 GetOutput();
};