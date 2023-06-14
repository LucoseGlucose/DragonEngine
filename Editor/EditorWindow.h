#pragma once

#include "imgui.h"

class EditorWindow
{
public:
	std::string title;
	bool open = true;
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_None;
	ImVec2 minSize = ImVec2(50, 50);

	void Show();
	virtual void OnGui();
};