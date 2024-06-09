#include "stdafx.h"
#include "GraphicsResource.h"

CD3DX12_RESOURCE_BARRIER GraphicsResource::TransitionToState(D3D12_RESOURCE_STATES newState)
{
	CD3DX12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(resourceBuffer.Get(), currentState, newState, 
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_BARRIER_FLAG_NONE);

	currentState = newState;

	return transition;
}