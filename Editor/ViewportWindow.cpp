#include "stdafx.h"
#include "ViewportWindow.h"

#include "IconsMaterialDesign.h"
#include "EditorLayer.h"

ViewportWindow::ViewportWindow() : EditorWindow(ICON_MD_TV" Viewport", ImVec2(256, 144))
{

}

void ViewportWindow::BeforeShow()
{
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
}

void ViewportWindow::OnGui()
{
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Image(EditorLayer::GetViewportTextureID(), ImGui::GetContentRegionAvail());
	ImGui::PopStyleVar();
}

void ViewportWindow::AfterShow()
{
	ImGui::PopStyleVar();
}
