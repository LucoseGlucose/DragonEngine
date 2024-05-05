#include "stdafx.h"
#include "EditorWindow.h"

#include "EditorLayer.h"

EditorWindow::EditorWindow(const std::string& title, const ImVec2& minSize) : title(title), minSize(minSize)
{

}

void EditorWindow::Show()
{
	if (!open) return;

	BeforeShow();
	ImGui::GetStyle().WindowMinSize = minSize;
	ImGui::Begin(title.c_str(), &open, windowFlags);

	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	if (windowSize.x != lastWindowSize.x || windowSize.y != lastWindowSize.y)
	{
		resizingWindow = true;
		sameWindowSizeFrames = 0;
	}
	else if (resizingWindow)
	{
		sameWindowSizeFrames++;

		if (sameWindowSizeFrames >= sameWindowSizeFrameMax)
		{
			resizingWindow = false;
			OnResizeWindow(windowSize);
		}
	}
	lastWindowSize = windowSize;

	OnGui();
	ImGui::End();

	AfterShow();
}

void EditorWindow::BeforeShow()
{

}

void EditorWindow::OnGui()
{

}

void EditorWindow::AfterShow()
{

}

void EditorWindow::OnResizeWindow(ImVec2 newSize)
{

}
