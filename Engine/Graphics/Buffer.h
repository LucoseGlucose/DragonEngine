#pragma once

#include "stdafx.h"
#include "GraphicsResource.h"

class Buffer : public GraphicsResource
{
public:

	Buffer();
	Buffer(UINT64 size, D3D12_RESOURCE_FLAGS resourceFlags, D3D12_HEAP_TYPE type, D3D12_RESOURCE_STATES startingState, D3D12_HEAP_FLAGS flags);
	Buffer(UINT64 size, D3D12_HEAP_TYPE type, D3D12_RESOURCE_STATES startingState);

	UINT64 size;
	D3D12_HEAP_TYPE type;

	void UploadData(CommandRecorder* recorder, Buffer* uploadBuffer, void* data);
};