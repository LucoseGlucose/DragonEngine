#include "stdafx.h"
#include "EditorWindow.h"

#include "EditorLayer.h"

EditorWindow::EditorWindow(const std::string& title, const ImVec2& minSize) : title(title), minSize(minSize)
{

}

void EditorWindow::Show()
{
	if (open)
	{
		ImGui::GetStyle().WindowMinSize = minSize;
		ImGui::Begin(title.c_str(), &open, windowFlags);

		ImVec2 windowSize = ImGui::GetContentRegionAvail();
		if (windowSize.x != lastWindowSize.x || windowSize.y != lastWindowSize.y)
		{

		}
		lastWindowSize = windowSize;

		OnGui();
		ImGui::End();
	}
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
