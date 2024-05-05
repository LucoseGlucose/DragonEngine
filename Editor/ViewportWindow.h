#pragma once

#include "EditorWindow.h"

class ViewportWindow : public EditorWindow
{
public:
	ViewportWindow();

	virtual void BeforeShow() override;
	virtual void OnGui() override;
	virtual void AfterShow() override;
	virtual void OnResizeWindow(ImVec2 newSize);
};