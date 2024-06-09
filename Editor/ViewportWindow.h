#pragma once

#include "EditorWindow.h"

#include "IconsMaterialDesign.h"

class ViewportWindow : public EditorWindow
{
public:
	ViewportWindow(EditorLayer* el, EditorWindowIndex windowIndex);

	static inline std::string GetTitle() { return ICON_MD_TV" Viewport"; };

	virtual void BeforeShow() override;
	virtual void OnGui() override;
	virtual void AfterShow() override;
	virtual void OnResizeWindow(ImVec2 newSize);
};