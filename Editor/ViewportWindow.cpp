#include "stdafx.h"
#include "ViewportWindow.h"

#include "IconsMaterialDesign.h"
#include "EditorLayer.h"

ViewportWindow::ViewportWindow()
{
	title = ICON_MD_TV" Viewport";
	minSize = ImVec2(128, 72);
}

void ViewportWindow::OnGui()
{
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Image(EditorLayer::GetViewportTextureID(), ImGui::GetContentRegionAvail());
	ImGui::PopStyleVar();
}
