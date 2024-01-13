#pragma once

#include "Layer.h"
#include "CommandRecorder.h"
#include "EditorWindow.h"
#include "imgui.h"
#include "ViewportWindow.h"

class EditorLayer : public Layer
{
	ViewportWindow* viewport;

public:
	static inline ComPtr<ID3D12DescriptorHeap> descHeap{};
	static inline std::vector<EditorWindow*> windows{};

	static ImTextureID GetViewportTextureID();

	virtual void OnPush() override;
	virtual void Update() override;
	virtual void Resize(XMUINT2 newSize) override;
	virtual void OnPop() override;
	virtual XMUINT2 GetViewportSize() override;

	template<typename T>
	T* GetWindow() requires std::is_base_of_v<EditorWindow, T>
	{
		for (size_t i = 0; i < windows.size(); i++)
		{
			T* casted = dynamic_cast<T*>(windows[i]);
			if (casted != nullptr) return casted;
		}

		return nullptr;
	}
};