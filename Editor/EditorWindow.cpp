#include "stdafx.h"
#include "EditorWindow.h"

#include "EditorLayer.h"

EditorWindow::EditorWindow(EditorLayer* editorLayer, EditorWindowIndex windowIndex, const std::string& title, const ImVec2& minSize) :
	editorLayer(editorLayer), title(title), minSize(minSize), windowIndex(windowIndex)
{

}

void EditorWindow::Show()
{
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

	if (!open) editorLayer->CloseEditorWindow(this);
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
