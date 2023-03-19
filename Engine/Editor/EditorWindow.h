#pragma once

#include <imgui.h>

class EditorWindow
{
public:
	EditorWindow(const std::string& title, const std::string& shortcut, bool open);

	std::string title;
	std::string shortcut;
	bool open;

	ImVec2 minSize = ImVec2(256.f, 256.f);

	virtual void Update();
	virtual void ShowWindow();
};