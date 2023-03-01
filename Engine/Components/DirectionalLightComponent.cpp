#include "stdafx.h"
#include "DirectionalLightComponent.h"

DirectionalLightComponent::DirectionalLightComponent(SceneObject* owner) : LightComponent(owner)
{

}

float DirectionalLightComponent::GetLightType()
{
	return 2;
}

void DirectionalLightComponent::GetLightData(LightData* data)
{
	LightComponent::GetLightData(data);
	data->direction = GetTransform()->GetForward();
}
