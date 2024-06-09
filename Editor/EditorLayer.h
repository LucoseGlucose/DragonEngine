#pragma once

#include <array>

#include "Layer.h"
#include "CommandRecorder.h"
#include "EditorWindow.h"
#include "imgui.h"
#include "ViewportWindow.h"
#include "SceneWindow.h"
#include "StatsWindow.h"
#include "Utils.h"
#include "EditorWindowData.h"

enum class EditorWindowIndex
{
	ViewportWindow,
	SceneWindow,
	StatsWindow,
};

#define WINDOW_ENTRY(name, defOpen) EditorWindowData(name##::GetTitle(), [](EditorLayer* el) -> name##* { return new name##(el, EditorWindowIndex::##name); }, defOpen, EditorWindowIndex::##name)

class EditorLayer : public Layer
{
	static inline std::array<EditorWindowData, 3> availableWindows
	{
		WINDOW_ENTRY(ViewportWindow, true),
		WINDOW_ENTRY(SceneWindow, true),
		WINDOW_ENTRY(StatsWindow, true),
	};

public:

	STATIC(ComPtr<ID3D12DescriptorHeap> descHeap);
	STATIC(std::vector<EditorWindow*> windows);

	ImTextureID GetViewportTextureID();

	void OpenEditorWindow(EditorWindowIndex windowIndex);
	void CloseEditorWindow(EditorWindow* window);

	virtual void OnPush() override;
	virtual void Update() override;
	virtual void Resize(Vector2 newSize) override;
	virtual void OnPop() override;

	EditorWindow* GetWindowByTitle(std::string title);
	bool TryGetWindowByTitle(std::string title, EditorWindow** out);

	template<typename T>
	T* GetWindowOfType() requires std::is_base_of_v<EditorWindow, T>
	{
		for (size_t i = 0; i < windows.size(); i++)
		{
			T* casted = dynamic_cast<T*>(windows[i]);
			if (casted != nullptr) return casted;
		}

		return nullptr;
	}
};