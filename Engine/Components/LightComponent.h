#pragma once

#include "Component.h"

struct LightData
{
	int type = -1;
	XMFLOAT3 color = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT2 falloff;
	XMFLOAT3 position;
	XMFLOAT3 direction;
	XMFLOAT2 radius;
};

class LightComponent : public Component
{
public:
	XMFLOAT3 color = XMFLOAT3(1.f, 1.f, 1.f);
	float strength = 1.f;
	XMFLOAT2 falloff = XMFLOAT2(.09f, .032f);
	XMFLOAT2 radius;

	XMFLOAT3 GetOutput();
	virtual int GetLightType();
};