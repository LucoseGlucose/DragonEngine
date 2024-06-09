#pragma once

#include "EditorWindow.h"

#include "IconsMaterialDesign.h"

class SceneWindow : public EditorWindow
{
public:
	SceneWindow(EditorLayer* el, EditorWindowIndex windowIndex);

	static inline std::string GetTitle() { return ICON_MD_PUBLIC" Scene"; };

	virtual void OnGui() override;
	void ShowObjectInTree(SceneObject* obj);
};