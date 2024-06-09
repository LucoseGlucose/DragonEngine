#pragma once

#include "CommandRecorder.h"

class GraphicsResource
{
public:

	D3D12_RESOURCE_STATES currentState;
	ComPtr<ID3D12Resource> resourceBuffer;
	D3D12_RESOURCE_DESC resourceDesc;
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;

	virtual CD3DX12_RESOURCE_BARRIER TransitionToState(D3D12_RESOURCE_STATES newState);
};