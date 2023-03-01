#include "stdafx.h"
#include "LightComponent.h"

LightComponent::LightComponent(SceneObject* owner) : Component(owner)
{

}

XMFLOAT3 LightComponent::GetOutput()
{
	XMFLOAT3 output;
	DirectX::XMStoreFloat3(&output, DirectX::XMLoadFloat3(&color) * strength);

	return output;
}

float LightComponent::GetLightType()
{
	return 0;
}

void LightComponent::GetLightData(LightData* data)
{
	data->type = GetLightType();
	data->color = GetOutput();
	data->position = GetTransform()->GetPosition();
}
