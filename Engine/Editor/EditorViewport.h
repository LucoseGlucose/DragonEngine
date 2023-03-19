#pragma once

#include "EditorWindow.h"

class EditorViewport : public EditorWindow
{
	ImVec2 viewportSize;

public:
	EditorViewport(const std::string& title, const std::string& shortcut, bool open);

	virtual void ShowWindow() override;
};