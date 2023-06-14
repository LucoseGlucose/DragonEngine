#pragma once

#include "Layer.h"
#include "CommandRecorder.h"
#include "EditorWindow.h"
#include "imgui.h"

class EditorLayer : public Layer
{
public:
	static inline ComPtr<ID3D12DescriptorHeap> descHeap{};
	static inline std::vector<EditorWindow*> windows{};

	static ImTextureID GetViewportTextureID();

	virtual void OnPush() override;
	virtual void Update() override;
	virtual void Resize(XMUINT2 newSize) override;
	virtual void OnPop() override;
};