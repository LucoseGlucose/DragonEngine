#pragma once

#include <string>
#include <functional>

#include "EditorWindow.h"

struct EditorWindowData
{
	std::string title;
	std::function<EditorWindow* (EditorLayer*)> creationFunc;
	bool open;
	EditorWindowIndex index;

	EditorWindowData(std::string title, std::function<EditorWindow* (EditorLayer*)> creationFunc, bool open,
		EditorWindowIndex index) : title(title), creationFunc(creationFunc), open(open), index(index)
	{

	}
};