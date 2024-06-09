#pragma once

#include "stdafx.h"

class DescriptorHeap 
{
public:

	DescriptorHeap() = default;
	DescriptorHeap(UINT size, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

	ComPtr<ID3D12DescriptorHeap> heap;

	UINT size;
	D3D12_DESCRIPTOR_HEAP_TYPE type;
	D3D12_DESCRIPTOR_HEAP_FLAGS flags;
	UINT handleSize;

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandleForIndex(UINT index);

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandleForIndex(UINT index);
};
