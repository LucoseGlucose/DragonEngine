#pragma once



class CommandRecorder
{
	bool recording;

public:
	CommandRecorder();
	~CommandRecorder();

	ComPtr<ID3D12CommandAllocator> allocator;
	ComPtr<ID3D12GraphicsCommandList> list;

	void StartRecording();
	void StopRecording();
	void Execute();
};