#pragma once

#include "CommandRecorder.h"

class Editor
{
public:
	static inline ComPtr<ID3D12DescriptorHeap> imGuiDescHeap{};

	static void Init();
	static void Update();
	static void Render(CommandRecorder* recorder);
	static void Close();
};