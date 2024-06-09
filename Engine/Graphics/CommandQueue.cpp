#include "stdafx.h"
#include "CommandQueue.h"

#include "Rendering.h"

CommandQueue::CommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC cQueueDesc{};
	cQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//cQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;

	Utils::ThrowIfFailed(Rendering::device->CreateCommandQueue(&cQueueDesc, IID_PPV_ARGS(&commandQueue)));
	NAME_D3D_OBJECT(commandQueue);

	Utils::ThrowIfFailed(Rendering::device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	NAME_D3D_OBJECT(fence);
}

CommandQueue::~CommandQueue()
{
	WaitForAllCommands();
}

const UINT64 CommandQueue::GetCurrentValue()
{
	return fence->GetCompletedValue();
}

void CommandQueue::WaitToValue(const UINT64 value)
{
	Utils::ThrowIfFailed(commandQueue->Signal(fence.Get(), value));

	if (fence->GetCompletedValue() < value)
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		Utils::ThrowIfFailed(fence->SetEventOnCompletion(value, event));

		WaitForSingleObjectEx(event, INFINITE, FALSE);
		CloseHandle(event);
	}
}

void CommandQueue::WaitForAllCommands()
{
	WaitToValue(GetCurrentValue() + 1);
}

void CommandQueue::Execute(ID3D12GraphicsCommandList* cmdList)
{
	ID3D12CommandList* commandLists[] = { cmdList };
	commandQueue->ExecuteCommandLists(1, commandLists);
}
