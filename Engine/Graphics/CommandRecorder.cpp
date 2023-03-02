#include "stdafx.h"
#include "CommandRecorder.h"

#include "Rendering.h"

CommandRecorder::CommandRecorder()
{
	Utils::ThrowIfFailed(Rendering::device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
	NAME_D3D_OBJECT(allocator);

	Utils::ThrowIfFailed(Rendering::device->CreateCommandList1(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&list)));
	NAME_D3D_OBJECT(list);
}

void CommandRecorder::StartRecording()
{
	if (recording) return;
	recording = true;

	Utils::ThrowIfFailed(allocator->Reset());
	Utils::ThrowIfFailed(list->Reset(allocator.Get(), nullptr));
}

void CommandRecorder::StopRecording()
{
	if (!recording) return;
	recording = false;

	list->Close();
}

void CommandRecorder::Execute()
{
	StopRecording();
	Rendering::commandQueue->Execute(list.Get());
}

bool CommandRecorder::IsRecording()
{
	return recording;
}
