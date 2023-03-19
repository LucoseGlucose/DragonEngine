#include "stdafx.h"
#include "EditorWindow.h"

EditorWindow::EditorWindow(const std::string& title, const std::string& shortcut, bool open)
	: title(title), shortcut(shortcut), open(open)
{

}

void EditorWindow::Update()
{
	if (!open) return;

	ImGui::GetStyle().WindowMinSize = minSize;

	if (ImGui::Begin(title.c_str(), &open))
	{
		ShowWindow();
		ImGui::End();
	}
}

void EditorWindow::ShowWindow()
{

}
