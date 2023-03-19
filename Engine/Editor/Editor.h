#pragma once

#include "CommandRecorder.h"
#include "EditorWindow.h"

class Editor
{
	static inline std::vector<EditorWindow*> windows{};

public:
	static inline ComPtr<ID3D12DescriptorHeap> imGuiDescHeap{};

	static void Init();
	static void Update();
	static void Render(CommandRecorder* recorder);
	static void Close();

	template<typename T>
	static T* GetWindow() requires std::is_base_of_v<EditorWindow, T>
	{
		for (size_t i = 0; i < windows.size(); i++)
		{
			T* ptr = dynamic_cast<T*>(windows[i]);
			if (ptr != nullptr) return ptr;
		}

		return nullptr;
	}
};