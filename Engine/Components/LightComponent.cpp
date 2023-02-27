#include "stdafx.h"
#include "LightComponent.h"

XMFLOAT3 LightComponent::GetOutput()
{
	XMFLOAT3 output;
	DirectX::XMStoreFloat3(&output, DirectX::XMLoadFloat3(&color) * strength);

	return output;
}

int LightComponent::GetLightType()
{
	return -1;
}
