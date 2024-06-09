#pragma once

#include "EditorWindow.h"

#include "IconsMaterialDesign.h"

class StatsWindow : public EditorWindow
{
	int framesSinceUpdate = 0;
	const double maxUpdateTime = .1;
	double totalDeltaTime = 0;
	double lastDeltaTime = 0;
	double startupTime = 0;

public:
	StatsWindow(EditorLayer* el, EditorWindowIndex windowIndex);

	static inline std::string GetTitle() { return ICON_MD_FORMAT_LIST_NUMBERED" Stats"; };

	virtual void OnGui() override;
};