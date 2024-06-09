#include "stdafx.h"
#include "DescriptorHeap.h"

#include "Rendering.h"

DescriptorHeap::DescriptorHeap(UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
	size(size), type(type), flags(flags), handleSize(Rendering::device->GetDescriptorHandleIncrementSize(type))
{
	if (size < 1) return;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NumDescriptors = size;
	heapDesc.Type = type;
	heapDesc.Flags = flags;

	Utils::ThrowIfFailed(Rendering::device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));
	NAME_D3D_OBJECT(heap);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandle()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart());
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandleForIndex(UINT index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), index, handleSize);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandle()
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart());
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandleForIndex(UINT index)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), index, handleSize);
}