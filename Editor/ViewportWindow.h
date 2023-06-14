#pragma once

#include "EditorWindow.h"

class ViewportWindow : public EditorWindow
{
public:
	ViewportWindow();

	virtual void OnGui() override;
};