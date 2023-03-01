#include "stdafx.h"
#include "PointLightComponent.h"

PointLightComponent::PointLightComponent(SceneObject* owner) : LightComponent(owner)
{

}

float PointLightComponent::GetLightType()
{
	return 1;
}

void PointLightComponent::GetLightData(LightData* data)
{
	LightComponent::GetLightData(data);
	data->falloff = falloff;
}
