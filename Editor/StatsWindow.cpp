#include "stdafx.h"
#include "StatsWindow.h"

StatsWindow::StatsWindow(EditorLayer* el, EditorWindowIndex windowIndex) : EditorWindow(el, windowIndex, GetTitle(), ImVec2(256, 72))
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
	ImGui::Text("Frames per second: %s", std::to_string(1.0 / lastDeltaTime).c_str());
}
