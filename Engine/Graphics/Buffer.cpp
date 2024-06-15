#include "stdafx.h"
#include "Buffer.h"

#include "Rendering.h"

Buffer::Buffer() : size(0), type(D3D12_HEAP_TYPE_CUSTOM)
{

}

Buffer::Buffer(UINT64 size, D3D12_RESOURCE_FLAGS resourceFlags, D3D12_HEAP_TYPE type, D3D12_RESOURCE_STATES startingState,
	D3D12_HEAP_FLAGS heapFlags) : size(size), type(type)
{
	currentState = startingState;

	resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size, resourceFlags);
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(type);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&heapProps, heapFlags,
		&resourceDesc, startingState, nullptr, IID_PPV_ARGS(&resourceBuffer)));
}

Buffer::Buffer(UINT64 size, D3D12_HEAP_TYPE type, D3D12_RESOURCE_STATES startingState) :
	Buffer(size, D3D12_RESOURCE_FLAG_NONE, type, startingState, D3D12_HEAP_FLAG_NONE)
{

}

void Buffer::UploadData(CommandRecorder* recorder, Buffer* uploadBuffer, void* data)
{
	D3D12_SUBRESOURCE_DATA subData{ data, size, size };

	UpdateSubresources(recorder->list.Get(), resourceBuffer.Get(), uploadBuffer->resourceBuffer.Get(), 0, 0, 1, &subData);
	currentState = D3D12_RESOURCE_STATE_COPY_DEST;
}
