#pragma once

#include "EditorWindow.h"

class SceneWindow : public EditorWindow
{
public:
	SceneWindow();

	virtual void OnGui() override;
	void ShowObjectInTree(SceneObject* obj);
};