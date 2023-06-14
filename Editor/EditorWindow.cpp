#include "stdafx.h"
#include "EditorWindow.h"

#include "EditorLayer.h"

void EditorWindow::Show()
{
	if (open)
	{
		ImGui::GetStyle().WindowMinSize = minSize;
		ImGui::Begin(title.c_str(), &open, windowFlags);
		OnGui();
		ImGui::End();
	}
}

void EditorWindow::OnGui()
{

}
