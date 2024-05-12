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
	totalDeltaTime += Application::GetDeltaTime();

	if (totalDeltaTime >= maxUpdateTime)
	{
		lastDeltaTime = totalDeltaTime / static_cast<double>(framesSinceUpdate);
		framesSinceUpdate = 0;
		totalDeltaTime = 0;
	}

	ImGui::Text("Startup time: %s s", std::to_string(startupTime).c_str());
	ImGui::Text("Last frame time: %s ms", std::to_string(lastDeltaTime * 1000).c_str());
}
