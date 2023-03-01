#pragma once

#include "LightComponent.h"

class DirectionalLightComponent : public LightComponent
{
public:
	DirectionalLightComponent(SceneObject* owner);

	virtual float GetLightType() override;
	virtual void GetLightData(LightData* data) override;
};