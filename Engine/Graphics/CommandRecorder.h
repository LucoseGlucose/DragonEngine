#pragma once



class CommandRecorder
{
	bool recording;

public:
	CommandRecorder();

	ComPtr<ID3D12CommandAllocator> allocator;
	ComPtr<ID3D12GraphicsCommandList4> list;

	void StartRecording();
	void StopRecording();
	void Execute();

	bool IsRecording();
};