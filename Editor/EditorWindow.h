#pragma once

#include "imgui.h"

class EditorWindow
{
protected:
	int sameWindowSizeFrames = 0;
	bool resizingWindow = false;
	const int sameWindowSizeFrameMax = 4;

public:
	EditorWindow(const std::string& title, const ImVec2& minSize);

	std::string title;
	bool open = true;
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_None;
	ImVec2 minSize = ImVec2(50, 50);
	ImVec2 lastWindowSize;

	void Show();
	virtual void BeforeShow();
	virtual void OnGui();
	virtual void AfterShow();
	virtual void OnResizeWindow(ImVec2 newSize);
};