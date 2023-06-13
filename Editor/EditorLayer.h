#pragma once

#include "Layer.h"
#include "CommandRecorder.h"

class EditorLayer : public Layer
{
public:
	static inline ComPtr<ID3D12DescriptorHeap> descHeap{};

	virtual void OnPush() override;
	virtual void Update() override;
	virtual void Resize(XMUINT2 newSize) override;
	virtual void OnPop() override;
};