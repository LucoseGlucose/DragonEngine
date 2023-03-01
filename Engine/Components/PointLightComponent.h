#pragma once

#include "LightComponent.h"

class PointLightComponent : public LightComponent
{
public:
	PointLightComponent(SceneObject* owner);

	XMFLOAT2 falloff = XMFLOAT2(.09f, .032f);

	virtual float GetLightType() override;
	virtual void GetLightData(LightData* data) override;
};