#include "stdafx.h"
#include "StatsWindow.h"

#include "IconsMaterialDesign.h"

StatsWindow::StatsWindow() : EditorWindow(ICON_MD_FORMAT_LIST_NUMBERED" Stats", ImVec2(256, 72))
{
	startupTime = glfwGetTime();
}

void StatsWindow::OnGui()
{
	framesSinceUpdate++;

	if (framesSinceUpdate >= maxFramesSinceUpdate)
	{
		framesSinceUpdate = 0;
		lastDeltaTime = Application::GetDeltaTime() * 1000;
	}

	ImGui::Text("Startup time: %s s", std::to_string(startupTime).c_str());
	ImGui::Text("Last frame time: %s ms", std::to_string(lastDeltaTime).c_str());
}
