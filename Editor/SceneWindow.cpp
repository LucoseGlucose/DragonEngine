#include "stdafx.h"
#include "SceneWindow.h"

#include "SceneManager.h"

SceneWindow::SceneWindow(EditorLayer* el, EditorWindowIndex windowIndex) : EditorWindow(el, windowIndex, GetTitle(), ImVec2(256, 72))
{

}

void SceneWindow::OnGui()
{
	std::vector<SceneObject*> objects = SceneManager::GetActiveScene()->FindObjects<SceneObject>();
	for (SceneObject*& obj : objects)
	{
		ShowObjectInTree(obj);
	}
}

void SceneWindow::ShowObjectInTree(SceneObject* obj)
{
	ImGui::Text(obj->name.c_str());
}
