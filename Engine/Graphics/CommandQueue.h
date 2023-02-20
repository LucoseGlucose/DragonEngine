#pragma once

class CommandQueue
{
public:
	CommandQueue();
	~CommandQueue();
	
	const UINT64 GetCurrentValue();
	void WaitToValue(const UINT64 value);
	void WaitForAllCommands();
	void Execute(ID3D12GraphicsCommandList* cmdList);

	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12Fence> fence;
};