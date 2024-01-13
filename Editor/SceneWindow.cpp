#include "stdafx.h"
#include "SceneWindow.h"

#include "IconsMaterialDesign.h"
#include "SceneManager.h"

SceneWindow::SceneWindow() : EditorWindow(ICON_MD_PUBLIC" Scene", ImVec2(256, 72))
{

}

void SceneWindow::OnGui()
{
	std::vector<SceneObject*>* objects = SceneManager::GetActiveScene()->FindObjects<SceneObject>();
	for (SceneObject*& obj : *objects)
	{
		ShowObjectInTree(obj);
	}
	delete objects;
}

void SceneWindow::ShowObjectInTree(SceneObject* obj)
{
	ImGui::Text(obj->name.c_str());
}
