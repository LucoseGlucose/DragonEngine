#pragma once

#include "EditorWindow.h"

class StatsWindow : public EditorWindow
{
	int framesSinceUpdate = 0;
	const int maxFramesSinceUpdate = 15;
	double lastDeltaTime = 0;
	double startupTime = 0;

public:
	StatsWindow();

	virtual void OnGui() override;
};